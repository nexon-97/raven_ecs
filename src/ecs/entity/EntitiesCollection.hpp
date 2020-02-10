#pragma once
#include "ecs/entity/EntityData.hpp"
#include "ecs/entity/Entity.hpp"
#include "ecs/storage/MemoryPool.hpp"

#include <unordered_map>
#include <deque>

namespace ecs
{

class EntitiesCollection
{
	friend struct Entity;

	using EntityIdsMap = std::unordered_map<EntityId, uint32_t>; // Mapping of entity id to entity storage location

public:
	EntitiesCollection();

	// Disable collection copy
	EntitiesCollection(const EntitiesCollection&) = delete;
	EntitiesCollection& operator=(const EntitiesCollection&) = delete;

	Entity GetEntityById(const EntityId id);
	Entity CreateEntity();

	void Clear();

private:
	using EntitiesStorageType = detail::MemoryPool<EntityData>;

	void MoveEntityData(EntityData& entityData, const uint32_t newLocation);
	void OnEntityDataDestroy(EntityId entityId);

	EntityComponentMapEntry& CreateComponentMappingEntry(EntityData& entityData);
	EntityComponentMapEntry* FindComponentMappingEntry(EntityData& entityData, const ComponentTypeId componentType);
	void RemoveComponentMappingEntry(EntityData& entityData, const ComponentTypeId componentType);
	ComponentsMapStorageType& GetComponentsMapStorage();

	EntityData* AllocateEntityData();
	EntityData* GetEntityData(const EntityId id);

private:
	EntitiesStorageType m_entitiesData;
	EntityIdsMap m_entityIdsMap;
	ComponentsMapStorageType m_entityComponentsMapping;
	EntityId m_nextEntityId = 0U;
	std::deque<uint32_t> m_storageHoles;
};

} // namespace ecs
