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
	assert(handle.IsValid());

	auto collection = GetCollection(handle.GetTypeIndex(), handle.GetOffset());
	std::size_t offset = handle.GetOffset() % k_defaultComponentPoolSize;
	collection->Destroy(offset);
}

IComponentCollection* Manager::FindCollectionToInsert(const std::type_index& typeIndex, int& storageIdx)
{
	auto storageIt = m_componentStorages.find(typeIndex);
	if (storageIt != m_componentStorages.end())
	{
		storageIdx = 0;
		for (const auto& storage : storageIt->second)
		{
			if (!storage->IsFull())
			{
				return storage.get();
			}

			++storageIdx;
		}
	}

	return nullptr;
}

IComponentCollection* Manager::GetCollection(const std::type_index& typeIndex, const std::size_t offset)
{
	auto storageIt = m_componentStorages.find(typeIndex);
	if (storageIt != m_componentStorages.end())
	{
		std::size_t idx = offset / k_defaultComponentPoolSize;
		return storageIt->second[idx].get();
	}

	return nullptr;
}

} // namespace ecs
