#pragma once
#include "Entity.hpp"
#include "ecs/ComponentHandle.hpp"

#include <vector>

namespace ecs
{

class EntitiesCollection
{
public:
	Entity& GetEntity(const uint32_t id);
	Entity& CreateEntity();
	void DestroyEntity(const uint32_t id);

	void AddComponent(Entity& entity, const ComponentHandle& handle);
	void RemoveComponent(Entity& entity, const ComponentHandle& handle);
	bool HasComponent(Entity& entity, const std::size_t componentType);

private:
	struct EntityData
	{
		Entity entity;
		bool isAlive : 1;
	};
	std::vector<EntityData> m_entities;
};

} // namespace ecs
