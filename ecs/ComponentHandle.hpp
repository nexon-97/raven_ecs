#pragma once
#include <type_traits>
#include <typeindex>

namespace ecs
{

struct ComponentHandle
{
	static const std::type_index k_invalidSystemId;

public:
	static const ComponentHandle k_invalidHandle;

	ComponentHandle()
		: typeId(k_invalidSystemId)
		, objectId(0U)
	{}

	explicit ComponentHandle(const std::type_index& typeId, const std::size_t objectId)
		: typeId(typeId)
		, objectId(objectId)
	{}

	bool operator==(const ComponentHandle& other) const
	{
		return typeId == other.typeId && objectId == other.objectId;
	}

	bool IsValid() const
	{
		return typeId != k_invalidSystemId;
	}

	std::type_index GetTypeIndex() const
	{
		return typeId;
	}

	std::size_t GetOffset() const
	{
		return objectId;
	}

	std::type_index typeId;
	std::size_t objectId;
};

} // namespace ecs
