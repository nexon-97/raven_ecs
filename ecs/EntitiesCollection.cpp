#include "EntitiesCollection.hpp"

namespace ecs
{

const uint32_t Entity::k_invalidId = static_cast<uint32_t>(-1);

Entity& EntitiesCollection::GetEntity(const uint32_t id)
{
	return m_entities[id].entity;
}

Entity& EntitiesCollection::CreateEntity()
{
	m_entities.emplace_back();

	auto& entityData = m_entities.back();
	entityData.isAlive = true;

	return entityData.entity;
}

void EntitiesCollection::DestroyEntity(const uint32_t id)
{
	m_entities[id].entity.id = Entity::k_invalidId;
	m_entities[id].isAlive = false;
}

} // namespace ecs
