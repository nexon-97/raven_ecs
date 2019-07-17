#pragma once
#include <cstdint>
#include <cstddef>
#include <bitset>

namespace ecs
{

namespace detail
{
template <class T> class MemoryPool;
}

using EntityId = uint32_t;
const std::size_t MaxComponentTypesCount = 128U;
using ComponentMaskType = std::bitset<MaxComponentTypesCount>;
using EntityHandleIndex = uint32_t;
using ComponentTypeId = uint8_t;

}
