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
	using SystemPtr = std::unique_ptr<System>;

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
	void RegisterComponentType(const std::string& name)
	{
		assert(!m_initialized);

		// Register type storage
		m_componentStorages.emplace(typeid(ComponentType), std::make_unique<ComponentCollectionImpl<ComponentType>>());
		m_componentNames.emplace(name, typeid(ComponentType));
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
		auto collection = GetCollection(typeid(ComponentType));
		assert(nullptr != collection);
		
		auto insertedId = collection->Create();
		result = static_cast<ComponentType*>(collection->Get(insertedId));

		return ComponentHandle(typeid(ComponentType), insertedId);
	}

	template <typename ComponentType>
	ComponentHandle CreateComponent()
	{
		auto collection = GetCollection(typeid(ComponentType));
		assert(nullptr != collection);

		return CreateComponentInternal(typeIndexIt->second, collection);
	}

	ComponentHandle CreateComponentByName(const std::string& name);
	void DestroyComponent(const ComponentHandle& handle);

	template <typename ComponentType>
	ComponentType* GetComponent(const ComponentHandle& handle)
	{
		auto collection = GetCollection(handle.GetTypeIndex());
		if (nullptr != collection)
		{
			return static_cast<ComponentType*>(collection->Get(handle.GetOffset()));
		}

		return nullptr;
	}

	template <typename ComponentType>
	ComponentCollectionImpl<ComponentType>* GetComponentCollection()
	{
		return static_cast<ComponentCollectionImpl<ComponentType>*>(GetCollection(typeid(ComponentType)));
	}

private:
	void RegisterSystemInternal(const std::type_index& typeIndex, SystemPtr&& system);
	IComponentCollection* GetCollection(const std::type_index& typeIndex);
	ComponentHandle CreateComponentInternal(const std::type_index& typeIndex, IComponentCollection* collection);

private:
	std::unordered_map<std::type_index, std::unique_ptr<IComponentCollection>> m_componentStorages;
	std::unordered_map<std::type_index, SystemPtr> m_systems;
	std::unordered_map<std::string, std::type_index> m_componentNames;
	bool m_initialized = false;
};

} // namespace ecs
