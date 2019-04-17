#pragma once
#include "ecs/ECSApiDef.hpp"
#include <cstdint>
#include <limits>
#include <bitset>

namespace ecs
{

class EntitiesCollection;
struct ComponentHandle;
using EntityId = uint32_t;
using HierarchyDepth = uint16_t;
const std::size_t MaxComponentTypesCount = 128U;

struct Entity
{
	EntityId id;
	EntityId parentId;
	std::bitset<MaxComponentTypesCount> componentsMask;
	uint32_t hierarchyDataOffset;
	uint32_t componentsDataOffset;
	HierarchyDepth hierarchyDepth;
	uint16_t orderInParent;

	static const EntityId ECS_API GetInvalidId();
	static const HierarchyDepth ECS_API GetInvalidHierarchyDepth();

	Entity()
		: id(Entity::GetInvalidId())
		, parentId(Entity::GetInvalidId())
		, hierarchyDataOffset(0U)
		, componentsDataOffset(0U)
		, hierarchyDepth(GetInvalidHierarchyDepth())
		, orderInParent(GetInvalidHierarchyDepth())
	{}

	// Disable entities copying
	Entity(const Entity&) = delete;
	Entity& operator=(const Entity&) = delete;
};

} // namespace ecs
