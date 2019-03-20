#pragma once
#include "ecs/ECSApiDef.hpp"
#include <cstdint>
#include <limits>

namespace ecs
{

class EntitiesCollection;
struct ComponentHandle;
using EntityId = uint32_t;
using HierarchyDepth = uint16_t;

struct Entity
{
	EntityId id;
	EntityId parentId;
	uint32_t hierarchyDataOffset;
	uint32_t componentsMask;
	uint32_t componentsDataOffset;
	HierarchyDepth hierarchyDepth;
	uint16_t orderInParent;

	static const EntityId ECS_API GetInvalidId();
	static const HierarchyDepth ECS_API GetInvalidHierarchyDepth();

	Entity()
		: id(Entity::GetInvalidId())
		, parentId(Entity::GetInvalidId())
		, hierarchyDataOffset(0U)
		, componentsMask(0U)
		, componentsDataOffset(0U)
		, hierarchyDepth(GetInvalidHierarchyDepth())
		, orderInParent(GetInvalidHierarchyDepth())
	{}

	//void ECS_API AddComponent(const ComponentHandle& handle);
	//void ECS_API RemoveComponent(const ComponentHandle& handle);
	//bool ECS_API HasComponent(const uint8_t componentType);
	//ECS_API void* GetComponent(const uint8_t componentType);
	//ECS_API Entity* GetParent();

	//template <typename ComponentType>
	//ComponentType* GetComponent() const
	//{
	//	uint8_t componentTypeId = s_collection->GetComponentTypeIdByTypeIndex(typeid(ComponentType));
	//	return static_cast<ComponentType*>(GetComponent(componentTypeId));
	//}
};

} // namespace ecs
