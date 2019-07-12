#pragma once
#include "ComponentHandle.hpp"
#include <cstddef>

namespace ecs
{

class IComponentCollection
{
public:
	virtual ~IComponentCollection() = default;

	// Create element inside collection and return inserted element index
	virtual ComponentHandle::HandleIndex* Create() = 0;
	// Destroy collection element by index. When destroy succeeds, component destructor is called.
	virtual void Destroy(const std::size_t index) = 0;
	// Retreives pointer to collection item by index
	virtual void* Get(const std::size_t index) = 0;
	// Methods to insert component data from external memory source
	virtual void CopyData(const std::size_t index, const void* dataSource) = 0;
	virtual void MoveData(const std::size_t index, void* dataSource) = 0;
	// Associates entity id with the component at index
	virtual void SetItemEntityId(const std::size_t index, const uint32_t entityId) = 0;
	virtual uint32_t GetItemEntityId(const std::size_t index) const = 0;
	virtual void SetItemEnabled(const std::size_t index, const bool enabled) = 0;
	virtual bool IsItemEnabled(const std::size_t index) const = 0;
	virtual void RefreshComponentActivation(const std::size_t index) = 0;
	virtual void RefreshComponentActivation(const std::size_t index, const bool ownerEnabled, const bool ownerActivated) = 0;
	virtual ComponentHandle::HandleIndex* CloneComponent(const std::size_t index) = 0;
	virtual void SetTypeId(const uint8_t typeId) = 0;
	virtual uint8_t GetTypeId() const = 0;

	// Debug method to dump collection internal state
	virtual void DumpState() = 0;
	// Debug method to validate collection internal state
	virtual bool Validate() = 0;
};

} // namespace ecs
