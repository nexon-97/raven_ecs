#pragma once
#include "Entity.hpp"
#include "ecs/ComponentHandle.hpp"
#include "ecs/ECSApiDef.hpp"
#include "storage/MemoryPool.hpp"

#include <typeindex>

namespace ecs
{

class Manager;

class EntitiesCollection
{
public:
	EntitiesCollection() = delete;
	explicit EntitiesCollection(Manager& ecsManager);

	// Disable collection copy
	EntitiesCollection(const EntitiesCollection&) = delete;
	EntitiesCollection& operator=(const EntitiesCollection&) = delete;

	ECS_API Entity& GetEntity(const uint32_t id);
	ECS_API Entity& CreateEntity();
	void ECS_API DestroyEntity(const uint32_t id);

	void ECS_API AddComponent(Entity& entity, const ComponentHandle& handle);
	void ECS_API RemoveComponent(Entity& entity, const ComponentHandle& handle);
	bool ECS_API HasComponent(Entity& entity, const uint8_t componentType);
	ECS_API void* GetComponent(Entity& entity, const uint8_t componentType) const;

	template <typename ComponentType>
	ComponentType* GetComponent(Entity& entity) const
	{
		auto componentTypeId = GetComponentTypeIdByTypeIndex(typeid(ComponentType));
		return static_cast<ComponentType*>(GetComponent(entity, componentTypeId));
	}

	void ECS_API AddChild(Entity& entity, Entity& child);
	void ECS_API RemoveChild(Entity& entity, Entity& child);
	uint16_t ECS_API GetChildrenCount(Entity& entity) const;
	ECS_API Entity* GetParent(Entity& entity);

	void ECS_API SetEntityEnabled(Entity& entity, const bool enabled);

public:
	struct EntityHierarchyData
	{
		uint32_t nextItemPtr = Entity::GetInvalidId();
		uint32_t childId = Entity::GetInvalidId();
	};
	using EntityHierarchyDataStorageType = detail::MemoryPool<EntityHierarchyData>;

	// Helper structure to make the particular entity children iterator
	struct ChildrenData
	{
		struct iterator
		{
			using iterator_category = std::forward_iterator_tag;
			using pointer = Entity*;
			using reference = Entity&;

			iterator() = default;
			iterator(ChildrenData& data, std::size_t offset)
				: data(data)
				, offset(offset)
			{}

			reference operator*()
			{
				return data.collection->GetEntity(data.hierarchyData[offset]->childId);
			}

			pointer operator->()
			{
				return &**this;
			}

			iterator& operator++()
			{
				offset = data.hierarchyData[offset]->nextItemPtr;
				return *this;
			}

			iterator operator++(int)
			{
				const auto temp(*this); ++*this; return temp;
			}

			bool operator==(const iterator& other) const
			{
				return offset == other.offset;
			}

			bool operator!=(const iterator& other) const
			{
				return !(*this == other);
			}

			ChildrenData& data;
			std::size_t offset;
		};

		ChildrenData(EntitiesCollection* collection, EntityHierarchyDataStorageType& hierarchyData
			, const std::size_t offsetBegin, const std::size_t offsetEnd)
			: offsetBegin(offsetBegin)
			, offsetEnd(offsetEnd)
			, collection(collection)
			, hierarchyData(hierarchyData)
		{}

		iterator begin()
		{
			return iterator(*this, offsetBegin);
		}

		iterator end()
		{
			return iterator(*this, offsetEnd);
		}

	private:
		EntitiesCollection* collection;
		EntityHierarchyDataStorageType& hierarchyData;
		std::size_t offsetBegin;
		std::size_t offsetEnd;
	};

	ChildrenData ECS_API GetChildrenData(Entity& entity);

protected:
	struct EntityData
	{
		Entity entity;
		uint16_t childrenCount = 0U;
		bool isAlive : 1;
		bool isEnabled : 1;
	};
	using EntitiesStorageType = detail::MemoryPool<EntityData>;
	EntitiesStorageType& GetEntitiesData();

private:
	uint8_t ECS_API GetComponentTypeIdByTypeIndex(const std::type_index& typeIndex) const;

private:
	struct EntityComponentMapEntry
	{
		uint32_t nextItemPtr = Entity::GetInvalidId();
		ComponentHandle handle;
	};
	using ComponentsMapStorageType = detail::MemoryPool<EntityComponentMapEntry>;

	EntitiesStorageType m_entities;
	ComponentsMapStorageType m_entityComponentsMapping;
	EntityHierarchyDataStorageType m_entityHierarchyData;
	Manager& m_ecsManager;
};

} // namespace ecs
