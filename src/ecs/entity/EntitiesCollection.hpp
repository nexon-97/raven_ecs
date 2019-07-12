#pragma once
#include "ecs/entity/EntityData.hpp"
#include "ecs/entity/Entity.hpp"
#include "ecs/component/ComponentHandle.hpp"
//#include "ecs/entity/EntityHierarchyManager.hpp"
#include "ecs/ECSApiDef.hpp"
#include "ecs/storage/MemoryPool.hpp"

#include <unordered_map>

namespace ecs
{

class Manager;

class EntitiesCollection
{
	friend struct Entity;
	friend struct EntityHandle;

	using EntityIdsMap = std::unordered_map<EntityId, uint32_t>; // Mapping of entity id to entity storage location

public:
	EntitiesCollection() = delete;
	explicit ECS_API EntitiesCollection(Manager& ecsManager);

	// Disable collection copy
	EntitiesCollection(const EntitiesCollection&) = delete;
	EntitiesCollection& operator=(const EntitiesCollection&) = delete;

	Entity ECS_API GetEntityById(const EntityId id);
	Entity ECS_API CreateEntity();

	void ECS_API AddChild(Entity& entity, Entity& child);
	void ECS_API RemoveChild(Entity& entity, Entity& child);
	void ECS_API ClearChildren(Entity& entity, bool destroyChildren = false);
	EntityId ECS_API GetChildByIdx(Entity& entity, const std::size_t idx) const;

	void ECS_API ActivateEntity(Entity& entity, const bool activate);

	// Entity hierarchy manager interface copy
	bool ECS_API CompareEntitiesInHierarchy(const Entity& lhs, const Entity& rhs) const;
	std::size_t ECS_API GetEntitiesCountInBranch(const EntityId& rootEntityId) const;
	std::size_t ECS_API GetActiveEntitiesCountInBranch(const EntityId& rootEntityId) const;
	int ECS_API GetEntityHierarchyOffsetRelativeToEntity(const EntityId& entityId, const EntityId& pivotId) const;

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
				//EntityHandle handle = data.collection->GetEntityHandleById(data.hierarchyData[offset]->childId);
				//return *data.collection->GetEntity(handle);

				return Entity();
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

	ChildrenData ECS_API GetChildrenData(EntityData& entityData);

protected:
	using EntitiesStorageType = detail::MemoryPool<EntityData>;
	ECS_API EntitiesStorageType& GetEntitiesData();

private:
	friend struct Entity;

	void RefreshActivation(EntityData& entityData, bool forceActivate = false);
	void RefreshComponentsActivation(EntityData& entityData);
	void RefreshChildrenActivation(EntityData& entityData);

	void RefreshHierarchyDepth(EntityData& entityData, const EntityId newParentId, bool constructNewHierarchyTree);

	void OnEntityDataDestroy(const EntityId entityId);
	void DestroyEntity(const EntityId entityId);

	void OnEntityEnabled(const EntityId entityId, const bool enabled);

	void MoveEntityData(EntityData& entityData, const uint32_t newLocation);
	Entity CloneEntity(const Entity& entity);

	EntityComponentMapEntry& CreateComponentMappingEntry(EntityData& entityData);
	EntityComponentMapEntry* FindComponentMappingEntry(EntityData& entityData, const ComponentTypeId componentType);
	void RemoveComponentMappingEntry(EntityData& entityData, const ComponentTypeId componentType);
	ComponentsMapStorageType& GetComponentsMapStorage();

private:
	EntitiesStorageType m_entitiesData;
	EntityIdsMap m_entityIdsMap;
	ComponentsMapStorageType m_entityComponentsMapping;
	EntityHierarchyDataStorageType m_entityHierarchyData;
	//EntityHierarchyManager m_hierarchyManager;
	std::size_t m_activeEntitiesCount = 0U;
	EntityId m_nextEntityId = 0U;
	Manager& m_ecsManager;
};

} // namespace ecs
