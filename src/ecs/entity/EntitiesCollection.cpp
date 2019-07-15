#include "ecs/entity/EntitiesCollection.hpp"
#include "ecs/Manager.hpp"

namespace ecs
{

EntitiesCollection::EntitiesCollection(ecs::Manager& ecsManager)
	: m_manager(ecsManager)
	//, m_hierarchyManager(&ecsManager)
	, m_entitiesData(1024U)
	, m_entityComponentsMapping(1024U)
	, m_entityHierarchyData(1024U)
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
	entityData->orderInParent = Entity::GetInvalidHierarchyDepth();
	// Invalid hierarchy depth means that entity is not a part of any hierarchy currently
	entityData->hierarchyDepth = Entity::GetInvalidHierarchyDepth();

	{
		auto componentMappingCreationResult = m_entityComponentsMapping.CreateItem();
		auto mappingItem = componentMappingCreationResult.second;
		mappingItem->handle = ComponentHandle(ComponentHandleInternal::GetInvalidTypeId(), nullptr);
		mappingItem->nextItemPtr = Entity::GetInvalidId();

		entityData->componentsDataOffset = static_cast<uint32_t>(componentMappingCreationResult.first);
	}

	{
		auto hierarchyDataCreationResult = m_entityHierarchyData.CreateItem();
		auto hierarchyData = hierarchyDataCreationResult.second;
		hierarchyData->childId = Entity::GetInvalidId();
		hierarchyData->nextItemPtr = Entity::GetInvalidId();

		entityData->hierarchyDataOffset = static_cast<uint32_t>(hierarchyDataCreationResult.first);
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

		if (forceActivate)
		{
			RefreshHierarchyDepth(entityData, entityData.parentId, true);
		}

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

void EntitiesCollection::RefreshHierarchyDepth(EntityData& entityData, const EntityId newParentId, bool constructNewHierarchyTree)
{
	//static const HierarchyDepth k_invalidDepth = Entity::GetInvalidHierarchyDepth();

	//Entity* parent = (newParentId != Entity::GetInvalidId()) ? m_entitiesData[newParentId] : nullptr;

	//EntityId currentDepth = entity.hierarchyDepth;
	//EntityId newDepth;
	//
	//if (nullptr != parent)
	//{
	//	newDepth = (parent->hierarchyDepth != k_invalidDepth) ? parent->hierarchyDepth + 1U : k_invalidDepth;
	//}
	//else if (constructNewHierarchyTree)
	//{
	//	newDepth = 0U;
	//}
	//else
	//{
	//	// If have no parent, should reset hierarchy depth to detached state
	//	newDepth = Entity::GetInvalidHierarchyDepth();
	//}

	//if (newDepth != currentDepth)
	//{
	//	if (newDepth == Entity::GetInvalidHierarchyDepth())
	//	{
	//		m_hierarchyManager.RemoveEntity(entity.id);
	//		entity.hierarchyDepth = newDepth;
	//		entity.parentId = newParentId;
	//	}
	//	else if (currentDepth == Entity::GetInvalidHierarchyDepth())
	//	{
	//		entity.hierarchyDepth = newDepth;
	//		entity.parentId = newParentId;
	//		m_hierarchyManager.AddEntity(entity.id);
	//	}
	//	else
	//	{
	//		m_hierarchyManager.RemoveEntity(entity.id);
	//		entity.hierarchyDepth = newDepth;
	//		entity.parentId = newParentId;
	//		m_hierarchyManager.AddEntity(entity.id);
	//	}
	//}

	//if (newDepth != currentDepth)
	//{
	//	// Refresh children hierarchy depths
	//	for (auto& child : GetChildrenData(entity))
	//	{
	//		RefreshHierarchyDepth(child, entity.id, false);
	//	}
	//}
}

bool EntitiesCollection::CompareEntitiesInHierarchy(const Entity& lhs, const Entity& rhs) const
{
	//return m_hierarchyManager.CompareEntitiesInHierarchy(lhs, rhs);
	return lhs.GetId() < rhs.GetId();
}

//std::size_t EntitiesCollection::GetEntitiesCountInBranch(const EntityId& rootEntityId) const
//{
//	return m_hierarchyManager.GetEntitiesCountInBranch(rootEntityId);
//}
//
//std::size_t EntitiesCollection::GetActiveEntitiesCountInBranch(const EntityId& rootEntityId) const
//{
//	return m_hierarchyManager.GetActiveEntitiesCountInBranch(rootEntityId);
//}
//
//int EntitiesCollection::GetEntityHierarchyOffsetRelativeToEntity(const EntityId& entityId, const EntityId& pivotId) const
//{
//	return m_hierarchyManager.GetHierarchyOrderDiff(entityId, pivotId);
//}

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

EntityHierarchyData& EntitiesCollection::CreateEntityHierarchyDataEntry(EntityData& entityData)
{
	// Make entry in children mapping
	bool placeForInsertionFound = false;
	EntityHierarchyDataOffset offset = entityData.hierarchyDataOffset;

	while (!placeForInsertionFound)
	{
		EntityHierarchyData* hierarchyData = m_entityHierarchyData[offset];
		if (hierarchyData->nextItemPtr == Entity::GetInvalidId())
		{
			// This is the end of the list, can be used as insertion point
			placeForInsertionFound = true;
		}
		else
		{
			// Jump to next item in the list
			offset = hierarchyData->nextItemPtr;
		}
	}

	EntityHierarchyDataOffset hierarchyDataId = offset;
	EntityHierarchyDataOffset newEntryId = hierarchyDataId;

	if (m_entityHierarchyData[hierarchyDataId]->childId != Entity::GetInvalidId())
	{
		auto creationResult = m_entityHierarchyData.CreateItem();
		newEntryId = static_cast<EntityHierarchyDataOffset>(creationResult.first);
	}

	// Insert component id here
	EntityHierarchyData* currentEntry = m_entityHierarchyData[hierarchyDataId];
	EntityHierarchyData* newEntry = m_entityHierarchyData[newEntryId];
	newEntry->nextItemPtr = Entity::GetInvalidId();

	if (currentEntry != newEntry)
	{
		// Link new entry to the list
		currentEntry->nextItemPtr = static_cast<EntityHierarchyDataOffset>(newEntryId);
	}

	return *newEntry;
}

void EntitiesCollection::RemoveEntityHierarchyDataEntry(EntityData& entityData, Entity& child)
{
	if (!child.IsValid())
		return;

	// Find the child in the list
	bool itemFound = false;
	std::size_t offset = entityData.hierarchyDataOffset;
	std::size_t prevOffset = Entity::GetInvalidId();

	while (!itemFound)
	{
		EntityHierarchyData* hierarchyData = m_entityHierarchyData[offset];

		if (hierarchyData->childId == child.GetId())
		{
			itemFound = true;
		}
		else if (hierarchyData->nextItemPtr == Entity::GetInvalidId())
		{
			break;
		}
		else
		{
			prevOffset = offset;
			offset = hierarchyData->nextItemPtr;
		}
	}

	if (itemFound)
	{
		// Item found, remove it from the list preserving connectivity
		EntityHierarchyData* hierarchyData = m_entityHierarchyData[offset];

		if (prevOffset == Entity::GetInvalidId())
		{
			if (hierarchyData->nextItemPtr != Entity::GetInvalidId())
			{
				entityData.hierarchyDataOffset = hierarchyData->nextItemPtr;
			}
		}
		else
		{
			m_entityHierarchyData[prevOffset]->nextItemPtr = hierarchyData->nextItemPtr;
		}

		hierarchyData->childId = Entity::GetInvalidId();
		hierarchyData->nextItemPtr = Entity::GetInvalidId();

		child.GetData()->orderInParent = Entity::GetInvalidHierarchyDepth();
		--entityData.childrenCount;

		RefreshHierarchyDepth(*child.GetData(), Entity::GetInvalidId(), false);
		RefreshActivation(*child.GetData());
	}
}

EntityHierarchyDataStorageType& EntitiesCollection::GetEntityHierarchyData()
{
	return m_entityHierarchyData;
}

} // namespace ecs
