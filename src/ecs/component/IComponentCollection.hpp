#pragma once
#include "ecs/component/ComponentPtr.hpp"
#include <cstddef>

namespace ecs
{

class IComponentCollection
{
public:
	virtual ~IComponentCollection() = default;

	virtual void Clear() = 0;
	// Create element inside collection and return inserted element index
	virtual ComponentPtr Create() = 0;
	// Destroy collection element by index. When destroy succeeds, component destructor is called.
	virtual void Destroy(const std::size_t index) = 0;
	// Retreives pointer to collection item by index
	virtual void* GetData(const std::size_t index) = 0;
	virtual ComponentPtrBlock* GetControlBlock(const std::size_t index) = 0;
	virtual ComponentPtr GetItemPtr(const std::size_t index) = 0;
	// Methods to insert component data from external memory source
	virtual void CopyData(const std::size_t index, const void* dataSource) = 0;
	virtual void MoveData(const std::size_t index, void* dataSource) = 0;
	virtual ComponentPtr CloneComponent(const std::size_t index) = 0;
};

} // namespace ecs
