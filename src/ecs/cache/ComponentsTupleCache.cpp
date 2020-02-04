#include "ecs/cache/ComponentsTupleCache.hpp"

namespace ecs
{

ComponentsTupleCache::ComponentsTupleCache(ComponentTypeId* componentTypesList, const std::size_t componentTypesCount)
	: m_componentsCount(componentTypesCount)
{
	if (componentTypesCount > 0)
	{
		m_componentTypesList = new ComponentTypeId[componentTypesCount];
		std::memcpy(m_componentTypesList, componentTypesList, componentTypesCount);
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

}
