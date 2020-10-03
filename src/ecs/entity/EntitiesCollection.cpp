#include "ecs/entity/EntitiesCollection.hpp"
#include "ecs/Manager.hpp"

#include <functional>

namespace
{
const uint16_t k_invalidOrderInParent = uint16_t(-1);
const ecs::EntityId k_invalidEntityId = std::numeric_limits<ecs::EntityId>::max();
}

namespace ecs
{

EntitiesCollection::EntitiesCollection()
	: m_entitiesData(1024U)
{}

void EntitiesCollection::Clear()
{
	m_entitiesData.Clear();
}

Entity EntitiesCollection::GetEntityById(const EntityId id)
{
	auto it = m_entityIdsMap.find(id);
	if (it != m_entityIdsMap.end())
	{
		uint32_t location = it->second;
		EntityData* entityData = GetEntityData(id);
		return Entity(entityData);
	}

	return Entity();
}

Entity EntitiesCollection::CreateEntity()
{
	EntityData* entityData = AllocateEntityData();
	const EntityId newEntityId = m_nextEntityId;
	++m_nextEntityId;

	entityData->id = newEntityId;
	entityData->parentId = Entity::GetInvalidId();
	entityData->orderInParent = k_invalidOrderInParent;

	// Add entity to ids map
	m_entityIdsMap.emplace(entityData->id, entityData->storageLocation);

	// Invoke global callback
	Entity createdEntity(entityData);
	Manager::Get()->GetEntityCreateDelegate().Broadcast(createdEntity);

	return createdEntity;
}

void EntitiesCollection::MoveEntityData(EntityData& entityData, const uint32_t newLocation)
{
	EntityData* newLocationDataPtr = m_entitiesData[newLocation];
	*newLocationDataPtr = std::move(entityData);
}

void EntitiesCollection::OnEntityDataDestroy(EntityId entityId)
{
	// Find entity data location
	auto it = m_entityIdsMap.find(entityId);
	assert(it != m_entityIdsMap.end());

	uint32_t location = it->second;
	EntityData* entityData = m_entitiesData[location];

	// Remove from entity id -> storage location mapping
	m_entityIdsMap.erase(it);

	// Detach all components from entity
	for (const auto& componentHandle : entityData->components)
	{
		entityData->componentsMask.reset(componentHandle.GetTypeId());
		componentHandle.m_block->entityId = k_invalidEntityId;

		Manager::Get()->HandleComponentDetach(entityId, componentHandle);
	}
	entityData->components.clear();

	// Replace the entity data with newly created one
	if (!Manager::Get()->m_isBeingDestroyed)
	{
		m_storageHoles.push_back(location);
		*entityData = EntityData();
	}

	// Invoke global callback, when entity has already been destroyed
	Manager::Get()->GetEntityDestroyDelegate().Broadcast(entityId);
}

EntityData* EntitiesCollection::AllocateEntityData()
{
	if (m_storageHoles.empty())
	{
		auto entityCreationResult = m_entitiesData.CreateItem();
		entityCreationResult.second->storageLocation = static_cast<EntityHandleIndex>(entityCreationResult.first);

		return entityCreationResult.second;
	}
	else
	{
		uint32_t location = m_storageHoles.front();
		m_storageHoles.pop_front();

		EntityData* data = m_entitiesData[location];
		data->storageLocation = location;
		
		return data;
	}
}

EntityData* EntitiesCollection::GetEntityData(const EntityId id)
{
	auto it = m_entityIdsMap.find(id);
	if (it != m_entityIdsMap.end())
	{
		return m_entitiesData[it->second];
	}

	return nullptr;
}

} // namespace ecs
