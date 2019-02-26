#pragma once
#include "ComponentHandle.hpp"

namespace ecs
{

class IComponentCollection
{
public:
	virtual ~IComponentCollection() = default;

	// Create element inside collection, and return pair. First is inserted element index, second - success of operation.
	virtual std::pair<std::size_t, bool> TryCreate() = 0;
	// Destroy collection element by index. When destroy succeeds, component destructor is called.
	virtual void Destroy(const std::size_t index) = 0;
	// Retreives pointer to collection item by index
	virtual void* Get(const std::size_t index) = 0;
	// Check if the collection memory fully loaded
	virtual bool IsFull() const = 0;
};

} // namespace ecs
