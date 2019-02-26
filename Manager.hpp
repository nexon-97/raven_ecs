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

class Manager
	: public ILifecycleCallback
{
	constexpr static std::size_t poolSize = 1024U;
	template <typename T>
	using ComponentPool = std::array<T, poolSize>;
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

		ComponentStoragesType storage;
		storage.emplace_back(std::make_unique<ComponentCollectionImpl<ComponentType, 1024U>>());
		m_componentStorages.emplace(typeid(ComponentType), std::move(storage));
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
		auto storageIt = m_componentStorages.find(typeid(ComponentType));
		if (storageIt != m_componentStorages.end())
		{
			for (const auto& storage : storageIt->second)
			{
				auto createResult = storage->TryCreate();
				if (createResult.second)
				{
					result = static_cast<ComponentType*>(storage->Get(createResult.first));
					return ComponentHandle(typeid(ComponentType), createResult.first);
				}
			}
		}

		return ComponentHandle();
	}

	void DestroyComponent(const ComponentHandle& handle);

	template <typename ComponentType>
	ComponentHandle CreateComponent()
	{
		auto storageIt = m_componentStorages.find(typeid(ComponentType));
		if (storageIt != m_componentStorages.end())
		{
			for (const auto& storage : storageIt->second)
			{
				auto createResult = storage->TryCreate();
				if (createResult.second)
				{
					return ComponentHandle(0, createResult.first);
				}
			}
		}

		return ComponentHandle();
	}

private:
	void RegisterSystemInternal(const std::type_index& typeIndex, SystemPtr&& system);

private:
	std::unordered_map<std::type_index, ComponentStoragesType> m_componentStorages;
	std::unordered_map<std::type_index, SystemPtr> m_systems;
	bool m_initialized = false;
};

} // namespace ecs
