#include "ecs/detail/Hash.hpp"

namespace ecs
{
namespace detail
{

void hash_combine(std::size_t& seed, std::size_t hash)
{
	hash += 0x9e3779b9 + (seed << 6) + (seed >> 2);
	seed ^= hash;
}

}
}
