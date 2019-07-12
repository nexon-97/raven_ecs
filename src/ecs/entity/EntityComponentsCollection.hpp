#pragma once
#include "ecs/component/ComponentHandle.hpp"
#include "ecs/storage/MemoryPool.hpp"
#include <cstddef>
#include <iterator>

namespace ecs
{

struct EntityComponentMapEntry
{
	uint32_t nextItemPtr;
	ComponentHandle handle;

	EntityComponentMapEntry();
};
using ComponentsMapStorageType = detail::MemoryPool<EntityComponentMapEntry>;

class EntityComponentsCollection
{
public:
	explicit EntityComponentsCollection(ComponentsMapStorageType& storageRef, const std::size_t mappingStartOffset);

	struct iterator
	{
		using iterator_category = std::forward_iterator_tag;
		using pointer = ComponentHandle*;
		using reference = ComponentHandle&;

		iterator() = delete;
		ECS_API iterator(ComponentsMapStorageType& dataRef, std::size_t offset);

		ECS_API reference operator*();
		ECS_API pointer operator->();

		ECS_API iterator& operator++();
		iterator ECS_API operator++(int);

		bool ECS_API operator==(const iterator& other) const;
		bool ECS_API operator!=(const iterator& other) const;

		ComponentsMapStorageType& dataRef;
		std::size_t offset;
	};

	iterator ECS_API begin();
	iterator ECS_API end();

private:
	ComponentsMapStorageType& m_storageRef;
	std::size_t m_mappingStartOffset;
};

} // namespace ecs
