#include "ecs/Entity.hpp"
#include "ecs/ComponentHandle.hpp"
#include "ecs/EntitiesCollection.hpp"

namespace ecs
{

const EntityId k_invalidId = std::numeric_limits<EntityId>::max();
const HierarchyDepth k_invalidHierarchyDepth = std::numeric_limits<HierarchyDepth>::max();

const EntityId Entity::GetInvalidId()
{
	return k_invalidId;
}

const HierarchyDepth Entity::GetInvalidHierarchyDepth()
{
	return k_invalidHierarchyDepth;
}

} // namespace ecs
