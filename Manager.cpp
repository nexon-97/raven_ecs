#include "Manager.hpp"

namespace ecs
{

const std::type_index ComponentHandle::k_invalidSystemId = typeid(nullptr);

void Manager::RegisterSystemInternal(const std::type_index& typeIndex, SystemPtr&& system)
{
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
	auto storageIt = m_componentStorages.find(handle.GetTypeIndex());
	if (storageIt != m_componentStorages.end())
	{
		storageIt->second[0]->Destroy(handle.GetOffset());
	}
}

} // namespace ecs
