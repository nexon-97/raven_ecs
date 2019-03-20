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

//EntitiesCollection* Entity::s_collection = nullptr;
//
//void Entity::AddComponent(const ComponentHandle& handle)
//{
//	s_collection->AddComponent(*this, handle);
//}
//
//void Entity::RemoveComponent(const ComponentHandle& handle)
//{
//	return s_collection->RemoveComponent(*this, handle);
//}
//
//bool Entity::HasComponent(const uint8_t componentType)
//{
//	return s_collection->HasComponent(*this, componentType);
//}
//
//void* Entity::GetComponent(const uint8_t componentType)
//{
//	return s_collection->GetComponent(*this, componentType);
//}
//
//Entity* Entity::GetParent()
//{
//	return s_collection->GetParent(*this);
//}

} // namespace ecs
