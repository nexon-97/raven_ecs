#pragma once
#include "Entity.hpp"
#include <vector>

namespace ecs
{

class EntitiesCollection
{
public:
	Entity& GetEntity(const uint32_t id);
	Entity& CreateEntity();
	void DestroyEntity(const uint32_t id);

private:
	struct EntityData
	{
		Entity entity;
		bool isAlive : 1;
	};
	std::vector<EntityData> m_entities;
};

} // namespace ecs
