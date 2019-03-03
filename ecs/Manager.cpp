#include "Manager.hpp"

namespace ecs
{

const std::type_index ComponentHandle::k_invalidSystemId = typeid(nullptr);
const ComponentHandle ComponentHandle::k_invalidHandle = ComponentHandle(ComponentHandle::k_invalidSystemId, 0U);

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
	auto typeIndexIt = m_componentNames.find(name);
	if (typeIndexIt != m_componentNames.end())
	{
		auto collection = GetCollection(typeIndexIt->second);
		assert(nullptr != collection);

		return CreateComponentInternal(typeIndexIt->second, collection);
	}

	return ComponentHandle::k_invalidHandle;
}

ComponentHandle Manager::CreateComponentInternal(const std::type_index& typeIndex, IComponentCollection* collection)
{
	auto insertedId = collection->Create();
	return ComponentHandle(typeIndex, insertedId);
}

IComponentCollection* Manager::GetCollection(const std::type_index& typeIndex) const
{
	auto storageIt = m_componentStorages.find(typeIndex);
	if (storageIt != m_componentStorages.end())
	{
		return storageIt->second.get();
	}

	return nullptr;
}

void Manager::SetComponentEntityId(const ComponentHandle& handle, const std::size_t id)
{
	auto collection = GetCollection(handle.GetTypeIndex());
	collection->SetItemEntityId(handle.GetOffset(), id);
}

std::size_t Manager::GetComponentEntityId(const ComponentHandle& handle) const
{
	auto collection = GetCollection(handle.GetTypeIndex());
	return collection->GetItemEntityId(handle.GetOffset());
}

EntitiesCollection& Manager::GetEntitiesCollection()
{
	return m_entitiesCollection;
}

} // namespace ecs
