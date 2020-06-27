#include "ecs/cache/ComponentsTupleCache.hpp"
#include "ecs/entity/Entity.hpp"
#include <cstring>

namespace ecs
{

ComponentsTupleCache::ComponentsTupleCache(ComponentTypeId* componentTypesList, const std::size_t componentTypesCount)
	: m_componentsCount(componentTypesCount)
{
	if (componentTypesCount > 0)
	{
		m_componentTypesList = new ComponentTypeId[componentTypesCount];
		std::memcpy(m_componentTypesList, componentTypesList, sizeof(ComponentTypeId) * componentTypesCount);
	}
}

ComponentsTupleCache::~ComponentsTupleCache()
{
	if (nullptr != m_componentTypesList)
	{
		delete[] m_componentTypesList;
	}
}

ComponentsTupleCache::ComponentsTupleCache(ComponentsTupleCache&& other)
	: m_componentTuples(std::move(other.m_componentTuples))
	, m_componentsCount(other.m_componentsCount)
	, m_componentTypesList(other.m_componentTypesList)
{
	other.m_componentTypesList = nullptr;
	other.m_componentsCount = 0;
}

ComponentsTupleCache& ComponentsTupleCache::operator=(ComponentsTupleCache&& other)
{
	m_componentTuples = std::move(other.m_componentTuples);
	m_componentsCount = other.m_componentsCount;
	m_componentTypesList = other.m_componentTypesList;

	other.m_componentTypesList = nullptr;
	other.m_componentsCount = 0;

	return *this;
}

std::unordered_map<EntityId, ComponentsTuple>& ComponentsTupleCache::GetData()
{
	return m_componentTuples;
}

void ComponentsTupleCache::TouchEntity(const Entity& entity)
{
	bool hasAllComponents = entity.HasComponents(m_componentTypesList, m_componentsCount);
	if (hasAllComponents)
	{
		auto it = m_componentTuples.find(entity.GetId());
		if (it == m_componentTuples.end())
		{
			// Fill components tuple
			ComponentsTuple componentsTuple(m_componentsCount);
			entity.GetComponentsOfTypes(componentsTuple.GetMutableData(), m_componentTypesList, m_componentsCount);

			// Add to cache
			m_componentTuples.emplace(entity.GetId(), std::move(componentsTuple));
		}
	}
	else
	{
		// Not all components set -> try to remove entity from cache
		auto it = m_componentTuples.find(entity.GetId());
		if (it != m_componentTuples.end())
		{
			m_componentTuples.erase(it);
		}
	}
}

}
