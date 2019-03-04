#pragma once
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <typeindex>

#include "ComponentCollectionImpl.hpp"
#include "ILifecycleCallback.hpp"
#include "System.hpp"
#include "EntitiesCollection.hpp"

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

		RegisterSystemInternal(typeid(SystemType), std::make_unique<SystemType>());
	}

	template <typename ComponentType>
	void RegisterComponentType(const std::string& name)
	{
		auto collection = std::make_unique<ComponentCollectionImpl<ComponentType>>();
		RegisterComponentTypeInternal(name, typeid(ComponentType), std::move(collection));
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
		auto typeId = GetComponentTypeIdByIndex(typeid(ComponentType));
		return CreateComponentInternal(typeId);
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
		auto it = m_typeIndexToComponentTypeIdMapping.find(typeid(ComponentType));
		if (it != m_typeIndexToComponentTypeIdMapping.end())
		{
			return static_cast<ComponentCollectionImpl<ComponentType>*>(GetCollection(it->second));
		}
		
		return nullptr;
	}

	void SetComponentEntityId(const ComponentHandle& handle, const std::size_t id);
	std::size_t GetComponentEntityId(const ComponentHandle& handle) const;

	EntitiesCollection& GetEntitiesCollection();

private:
	/**
	* @brief Registers system inside internal systems collection
	*/
	void RegisterSystemInternal(const std::type_index& typeIndex, SystemPtr&& system);

	/**
	* @brief Returns component collection for components with particular id
	* @param typeId - id of component type
	* @return Component collection, associated with provided type id
	*/
	IComponentCollection* GetCollection(const uint8_t typeId) const;

	/**
	* @brief Requests component instance creation given the type id of requested component
	* @param typeId - id of requested component type
	* @return Handle to created component instance
	*/
	ComponentHandle CreateComponentInternal(const uint8_t typeId);

	/**
	* Returns component type id by type index
	* @param typeIndex - type index of component
	* @return Id of component type
	*/
	uint8_t GetComponentTypeIdByIndex(const std::type_index& typeIndex) const;

	void RegisterComponentTypeInternal(const std::string& name, const std::type_index& typeIndex, std::unique_ptr<IComponentCollection>&& collection);

private:
	std::vector<std::unique_ptr<IComponentCollection>> m_componentStorages;
	std::vector<std::type_index> m_componentTypeIndexes;
	std::unordered_map<std::type_index, SystemPtr> m_systems;
	std::unordered_map<std::string, uint8_t> m_componentNameToIdMapping;
	std::unordered_map<std::type_index, uint8_t> m_typeIndexToComponentTypeIdMapping;
	EntitiesCollection m_entitiesCollection;
	bool m_initialized = false;
};

} // namespace ecs
