#include "EntitiesCollection.hpp"
#include "Manager.hpp"

// TODO
// * Make proper entities destruction, mark all owned entities as disabled
// * Fix components iterator logic in order to skip unowned/disabled components

namespace ecs
{

const uint32_t Entity::k_invalidId = static_cast<uint32_t>(-1);

EntitiesCollection::EntitiesCollection(ecs::Manager& ecsManager)
	: m_ecsManager(ecsManager)
{
	// [TODO] Remove this hack later
	m_entities.reserve(1024);

	Entity::s_collection = this;
}

Entity& EntitiesCollection::GetEntity(const uint32_t id)
{
	return m_entities[id].entity;
}

Entity& EntitiesCollection::CreateEntity()
{
	uint32_t newEntityId = static_cast<uint32_t>(m_entities.size());
	m_entities.emplace_back();

	auto& entityData = m_entities.back();
	entityData.isAlive = true;
	entityData.entity.id = newEntityId;
	entityData.entity.componentsDataOffset = static_cast<uint32_t>(m_entityComponentsMapping.size());
	entityData.entity.hierarchyDataOffset = static_cast<uint32_t>(m_entityHierarchyData.size());
	entityData.entity.parentId = Entity::k_invalidId;

	m_entityComponentsMapping.emplace_back();
	auto& mappingItem = m_entityComponentsMapping.back();
	mappingItem.handle = ComponentHandle(ComponentHandleInternal::k_invalidTypeId, nullptr);
	mappingItem.nextItemPtr = Entity::k_invalidId;

	m_entityHierarchyData.emplace_back();
	auto& hierarchyData = m_entityHierarchyData.back();
	hierarchyData.childId = Entity::k_invalidId;
	hierarchyData.nextItemPtr = Entity::k_invalidId;

	return entityData.entity;
}

void EntitiesCollection::DestroyEntity(const uint32_t id)
{
	auto& entityData = m_entities[id];
	auto& entity = entityData.entity;

	// Disable all owned components
	auto& componentNode = m_entityComponentsMapping[entity.componentsDataOffset];
	bool reachedEndOfList = false;

	while (!reachedEndOfList)
	{
		if (componentNode.handle.IsValid())
		{
			m_ecsManager.DestroyComponent(componentNode.handle);
		}

		reachedEndOfList = (componentNode.nextItemPtr == Entity::k_invalidId);

		if (!reachedEndOfList)
		{
			componentNode = m_entityComponentsMapping[componentNode.nextItemPtr];
		}
	}

	// Erase list items
	componentNode = m_entityComponentsMapping[entity.componentsDataOffset];
	componentNode.handle = ComponentHandle(ComponentHandleInternal::k_invalidTypeId, nullptr);
	componentNode.nextItemPtr = Entity::k_invalidId;

	// Mark entity as deleted
	entity.id = Entity::k_invalidId;
	entity.componentsMask = 0;
	entityData.isAlive = false;
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
		if (mappingEntry.nextItemPtr == Entity::k_invalidId)
		{
			// This is the end of the list, can be used as insertion point
			placeForInsertionFound = true;
		}
		else
		{
			// Jump to next item in the list
			offset = mappingEntry.nextItemPtr;
		}
	}

	auto mappingEntryId = offset;
	auto newEntryId = mappingEntryId;

	if (m_entityComponentsMapping[mappingEntryId].handle.IsValid())
	{
		m_entityComponentsMapping.emplace_back();
		newEntryId = m_entityComponentsMapping.size() - 1;
	}

	// Insert component id here
	auto currentEntry = &m_entityComponentsMapping[mappingEntryId];
	auto newEntry = &m_entityComponentsMapping[newEntryId];
	newEntry->handle = handle;
	newEntry->nextItemPtr = Entity::k_invalidId;

	if (currentEntry != newEntry)
	{
		// Link new entry to the list
		currentEntry->nextItemPtr = static_cast<uint32_t>(newEntryId);
	}
}

void EntitiesCollection::RemoveComponent(Entity& entity, const ComponentHandle& handle)
{
	if (HasComponent(entity, handle.GetTypeIndex()))
	{
		const int typeMask = 1 << handle.GetTypeIndex();
		entity.componentsMask ^= typeMask;

		m_ecsManager.SetComponentEntityId(handle, Entity::k_invalidId);
	}
}

bool EntitiesCollection::HasComponent(Entity& entity, const uint8_t componentType)
{
	const int typeMask = 1 << componentType;
	bool hasComponent = entity.componentsMask & typeMask;

	return hasComponent;
}

void* EntitiesCollection::GetComponent(Entity& entity, const uint8_t componentType)
{
	auto& componentNode = m_entityComponentsMapping[entity.componentsDataOffset];
	bool reachedEndOfList = false;

	while (!reachedEndOfList)
	{
		if (componentNode.handle.GetTypeIndex() == componentType)
		{
			// Component type found, return it
			return m_ecsManager.GetComponent<void>(componentNode.handle);
		}

		reachedEndOfList = (componentNode.nextItemPtr == Entity::k_invalidId);

		if (!reachedEndOfList)
		{
			componentNode = m_entityComponentsMapping[componentNode.nextItemPtr];
		}
	}

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
		const auto& hierarchyData = m_entityHierarchyData[offset];
		if (hierarchyData.nextItemPtr == Entity::k_invalidId)
		{
			// This is the end of the list, can be used as insertion point
			placeForInsertionFound = true;
		}
		else
		{
			// Jump to next item in the list
			offset = hierarchyData.nextItemPtr;
		}
	}

	auto hierarchyDataId = offset;
	auto newEntryId = hierarchyDataId;

	if (m_entityHierarchyData[hierarchyDataId].childId != Entity::k_invalidId)
	{
		m_entityHierarchyData.emplace_back();
		newEntryId = m_entityHierarchyData.size() - 1;
	}

	// Insert component id here
	auto currentEntry = &m_entityHierarchyData[hierarchyDataId];
	auto newEntry = &m_entityHierarchyData[newEntryId];
	newEntry->childId = child.id;
	newEntry->nextItemPtr = Entity::k_invalidId;

	if (currentEntry != newEntry)
	{
		// Link new entry to the list
		currentEntry->nextItemPtr = static_cast<uint32_t>(newEntryId);
	}

	++m_entities[entity.id].childrenCount;
	child.parentId = entity.id;
}

void EntitiesCollection::RemoveChild(Entity& entity, Entity& child)
{
	assert(child.id != Entity::k_invalidId);

	// Find the child in the list
	bool itemFound = false;
	std::size_t offset = entity.hierarchyDataOffset;
	std::size_t prevOffset = Entity::k_invalidId;
	while (!itemFound)
	{
		const auto& hierarchyData = m_entityHierarchyData[offset];

		if (hierarchyData.childId == child.id)
		{
			itemFound = true;
		}
		else if (hierarchyData.nextItemPtr == Entity::k_invalidId)
		{
			break;
		}
		else
		{
			prevOffset = offset;
			offset = hierarchyData.nextItemPtr;
		}
	}

	if (itemFound)
	{
		// Item found, remove it from the list preserving connectivity
		auto& hierarchyData = m_entityHierarchyData[offset];

		if (prevOffset == Entity::k_invalidId)
		{
			entity.hierarchyDataOffset = hierarchyData.nextItemPtr;
		}
		else
		{
			m_entityHierarchyData[prevOffset].nextItemPtr = hierarchyData.nextItemPtr;
		}

		hierarchyData.childId = Entity::k_invalidId;
		hierarchyData.nextItemPtr = Entity::k_invalidId;

		child.parentId = Entity::k_invalidId;
		--m_entities[entity.id].childrenCount;
	}
}

uint16_t EntitiesCollection::GetChildrenCount(Entity& entity) const
{
	return m_entities[entity.id].childrenCount;
}

Entity* EntitiesCollection::GetParent(Entity& entity)
{
	if (entity.parentId != Entity::k_invalidId)
	{
		return &m_entities[entity.parentId].entity;
	}
	else
	{
		return nullptr;
	}
}

EntitiesCollection::ChildrenData EntitiesCollection::GetChildrenData(Entity& entity)
{
	auto offsetBegin = m_entities[entity.id].entity.hierarchyDataOffset;

	std::size_t offsetEnd = offsetBegin;
	bool endFound = false;
	while (!endFound)
	{
		const auto& hierarchyData = m_entityHierarchyData[offsetEnd];
		if (hierarchyData.childId == Entity::k_invalidId)
		{
			endFound = true;
		}
		else
		{
			offsetEnd = hierarchyData.nextItemPtr;

			if (offsetEnd == Entity::k_invalidId)
			{
				endFound = true;
			}
		}
	}

	return EntitiesCollection::ChildrenData(this, m_entityHierarchyData, offsetBegin, offsetEnd);
}

EntitiesCollection::EntitiesStorageType& EntitiesCollection::GetEntitiesData()
{
	return m_entities;
}

} // namespace ecs
