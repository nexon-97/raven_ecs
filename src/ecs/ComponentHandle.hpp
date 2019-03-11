#pragma once
#include "ecs/ECSApiDef.hpp"
#include <cstdint>

namespace ecs
{

struct ComponentHandleInternal
{
	static const uint8_t ECS_API GetInvalidTypeId()
	{
		static const uint8_t k_invalidTypeId = static_cast<uint8_t>(-1);
		return k_invalidTypeId;
	}
};

using HandleIndex = uint16_t;

struct ComponentHandle
{
public:
	ComponentHandle() = default;
	explicit ComponentHandle(const uint8_t typeId, HandleIndex* handleIndexPtr)
		: m_typeId(typeId)
		, m_handleIndexPtr(handleIndexPtr)
	{
		if (handleIndexPtr)
		{
			m_indexCopy = *handleIndexPtr;
		}
	}

	bool operator==(const ComponentHandle& other) const
	{
		return m_typeId == other.m_typeId && m_handleIndexPtr == other.m_handleIndexPtr;
	}

	bool IsValid() const
	{
		return (m_typeId != ComponentHandleInternal::GetInvalidTypeId());
	}

	uint8_t GetTypeIndex() const
	{
		return m_typeId;
	}

	HandleIndex GetOffset() const
	{
		return *m_handleIndexPtr;
	}

	HandleIndex* GetOffsetPtr() const
	{
		return m_handleIndexPtr;
	}

	HandleIndex GetDebugOffset() const
	{
		return m_indexCopy;
	}

private:
	uint8_t m_typeId;
	HandleIndex* m_handleIndexPtr = nullptr;
	HandleIndex m_indexCopy;
};

} // namespace ecs
