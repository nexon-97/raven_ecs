#include "Manager.hpp"
#include "ComponentCollectionImpl.hpp"
#include <algorithm>

namespace ecs
{

const std::string k_invalidComponentName = "[UNDEFINED]";

Manager::Manager()
	: m_entitiesCollection(*this)
{
	ComponentHandle::SetManagerInstance(this);
	Entity::SetManagerInstance(this);
	detail::ComponentCollectionManagerConnection::SetManagerInstance(this);
}

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
	system->Destroy();

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
	
}

void Manager::Destroy()
{
	for (ecs::System* system : m_orderedSystems)
	{
		system->Destroy();
	}

	m_systemsTypeIdMapping.clear();
	m_systemsStorage.clear();
	m_componentStorages.clear();
	m_componentTypeIndexes.clear();
	m_componentNameToIdMapping.clear();
	m_typeIndexToComponentTypeIdMapping.clear();
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

void Manager::Update()
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

			system->Init();
		}

		m_newSystems.clear();
	}

	if (m_systemPrioritiesChanged)
	{
		SortOrderedSystemsList();
	}

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
	std::stable_sort(m_orderedSystems.begin(), m_orderedSystems.end());
}

void Manager::DestroyComponent(const ComponentHandle& handle)
{
	assert(handle.IsValid());

	auto collection = GetCollection(handle.GetTypeIndex());
	collection->Destroy(handle.GetOffset());
}

ComponentHandle Manager::CreateComponentByName(const std::string& name)
{
	auto typeIdIt = m_componentNameToIdMapping.find(name);
	if (typeIdIt != m_componentNameToIdMapping.end())
	{
		return CreateComponentInternal(typeIdIt->second);
	}

	return ComponentHandle(ComponentHandleInternal::GetInvalidTypeId(), nullptr);
}

ComponentHandle Manager::CreateComponentByTypeId(const ComponentTypeId typeId)
{
	return CreateComponentInternal(typeId);
}

ComponentHandle Manager::CreateComponentInternal(const ComponentTypeId typeId)
{
	ComponentHandle::HandleIndex* handleIndex = GetCollection(typeId)->Create();
	return ComponentHandle(typeId, handleIndex);
}

ComponentTypeId Manager::GetComponentTypeIdByIndex(const std::type_index& typeIndex) const
{
	auto it = m_typeIndexToComponentTypeIdMapping.find(typeIndex);
	if (it != m_typeIndexToComponentTypeIdMapping.end())
	{
		return it->second;
	}

	return ComponentHandleInternal::GetInvalidTypeId();
}

ComponentTypeId Manager::GetComponentTypeIdByName(const std::string& name) const
{
	auto it = m_componentNameToIdMapping.find(name);
	if (it != m_componentNameToIdMapping.end())
	{
		return it->second;
	}

	return ComponentHandleInternal::GetInvalidTypeId();
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

void Manager::SetComponentEntityId(const ComponentHandle& handle, const EntityId id)
{
	auto collection = GetCollection(handle.GetTypeIndex());
	collection->SetItemEntityId(handle.GetOffset(), id);
}

EntityId Manager::GetComponentEntityId(const ComponentHandle& handle) const
{
	auto collection = GetCollection(handle.GetTypeIndex());
	return collection->GetItemEntityId(handle.GetOffset());
}

EntitiesCollection& Manager::GetEntitiesCollection()
{
	return m_entitiesCollection;
}

void Manager::RegisterComponentTypeInternal(const std::string& name, const std::type_index& typeIndex, std::unique_ptr<IComponentCollection>&& collection)
{
	// Register type storage
	ComponentTypeId typeId = static_cast<ComponentTypeId>(m_componentStorages.size());
	collection->SetTypeId(typeId);
	m_componentStorages.emplace_back(std::move(collection));
	m_componentNameToIdMapping.emplace(name, typeId);
	m_componentTypeIndexes.push_back(typeIndex);
	m_typeIndexToComponentTypeIdMapping.emplace(typeIndex, typeId);
}

void Manager::SetComponentEnabled(const ComponentHandle& handle, const bool enabled)
{
	auto collection = GetCollection(handle.GetTypeIndex());
	collection->SetItemEnabled(handle.GetOffset(), enabled);
}

void Manager::RefreshComponentActivation(const ComponentHandle& handle, const bool ownerEnabled, const bool ownerActivated)
{
	auto collection = GetCollection(handle.GetTypeIndex());
	collection->RefreshComponentActivation(handle.GetOffset(), ownerEnabled, ownerActivated);
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

ComponentHandle Manager::CloneComponent(const ComponentHandle& handle)
{
	auto collection = GetCollection(handle.GetTypeIndex());
	auto handleIndex = collection->CloneComponent(handle.GetOffset());
	return ComponentHandle(handle.GetTypeIndex(), handleIndex);
}

void* Manager::GetComponent(const ComponentHandle& handle) const
{
	return m_componentStorages[handle.GetTypeIndex()]->Get(handle.GetOffset());
}

void Manager::MoveComponentData(const ComponentHandle& handle, void* dataPtr)
{
	IComponentCollection* collection = GetCollection(handle.GetTypeIndex());
	collection->MoveData(handle.GetOffset(), dataPtr);
}

void Manager::NotifySystemPriorityChanged()
{
	m_systemPrioritiesChanged = true;
}

} // namespace ecs
