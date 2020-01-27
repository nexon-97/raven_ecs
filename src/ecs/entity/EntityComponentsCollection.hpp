#pragma once
#include "ecs/storage/MemoryPool.hpp"
#include "ecs/component/ComponentPtr.hpp"
#include <cstddef>
#include <iterator>

namespace ecs
{

struct EntityComponentMapEntry
{
	uint32_t nextItemPtr;
	ComponentPtr componentPtr;

	EntityComponentMapEntry();
};
using ComponentsMapStorageType = detail::MemoryPool<EntityComponentMapEntry>;

class ECS_API EntityComponentsCollection
{
public:
	explicit EntityComponentsCollection(ComponentsMapStorageType& storageRef, const std::size_t mappingStartOffset);

	struct ECS_API iterator
	{
		using iterator_category = std::forward_iterator_tag;
		using pointer = ComponentPtr*;
		using reference = ComponentPtr&;

		iterator() = delete;
		iterator(ComponentsMapStorageType& dataRef, std::size_t offset);

		reference operator*();
		pointer operator->();

		iterator& operator++();
		iterator operator++(int);

		bool operator==(const iterator& other) const;
		bool operator!=(const iterator& other) const;

		ComponentsMapStorageType& dataRef;
		std::size_t offset;
	};

	iterator begin();
	iterator end();

private:
	ComponentsMapStorageType& m_storageRef;
	std::size_t m_mappingStartOffset;
};

} // namespace ecs
