#include "EntitiesCollection.hpp"
#include "Manager.hpp"

namespace ecs
{

EntitiesCollection::EntitiesCollection(ecs::Manager& ecsManager)
	: m_ecsManager(ecsManager)
	, m_hierarchyManager(&ecsManager)
	, m_entities(1024U)
	, m_entityComponentsMapping(1024U)
	, m_entityHierarchyData(1024U)
{
	//Entity::s_collection = this;
}

Entity& EntitiesCollection::GetEntity(const EntityId id)
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
	entityData.entity.orderInParent = Entity::GetInvalidHierarchyDepth();
	entityData.entity.hierarchyDepth = 0U;

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

void EntitiesCollection::DestroyEntity(const EntityId id)
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

bool EntitiesCollection::HasComponent(const Entity& entity, const uint8_t componentType) const
{
	const int typeMask = 1 << componentType;
	bool hasComponent = entity.componentsMask & typeMask;

	return hasComponent;
}

void* EntitiesCollection::GetComponent(const Entity& entity, const uint8_t componentType) const
{
	if (HasComponent(entity, componentType))
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
	}

	return nullptr;
}

void* EntitiesCollection::GetComponent(const EntityId entityId, const uint8_t componentType) const
{
	const auto& entity = m_entities[entityId]->entity;
	return GetComponent(entity, componentType);
}

void* EntitiesCollection::GetComponent(const Entity& entity, const uint8_t componentType, ComponentHandle& handle) const
{
	const int typeMask = 1 << componentType;
	bool hasComponent = entity.componentsMask & typeMask;

	if (hasComponent)
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
	}

	handle = ComponentHandle(ComponentHandleInternal::GetInvalidTypeId(), nullptr);
	return nullptr;
}

void* EntitiesCollection::GetComponent(const EntityId entityId, const uint8_t componentType, ComponentHandle& handle) const
{
	const auto& entity = m_entities[entityId]->entity;
	return GetComponent(entity, componentType, handle);
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

	auto parentData = m_entities[entity.id];
	child.parentId = entity.id;
	child.orderInParent = parentData->childrenCount;
	child.hierarchyDepth = entity.hierarchyDepth + 1;
	++parentData->childrenCount;

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
		child.hierarchyDepth = 0U;
		child.orderInParent = Entity::GetInvalidHierarchyDepth();
		--m_entities[entity.id]->childrenCount;

		RefreshActivation(child);
	}
}

uint16_t EntitiesCollection::GetChildrenCount(const Entity& entity) const
{
	return GetChildrenCount(entity.id);
}

uint16_t EntitiesCollection::GetChildrenCount(EntityId entityId) const
{
	return m_entities[entityId]->childrenCount;
}

Entity* EntitiesCollection::GetParent(const Entity& entity) const
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

Entity* EntitiesCollection::GetParent(const ComponentHandle& handle) const
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

Entity* EntitiesCollection::GetParent(const EntityId entityId) const
{
	assert(entityId != Entity::GetInvalidId());
	return GetParent(m_entities[entityId]->entity);
}

void EntitiesCollection::ClearChildren(Entity& entity)
{
	for (auto& child : GetChildrenData(entity))
	{
		child.parentId = Entity::GetInvalidId();

		RefreshActivation(child);
	}

	auto entityData = m_entities[entity.id];
	entityData->childrenCount = 0;

	auto entityHierarchyEntry = m_entityHierarchyData[entity.hierarchyDataOffset];
	entityHierarchyEntry->childId = Entity::GetInvalidId();
	entityHierarchyEntry->nextItemPtr = Entity::GetInvalidId();
}

EntitiesCollection::ChildrenData EntitiesCollection::GetChildrenData(const Entity& entity)
{
	return GetChildrenData(entity.id);
}

EntitiesCollection::ChildrenData EntitiesCollection::GetChildrenData(const EntityId entityId)
{
	auto offsetBegin = m_entities[entityId]->entity.hierarchyDataOffset;

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

EntitiesCollection::ComponentsData EntitiesCollection::GetComponentsData(const Entity& entity)
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

void EntitiesCollection::ActivateEntity(const std::size_t entityId, const bool activate)
{
	auto entityData = m_entities[entityId];

	if (activate != entityData->isActivated)
	{
		RefreshActivation(entityData->entity, activate);
	}
}

bool EntitiesCollection::IsEntityEnabled(const std::size_t entityId) const
{
	return m_entities[entityId]->isEnabled;
}

bool EntitiesCollection::IsEntityActivated(const std::size_t entityId) const
{
	return m_entities[entityId]->isActivated;
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

Entity& EntitiesCollection::CloneEntity(const Entity& entity)
{
	auto& clone = CreateEntity();

	// Clone components
	for (auto& componentHandle : GetComponentsData(entity))
	{
		auto componentCloneHandle = m_ecsManager.CloneComponent(componentHandle);
		AddComponent(clone, componentCloneHandle);
	}

	// Clone children
	for (auto& child : GetChildrenData(entity))
	{
		auto& childClone = CloneEntity(child);
		AddChild(clone, childClone);
	}

	return clone;
}

bool EntitiesCollection::CompareEntitiesInHierarchy(const Entity& lhs, const Entity& rhs) const
{
	return m_hierarchyManager.CompareEntitiesInHierarchy(lhs, rhs);
}

std::size_t EntitiesCollection::GetEntitiesCountInBranch(const EntityId& rootEntityId) const
{
	return m_hierarchyManager.GetEntitiesCountInBranch(rootEntityId);
}

std::size_t EntitiesCollection::GetActiveEntitiesCountInBranch(const EntityId& rootEntityId) const
{
	return m_hierarchyManager.GetActiveEntitiesCountInBranch(rootEntityId);
}

std::size_t EntitiesCollection::GetEntityHierarchyOffsetRelativeToEntity(const EntityId& entityId, const EntityId& pivotId) const
{
	// TODO: Redesign this to use hierarchy manager
	return static_cast<std::size_t>(static_cast<int>(entityId) - static_cast<int>(pivotId));
}

} // namespace ecs
