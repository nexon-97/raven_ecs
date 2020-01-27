#pragma once
#include "ecs/TypeAliases.hpp"

namespace ecs
{

struct ComponentPtrBlock
{
	ComponentTypeId typeId = -1;
	int32_t dataIndex = -1;
	EntityId entityId = -1;
	int32_t refCount = 0;

	ComponentPtrBlock() = default;
	ComponentPtrBlock(ComponentTypeId inTypeId, int32_t inDataIndex, EntityId inEntityId, int32_t inRefCount)
		: typeId(inTypeId)
		, dataIndex(inDataIndex)
		, entityId(inEntityId)
		, refCount(inRefCount)
	{}
};

struct Entity;

}
