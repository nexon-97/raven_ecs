#pragma once
#include "ecs/entity/IEntityCollection.hpp"
#include <vector>

namespace ecs
{

class EntityLayer;
struct EntityLayerIterator
{
	using iterator_category = std::forward_iterator_tag;

	explicit EntityLayerIterator(EntityLayer& layer, std::size_t index)
		: layer(layer)
		, index(index)
	{}

	Entity operator*();

	EntityLayerIterator& operator++();
	EntityLayerIterator operator++(int);

	bool operator==(const EntityLayerIterator& other) const;
	bool operator!=(const EntityLayerIterator& other) const;

private:
	EntityLayer& layer;
	std::size_t index;
};

/*
* Entity layer is an implementation of entity collection, used for entities layering
* Add, Remove and Contains methods cost is O(logN)
*/
class EntityLayer
	: public IEntityCollection<EntityLayerIterator>
{
public:
	void ECS_API Clear(const EntityId entityId) override;
	void ECS_API Add(const EntityId entityId) override;
	void ECS_API Remove(const EntityId entityId) override;
	bool ECS_API Contains(const EntityId entityId) override;

	EntityLayerIterator ECS_API begin() override;
	EntityLayerIterator ECS_API end() override;

	// Force remove invalid entities entries from layer
	void ECS_API Shrink();

private:
	friend struct EntityLayerIterator;

	EntityId GetByIndex(const std::size_t index) const;
	std::size_t GetNextValidEntityIndex(const std::size_t startFrom) const;

private:
	// Sorted list of entities ids
	std::vector<EntityId> m_entities;
};

}
