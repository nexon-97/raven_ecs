#include "Manager.hpp"

namespace ecs
{

const uint8_t ComponentHandleInternal::k_invalidTypeId = static_cast<uint8_t>(-1);

Manager::Manager()
	: m_entitiesCollection(*this)
{}

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

void Manager::Render()
{
	assert(m_initialized);

	for (const auto& system : m_systems)
	{
		system.second->Render();
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

	return ComponentHandle(ComponentHandleInternal::k_invalidTypeId, nullptr);
}

ComponentHandle Manager::CreateComponentInternal(const uint8_t typeId)
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

	return ComponentHandleInternal::k_invalidTypeId;
}

std::type_index Manager::GetComponentTypeIndexByTypeId(const uint8_t typeId) const
{
	return m_componentTypeIndexes[typeId];
}

IComponentCollection* Manager::GetCollection(const uint8_t typeId) const
{
	assert(typeId < m_componentStorages.size());
	return m_componentStorages[typeId].get();
}

void Manager::SetComponentEntityId(const ComponentHandle& handle, const uint32_t id)
{
	auto collection = GetCollection(handle.GetTypeIndex());
	collection->SetItemEntityId(handle.GetOffset(), id);
}

uint32_t Manager::GetComponentEntityId(const ComponentHandle& handle) const
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

} // namespace ecs
