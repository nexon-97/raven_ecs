#pragma once
#include "ecs/TypeAliases.hpp"
#include "ecs/component/ComponentPtr.hpp"

#include <list>
#include <vector>
#include <memory>

namespace ecs
{

struct Entity;
using EntityChildrenContainer = std::list<Entity>;
using EntityComponentsContainer = std::vector<ComponentPtr>;

/*
* @brief EntityData is a data container, which fully describes entity, its state, components and reference counting.
* EntityData is created inside EntitiesCollection, and user have not access to it directly, only via Entity wrapper class.
*/
struct EntityData
{
	EntityId id;
	EntityId parentId;
	ComponentMaskType componentsMask;
	EntityComponentsContainer components;
	uint16_t orderInParent;
	uint16_t refCount;
	EntityHandleIndex storageLocation;
	EntityChildrenContainer children;
	std::unique_ptr<std::string> name;
	bool isIteratingComponents : 1; // Indicates if the user is currently iterating components of an entity

	EntityData(const EntityData&) = delete;
	EntityData& operator=(const EntityData&) = delete;

	~EntityData() = default;

private:
	friend class EntitiesCollection;
	friend class detail::MemoryPool<EntityData>;

	EntityData();

	EntityData(EntityData&&) noexcept;
	EntityData& operator=(EntityData&&) noexcept;
};

} // namespace ecs
