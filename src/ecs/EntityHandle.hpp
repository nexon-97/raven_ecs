#pragma once
#include "ecs/ECSApiDef.hpp"
#include <cstdint>

namespace ecs
{

struct Entity;

struct EntityHandle
{
	friend class Manager;

public:
	using HandleIndex = uint32_t;

	ECS_API EntityHandle() noexcept;
	explicit ECS_API EntityHandle(HandleIndex* handleIndexPtr) noexcept;
	ECS_API ~EntityHandle() noexcept;

	ECS_API EntityHandle(const EntityHandle&) noexcept;
	ECS_API EntityHandle& operator=(const EntityHandle&) noexcept;
	ECS_API EntityHandle(EntityHandle&&) noexcept;
	ECS_API EntityHandle& operator=(EntityHandle&&) noexcept;

	bool ECS_API operator==(const EntityHandle& other) const;
	bool ECS_API operator!=(const EntityHandle& other) const;

	bool ECS_API IsValid() const;
	HandleIndex ECS_API GetIndex() const;

	static void ECS_API SetManagerInstance(ecs::Manager* manager);

private:
	HandleIndex* m_handleIndexPtr = nullptr;
};

} // namespace ecs
