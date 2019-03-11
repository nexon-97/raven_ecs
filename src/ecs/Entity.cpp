#include "ecs/Entity.hpp"
#include "ecs/ComponentHandle.hpp"
#include "ecs/EntitiesCollection.hpp"

namespace ecs
{

const uint32_t k_invalidId = static_cast<uint32_t>(-1);

const uint32_t Entity::GetInvalidId()
{
	return k_invalidId;
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
