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
	explicit ECS_API EntitiesCollection(Manager& ecsManager);

	// Disable collection copy
	EntitiesCollection(const EntitiesCollection&) = delete;
	EntitiesCollection& operator=(const EntitiesCollection&) = delete;

	ECS_API Entity& GetEntity(const uint32_t id);
	ECS_API Entity& CreateEntity();
	void ECS_API DestroyEntity(const uint32_t id);

	void ECS_API AddComponent(Entity& entity, const ComponentHandle& handle);
	void ECS_API RemoveComponent(Entity& entity, const ComponentHandle& handle);
	bool ECS_API HasComponent(const Entity& entity, const uint8_t componentType);
	ECS_API void* GetComponent(const Entity& entity, const uint8_t componentType) const;
	ECS_API void* GetComponent(const Entity& entity, const uint8_t componentType, ComponentHandle& handle) const;

	template <typename ComponentType>
	ComponentType* GetComponent(Entity& entity) const
	{
		auto componentTypeId = GetComponentTypeIdByTypeIndex(typeid(ComponentType));
		return static_cast<ComponentType*>(GetComponent(entity, componentTypeId));
	}

	template <typename ComponentType>
	ComponentType* GetComponent(Entity& entity, ComponentHandle& handle) const
	{
		auto componentTypeId = GetComponentTypeIdByTypeIndex(typeid(ComponentType));
		return static_cast<ComponentType*>(GetComponent(entity, componentTypeId, handle));
	}

	void ECS_API AddChild(Entity& entity, Entity& child);
	void ECS_API RemoveChild(Entity& entity, Entity& child);
	uint16_t ECS_API GetChildrenCount(const Entity& entity) const;
	ECS_API Entity* GetParent(const Entity& entity);
	ECS_API Entity* GetParent(const ComponentHandle& handle);

	void ECS_API SetEntityEnabled(Entity& entity, const bool enabled);
	void ECS_API ActivateEntity(Entity& entity, const bool activate);

	void ECS_API RefreshActivation(Entity& entity, bool forceActivate = false);
	void ECS_API RefreshComponentsActivation(Entity& entity);
	void ECS_API RefreshChildrenActivation(Entity& entity);

	bool ECS_API IsEntityEnabled(const std::size_t entityId) const;
	bool ECS_API IsEntityActivated(const std::size_t entityId) const;

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

public:
	struct EntityComponentMapEntry
	{
		uint32_t nextItemPtr = Entity::GetInvalidId();
		ComponentHandle handle;
	};
	using ComponentsMapStorageType = detail::MemoryPool<EntityComponentMapEntry>;

	// Helper structure to make the particular entity children iterator
	struct ComponentsData
	{
		struct iterator
		{
			using iterator_category = std::forward_iterator_tag;
			using pointer = ComponentHandle *;
			using reference = ComponentHandle&;

			iterator() = delete;
			iterator(ComponentsMapStorageType& data, std::size_t offset)
				: data(data)
				, offset(offset)
			{}

			reference operator*()
			{
				return data[offset]->handle;
			}

			pointer operator->()
			{
				return &**this;
			}

			iterator& operator++()
			{
				offset = data[offset]->nextItemPtr;
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

			ComponentsMapStorageType& data;
			std::size_t offset;
		};

		friend struct iterator;

		ComponentsData(ComponentsMapStorageType& storage, const std::size_t offsetBegin)
			: offsetBegin(offsetBegin)
			, storage(storage)
		{}

		iterator begin()
		{
			return iterator(storage, offsetBegin);
		}

		iterator end()
		{
			return iterator(storage, Entity::GetInvalidId());
		}

	private:
		ComponentsMapStorageType& storage;
		std::size_t offsetBegin;
	};

	ComponentsData ECS_API GetComponentsData(Entity& entity);

protected:
	struct EntityData
	{
		Entity entity;
		uint16_t childrenCount;
		bool isEnabled : 1;		// Indicates if the entity is enabled (though can be not registered in world)
		bool isActivated : 1;	// Indicates if the entity is currently activated (is actually registered in world)

		EntityData()
			: childrenCount(0U)
			, isEnabled(true)
			, isActivated(false)
		{}
	};
	using EntitiesStorageType = detail::MemoryPool<EntityData>;
	ECS_API EntitiesStorageType& GetEntitiesData();

private:
	uint8_t ECS_API GetComponentTypeIdByTypeIndex(const std::type_index& typeIndex) const;

private:
	EntitiesStorageType m_entities;
	ComponentsMapStorageType m_entityComponentsMapping;
	EntityHierarchyDataStorageType m_entityHierarchyData;
	std::size_t m_activeEntitiesCount = 0U;
	Manager& m_ecsManager;
};

} // namespace ecs
