#pragma once
#include "ecs/ECSApiDef.hpp"
#include "ecs/entity/EntityData.hpp"
#include "ecs/entity/EntityComponentsCollection.hpp"

namespace ecs
{

struct EntityData;
class EntitiesCollection;
class Manager;

/*
* @brief Entity is a wrapper around entity data, that gives the interface for manipuling the data using ecs infrastructure.
* Entity is a kind of reference couting wrapper, each entity adds to the alive references count.
* Entity can contain no entity data, it is a valid situation, but this object cannot be used to access the data, and the
* object can be tested using simple operator bool or IsValid() method.
*
* Entity has interface to manipulate entity components
* Entity has interface to manipulate its allowed properties
* User can copy and move entity instances
* User has no access to underlying entity data
*/
struct Entity
{
	static const EntityId ECS_API GetInvalidId();
	static const HierarchyDepth ECS_API GetInvalidHierarchyDepth();

	ECS_API Entity();
	ECS_API ~Entity();

	bool ECS_API IsValid() const;
	ECS_API operator bool() const;

	void ECS_API Reset();

	Entity ECS_API Clone();

	void ECS_API AddComponent(const ComponentHandle& handle);
	void ECS_API RemoveComponent(const ComponentHandle& handle);
	bool ECS_API HasComponent(const ComponentTypeId componentType) const;
	ComponentHandle ECS_API GetComponentHandle(const ComponentTypeId componentType) const;

	EntityComponentsCollection ECS_API GetComponents() const;

	EntityId ECS_API GetId() const;
	std::size_t ECS_API GetChildrenCount() const;
	EntityId ECS_API GetParentId() const;
	void ECS_API SetEnabled(const bool enable);
	bool ECS_API IsEnabled() const;
	HierarchyDepth ECS_API GetHierarchyDepth() const;

	template <typename ComponentType>
	bool HasComponent() const
	{
		ComponentTypeId componentTypeId = s_ecsManager->GetComponentTypeIdByIndex(typeid(ComponentType));
		return HasComponent(componentTypeId);
	}

	template <typename ComponentType>
	ComponentType* GetComponent() const
	{
		ComponentTypeId componentTypeId = s_ecsManager->GetComponentTypeIdByIndex(typeid(ComponentType));
		ComponentHandle handle = GetComponentHandle(componentTypeId);
		if (handle.IsValid())
		{
			return static_cast<ComponentType*>(DoGetComponentPtr(handle));
		}
		else
		{
			return nullptr;
		}
	}

	template <typename ComponentType>
	ComponentHandle GetComponentHandle() const
	{
		ComponentTypeId componentTypeId = s_ecsManager->GetComponentTypeIdByIndex(typeid(ComponentType));
		return GetComponentHandle(componentTypeId);
	}

	// Copy is available
	Entity(const Entity&) noexcept;
	Entity& operator=(const Entity&) noexcept;

	// Move is available
	ECS_API Entity(Entity&&) noexcept;
	ECS_API Entity& operator=(Entity&&) noexcept;

	static void SetManagerInstance(Manager* manager);

private:
	ECS_API void* DoGetComponentPtr(const ComponentHandle handle) const;

	// Must be constructed only by EntitiesCollection class
	friend class EntitiesCollection;
	Entity(EntityData* data);

	void AddRef();
	void RemoveRef();

	EntityData* GetData() const;

	static Manager* s_ecsManager;
	static Manager* GetManagerInstance();

	EntityData* m_data = nullptr;
};

} // namespace ecs
