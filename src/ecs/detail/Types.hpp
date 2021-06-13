#pragma once
#include "raven_ecs_export.h"
#include "ecs/TypeAliases.hpp"

namespace ecs
{

struct ECS_API ComponentPtrBlock
{
	ComponentTypeId typeId;
	int32_t dataIndex;
	EntityId entityId;
	int32_t refCount;

	ComponentPtrBlock();
	ComponentPtrBlock(ComponentTypeId inTypeId, int32_t inDataIndex, EntityId inEntityId, int32_t inRefCount);
};

struct Entity;

}
