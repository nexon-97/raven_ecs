#pragma once
#include "ComponentHandle.hpp"

namespace ecs
{

class IComponentCollection
{
public:
	virtual ~IComponentCollection() = default;

	// Create element inside collection and return inserted element index
	virtual std::size_t Create() = 0;
	// Destroy collection element by index. When destroy succeeds, component destructor is called.
	virtual void Destroy(const std::size_t index) = 0;
	// Retreives pointer to collection item by index
	virtual void* Get(const std::size_t index) = 0;
};

} // namespace ecs
