#pragma once
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <typeindex>

#include "ComponentCollectionImpl.hpp"
#include "ILifecycleCallback.hpp"
#include "System.hpp"

namespace ecs
{

constexpr std::size_t k_defaultComponentPoolSize = 1024U;

class Manager
	: public ILifecycleCallback
{
	template <typename T>
	using ComponentPool = std::array<T, k_defaultComponentPoolSize>;
	using SystemPtr = std::unique_ptr<System>;
	using ComponentStoragesType = std::vector<std::unique_ptr<IComponentCollection>>;

public:
	void Init() final;
	void Destroy() final;
	void Update() final;
	void Render() final;

	/**
	* @brief Register system type, provided as template parameter SystemType, in ECS.
	* Type should be a derivative of ecs::System class.
	*/
	template <class SystemType>
	void RegisterSystem()
	{
		static_assert(std::is_base_of<System, SystemType>::value, "System type must be derived from ecs::System!");
		assert(!m_initialized);

		auto system = std::make_unique<SystemType>();
		RegisterSystemInternal(typeid(SystemType), std::move(system));
	}

	template <typename ComponentType>
	void RegisterComponentType()
	{
		assert(!m_initialized);

		// Register type storage
		m_componentStorages.emplace(typeid(ComponentType), ComponentStoragesType());

		// Create first storage
		CreateNewStorage<ComponentType>();
	}

	template <class SystemType>
	SystemType* GetSystem() const
	{
		static_assert(std::is_base_of<System, SystemType>::value, "System type must be derived from ecs::System!");

		auto it = m_systems.find(typeid(SystemType));
		if (it != m_systems.end())
		{
			return static_cast<SystemType*>(it->second.get());
		}

		return nullptr;
	}

	template <typename ComponentType>
	ComponentHandle CreateComponent(ComponentType*& result)
	{
		assert(m_componentStorages.find(typeid(ComponentType)) != m_componentStorages.end());

		int storageIdx = 0;
		auto storage = FindCollectionToInsert(typeid(ComponentType), storageIdx);

		// If storage for insertion not found - allocate new one
		if (nullptr == storage)
		{
			storage = CreateNewStorage<ComponentType>();
		}

		// Storage must be created at this step
		assert(nullptr != storage);

		auto createResult = storage->TryCreate();
		result = static_cast<ComponentType*>(storage->Get(createResult.first));
		assert(createResult.second);
		return ComponentHandle(typeid(ComponentType), createResult.first + storageIdx * k_defaultComponentPoolSize);
	}

	void DestroyComponent(const ComponentHandle& handle);

	template <typename ComponentType>
	ComponentType* GetComponent(const ComponentHandle& handle)
	{
		auto it = m_componentStorages.find(handle.GetTypeIndex());
		if (it != m_componentStorages.end())
		{
			std::size_t poolIndex = handle.GetOffset() / k_defaultComponentPoolSize;
			std::size_t poolOffset = handle.GetOffset() % k_defaultComponentPoolSize;

			return static_cast<ComponentType*>(it->second[poolIndex]->Get(poolOffset));
		}

		return nullptr;
	}

	template <typename ComponentType>
	ComponentHandle CreateComponent()
	{
		assert(m_componentStorages.find(typeid(ComponentType)) != m_componentStorages.end());

		int storageIdx = 0;
		auto storage = FindCollectionToInsert(typeid(ComponentType), storageIdx);

		// If storage for insertion not found - allocate new one
		if (nullptr == storage)
		{
			storage = CreateNewStorage(typeid(ComponentType));
		}

		// Storage must be created at this step
		assert(nullptr != storage);

		auto createResult = storage->TryCreate();
		assert(createResult.second);
		return ComponentHandle(typeid(ComponentType), createResult.first + storageIdx * k_defaultComponentPoolSize);
	}

private:
	void RegisterSystemInternal(const std::type_index& typeIndex, SystemPtr&& system);

	IComponentCollection* FindCollectionToInsert(const std::type_index& typeIndex, int& storageIdx);
	IComponentCollection* GetCollection(const std::type_index& typeIndex, const std::size_t offset);

	template <typename ComponentType>
	IComponentCollection* CreateNewStorage()
	{
		auto storageIt = m_componentStorages.find(typeid(ComponentType));
		assert(storageIt != m_componentStorages.end());

		storageIt->second.emplace_back(std::make_unique<ComponentCollectionImpl<ComponentType, k_defaultComponentPoolSize>>());
		return storageIt->second.back().get();
	}

private:
	std::unordered_map<std::type_index, ComponentStoragesType> m_componentStorages;
	std::unordered_map<std::type_index, SystemPtr> m_systems;
	bool m_initialized = false;
};

} // namespace ecs
