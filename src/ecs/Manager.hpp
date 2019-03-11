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
	ECS_API Manager();

	void ECS_API Init() final;
	void ECS_API Destroy() final;
	void ECS_API Update() final;
	void ECS_API Render() final;

	/**
	* @brief Register system type, provided as template parameter SystemType, in ECS.
	* Type should be a derivative of ecs::System class.
	*/
	template <class SystemType>
	void RegisterSystem()
	{
		static_assert(std::is_base_of<System, SystemType>::value, "System type must be derived from ecs::System!");

		RegisterSystemInternal(typeid(SystemType), std::make_unique<SystemType>(*this));
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
		auto typeId = GetComponentTypeIdByIndex(typeid(ComponentType));
		auto handle = CreateComponentInternal(typeId);

		result = static_cast<ComponentType*>(GetCollection(typeId)->Get(handle.GetOffset()));

		return handle;
	}

	template <typename ComponentType>
	ComponentHandle CreateComponent()
	{
		auto typeId = GetComponentTypeIdByIndex(typeid(ComponentType));
		return CreateComponentInternal(typeId);
	}

	ComponentHandle ECS_API CreateComponentByName(const std::string& name);
	void ECS_API DestroyComponent(const ComponentHandle& handle);

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

	void ECS_API SetComponentEntityId(const ComponentHandle& handle, const uint32_t id);
	uint32_t ECS_API GetComponentEntityId(const ComponentHandle& handle) const;

	ECS_API EntitiesCollection& GetEntitiesCollection();

	/**
	* Returns component type id by type index
	* @param typeIndex - type index of component
	* @return Id of component type
	*/
	uint8_t ECS_API GetComponentTypeIdByIndex(const std::type_index& typeIndex) const;
	std::type_index ECS_API GetComponentTypeIndexByTypeId(const uint8_t typeId) const;

	void ECS_API SetComponentEnabled(const ComponentHandle& handle, const bool enabled);

private:
	/**
	* @brief Registers system inside internal systems collection
	*/
	void ECS_API RegisterSystemInternal(const std::type_index& typeIndex, SystemPtr&& system);

	/**
	* @brief Returns component collection for components with particular id
	* @param typeId - id of component type
	* @return Component collection, associated with provided type id
	*/
	ECS_API IComponentCollection* GetCollection(const uint8_t typeId) const;

	/**
	* @brief Requests component instance creation given the type id of requested component
	* @param typeId - id of requested component type
	* @return Handle to created component instance
	*/
	ComponentHandle ECS_API CreateComponentInternal(const uint8_t typeId);

	void ECS_API RegisterComponentTypeInternal(const std::string& name, const std::type_index& typeIndex, std::unique_ptr<IComponentCollection>&& collection);

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
