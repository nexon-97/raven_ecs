#pragma once
#include "ecs/ECSApiDef.hpp"
#include "ecs/Entity.hpp"
#include <cstdint>
#include <typeindex>
#include <limits>

#undef max

namespace ecs
{

class Manager;

struct ComponentHandleInternal
{
	static const uint8_t ECS_API GetInvalidTypeId()
	{
		static const uint8_t k_invalidTypeId = std::numeric_limits<uint8_t>::max();
		return k_invalidTypeId;
	}
};

using HandleIndex = uint16_t;
using ComponentTypeId = uint8_t;

struct ComponentHandle
{
	friend class Manager;

public:
	ECS_API ComponentHandle();
	explicit ECS_API ComponentHandle(const ComponentTypeId typeId, HandleIndex* handleIndexPtr);

	bool ECS_API operator==(const ComponentHandle& other) const;
	bool ECS_API operator!=(const ComponentHandle& other) const;

	bool ECS_API IsValid() const;
	ComponentTypeId ECS_API GetTypeIndex() const;
	std::type_index ECS_API GetStdTypeIndex() const; // Weird method name, rename
	HandleIndex ECS_API GetOffset() const;
	EntityId ECS_API GetEntityId() const;

	template <typename T>
	bool IsOfType() const
	{
		return IsOfTypeImpl(typeid(T));
	}

private:
	static void ECS_API SetManagerInstance(ecs::Manager* manager);
	bool ECS_API IsOfTypeImpl(const std::type_index& typeIndex) const;

private:
	ComponentTypeId m_typeId;
	HandleIndex* m_handleIndexPtr = nullptr;
};

} // namespace ecs
