#pragma once
#include "ecs/ECSApiDef.hpp"
#include "ecs/storage/MemoryPool.hpp"
#include "ecs/entity/EntityData.hpp"

namespace ecs
{

class Manager;

using EntityHierarchyDataOffset = uint32_t;
struct EntityHierarchyData
{
	EntityHierarchyDataOffset nextItemPtr;
	EntityId childId;

	EntityHierarchyData();
};
using EntityHierarchyDataStorageType = detail::MemoryPool<EntityHierarchyData>;
struct Entity;

class EntityChildrenCollection
{
public:
	EntityChildrenCollection(EntityHierarchyDataStorageType& hierarchyData, const EntityHierarchyDataOffset offsetBegin);

	struct iterator
	{
		using iterator_category = std::forward_iterator_tag;
		using pointer = Entity*;

		iterator() = default;
		ECS_API iterator(EntityHierarchyDataStorageType& hierarchyData, const EntityHierarchyDataOffset offset);

		ECS_API Entity operator*();
		ECS_API iterator& operator++();
		iterator ECS_API operator++(int);

		bool ECS_API operator==(const iterator& other) const;
		bool ECS_API operator!=(const iterator& other) const;

		EntityHierarchyDataStorageType& data;
		EntityHierarchyDataOffset offset;
	};

	iterator ECS_API begin() const;
	iterator ECS_API end() const;

	Entity ECS_API operator[](const std::size_t index);

	bool ECS_API empty() const;
	void ECS_API clear();

	static void SetManagerInstance(Manager* manager);

private:
	EntityHierarchyDataStorageType& m_hierarchyData;
	EntityHierarchyDataOffset m_offsetBegin;
};

} // namespace ecs
