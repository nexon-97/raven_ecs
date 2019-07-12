#pragma once
#include <cstdint>
#include <bitset>

namespace ecs
{
namespace detail
{
template <class T> class MemoryPool;
}

using EntityId = uint32_t;
using HierarchyDepth = uint16_t;
const std::size_t MaxComponentTypesCount = 128U;
using ComponentMaskType = std::bitset<MaxComponentTypesCount>;
using EntityHandleIndex = uint32_t;

/*
* @brief EntityData is a data container, which fully describes entity, its state, components and reference counting.
* EntityData is created inside EntitiesCollection, and user have not access to it directly, only via Entity wrapper class.
*/
struct EntityData
{
	EntityId id;
	EntityId parentId;
	ComponentMaskType componentsMask;
	uint32_t hierarchyDataOffset;
	uint32_t componentsDataOffset;
	HierarchyDepth hierarchyDepth;
	uint16_t orderInParent;
	uint16_t refCount;
	uint16_t childrenCount;
	EntityHandleIndex storageLocation;
	bool isEnabled : 1;		// Indicates if the entity is enabled (though can be not registered in world)
	bool isActivated : 1;	// Indicates if the entity is currently activated (is actually registered in world)
	bool isIteratingComponents : 1; // Indicates if the user is currently iterating components of an entity

	EntityData(const EntityData&) = delete;
	EntityData& operator=(const EntityData&) = delete;

private:
	friend class EntitiesCollection;
	friend class detail::MemoryPool<EntityData>;

	EntityData();
	~EntityData();

	EntityData(EntityData&&) noexcept;
	EntityData& operator=(EntityData&&) noexcept;
};

} // namespace ecs
