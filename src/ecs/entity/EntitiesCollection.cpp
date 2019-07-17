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

Entity EntitiesCollection::GetEntityById(const EntityId id)
{
	auto it = m_entityIdsMap.find(id);
	if (it != m_entityIdsMap.end())
	{
		uint32_t location = it->second;
		return Entity(m_entitiesData[location]);
	}

	return Entity();
}

Entity EntitiesCollection::CreateEntity()
{
	auto entityCreationResult = m_entitiesData.CreateItem();
	const EntityId newEntityId = m_nextEntityId;
	const std::size_t storageLocation = entityCreationResult.first;
	++m_nextEntityId;

	EntityData* entityData = entityCreationResult.second;
	entityData->id = newEntityId;
	entityData->storageLocation = static_cast<EntityHandleIndex>(storageLocation);
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

void EntitiesCollection::DestroyEntity(const EntityId id)
{
	EntityData& entityData = *m_entitiesData[id];

	// Disable all owned components
	auto componentNode = m_entityComponentsMapping[entityData.componentsDataOffset];
	bool reachedEndOfList = false;

	while (!reachedEndOfList)
	{
		if (componentNode->handle.IsValid())
		{
			m_manager.DestroyComponent(componentNode->handle);
		}

		reachedEndOfList = (componentNode->nextItemPtr == Entity::GetInvalidId());

		if (!reachedEndOfList)
		{
			componentNode = m_entityComponentsMapping[componentNode->nextItemPtr];
		}
	}

	// Erase list items
	componentNode = m_entityComponentsMapping[entityData.componentsDataOffset];
	componentNode->handle = ComponentHandle(ComponentHandleInternal::GetInvalidTypeId(), nullptr);
	componentNode->nextItemPtr = Entity::GetInvalidId();

	// TODO: Performs logic consistency checks. Entity should be properly logically unloaded before actually destroying it...
}

EntitiesCollection::EntitiesStorageType& EntitiesCollection::GetEntitiesData()
{
	return m_entitiesData;
}

void EntitiesCollection::OnEntityEnabled(const EntityId entityId, const bool enabled)
{
	RefreshActivation(*m_entitiesData[entityId]);
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
		ecs::EntityData* parentData = (entityData.parentId != Entity::GetInvalidId()) ? m_entitiesData[entityData.parentId] : nullptr;
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

Entity EntitiesCollection::CloneEntity(const Entity& entity)
{
	Entity clone = CreateEntity();

	// Clone components
	for (const ecs::ComponentHandle& componentHandle : entity.GetComponents())
	{
		ecs::ComponentHandle componentCloneHandle = m_manager.CloneComponent(componentHandle);
		clone.AddComponent(componentCloneHandle);
	}

	// Clone children
	for (ecs::Entity& child : entity.GetChildren())
	{
		ecs::Entity childClone = child.Clone();
		clone.AddChild(childClone);
	}

	return clone;
}

void EntitiesCollection::OnEntityDataDestroy(const EntityId entityId)
{

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

} // namespace ecs
