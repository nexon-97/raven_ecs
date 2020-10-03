#include "Manager.hpp"
#include "ecs/component/ComponentCollectionImpl.hpp"
#include "ecs/detail/Hash.hpp"
#include <algorithm>

namespace
{
ecs::Manager* ManagerInstance = nullptr;
}

namespace ecs
{

const std::string k_invalidComponentName = "[UNDEFINED]";

Manager::Manager()
{}

System* Manager::GetSystemByTypeIndex(const std::type_index& typeIndex) const
{
	auto it = m_systemsTypeIdMapping.find(typeIndex);
	if (it != m_systemsTypeIdMapping.end())
	{
		return it->second;
	}

	return nullptr;
}

void Manager::AddSystemToStorage(SystemPtr&& system)
{
	m_systemsStorage.push_back(std::move(system));
	AddSystem(m_systemsStorage.back().get());
}

void Manager::AddSystem(System* system)
{
	m_systemsTypeIdMapping.emplace(typeid(*system), system);
	m_newSystems.emplace_back(system, m_isUpdatingSystems);

	if (!m_isUpdatingSystems)
	{
		AddSystemToOrderedSystemsList(system);
	}
}

void Manager::RemoveSystem(System* system)
{
	if (m_isUpdatingSystems)
	{
		m_removedSystems.push_back(system);
	}
	else
	{
		DoRemoveSystem(system);
	}
}

void Manager::DoRemoveSystem(System* system)
{
	system->DispatchDestroy();

	{
		auto it = m_systemsTypeIdMapping.find(typeid(*system));
		if (it != m_systemsTypeIdMapping.end())
		{
			m_systemsTypeIdMapping.erase(it);
		}
	}
	
	{
		auto it = std::find(m_orderedSystems.begin(), m_orderedSystems.end(),system);
		if (it != m_orderedSystems.end())
		{
			m_orderedSystems.erase(it);
		}
	}

	{
		auto predicate = [system](const SystemPtr& systemPtr)
		{
			return systemPtr.get() == system;
		};

		auto it = std::find_if(m_systemsStorage.begin(), m_systemsStorage.end(), predicate);
		if (it != m_systemsStorage.end())
		{
			m_systemsStorage.erase(it);
		}
	}
}

void Manager::Init()
{
	m_componentAttachedDelegate.BindMemberFunction(&Manager::DefaultComponentAttachedDelegate, this);
	m_componentDetachedDelegate.BindMemberFunction(&Manager::DefaultComponentDetachedDelegate, this);

	InitNewSystems();
}

void Manager::Destroy()
{
	m_isBeingDestroyed = true;

	// Destroy systems
	for (ecs::System* system : m_orderedSystems)
	{
		system->DispatchDestroy();
	}
	m_systemsTypeIdMapping.clear();
	m_systemsStorage.clear();
	m_orderedSystems.clear();

	// Destroy entities
	m_entitiesCollection.Clear();

	// Destroy components
	for (auto& storage : m_componentStorages)
	{
		storage->Clear();
		storage.reset();
	}
	m_componentStorages.clear();

	m_componentTypeIndexes.clear();
	m_componentNameToIdMapping.clear();
	m_typeIndexToComponentTypeIdMapping.clear();

	m_componentAttachedDelegate.UnbindAll();
	m_componentDetachedDelegate.UnbindAll();

	m_isBeingDestroyed = false;
}

void Manager::AddSystemToOrderedSystemsList(System* system)
{
	assert(!m_isUpdatingSystems);

	auto predicate = [](System* lhs, System* rhs)
	{
		return *lhs < *rhs;
	};
	auto insertIterator = std::upper_bound(m_orderedSystems.begin(), m_orderedSystems.end(), system, predicate);

	m_orderedSystems.insert(insertIterator, system);
}

void Manager::InitNewSystems()
{
	if (!m_newSystems.empty())
	{
		for (auto& systemData : m_newSystems)
		{
			ecs::System* system = systemData.first;
			const bool mustBeAddedToOrderedSystemsList = systemData.second;

			if (mustBeAddedToOrderedSystemsList)
			{
				AddSystemToOrderedSystemsList(system);
			}

			system->DispatchInit();
		}

		m_newSystems.clear();
	}

	if (m_systemPrioritiesChanged)
	{
		SortOrderedSystemsList();
	}
}

void Manager::Update()
{
	InitNewSystems();

	UpdateSystems();

	// Remove system that are waiting for removal
	if (!m_removedSystems.empty())
	{
		for (ecs::System* system : m_removedSystems)
		{
			DoRemoveSystem(system);
		}

		m_removedSystems.clear();
	}
}

void Manager::UpdateSystems()
{
	m_isUpdatingSystems = true;

	for (ecs::System* system : m_orderedSystems)
	{
		system->Update();
	}

	m_isUpdatingSystems = false;
}

void Manager::SortOrderedSystemsList()
{
	auto predicate = [](System* lhs, System* rhs)
	{
		return *lhs < *rhs;
	};
	std::stable_sort(m_orderedSystems.begin(), m_orderedSystems.end(), predicate);

	m_systemPrioritiesChanged = false;
}

ComponentPtr Manager::CreateComponentByName(const std::string& name)
{
	auto typeIdIt = m_componentNameToIdMapping.find(name);
	if (typeIdIt != m_componentNameToIdMapping.end())
	{
		return CreateComponentInternal(typeIdIt->second);
	}

	return ComponentPtr();
}

ComponentPtr Manager::CreateComponentByTypeId(const ComponentTypeId typeId)
{
	return CreateComponentInternal(typeId);
}

ComponentPtr Manager::CreateComponentInternal(const ComponentTypeId typeId)
{
	return GetCollection(typeId)->Create();
}

void Manager::ReleaseComponent(ComponentTypeId componentType, int32_t index)
{
	GetCollection(componentType)->Destroy(index);
}

ComponentTypeId Manager::GetComponentTypeIdByIndex(const std::type_index& typeIndex) const
{
	auto it = m_typeIndexToComponentTypeIdMapping.find(typeIndex);
	if (it != m_typeIndexToComponentTypeIdMapping.end())
	{
		return it->second;
	}

	return GetInvalidComponentTypeId();
}

ComponentTypeId Manager::GetComponentTypeIdByName(const std::string& name) const
{
	auto it = m_componentNameToIdMapping.find(name);
	if (it != m_componentNameToIdMapping.end())
	{
		return it->second;
	}

	return GetInvalidComponentTypeId();
}

std::type_index Manager::GetComponentTypeIndexByTypeId(const ComponentTypeId typeId) const
{
	return m_componentTypeIndexes[typeId];
}

IComponentCollection* Manager::GetCollection(const ComponentTypeId typeId) const
{
	assert(typeId < m_componentStorages.size());
	return m_componentStorages[typeId].get();
}

EntitiesCollection& Manager::GetEntitiesCollection()
{
	return m_entitiesCollection;
}

void Manager::RegisterComponentTypeInternal(const std::string& name, const std::type_index& typeIndex, const ComponentTypeId typeId, std::unique_ptr<IComponentCollection>&& collection)
{
	// Register type storage
	m_componentStorages.emplace_back(std::move(collection));
	m_componentNameToIdMapping.emplace(name, typeId);
	m_componentTypeIndexes.push_back(typeIndex);
	m_typeIndexToComponentTypeIdMapping.emplace(typeIndex, typeId);

	// Register caches link
	m_componentTypeCaches.emplace(typeId, std::vector<ComponentsTupleCache*>());
}

ComponentsTupleCache* Manager::GetComponentsTupleCache(const std::vector<ComponentTypeId>& typeIds)
{
	uint32_t hash = GetComponentsTupleId(typeIds);
	return GetComponentsTupleCacheById(hash);
}

ComponentsTupleCache* Manager::GetComponentsTupleCacheById(const uint32_t cacheId)
{
	auto it = m_tupleCaches.find(cacheId);
	if (it != m_tupleCaches.end())
	{
		return it->second.get();
	}

	return nullptr;
}

GenericComponentsCacheView Manager::GetComponentsTupleById(const uint32_t tupleId)
{
	ComponentsTupleCache* tupleCache = GetComponentsTupleCacheById(tupleId);
	return GenericComponentsCacheView(tupleCache);
}

uint32_t Manager::GetComponentsTupleId(const std::vector<ComponentTypeId>& typeIds) const
{
	if (typeIds.empty())
		return 0U;

	std::size_t outHash = std::hash<ecs::ComponentTypeId>()(typeIds[0]);
	for (std::size_t i = 1; i < typeIds.size(); ++i)
	{
		detail::hash_combine(outHash, std::hash<ecs::ComponentTypeId>()(typeIds[i]));
	}

	return static_cast<uint32_t>(outHash);
}

const std::string& Manager::GetComponentNameByTypeId(const ComponentTypeId typeId) const
{
	auto predicate = [typeId](const std::pair<std::string, ComponentTypeId>& data)
	{
		return typeId == data.second;
	};
	auto it = std::find_if(m_componentNameToIdMapping.begin(), m_componentNameToIdMapping.end(), predicate);

	if (it != m_componentNameToIdMapping.end())
	{
		return it->first;
	}

	return k_invalidComponentName;
}

const std::string& Manager::GetComponentNameByTypeId(const std::type_index& typeIndex) const
{
	auto it = m_typeIndexToComponentTypeIdMapping.find(typeIndex);
	if (it != m_typeIndexToComponentTypeIdMapping.end())
	{
		return GetComponentNameByTypeId(it->second);
	}
	
	return k_invalidComponentName;
}

ComponentPtr Manager::CloneComponent(const ComponentPtr& handle)
{
	auto collection = GetCollection(handle.GetTypeId());
	return collection->CloneComponent(handle.m_block->dataIndex);
}

void Manager::MoveComponentData(const ComponentPtr& handle, void* dataPtr)
{
	IComponentCollection* collection = GetCollection(handle.GetTypeId());
	collection->MoveData(handle.m_block->dataIndex, dataPtr);
}

void Manager::NotifySystemPriorityChanged()
{
	m_systemPrioritiesChanged = true;
}

ComponentCreateDelegate& Manager::GetComponentCreateDelegate()
{
	return m_componentCreateDelegate;
}

ComponentDestroyDelegate& Manager::GetComponentDestroyDelegate()
{
	return m_componentDestroyDelegate;
}

ComponentAttachedDelegate& Manager::GetComponentAttachedDelegate()
{
	return m_componentAttachedDelegate;
}

ComponentDetachedDelegate& Manager::GetComponentDetachedDelegate()
{
	return m_componentDetachedDelegate;
}

ComponentCreateDelegate& Manager::GetSpecializedComponentCreateDelegate(const ComponentTypeId typeId)
{
	return m_componentSpecificCreateDelegates[typeId];
}

ComponentDestroyDelegate& Manager::GetSpecializedComponentDestroyDelegate(const ComponentTypeId typeId)
{
	return m_componentSpecificDestroyDelegates[typeId];
}

ComponentAttachedDelegate& Manager::GetSpecializedComponentAttachedDelegate(const ComponentTypeId typeId)
{
	return m_componentSpecificAttachDelegates[typeId];
}

ComponentDetachedDelegate& Manager::GetSpecializedComponentDetachedDelegate(const ComponentTypeId typeId)
{
	return m_componentSpecificDetachDelegates[typeId];
}

EntityCreateDelegate& Manager::GetEntityCreateDelegate()
{
	return m_entityCreateDelegate;
}

EntityDestroyDelegate& Manager::GetEntityDestroyDelegate()
{
	return m_entityDestroyDelegate;
}

Entity Manager::GetEntityById(const EntityId id)
{
	return m_entitiesCollection.GetEntityById(id);
}

Entity Manager::CreateEntity()
{
	return m_entitiesCollection.CreateEntity();
}

void* Manager::GetComponentRaw(ComponentTypeId componentType, int32_t index)
{
	auto collection = GetCollection(componentType);
	if (nullptr != collection)
	{
		return collection->GetData(index);
	}

	return nullptr;
}

Manager* Manager::Get()
{
	return ManagerInstance;
}

void Manager::InitECSManager()
{
	ManagerInstance = new Manager();
}

void Manager::ShutdownECSManager()
{
	if (nullptr != ManagerInstance)
	{
		ManagerInstance->Destroy();
		delete ManagerInstance;
		ManagerInstance = nullptr;
	}
}

ComponentTypeId Manager::GetInvalidComponentTypeId()
{
	return std::numeric_limits<ComponentTypeId>::max();
}

uint32_t Manager::RegisterComponentsTupleIterator(std::vector<ComponentTypeId>& typeIds)
{
	if (typeIds.size() > 0)
	{
		uint32_t typeIdsHash = GetComponentsTupleId(typeIds);
		std::unique_ptr<ComponentsTupleCache> cache = std::make_unique<ComponentsTupleCache>(typeIds.data(), typeIds.size());
		ComponentsTupleCache* cachePtr = cache.get();
		m_tupleCaches.emplace(std::piecewise_construct, std::forward_as_tuple(typeIdsHash), std::forward_as_tuple(std::move(cache)));

		// Register cache in type id -> cache mapping
		for (ComponentTypeId typeId : typeIds)
		{
			auto it = m_componentTypeCaches.find(typeId);
			if (it != m_componentTypeCaches.end())
			{
				it->second.push_back(cachePtr);
			}
		}

		return typeIdsHash;
	}

	return 0U;
}

void Manager::DefaultComponentAttachedDelegate(ecs::Entity entity, ecs::ComponentPtr component)
{
	// Invoke specialized delegate
	GetSpecializedComponentAttachedDelegate(component.GetTypeId()).Broadcast(entity, component);

	// Touch tuple caches
	ComponentTypeId typeId = component.GetTypeId();
	auto it = m_componentTypeCaches.find(typeId);
	if (it != m_componentTypeCaches.end())
	{
		const auto& cachesList = it->second;
		for (ComponentsTupleCache* cache : cachesList)
		{
			cache->TouchEntity(entity.GetId());
		}
	}
}

void Manager::DefaultComponentDetachedDelegate(ecs::ComponentPtr component)
{
	// Invoke specialized delegate
	GetSpecializedComponentDetachedDelegate(component.GetTypeId()).Broadcast(component);
}

void Manager::HandleComponentDetach(const ecs::EntityId entityId, const ecs::ComponentPtr& component)
{
	// Touch tuple caches
	ComponentTypeId typeId = component.GetTypeId();
	auto it = m_componentTypeCaches.find(typeId);
	if (it != m_componentTypeCaches.end())
	{
		const auto& cachesList = it->second;
		for (ComponentsTupleCache* cache : cachesList)
		{
			cache->TouchEntity(entityId);
		}
	}

	m_componentDetachedDelegate.Broadcast(component);
}

} // namespace ecs
