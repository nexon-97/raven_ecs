#pragma once
#include <cstdint>

namespace ecs
{

struct ComponentHandleInternal
{
	static const uint8_t k_invalidTypeId;

	uint32_t objectId;
	uint8_t typeId;
};

struct ComponentHandle
{
public:
	ComponentHandle() = default;
	explicit ComponentHandle(const ComponentHandleInternal* handle)
		: m_handle(handle)
	{}

	bool operator==(const ComponentHandle& other) const
	{
		return m_handle == other.m_handle;
	}

	bool IsValid() const
	{
		return (nullptr != m_handle) && (m_handle->typeId != ComponentHandleInternal::k_invalidTypeId);
	}

	uint8_t GetTypeIndex() const
	{
		return m_handle->typeId;
	}

	uint32_t GetOffset() const
	{
		return m_handle->objectId;
	}

private:
	const ComponentHandleInternal* m_handle = nullptr;
};

} // namespace ecs
