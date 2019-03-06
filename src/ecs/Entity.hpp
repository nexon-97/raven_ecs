#pragma once
#include <cstdint>

namespace ecs
{

struct Entity
{
	uint32_t id;
	uint32_t hierarchyDataOffset;
	uint32_t componentsMask;
	uint32_t componentsDataOffset;

	static const uint32_t k_invalidId;
};

} // namespace ecs
