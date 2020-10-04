#pragma once
#include "ecs/entity/Entity.hpp"

namespace ecs
{

/*
* Generic entity collection, used inside ECS to implement entity layers,
* but can be implemented differently on user side
*/
template <class IteratorT>
class IEntityCollection
{
public:
	virtual ~IEntityCollection() = default;

	virtual void Clear(const EntityId entityId) = 0;
	virtual void Add(const EntityId entityId) = 0;
	virtual void Remove(const EntityId entityId) = 0;
	virtual bool Contains(const EntityId entityId) = 0;

	virtual IteratorT begin() = 0;
	virtual IteratorT end() = 0;
};

}
