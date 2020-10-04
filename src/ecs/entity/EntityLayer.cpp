#include "ecs/entity/EntityLayer.hpp"
#include "ecs/Manager.hpp"
#include <algorithm>

namespace ecs
{

// Iterator
Entity EntityLayerIterator::operator*()
{
	return ecs::Manager::Get()->GetEntityById(layer.GetByIndex(index));
}

EntityLayerIterator& EntityLayerIterator::operator++()
{
	index = layer.GetNextValidEntityIndex(index + 1U);
	return *this;
}

EntityLayerIterator EntityLayerIterator::operator++(int)
{
	const auto temp(*this);
	++*this;
	return temp;
}

bool EntityLayerIterator::operator==(const EntityLayerIterator& other) const
{
	return index == other.index;
}

bool EntityLayerIterator::operator!=(const EntityLayerIterator& other) const
{
	return !(*this == other);
}

// Entity layer
void EntityLayer::Clear(const EntityId entityId)
{
	m_entities.clear();
}

void EntityLayer::Add(const EntityId entityId)
{
	auto it = std::lower_bound(m_entities.begin(), m_entities.end(), entityId);
	if (*it != entityId)
	{
		m_entities.insert(it, entityId);
	}
}

void EntityLayer::Remove(const EntityId entityId)
{
	auto it = std::lower_bound(m_entities.begin(), m_entities.end(), entityId);
	if (*it == entityId)
	{
		m_entities.erase(it);
	}
}

bool EntityLayer::Contains(const EntityId entityId)
{
	auto it = std::lower_bound(m_entities.begin(), m_entities.end(), entityId);
	return (*it == entityId);
}

EntityLayerIterator EntityLayer::begin()
{
	Manager* manager = Manager::Get();

	// Find invalid entities at the beginning and remove them to keep list valid
	std::size_t removeCount = 0U;
	const std::size_t count = m_entities.size();
	for (std::size_t i = 0U; i < count; i++)
	{
		if (manager->GetEntityById(m_entities[i]))
		{
			break;
		}
		else
		{
			removeCount++;
		}
	}

	// Remove entities from the beginning, if invalid ones are present
	if (removeCount > 0U)
	{
		m_entities.erase(m_entities.begin(), m_entities.begin() + removeCount);
	}

	// After shrinking, valid entity will be at position zero, or if there are no valid pointers, it will be end automatically
	return EntityLayerIterator(*this, 0);
}

EntityLayerIterator EntityLayer::end()
{
	return EntityLayerIterator(*this, m_entities.size());
}

EntityId EntityLayer::GetByIndex(const std::size_t index) const
{
	return m_entities[index];
}

std::size_t EntityLayer::GetNextValidEntityIndex(const std::size_t startFrom) const
{
	Manager* manager = Manager::Get();
	const std::size_t count = m_entities.size();
	for (std::size_t i = startFrom; i < count; i++)
	{
		if (manager->GetEntityById(m_entities[i]))
		{
			return i;
		}
	}

	return count;
}

void EntityLayer::Shrink()
{
	Manager* manager = Manager::Get();
	for (int i = static_cast<int>(m_entities.size()) - 1; i >= 0; i--)
	{
		if (!manager->GetEntityById(m_entities[i]))
		{
			m_entities.erase(m_entities.begin() + i);
		}
	}
}

}
