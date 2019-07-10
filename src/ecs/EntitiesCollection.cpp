#include "EntitiesCollection.hpp"
#include "Manager.hpp"

namespace ecs
{

EntitiesCollection::EntitiesCollection(ecs::Manager& ecsManager)
	: m_ecsManager(ecsManager)
	, m_hierarchyManager(&ecsManager)
	, m_entitiesData(1024U)
	, m_entityComponentsMapping(1024U)
	, m_entityHierarchyData(1024U)
	, m_handles(1024U)
{}

Entity EntitiesCollection::GetEntityById(const EntityId id)
{
	return Entity(m_entitiesData[id]);
}

EntityHandle EntitiesCollection::CreateEntity()
{
	auto entityCreationResult = m_entitiesData.CreateItem();
	EntityHandle::HandleIndex* handleIndex = m_handles.CreateItem().second;
	EntityId newEntityId = static_cast<EntityId>(entityCreationResult.first);

	EntityData& entityData = *entityCreationResult.second;
	entityData.id = newEntityId;
	*handleIndex = entityData.id; // Write entity id to handle value
	entityData.parentId = Entity::GetInvalidId();
	entityData.orderInParent = Entity::GetInvalidHierarchyDepth();
	// Invalid hierarchy depth means that entity is not a part of any hierarchy currently
	entityData.hierarchyDepth = Entity::GetInvalidHierarchyDepth();

	{
		auto componentMappingCreationResult = m_entityComponentsMapping.CreateItem();
		auto mappingItem = componentMappingCreationResult.second;
		mappingItem->handle = ComponentHandle(ComponentHandleInternal::GetInvalidTypeId(), nullptr);
		mappingItem->nextItemPtr = Entity::GetInvalidId();

		entityData.componentsDataOffset = static_cast<uint32_t>(componentMappingCreationResult.first);
	}

	{
		auto hierarchyDataCreationResult = m_entityHierarchyData.CreateItem();
		auto hierarchyData = hierarchyDataCreationResult.second;
		hierarchyData->childId = Entity::GetInvalidId();
		hierarchyData->nextItemPtr = Entity::GetInvalidId();

		entityData.hierarchyDataOffset = static_cast<uint32_t>(hierarchyDataCreationResult.first);
	}

	return EntityHandle(handleIndex);
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
			m_ecsManager.DestroyComponent(componentNode->handle);
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

void EntitiesCollection::AddComponent(EntityData& entityData, const ComponentHandle& handle)
{
	if (!handle.IsValid())
		return;

	entityData.componentsMask.set(handle.GetTypeIndex());
	m_ecsManager.SetComponentEntityId(handle, entityData.id);

	// Make entry in components mapping
	bool placeForInsertionFound = false;
	std::size_t offset = entityData.componentsDataOffset;
	while (!placeForInsertionFound)
	{
		const auto& mappingEntry = m_entityComponentsMapping[offset];
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

	auto mappingEntryId = offset;
	auto newEntryId = mappingEntryId;

	if (m_entityComponentsMapping[mappingEntryId]->handle.IsValid())
	{
		auto creationResult = m_entityComponentsMapping.CreateItem();
		newEntryId = creationResult.first;
	}

	// Insert component id here
	auto currentEntry = m_entityComponentsMapping[mappingEntryId];
	auto newEntry = m_entityComponentsMapping[newEntryId];
	newEntry->handle = handle;
	newEntry->nextItemPtr = Entity::GetInvalidId();

	if (currentEntry != newEntry)
	{
		// Link new entry to the list
		currentEntry->nextItemPtr = static_cast<uint32_t>(newEntryId);
	}

	m_ecsManager.RefreshComponentActivation(handle, entityData.isEnabled, entityData.isActivated);
}

void EntitiesCollection::RemoveComponent(EntityData& entityData, const ComponentHandle& handle)
{
	if (!handle.IsValid())
		return;

	auto componentType = handle.GetTypeIndex();

	if (HasComponent(entityData, componentType))
	{
		std::size_t offset = entityData.componentsDataOffset;
		std::size_t prevOffset = Entity::GetInvalidId();
		while (offset != Entity::GetInvalidId())
		{
			auto componentNode = m_entityComponentsMapping[offset];

			if (componentNode->handle.GetTypeIndex() == componentType)
			{
				// Component type found, remove it
				entityData.componentsMask.reset(componentType);
				m_ecsManager.SetComponentEntityId(handle, Entity::GetInvalidId());

				// Link the list
				if (prevOffset == Entity::GetInvalidId())
				{
					if (componentNode->nextItemPtr != Entity::GetInvalidId())
					{
						entityData.componentsDataOffset = componentNode->nextItemPtr;
					}
				}
				else
				{
					m_entityComponentsMapping[prevOffset]->nextItemPtr = componentNode->nextItemPtr;
				}

				return;
			}

			prevOffset = offset;
			offset = componentNode->nextItemPtr;
		}
	}
}

bool EntitiesCollection::HasComponent(const EntityData& entityData, const ComponentTypeId componentType) const
{
	return entityData.componentsMask.test(componentType);
}

ComponentHandle EntitiesCollection::GetComponentHandle(const EntityData& entityData, const ComponentTypeId componentType) const
{
	if (HasComponent(entityData, componentType))
	{
		auto componentNode = m_entityComponentsMapping[entityData.componentsDataOffset];
		bool reachedEndOfList = false;

		while (!reachedEndOfList)
		{
			if (componentNode->handle.GetTypeIndex() == componentType)
			{
				// Component type found, return it
				return componentNode->handle;
			}

			reachedEndOfList = (componentNode->nextItemPtr == Entity::GetInvalidId());

			if (!reachedEndOfList)
			{
				componentNode = m_entityComponentsMapping[componentNode->nextItemPtr];
			}
		}
	}

	return ComponentHandle();
}

ComponentTypeId EntitiesCollection::GetComponentTypeIdByTypeIndex(const std::type_index& typeIndex) const
{
	return m_ecsManager.GetComponentTypeIdByIndex(typeIndex);
}

void EntitiesCollection::AddChild(Entity& entity, Entity& child)
{
	//// Make entry in children mapping
	//bool placeForInsertionFound = false;
	//std::size_t offset = entity.hierarchyDataOffset;
	//while (!placeForInsertionFound)
	//{
	//	auto hierarchyData = m_entityHierarchyData[offset];
	//	if (hierarchyData->nextItemPtr == Entity::GetInvalidId())
	//	{
	//		// This is the end of the list, can be used as insertion point
	//		placeForInsertionFound = true;
	//	}
	//	else
	//	{
	//		// Jump to next item in the list
	//		offset = hierarchyData->nextItemPtr;
	//	}
	//}

	//auto hierarchyDataId = offset;
	//auto newEntryId = hierarchyDataId;

	//if (m_entityHierarchyData[hierarchyDataId]->childId != Entity::GetInvalidId())
	//{
	//	auto creationResult = m_entityHierarchyData.CreateItem();
	//	newEntryId = creationResult.first;
	//}

	//// Insert component id here
	//auto currentEntry = m_entityHierarchyData[hierarchyDataId];
	//auto newEntry = m_entityHierarchyData[newEntryId];
	//newEntry->childId = child.id;
	//newEntry->nextItemPtr = Entity::GetInvalidId();

	//if (currentEntry != newEntry)
	//{
	//	// Link new entry to the list
	//	currentEntry->nextItemPtr = static_cast<uint32_t>(newEntryId);
	//}

	//auto parentData = m_entities[entity.id];
	////child.parentId = entity.id;
	//child.orderInParent = parentData->childrenCount;
	//++parentData->childrenCount;

	//RefreshHierarchyDepth(child.id, entity.id, false);
	//RefreshActivation(child);
}

void EntitiesCollection::RemoveChild(Entity& entity, Entity& child)
{
	//assert(child.id != Entity::GetInvalidId());

	//// Find the child in the list
	//bool itemFound = false;
	//std::size_t offset = entity.hierarchyDataOffset;
	//std::size_t prevOffset = Entity::GetInvalidId();
	//while (!itemFound)
	//{
	//	auto hierarchyData = m_entityHierarchyData[offset];

	//	if (hierarchyData->childId == child.id)
	//	{
	//		itemFound = true;
	//	}
	//	else if (hierarchyData->nextItemPtr == Entity::GetInvalidId())
	//	{
	//		break;
	//	}
	//	else
	//	{
	//		prevOffset = offset;
	//		offset = hierarchyData->nextItemPtr;
	//	}
	//}

	//if (itemFound)
	//{
	//	// Item found, remove it from the list preserving connectivity
	//	auto hierarchyData = m_entityHierarchyData[offset];

	//	if (prevOffset == Entity::GetInvalidId())
	//	{
	//		if (hierarchyData->nextItemPtr != Entity::GetInvalidId())
	//		{
	//			entity.hierarchyDataOffset = hierarchyData->nextItemPtr;
	//		}
	//	}
	//	else
	//	{
	//		m_entityHierarchyData[prevOffset]->nextItemPtr = hierarchyData->nextItemPtr;
	//	}

	//	hierarchyData->childId = Entity::GetInvalidId();
	//	hierarchyData->nextItemPtr = Entity::GetInvalidId();

	//	child.orderInParent = Entity::GetInvalidHierarchyDepth();
	//	--m_entities[entity.id]->childrenCount;

	//	RefreshHierarchyDepth(child.id, Entity::GetInvalidId(), false);
	//	RefreshActivation(child);
	//}
}

EntityId EntitiesCollection::GetChildByIdx(Entity& entity, const std::size_t idx) const
{
	//auto entityData = m_entities[entity.id];
	//if (idx >= entityData->childrenCount)
	//	return Entity::GetInvalidId();

	//// Find the child in the list
	//std::size_t currentIdx = 0U;
	//std::size_t offset = entity.hierarchyDataOffset;
	//std::size_t prevOffset = Entity::GetInvalidId();

	//while (offset != Entity::GetInvalidId())
	//{
	//	auto hierarchyData = m_entityHierarchyData[offset];

	//	if (currentIdx == idx)
	//	{
	//		return hierarchyData->childId;
	//	}

	//	prevOffset = offset;
	//	offset = hierarchyData->nextItemPtr;
	//	++currentIdx;
	//}

	return Entity::GetInvalidId();
}

void EntitiesCollection::ClearChildren(Entity& entity, bool destroyChildren)
{
	//for (auto& child : GetChildrenData(entity))
	//{
	//	child.orderInParent = Entity::GetInvalidHierarchyDepth();

	//	RefreshHierarchyDepth(child.id, Entity::GetInvalidId(), false);
	//	RefreshActivation(child);

	//	if (destroyChildren)
	//	{
	//		DestroyEntity(child.id);
	//	}
	//}

	//auto entityData = m_entities[entity.id];
	//entityData->childrenCount = 0;

	//auto entityHierarchyEntry = m_entityHierarchyData[entity.hierarchyDataOffset];
	//entityHierarchyEntry->childId = Entity::GetInvalidId();
	//entityHierarchyEntry->nextItemPtr = Entity::GetInvalidId();
}

EntitiesCollection::ChildrenData EntitiesCollection::GetChildrenData(EntityData& entityData)
{
	auto offsetBegin = entityData.hierarchyDataOffset;

	std::size_t offsetEnd = offsetBegin;
	bool endFound = false;
	while (!endFound)
	{
		auto hierarchyData = m_entityHierarchyData[offsetEnd];
		if (hierarchyData->childId == Entity::GetInvalidId())
		{
			endFound = true;
		}
		else
		{
			offsetEnd = hierarchyData->nextItemPtr;

			if (offsetEnd == Entity::GetInvalidId())
			{
				endFound = true;
			}
		}
	}

	return EntitiesCollection::ChildrenData(this, m_entityHierarchyData, offsetBegin, offsetEnd);
}

EntitiesCollection::ComponentsData EntitiesCollection::GetComponentsData(EntityData& entityData)
{
	auto offsetBegin = entityData.componentsDataOffset;
	return EntitiesCollection::ComponentsData(m_entityComponentsMapping, offsetBegin);
}

EntitiesCollection::EntitiesStorageType& EntitiesCollection::GetEntitiesData()
{
	return m_entitiesData;
}

void EntitiesCollection::OnEntityEnabled(const EntityId entityId, const bool enabled)
{
	RefreshActivation(*m_entitiesData[entityId]);
}

void EntitiesCollection::ActivateEntity(EntityData& entityData, const bool activate)
{
	if (activate != entityData.isActivated)
	{
		RefreshActivation(entityData, activate);
	}
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
	for (ComponentHandle& componentHandle : GetComponentsData(entityData))
	{
		m_ecsManager.RefreshComponentActivation(componentHandle, entityData.isEnabled, entityData.isActivated);
	}
}

void EntitiesCollection::RefreshChildrenActivation(EntityData& entityData)
{
	for (auto& childData : GetChildrenData(entityData))
	{
		//RefreshActivation(childData);
	}
}

Entity EntitiesCollection::CloneEntity(const Entity& entity)
{
	EntityHandle clone = CreateEntity();

	//// Clone components
	//for (auto& componentHandle : GetComponentsData(entity))
	//{
	//	auto componentCloneHandle = m_ecsManager.CloneComponent(componentHandle);
	//	AddComponent(clone, componentCloneHandle);
	//}

	//// Clone children
	//for (auto& child : GetChildrenData(entity))
	//{
	//	auto& childClone = CloneEntity(child);
	//	AddChild(clone, childClone);
	//}

	return clone;
}

void EntitiesCollection::RefreshHierarchyDepth(EntityData& entityData, const EntityId newParentId, bool constructNewHierarchyTree)
{
	static const HierarchyDepth k_invalidDepth = Entity::GetInvalidHierarchyDepth();

	EntityData* parent = (newParentId != Entity::GetInvalidId()) ? m_entitiesData[newParentId] : nullptr;

	EntityId currentDepth = entityData.hierarchyDepth;
	EntityId newDepth;
	
	if (nullptr != parent)
	{
		newDepth = (parent->hierarchyDepth != k_invalidDepth) ? parent->hierarchyDepth + 1U : k_invalidDepth;
	}
	else if (constructNewHierarchyTree)
	{
		newDepth = 0U;
	}
	else
	{
		// If have no parent, should reset hierarchy depth to detached state
		newDepth = Entity::GetInvalidHierarchyDepth();
	}

	if (newDepth != currentDepth)
	{
		if (newDepth == Entity::GetInvalidHierarchyDepth())
		{
			m_hierarchyManager.RemoveEntity(entityData.id);
			entityData.hierarchyDepth = newDepth;
			entityData.parentId = newParentId;
		}
		else if (currentDepth == Entity::GetInvalidHierarchyDepth())
		{
			entityData.hierarchyDepth = newDepth;
			entityData.parentId = newParentId;
			m_hierarchyManager.AddEntity(entityData.id);
		}
		else
		{
			m_hierarchyManager.RemoveEntity(entityData.id);
			entityData.hierarchyDepth = newDepth;
			entityData.parentId = newParentId;
			m_hierarchyManager.AddEntity(entityData.id);
		}
	}

	if (newDepth != currentDepth)
	{
		// Refresh children hierarchy depths
		for (auto& child : GetChildrenData(entityData))
		{
			//RefreshHierarchyDepth(child.id, entityData.id, false);
		}
	}
}

bool EntitiesCollection::CompareEntitiesInHierarchy(const Entity& lhs, const Entity& rhs) const
{
	//return m_hierarchyManager.CompareEntitiesInHierarchy(lhs, rhs);
	return lhs.GetId() < rhs.GetId();
}

std::size_t EntitiesCollection::GetEntitiesCountInBranch(const EntityId& rootEntityId) const
{
	return m_hierarchyManager.GetEntitiesCountInBranch(rootEntityId);
}

std::size_t EntitiesCollection::GetActiveEntitiesCountInBranch(const EntityId& rootEntityId) const
{
	return m_hierarchyManager.GetActiveEntitiesCountInBranch(rootEntityId);
}

int EntitiesCollection::GetEntityHierarchyOffsetRelativeToEntity(const EntityId& entityId, const EntityId& pivotId) const
{
	return m_hierarchyManager.GetHierarchyOrderDiff(entityId, pivotId);
}

void EntitiesCollection::OnEntityDataDestroy(const EntityId entityId)
{

}

EntityData* EntitiesCollection::GetEntityData(const EntityId id)
{
	return m_entitiesData[id];
}

} // namespace ecs
