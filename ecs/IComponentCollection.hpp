#pragma once
#include "ComponentHandle.hpp"

namespace ecs
{

class IComponentCollection
{
public:
	virtual ~IComponentCollection() = default;

	// Create element inside collection and return inserted element index
	virtual ComponentHandleInternal* Create() = 0;
	// Destroy collection element by index. When destroy succeeds, component destructor is called.
	virtual void Destroy(const std::size_t index) = 0;
	// Retreives pointer to collection item by index
	virtual void* Get(const std::size_t index) = 0;
	// Associates entity id with the component at index
	virtual void SetItemEntityId(const std::size_t index, const std::size_t entityId) = 0;
	virtual std::size_t GetItemEntityId(const std::size_t index) const = 0;
	virtual void SetTypeId(const uint8_t typeId) = 0;
	virtual uint8_t GetTypeId() const = 0;
};

} // namespace ecs
