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
{}

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

	m_entityComponentsMapping.emplace_back();
	auto& mappingItem = m_entityComponentsMapping.back();
	mappingItem.componentId = 0U;
	mappingItem.nextItemPtr = Entity::k_invalidId;
	mappingItem.componentType = ComponentHandleInternal::k_invalidTypeId;

	return entityData.entity;
}

void EntitiesCollection::DestroyEntity(const uint32_t id)
{
	m_entities[id].entity.id = Entity::k_invalidId;
	m_entities[id].isAlive = false;
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

	if (m_entityComponentsMapping[mappingEntryId].componentType != ComponentHandleInternal::k_invalidTypeId)
	{
		m_entityComponentsMapping.emplace_back();
		newEntryId = m_entityComponentsMapping.size() - 1;
	}

	// Insert component id here
	auto currentEntry = &m_entityComponentsMapping[mappingEntryId];
	auto newEntry = &m_entityComponentsMapping[newEntryId];
	newEntry->componentId = handle.GetOffset();
	newEntry->componentType = handle.GetTypeIndex();
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
		if (componentNode.componentType == componentType)
		{
			// Component type found, return it
			ComponentHandleInternal handleInternal = { componentNode.componentId, componentNode.componentType };
			ComponentHandle handle(&handleInternal);
			return m_ecsManager.GetComponent<void>(handle);
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

} // namespace ecs
