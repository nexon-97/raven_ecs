#include "ecs/entity/EntitiesCollection.hpp"
#include "ecs/Manager.hpp"

namespace
{
const uint16_t k_invalidOrderInParent = uint16_t(-1);
}

namespace ecs
{

EntitiesCollection::EntitiesCollection(ecs::Manager& ecsManager)
	: m_manager(ecsManager)
	, m_entitiesData(1024U)
	, m_entityComponentsMapping(1024U)
{}

void EntitiesCollection::Clear()
{
	m_entitiesData.Clear();
	m_entityComponentsMapping.Clear();
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

	//EntityData* entityData = entityCreationResult.second;
	entityData->id = newEntityId;
	entityData->parentId = Entity::GetInvalidId();
	entityData->orderInParent = k_invalidOrderInParent;

	{
		auto componentMappingCreationResult = m_entityComponentsMapping.CreateItem();
		auto mappingItem = componentMappingCreationResult.second;
		mappingItem->handle = ComponentHandle(ComponentHandleInternal::GetInvalidTypeId(), nullptr);
		mappingItem->nextItemPtr = Entity::GetInvalidId();

		entityData->componentsDataOffset = static_cast<uint32_t>(componentMappingCreationResult.first);
	}

	// Add entity to ids map
	m_entityIdsMap.emplace(entityData->id, entityData->storageLocation);

	return Entity(entityData);
}

void EntitiesCollection::MoveEntityData(EntityData& entityData, const uint32_t newLocation)
{
	EntityData* newLocationDataPtr = m_entitiesData[newLocation];
	*newLocationDataPtr = std::move(entityData);
}

//void EntitiesCollection::DestroyEntity(const EntityId id)
//{
//	EntityData& entityData = *m_entitiesData[id];
//
//	// Disable all owned components
//	auto componentNode = m_entityComponentsMapping[entityData.componentsDataOffset];
//	bool reachedEndOfList = false;
//
//	while (!reachedEndOfList)
//	{
//		if (componentNode->handle.IsValid())
//		{
//			m_manager.DestroyComponent(componentNode->handle);
//		}
//
//		reachedEndOfList = (componentNode->nextItemPtr == Entity::GetInvalidId());
//
//		if (!reachedEndOfList)
//		{
//			componentNode = m_entityComponentsMapping[componentNode->nextItemPtr];
//		}
//	}
//
//	// Erase list items
//	componentNode = m_entityComponentsMapping[entityData.componentsDataOffset];
//	componentNode->handle = ComponentHandle(ComponentHandleInternal::GetInvalidTypeId(), nullptr);
//	componentNode->nextItemPtr = Entity::GetInvalidId();
//
//	// TODO: Performs logic consistency checks. Entity should be properly logically unloaded before actually destroying it...
//}

void EntitiesCollection::OnEntityEnabled(const EntityId entityId, const bool enabled)
{
	RefreshActivation(*GetEntityData(entityId));
}

void EntitiesCollection::ActivateEntity(Entity& entity, const bool activate)
{
	EntityData* entityData = entity.GetData();
	if (activate != entityData->isActivated)
	{
		RefreshActivation(*entityData, activate);
	}
}

bool EntitiesCollection::IsEntityActivated(Entity& entity) const
{
	EntityData* entityData = entity.GetData();
	return entityData->isActivated;
}

void EntitiesCollection::RefreshActivation(EntityData& entityData, bool forceActivate)
{
	bool shouldBeAddedToWorld = forceActivate;

	if (!shouldBeAddedToWorld)
	{
		ecs::EntityData* parentData = (entityData.parentId != Entity::GetInvalidId()) ? GetEntityData(entityData.parentId) : nullptr;
		shouldBeAddedToWorld = entityData.isEnabled && (nullptr != parentData && parentData->isActivated);
	}
	
	if (shouldBeAddedToWorld != entityData.isActivated)
	{
		entityData.isActivated = shouldBeAddedToWorld;

		RefreshComponentsActivation(entityData);
		RefreshChildrenActivation(entityData);
	}
}

void EntitiesCollection::RefreshComponentsActivation(EntityData& entityData)
{
	EntityComponentsCollection componentCollection(m_entityComponentsMapping, entityData.componentsDataOffset);
	for (ComponentHandle& componentHandle : componentCollection)
	{
		m_manager.RefreshComponentActivation(componentHandle, entityData.isEnabled, entityData.isActivated);
	}
}

void EntitiesCollection::RefreshChildrenActivation(EntityData& entityData)
{
	Entity entity(&entityData);
	for (Entity& child : entity.GetChildren())
	{
		RefreshActivation(*child.GetData());
	}
}

void EntitiesCollection::OnEntityDataDestroy(const EntityId entityId)
{
	// Find entity data location
	auto it = m_entityIdsMap.find(entityId);
	assert(it != m_entityIdsMap.end());

	uint32_t location = it->second;
	EntityData* entityData = m_entitiesData[location];

	// Destroy all owned components
	auto componentsCollection = EntityComponentsCollection(m_entityComponentsMapping, entityData->componentsDataOffset);
	std::vector<EntityComponentsCollection::iterator> collectionIterators;
	for (auto it = componentsCollection.begin(); it != componentsCollection.end(); ++it)
	{
		collectionIterators.push_back(it);
	}

	for (EntityComponentsCollection::iterator& it : collectionIterators)
	{
		m_manager.SetComponentEntityId(*it, Entity::GetInvalidId());
		m_manager.DestroyComponent(*it);
		*m_entityComponentsMapping[it.offset] = EntityComponentMapEntry();
	}

	// Replace the entity data with newly created one
	m_storageHoles.push_back(location);
	*entityData = EntityData();

	// Remove from entity id -> storage location mapping
	m_entityIdsMap.erase(it);

	// Invoke global entity destroy delegate
	if (nullptr != m_manager.m_entityDestroyDelegate)
	{
		std::invoke(*m_manager.m_entityDestroyDelegate, entityId);
	}
}

EntityComponentMapEntry& EntitiesCollection::CreateComponentMappingEntry(EntityData& entityData)
{
	// Make entry in components mapping
	bool placeForInsertionFound = false;
	std::size_t offset = entityData.componentsDataOffset;

	while (!placeForInsertionFound)
	{
		EntityComponentMapEntry* mappingEntry = m_entityComponentsMapping[offset];
		if (mappingEntry->nextItemPtr == Entity::GetInvalidId())
		{
			// This is the end of the list, can be used as insertion point
			placeForInsertionFound = true;
		}
		else
		{
			// Jump to next item in the list
			offset = mappingEntry->nextItemPtr;
		}
	}

	std::size_t mappingEntryId = offset;
	std::size_t newEntryId = mappingEntryId;

	if (m_entityComponentsMapping[mappingEntryId]->handle.IsValid())
	{
		auto creationResult = m_entityComponentsMapping.CreateItem();
		newEntryId = creationResult.first;
	}

	// Insert component id here
	EntityComponentMapEntry* currentEntry = m_entityComponentsMapping[mappingEntryId];
	EntityComponentMapEntry* newEntry = m_entityComponentsMapping[newEntryId];
	newEntry->nextItemPtr = Entity::GetInvalidId();

	if (currentEntry != newEntry)
	{
		// Link new entry to the list
		currentEntry->nextItemPtr = static_cast<uint32_t>(newEntryId);
	}

	return *newEntry;
}

EntityComponentMapEntry* EntitiesCollection::FindComponentMappingEntry(EntityData& entityData, const ComponentTypeId componentType)
{
	EntityComponentMapEntry* componentNode = m_entityComponentsMapping[entityData.componentsDataOffset];
	bool reachedEndOfList = false;

	while (!reachedEndOfList)
	{
		if (componentNode->handle.GetTypeId() == componentType)
		{
			return componentNode;
		}

		reachedEndOfList = (componentNode->nextItemPtr == Entity::GetInvalidId());

		if (!reachedEndOfList)
		{
			componentNode = m_entityComponentsMapping[componentNode->nextItemPtr];
		}
	}

	return nullptr;
}

void EntitiesCollection::RemoveComponentMappingEntry(EntityData& entityData, const ComponentTypeId componentType)
{
	std::size_t offset = entityData.componentsDataOffset;
	std::size_t prevOffset = Entity::GetInvalidId();

	while (offset != Entity::GetInvalidId())
	{
		EntityComponentMapEntry* mappingEntry = m_entityComponentsMapping[offset];

		if (mappingEntry->handle.GetTypeId() == componentType)
		{
			// Component type found, remove it
			entityData.componentsMask.reset(componentType);

			// Link the list
			if (prevOffset == Entity::GetInvalidId())
			{
				if (mappingEntry->nextItemPtr != Entity::GetInvalidId())
				{
					entityData.componentsDataOffset = mappingEntry->nextItemPtr;
				}
			}
			else
			{
				m_entityComponentsMapping[prevOffset]->nextItemPtr = mappingEntry->nextItemPtr;
			}

			return;
		}

		prevOffset = offset;
		offset = mappingEntry->nextItemPtr;
	}
}

ComponentsMapStorageType& EntitiesCollection::GetComponentsMapStorage()
{
	return m_entityComponentsMapping;
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
