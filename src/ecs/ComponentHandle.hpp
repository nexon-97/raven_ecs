#pragma once
#include "ecs/ECSApiDef.hpp"
#include <cstdint>

namespace ecs
{

class Manager;

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
	explicit ECS_API ComponentHandle(const uint8_t typeId, HandleIndex* handleIndexPtr);

	bool ECS_API operator==(const ComponentHandle& other) const;

	bool ECS_API IsValid() const;
	uint8_t ECS_API GetTypeIndex() const;
	HandleIndex ECS_API GetOffset() const;
	uint32_t ECS_API GetEntityId() const;

	static void ECS_API SetManagerInstance(ecs::Manager* manager);

private:
	uint8_t m_typeId;
	HandleIndex* m_handleIndexPtr = nullptr;
};

} // namespace ecs
