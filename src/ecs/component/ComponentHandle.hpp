#pragma once
#include "ecs/TypeAliases.hpp"
#include <cstdint>
#include <typeindex>
#include <limits>

#undef max

namespace ecs
{

class Manager;

struct ComponentHandleInternal
{
	static const ComponentTypeId ECS_API GetInvalidTypeId()
	{
		static const ComponentTypeId k_invalidTypeId = std::numeric_limits<ComponentTypeId>::max();
		return k_invalidTypeId;
	}
};

struct ComponentHandle
{
	friend class Manager;

public:
	using HandleIndex = uint16_t;

	ECS_API ComponentHandle();
	explicit ECS_API ComponentHandle(const ComponentTypeId typeId, HandleIndex* handleIndexPtr);

	bool ECS_API operator==(const ComponentHandle& other) const;
	bool ECS_API operator!=(const ComponentHandle& other) const;

	bool ECS_API IsValid() const;
	ComponentTypeId ECS_API GetTypeId() const;
	std::type_index ECS_API GetStdTypeIndex() const; // Weird method name, rename
	HandleIndex ECS_API GetOffset() const;
	EntityId ECS_API GetEntityId() const;

	/* Indicates if the component is activated and attached to activated entity */
	bool ECS_API IsActive() const;

	template <typename T>
	bool IsOfType() const
	{
		return IsOfTypeImpl(typeid(T));
	}

	ECS_API void* GetValue() const;

protected:
	static void ECS_API SetManagerInstance(ecs::Manager* manager);
	static ECS_API ecs::Manager* GetManagerInstance();

	bool ECS_API IsOfTypeImpl(const std::type_index& typeIndex) const;

protected:
	ComponentTypeId m_typeId;
	HandleIndex* m_handleIndexPtr = nullptr;
};

} // namespace ecs

namespace std
{

template <>
struct hash<ecs::ComponentHandle>
{
	void hash_combine(size_t &seed, size_t hash) const
	{
		hash += 0x9e3779b9 + (seed << 6) + (seed >> 2);
		seed ^= hash;
	}

	std::size_t operator()(const ecs::ComponentHandle& handle) const
	{
		size_t combined = hash<ecs::ComponentTypeId>()(handle.GetTypeId());
		hash_combine(combined, hash<uint16_t>()(handle.GetOffset()));

		return combined;
	}
};

}
