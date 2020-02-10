#pragma once
#include <unordered_map>
#include "ecs/detail/Types.hpp"
#include "ecs/component/ComponentPtr.hpp"
#include "ecs/cache/ComponentsTuple.hpp"

namespace ecs
{

class ComponentsTupleCache
{
public:
	ComponentsTupleCache() = delete;
	ECS_API ComponentsTupleCache(ComponentTypeId* componentTypesList, const std::size_t componentTypesCount);
	ECS_API ~ComponentsTupleCache();

	ComponentsTupleCache(const ComponentsTupleCache&) = delete;
	ComponentsTupleCache& operator=(const ComponentsTupleCache&) = delete;

	// Move is enabled
	ECS_API ComponentsTupleCache(ComponentsTupleCache&& other);
	ECS_API ComponentsTupleCache& operator=(ComponentsTupleCache&& other);

	ECS_API std::unordered_map<EntityId, ComponentsTuple>& GetData();
	void ECS_API TouchEntity(const Entity& entity);

private:
	std::unordered_map<EntityId, ComponentsTuple> m_componentTuples;
	ComponentTypeId* m_componentTypesList;
	std::size_t m_componentsCount;
};

}
