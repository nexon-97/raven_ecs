#include "ecs/entity/EntityChildrenCollection.hpp"
#include "ecs/entity/Entity.hpp"
#include "ecs/entity/EntitiesCollection.hpp"
#include "ecs/Manager.hpp"

namespace
{
ecs::EntitiesCollection* g_entitiesCollection = nullptr;
}

namespace ecs
{

void EntityChildrenCollection::SetManagerInstance(Manager* manager)
{
	g_entitiesCollection = &manager->GetEntitiesCollection();
}

EntityHierarchyData::EntityHierarchyData()
	: nextItemPtr(Entity::GetInvalidId())
	, childId(Entity::GetInvalidId())
{}

EntityChildrenCollection::EntityChildrenCollection(EntityHierarchyDataStorageType& hierarchyData, const EntityHierarchyDataOffset offsetBegin)
	: m_offsetBegin(offsetBegin)
	, m_hierarchyData(hierarchyData)
{}

EntityChildrenCollection::iterator::iterator(EntityHierarchyDataStorageType& hierarchyData, const EntityHierarchyDataOffset offset)
	: data(hierarchyData)
	, offset(offset)
{}

Entity EntityChildrenCollection::iterator::operator*()
{
	EntityId entityId = data[offset]->childId;
	return g_entitiesCollection->GetEntityById(entityId);
}

EntityChildrenCollection::iterator EntityChildrenCollection::begin() const
{
	return iterator(m_hierarchyData, m_offsetBegin);
}

EntityChildrenCollection::iterator EntityChildrenCollection::end() const
{
	return iterator(m_hierarchyData, std::numeric_limits<EntityHierarchyDataOffset>::max());
}

bool EntityChildrenCollection::empty() const
{
	return begin() == end();
}

void EntityChildrenCollection::clear()
{
	
}

Entity EntityChildrenCollection::operator[](const std::size_t index)
{
	std::size_t currentIndex = 0U;
	for (Entity& entity : *this)
	{
		if (index == currentIndex)
		{
			return entity;
		}

		++currentIndex;
	}

	return Entity();
}

EntityChildrenCollection::iterator& EntityChildrenCollection::iterator::operator++()
{
	offset = data[offset]->nextItemPtr;
	return *this;
}

EntityChildrenCollection::iterator EntityChildrenCollection::iterator::operator++(int)
{
	const auto temp(*this); ++* this; return temp;
}

bool EntityChildrenCollection::iterator::operator==(const iterator& other) const
{
	return offset == other.offset;
}

bool EntityChildrenCollection::iterator::operator!=(const iterator& other) const
{
	return !(*this == other);
}

} // namespace ecs
