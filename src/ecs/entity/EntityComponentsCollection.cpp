#include "ecs/entity/EntityComponentsCollection.hpp"
#include "ecs/entity/Entity.hpp"

namespace ecs
{

EntityComponentMapEntry::EntityComponentMapEntry()
	: nextItemPtr(Entity::GetInvalidId())
{}

EntityComponentsCollection::EntityComponentsCollection(ComponentsMapStorageType& storageRef, const std::size_t mappingStartOffset)
	: m_storageRef(storageRef)
	, m_mappingStartOffset(mappingStartOffset)
{}

EntityComponentsCollection::iterator EntityComponentsCollection::begin()
{
	if (m_storageRef[m_mappingStartOffset]->handle.IsValid())
	{
		return iterator(m_storageRef, m_mappingStartOffset);
	}

	return end();
}

EntityComponentsCollection::iterator EntityComponentsCollection::end()
{
	return iterator(m_storageRef, Entity::GetInvalidId());
}

EntityComponentsCollection::iterator::iterator(ComponentsMapStorageType& dataRef, std::size_t offset)
	: dataRef(dataRef)
	, offset(offset)
{}

EntityComponentsCollection::iterator::reference EntityComponentsCollection::iterator::operator*()
{
	return dataRef[offset]->handle;
}

EntityComponentsCollection::iterator::pointer EntityComponentsCollection::iterator::operator->()
{
	return &**this;
}

EntityComponentsCollection::iterator& EntityComponentsCollection::iterator::operator++()
{
	offset = dataRef[offset]->nextItemPtr;
	return *this;
}

EntityComponentsCollection::iterator EntityComponentsCollection::iterator::operator++(int)
{
	const auto temp(*this); ++* this; return temp;
}

bool EntityComponentsCollection::iterator::operator==(const iterator& other) const
{
	return offset == other.offset;
}

bool EntityComponentsCollection::iterator::operator!=(const iterator& other) const
{
	return !(*this == other);
}

} // namespace ecs
