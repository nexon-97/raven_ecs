#include "EntitiesCollection.hpp"
#include "Manager.hpp"

namespace ecs
{

EntitiesCollection::EntitiesCollection(ecs::Manager& ecsManager)
	: m_ecsManager(ecsManager)
	, m_entities(1024U)
	, m_entityComponentsMapping(1024U)
	, m_entityHierarchyData(1024U)
{
	//Entity::s_collection = this;
}

Entity& EntitiesCollection::GetEntity(const uint32_t id)
{
	return m_entities[id]->entity;
}

Entity& EntitiesCollection::CreateEntity()
{
	auto entityCreationResult = m_entities.CreateItem();
	uint32_t newEntityId = static_cast<uint32_t>(entityCreationResult.first);

	auto& entityData = *entityCreationResult.second;
	entityData.entity.id = newEntityId;
	entityData.entity.parentId = Entity::GetInvalidId();

	{
		auto componentMappingCreationResult = m_entityComponentsMapping.CreateItem();
		auto mappingItem = componentMappingCreationResult.second;
		mappingItem->handle = ComponentHandle(ComponentHandleInternal::GetInvalidTypeId(), nullptr);
		mappingItem->nextItemPtr = Entity::GetInvalidId();

		entityData.entity.componentsDataOffset = static_cast<uint32_t>(componentMappingCreationResult.first);
	}

	{
		auto hierarchyDataCreationResult = m_entityHierarchyData.CreateItem();
		auto hierarchyData = hierarchyDataCreationResult.second;
		hierarchyData->childId = Entity::GetInvalidId();
		hierarchyData->nextItemPtr = Entity::GetInvalidId();

		entityData.entity.hierarchyDataOffset = static_cast<uint32_t>(hierarchyDataCreationResult.first);
	}

	return entityData.entity;
}

void EntitiesCollection::DestroyEntity(const uint32_t id)
{
	auto& entityData = *m_entities[id];
	auto& entity = entityData.entity;

	// Disable all owned components
	auto componentNode = m_entityComponentsMapping[entity.componentsDataOffset];
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
	componentNode = m_entityComponentsMapping[entity.componentsDataOffset];
	componentNode->handle = ComponentHandle(ComponentHandleInternal::GetInvalidTypeId(), nullptr);
	componentNode->nextItemPtr = Entity::GetInvalidId();

	// Mark entity as deleted
	entity.id = Entity::GetInvalidId();
	entity.componentsMask = 0;
}

void EntitiesCollection::AddComponent(Entity& entity, const ComponentHandle& handle)
{
	const int typeMask = 1 << handle.GetTypeIndex();
	entity.componentsMask |= typeMask;
	m_ecsManager.SetComponentEntityId(handle, entity.id);

	// Make entry in components mapping
	bool placeForInsertionFound = false;
	std::size_t offset = entity.componentsDataOffset;
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

	const auto& entityData = m_entities[entity.id];
	m_ecsManager.RefreshComponentActivation(handle, entityData->isEnabled, entityData->isActivated);
}

void EntitiesCollection::RemoveComponent(Entity& entity, const ComponentHandle& handle)
{
	if (HasComponent(entity, handle.GetTypeIndex()))
	{
		const int typeMask = 1 << handle.GetTypeIndex();
		entity.componentsMask ^= typeMask;

		m_ecsManager.SetComponentEntityId(handle, Entity::GetInvalidId());
		const auto& entityData = m_entities[entity.id];
		m_ecsManager.RefreshComponentActivation(handle, entityData->isEnabled, entityData->isActivated);
	}
}

bool EntitiesCollection::HasComponent(Entity& entity, const uint8_t componentType)
{
	const int typeMask = 1 << componentType;
	bool hasComponent = entity.componentsMask & typeMask;

	return hasComponent;
}

void* EntitiesCollection::GetComponent(Entity& entity, const uint8_t componentType) const
{
	auto componentNode = m_entityComponentsMapping[entity.componentsDataOffset];
	bool reachedEndOfList = false;

	while (!reachedEndOfList)
	{
		if (componentNode->handle.GetTypeIndex() == componentType)
		{
			// Component type found, return it
			return m_ecsManager.GetComponent<void>(componentNode->handle);
		}

		reachedEndOfList = (componentNode->nextItemPtr == Entity::GetInvalidId());

		if (!reachedEndOfList)
		{
			componentNode = m_entityComponentsMapping[componentNode->nextItemPtr];
		}
	}

	return nullptr;
}

void* EntitiesCollection::GetComponent(Entity& entity, const uint8_t componentType, ComponentHandle& handle) const
{
	auto componentNode = m_entityComponentsMapping[entity.componentsDataOffset];
	bool reachedEndOfList = false;

	while (!reachedEndOfList)
	{
		if (componentNode->handle.GetTypeIndex() == componentType)
		{
			// Component type found, return it
			handle = componentNode->handle;
			return m_ecsManager.GetComponent<void>(handle);
		}

		reachedEndOfList = (componentNode->nextItemPtr == Entity::GetInvalidId());

		if (!reachedEndOfList)
		{
			componentNode = m_entityComponentsMapping[componentNode->nextItemPtr];
		}
	}

	handle = ComponentHandle(ComponentHandleInternal::GetInvalidTypeId(), nullptr);
	return nullptr;
}

uint8_t EntitiesCollection::GetComponentTypeIdByTypeIndex(const std::type_index& typeIndex) const
{
	return m_ecsManager.GetComponentTypeIdByIndex(typeIndex);
}

void EntitiesCollection::AddChild(Entity& entity, Entity& child)
{
	// Make entry in children mapping
	bool placeForInsertionFound = false;
	std::size_t offset = entity.hierarchyDataOffset;
	while (!placeForInsertionFound)
	{
		auto hierarchyData = m_entityHierarchyData[offset];
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

	auto hierarchyDataId = offset;
	auto newEntryId = hierarchyDataId;

	if (m_entityHierarchyData[hierarchyDataId]->childId != Entity::GetInvalidId())
	{
		auto creationResult = m_entityHierarchyData.CreateItem();
		newEntryId = creationResult.first;
	}

	// Insert component id here
	auto currentEntry = m_entityHierarchyData[hierarchyDataId];
	auto newEntry = m_entityHierarchyData[newEntryId];
	newEntry->childId = child.id;
	newEntry->nextItemPtr = Entity::GetInvalidId();

	if (currentEntry != newEntry)
	{
		// Link new entry to the list
		currentEntry->nextItemPtr = static_cast<uint32_t>(newEntryId);
	}

	++m_entities[entity.id]->childrenCount;
	child.parentId = entity.id;

	RefreshActivation(child);
}

void EntitiesCollection::RemoveChild(Entity& entity, Entity& child)
{
	assert(child.id != Entity::GetInvalidId());

	// Find the child in the list
	bool itemFound = false;
	std::size_t offset = entity.hierarchyDataOffset;
	std::size_t prevOffset = Entity::GetInvalidId();
	while (!itemFound)
	{
		auto hierarchyData = m_entityHierarchyData[offset];

		if (hierarchyData->childId == child.id)
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
		auto hierarchyData = m_entityHierarchyData[offset];

		if (prevOffset == Entity::GetInvalidId())
		{
			entity.hierarchyDataOffset = hierarchyData->nextItemPtr;
		}
		else
		{
			m_entityHierarchyData[prevOffset]->nextItemPtr = hierarchyData->nextItemPtr;
		}

		hierarchyData->childId = Entity::GetInvalidId();
		hierarchyData->nextItemPtr = Entity::GetInvalidId();

		child.parentId = Entity::GetInvalidId();
		--m_entities[entity.id]->childrenCount;

		RefreshActivation(child);
	}
}

uint16_t EntitiesCollection::GetChildrenCount(Entity& entity) const
{
	return m_entities[entity.id]->childrenCount;
}

Entity* EntitiesCollection::GetParent(Entity& entity)
{
	if (entity.parentId != Entity::GetInvalidId())
	{
		return &m_entities[entity.parentId]->entity;
	}
	else
	{
		return nullptr;
	}
}

Entity* EntitiesCollection::GetParent(const ComponentHandle& handle)
{
	auto entityId = handle.GetEntityId();
	auto entityData = m_entities[entityId];
	auto parentId = entityData->entity.parentId;

	if (parentId != Entity::GetInvalidId())
	{
		return &m_entities[parentId]->entity;
	}
	else
	{
		return nullptr;
	}
}

EntitiesCollection::ChildrenData EntitiesCollection::GetChildrenData(Entity& entity)
{
	auto offsetBegin = m_entities[entity.id]->entity.hierarchyDataOffset;

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

EntitiesCollection::ComponentsData EntitiesCollection::GetComponentsData(Entity& entity)
{
	auto offsetBegin = m_entities[entity.id]->entity.componentsDataOffset;
	return EntitiesCollection::ComponentsData(m_entityComponentsMapping, offsetBegin);
}

EntitiesCollection::EntitiesStorageType& EntitiesCollection::GetEntitiesData()
{
	return m_entities;
}

void EntitiesCollection::SetEntityEnabled(Entity& entity, const bool enabled)
{
	auto entityData = m_entities[entity.id];

	if (entityData->isEnabled != enabled)
	{
		for (auto& componentHandle : GetComponentsData(entity))
		{
			m_ecsManager.SetComponentEnabled(componentHandle, enabled);
		}

		entityData->isEnabled = enabled;

		RefreshActivation(entity);
	}
}

void EntitiesCollection::ActivateEntity(Entity& entity, const bool activate)
{
	auto entityData = m_entities[entity.id];

	if (activate != entityData->isActivated)
	{
		RefreshActivation(entity, activate);
	}
}

void EntitiesCollection::RefreshActivation(Entity& entity, bool forceActivate)
{
	auto entityData = m_entities[entity.id];
	bool shouldBeAddedToWorld = forceActivate;

	if (!shouldBeAddedToWorld)
	{
		auto parentData = (entity.parentId != Entity::GetInvalidId()) ? m_entities[entity.parentId] : nullptr;
		shouldBeAddedToWorld = entityData->isEnabled && (nullptr != parentData && parentData->isActivated);
	}
	
	if (shouldBeAddedToWorld != entityData->isActivated)
	{
		entityData->isActivated = shouldBeAddedToWorld;
		RefreshComponentsActivation(entity);
		RefreshChildrenActivation(entity);
	}
}

void EntitiesCollection::RefreshComponentsActivation(Entity& entity)
{
	const auto& entityData = m_entities[entity.id];
	for (auto& componentHandle : GetComponentsData(entity))
	{
		m_ecsManager.RefreshComponentActivation(componentHandle, entityData->isEnabled, entityData->isActivated);
	}
}

void EntitiesCollection::RefreshChildrenActivation(Entity& entity)
{
	for (auto& childData : GetChildrenData(entity))
	{
		RefreshActivation(childData);
	}
}

} // namespace ecs
