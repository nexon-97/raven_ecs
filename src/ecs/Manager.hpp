#pragma once
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <typeindex>
#include <vector>

#include "ecs/component/ComponentCollectionImpl.hpp"
#include "ecs/System.hpp"
#include "ecs/entity/EntitiesCollection.hpp"
#include "ecs/cache/ComponentsTupleCache.hpp"
#include "ecs/events/DelegateMacro.hpp"

DECLARE_MULTICAST_DELEGATE(EntityCreateDelegate, ecs::Entity);
DECLARE_MULTICAST_DELEGATE(EntityDestroyDelegate, ecs::EntityId);
DECLARE_MULTICAST_DELEGATE(ComponentCreateDelegate, ecs::ComponentPtr);
DECLARE_MULTICAST_DELEGATE(ComponentDestroyDelegate, ecs::ComponentPtr);
DECLARE_MULTICAST_DELEGATE(ComponentAttachedDelegate, ecs::Entity, ecs::ComponentPtr);
DECLARE_MULTICAST_DELEGATE(ComponentDetachedDelegate, ecs::Entity, ecs::ComponentPtr);
DECLARE_MULTICAST_DELEGATE(SystemPriorityChangedDelegate, ecs::System*);

namespace ecs
{

constexpr std::size_t k_defaultComponentPoolSize = 1024U;

class Manager
{
	using SystemPtr = std::unique_ptr<System>;
	friend class EntitiesCollection;
	friend class ComponentPtr;
	friend struct Entity;

public:
	ECS_API Manager();

	void ECS_API Init();
	void ECS_API Destroy();
	void ECS_API Update();

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Section for systems management

	/**
	* @brief Creates system of SystemType, and stores it inside manager. Then, safely adds the system to the systems list.
	* Type must be a derivative of ecs::System class.
	*/
	template <class SystemType, typename ...Args>
	SystemType* AddSystem(Args&&... args)
	{
		static_assert(std::is_base_of<System, SystemType>::value, "System type must be derived from ecs::System!");

		std::unique_ptr<SystemType> system = std::make_unique<SystemType>(std::forward<Args>(args)...);
		SystemType* systemRawPtr = system.get();

		AddSystemToStorage(std::move(system));

		return systemRawPtr;
	}

	void ECS_API AddSystem(System* system);
	void ECS_API RemoveSystem(System* system);
	ECS_API System* GetSystemByTypeIndex(const std::type_index& typeIndex) const;

	template <class SystemType>
	SystemType* GetSystem() const
	{
		static_assert(std::is_base_of<System, SystemType>::value, "System type must be derived from ecs::System!");

		System* system = GetSystemByTypeIndex(typeid(SystemType));
		return static_cast<SystemType*>(system);
	}

	void ECS_API NotifySystemPriorityChanged();

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template <typename ComponentType>
	void RegisterComponentType(const std::string& name)
	{
		ComponentTypeId typeId = static_cast<ComponentTypeId>(m_componentStorages.size());
		auto collection = std::make_unique<ComponentCollectionImpl<ComponentType>>(typeId);
		
		RegisterComponentTypeInternal(name, typeid(ComponentType), typeId, std::move(collection));
	}

	template <typename ComponentType>
	TComponentPtr<ComponentType> CreateComponent()
	{
		ComponentTypeId typeId = GetComponentTypeIdByIndex(typeid(ComponentType));
		return TComponentPtr<ComponentType>(CreateComponentInternal(typeId));
	}

	template <typename ComponentType>
	ComponentType* GetComponent(ComponentTypeId componentType, int32_t index)
	{
		void* componentRaw = GetComponentRaw(componentType, index);
		return static_cast<ComponentType*>(componentRaw);
	}

	ECS_API void* GetComponentRaw(ComponentTypeId componentType, int32_t index);
	ComponentPtr ECS_API CreateComponentByName(const std::string& name);
	ComponentPtr ECS_API CreateComponentByTypeId(const ComponentTypeId typeId);
	void ECS_API MoveComponentData(const ComponentPtr& handle, void* dataPtr);

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

	/**
	* Returns component type id by type index
	* @param typeIndex - type index of component
	* @return Id of component type
	*/
	ComponentTypeId ECS_API GetComponentTypeIdByIndex(const std::type_index& typeIndex) const;
	ComponentTypeId ECS_API GetComponentTypeIdByName(const std::string& name) const;
	std::type_index ECS_API GetComponentTypeIndexByTypeId(const ComponentTypeId typeId) const;

	ComponentPtr ECS_API CloneComponent(const ComponentPtr& componentPtr);

	ECS_API const std::string& GetComponentNameByTypeId(const ComponentTypeId typeId) const;
	ECS_API const std::string& GetComponentNameByTypeId(const std::type_index& typeIndex) const;

	ECS_API EntityCreateDelegate& GetEntityCreateDelegate();
	ECS_API EntityDestroyDelegate& GetEntityDestroyDelegate();
	ECS_API ComponentCreateDelegate& GetComponentCreateDelegate();
	ECS_API ComponentDestroyDelegate& GetComponentDestroyDelegate();
	ECS_API ComponentAttachedDelegate& GetComponentAttachedDelegate();
	ECS_API ComponentDetachedDelegate& GetComponentDetachedDelegate();

	Entity ECS_API GetEntityById(const EntityId id);
	Entity ECS_API CreateEntity();

	template <class ...ComponentT>
	void RegisterComponentsTupleIterator()
	{
		std::vector<ComponentTypeId> typeIds;
		(typeIds.push_back(GetComponentTypeIdByIndex(typeid(ComponentT))), ...);

		RegisterComponentsTupleIterator(typeIds);
	}

	void ECS_API RegisterComponentsTupleIterator(std::vector<ComponentTypeId>& typeIds);

	static ECS_API Manager* Get();
	static void ECS_API InitECSManager();
	static void ECS_API ShutdownECSManager();

	static ComponentTypeId ECS_API GetInvalidComponentTypeId();

private:
	/**
	* @brief Registers system inside internal systems collection
	*/
	void ECS_API AddSystemToStorage(SystemPtr&& system);

	// Private methods for systems internal management
	void UpdateSystems();
	void SortOrderedSystemsList();
	void AddSystemToOrderedSystemsList(System* system);
	void DoRemoveSystem(System* system);

	/**
	* @brief Returns component collection for components with particular id
	* @param typeId - id of component type
	* @return Component collection, associated with provided type id
	*/
	ECS_API IComponentCollection* GetCollection(const ComponentTypeId typeId) const;

	EntitiesCollection& GetEntitiesCollection();

	/**
	* @brief Requests component instance creation given the type id of requested component
	* @param typeId - id of requested component type
	* @return Handle to created component instance
	*/
	ComponentPtr ECS_API CreateComponentInternal(const ComponentTypeId typeId);

	void ReleaseComponent(ComponentTypeId componentType, int32_t index);

	void ECS_API RegisterComponentTypeInternal(const std::string& name, const std::type_index& typeIndex, const ComponentTypeId typeId, std::unique_ptr<IComponentCollection>&& collection);

private:
	std::vector<std::unique_ptr<IComponentCollection>> m_componentStorages;
	std::vector<std::type_index> m_componentTypeIndexes;
	std::unordered_map<std::string, ComponentTypeId> m_componentNameToIdMapping;
	std::unordered_map<std::type_index, ComponentTypeId> m_typeIndexToComponentTypeIdMapping;

	std::vector<SystemPtr> m_systemsStorage; // Systems that are created just inside ecs manager, and owned by the external code
	std::unordered_map<std::type_index, System*> m_systemsTypeIdMapping; // Mapping of type id to the systems
	std::vector<System*> m_orderedSystems; // Ordered systems pointers list, for strict execution order

	// Pairs of new systems, that have not been initialized yet, and will be initialized at next update,
	// and the bool indicator, which tells the system must be deferredly added to the m_orderedSystems
	std::vector<std::pair<System*, bool>> m_newSystems; 
	std::vector<System*> m_removedSystems; // Removed systems, that have not been destroyed and removed yet
	
	EntitiesCollection m_entitiesCollection; // Class, that manages entities and their functionality

	std::unordered_map<uint32_t, ComponentsTupleCache> m_tupleCaches;

	// Global ecs state delegates
	EntityCreateDelegate m_entityCreateDelegate;
	EntityDestroyDelegate m_entityDestroyDelegate;
	ComponentCreateDelegate m_componentCreateDelegate;
	ComponentDestroyDelegate m_componentDestroyDelegate;
	ComponentAttachedDelegate m_componentAttachedDelegate;
	ComponentDetachedDelegate m_componentDetachedDelegate;
	SystemPriorityChangedDelegate m_systemPriorityChangedDelegate;

	bool m_systemPrioritiesChanged = true; // Flag, indicating that systems need to be sorted prior next update
	bool m_isUpdatingSystems = false; // Flag, indicating that manager is currently updating exisiting systems
	bool m_isBeingDestroyed = false;
};

} // namespace ecs
