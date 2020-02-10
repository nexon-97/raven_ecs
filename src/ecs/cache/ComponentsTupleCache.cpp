#include "ecs/cache/ComponentsTupleCache.hpp"
#include "ecs/entity/Entity.hpp"

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
	bool hasAllComponents = true;
	for (std::size_t i = 0; i < m_componentsCount; ++i)
	{
		if (!entity.HasComponent(m_componentTypesList[i]))
		{
			hasAllComponents = false;
			break;
		}
	}

	if (hasAllComponents)
	{
		// Fill components tuple
		ComponentsTuple components(m_componentsCount);
		for (std::size_t i = 0; i < m_componentsCount; ++i)
		{
			components[i] = entity.GetComponent(m_componentTypesList[i]);
		}

		// Add to cache
		m_componentTuples.try_emplace(entity.GetId(), std::move(components));
	}
	else
	{
		auto it = m_componentTuples.find(entity.GetId());
		if (it != m_componentTuples.end())
		{
			m_componentTuples.erase(it);
		}
	}
}

}
