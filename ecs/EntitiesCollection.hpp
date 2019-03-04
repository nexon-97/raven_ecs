#pragma once
#include "Entity.hpp"
#include "ecs/ComponentHandle.hpp"

#include <vector>
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

	Entity& GetEntity(const uint32_t id);
	Entity& CreateEntity();
	void DestroyEntity(const uint32_t id);

	void AddComponent(Entity& entity, const ComponentHandle& handle);
	void RemoveComponent(Entity& entity, const ComponentHandle& handle);
	bool HasComponent(Entity& entity, const uint8_t componentType);
	void* GetComponent(Entity& entity, const uint8_t componentType);

	template <typename ComponentType>
	ComponentType* GetComponent(Entity& entity)
	{
		auto componentTypeId = GetComponentTypeIdByTypeIndex(typeid(ComponentType));
		return static_cast<ComponentType*>(GetComponent(entity, componentTypeId));
	}

private:
	uint8_t GetComponentTypeIdByTypeIndex(const std::type_index& typeIndex) const;

private:
	struct EntityData
	{
		Entity entity;
		bool isAlive : 1;
	};

	struct EntityComponentMapEntry
	{
		uint32_t componentId;
		uint32_t nextItemPtr = Entity::k_invalidId;
		uint8_t componentType;
	};

	std::vector<EntityData> m_entities;
	std::vector<EntityComponentMapEntry> m_entityComponentsMapping;
	Manager& m_ecsManager;
};

} // namespace ecs
