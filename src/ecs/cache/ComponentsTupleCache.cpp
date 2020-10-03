#include "ecs/cache/ComponentsTupleCache.hpp"
#include "ecs/entity/Entity.hpp"
#include "ecs/Manager.hpp"
#include <cstring>
#include <algorithm>

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

void ComponentsTupleCache::TouchEntity(const EntityId entityId)
{
	EntityData* entityData = Manager::Get()->GetEntitiesCollection().GetEntityData(entityId);
	if (nullptr == entityData)
	{
		// If there is no alive entity with such id and it's still cached - remove cache entry
		auto it = m_componentTuples.find(entityId);
		if (it != m_componentTuples.end())
		{
			m_componentTuples.erase(it);
		}
	}
	else
	{
		// Check if given entity has all component to be in tuple
		bool hasAllComponents = true;
		for (std::size_t i = 0U; i < m_componentsCount; i++)
		{
			if (!entityData->componentsMask.test(m_componentTypesList[i]))
			{
				hasAllComponents = false;
				break;
			}
		}

		// Handle add/remove from tuple cache
		auto it = m_componentTuples.find(entityId);
		if (hasAllComponents)
		{
			if (it == m_componentTuples.end())
			{
				// Fill components tuple
				ComponentsTuple componentsTuple(m_componentsCount);

				for (std::size_t i = 0; i < m_componentsCount; i++)
				{
					ComponentTypeId typeId = m_componentTypesList[i];
					auto it = std::find_if(entityData->components.begin(), entityData->components.end(), [typeId](const ComponentPtr& component) {
						return component.GetTypeId() == typeId;
					});
					
					if (it != entityData->components.end())
					{
						componentsTuple[i] = *it;
					}
				}
				//entity.GetComponentsOfTypes(componentsTuple.GetMutableData(), m_componentTypesList, m_componentsCount);

				// Add to cache
				m_componentTuples.emplace(entityId, std::move(componentsTuple));
			}
		}
		else
		{
			// Not all components set -> try to remove entity from cache
			if (it != m_componentTuples.end())
			{
				m_componentTuples.erase(it);
			}
		}
	}
}

}
