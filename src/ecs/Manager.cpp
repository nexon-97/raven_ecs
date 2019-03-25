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
	detail::ComponentCollectionManagerConnection::SetManagerInstance(this);
}

void Manager::RegisterSystemInternal(const std::type_index& typeIndex, SystemPtr&& system)
{
	assert(!m_initialized);
	m_systems.emplace(typeIndex, std::move(system));
}

void Manager::Init()
{
	assert(!m_initialized);

	for (const auto& system : m_systems)
	{
		system.second->Init();
	}

	m_initialized = true;
}

void Manager::Destroy()
{
	assert(m_initialized);

	for (const auto& system : m_systems)
	{
		system.second->Destroy();
	}

	m_systems.clear();
	m_componentStorages.clear();
	m_componentTypeIndexes.clear();
	m_componentNameToIdMapping.clear();
	m_typeIndexToComponentTypeIdMapping.clear();

	m_initialized = false;
}

void Manager::Update()
{
	assert(m_initialized);

	for (const auto& system : m_systems)
	{
		system.second->Update();
	}
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

ComponentHandle Manager::CreateComponentInternal(const ComponentTypeId typeId)
{
	HandleIndex* handleIndex = GetCollection(typeId)->Create();
	return ComponentHandle(typeId, handleIndex);
}

uint8_t Manager::GetComponentTypeIdByIndex(const std::type_index& typeIndex) const
{
	auto it = m_typeIndexToComponentTypeIdMapping.find(typeIndex);
	if (it != m_typeIndexToComponentTypeIdMapping.end())
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
	assert(!m_initialized);

	// Register type storage
	uint8_t typeId = static_cast<uint8_t>(m_componentStorages.size());
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

} // namespace ecs
