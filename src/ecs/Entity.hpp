#pragma once
#include "ecs/ECSApiDef.hpp"
#include "ecs/ComponentHandle.hpp"
#include "storage/MemoryPool.hpp"
#include <cstdint>
#include <limits>
#include <bitset>
#include <cassert>

namespace ecs
{

class EntitiesCollection;
struct ComponentHandle;
using EntityId = uint32_t;
using HierarchyDepth = uint16_t;
const std::size_t MaxComponentTypesCount = 128U;
class Manager;

struct Entity
{
	friend class detail::MemoryPool<Entity>;

	static const EntityId ECS_API GetInvalidId();
	static const HierarchyDepth ECS_API GetInvalidHierarchyDepth();

	~Entity() = default;

	void ECS_API AddComponent(const ComponentHandle& handle);
	void ECS_API RemoveComponent(const ComponentHandle& handle);
	bool ECS_API HasComponent(const ComponentTypeId componentType) const;
	ComponentHandle ECS_API GetComponentHandle(const ComponentTypeId componentType) const;

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
	Entity(const Entity&) = delete;
	Entity& operator=(const Entity&) = delete;

	// Move is available
	ECS_API Entity(Entity&&) noexcept;
	ECS_API Entity& operator=(Entity&&) noexcept;

	void ECS_API AddRef();
	void ECS_API RemoveRef();

	static void SetManagerInstance(Manager* manager);

private:
	ECS_API void* DoGetComponentPtr(const ComponentHandle handle) const;

	// Must be constructed only by EntitiesCollection class
	friend class EntitiesCollection;
	Entity();

	static Manager* s_ecsManager;
	static Manager* GetManagerInstance();

	EntityId id;
	EntityId parentId;
	std::bitset<MaxComponentTypesCount> componentsMask;
	uint32_t hierarchyDataOffset;
	uint32_t componentsDataOffset;
	HierarchyDepth hierarchyDepth;
	uint16_t orderInParent;
	uint16_t refCount;
	uint16_t childrenCount;
	bool isEnabled : 1;		// Indicates if the entity is enabled (though can be not registered in world)
	bool isActivated : 1;	// Indicates if the entity is currently activated (is actually registered in world)
	bool isIteratingComponents : 1; // Indicates if the user is currently iterating components of an entity
};

} // namespace ecs
