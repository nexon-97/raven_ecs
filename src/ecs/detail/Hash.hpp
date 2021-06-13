#pragma once
#include "raven_ecs_export.h"
#include <cstddef>

namespace ecs
{
namespace detail
{

void ECS_API hash_combine(std::size_t& seed, std::size_t hash);

}
}
