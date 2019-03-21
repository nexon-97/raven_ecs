#include "ecs/EntityHierarchyManager.hpp"
#include "ecs/Manager.hpp"

#include <algorithm>

// Undefine max to avoid macro collisions
#undef max

namespace ecs
{

Manager* g_ecsManager = nullptr;
EntitiesCollection* g_entitiesCollection = nullptr;
EntityHierarchyManager* g_hierarchyManager = nullptr;

///////////////////////////////////////////////////////////////////////////////////////

struct EntitiesComparator
{
	bool operator()(const EntityId lhsId, const EntityId rhsId) const
	{
		auto& lhs = g_entitiesCollection->GetEntity(lhsId);
		auto& rhs = g_entitiesCollection->GetEntity(rhsId);
		return g_hierarchyManager->CompareEntitiesInHierarchy(lhs, rhs);
	}
};
EntitiesComparator g_comparator;

///////////////////////////////////////////////////////////////////////////////////////

EntityHierarchyManager::EntityHierarchyManager(Manager* manager)
{
	g_hierarchyManager = this;
	g_ecsManager = manager;
	g_entitiesCollection = &g_ecsManager->GetEntitiesCollection();
}

bool EntityHierarchyManager::CompareEntitiesInHierarchy(const Entity& lhs, const Entity& rhs) const
{
	if (&lhs == &rhs)
		return false;

	assert(lhs.hierarchyDepth != Entity::GetInvalidHierarchyDepth());
	assert(rhs.hierarchyDepth != Entity::GetInvalidHierarchyDepth());

	Entity const* parents[2]{ &lhs, &rhs };
	bool isReverse = lhs.hierarchyDepth < rhs.hierarchyDepth;
	int depthDifference = abs(static_cast<int>(lhs.hierarchyDepth) - static_cast<int>(rhs.hierarchyDepth));

	if (isReverse)
	{
		std::swap(parents[0], parents[1]);
	}

	// Downgrade one entity, when hierarchy depth is different
	if (depthDifference != 0)
	{
		for (int i = 0; i < depthDifference; ++i)
		{	
			parents[0] = g_entitiesCollection->GetParent(parents[0]->id);
		}

		// Check for the parents equality
		// If parents equal, and depth was different, one is the parent of the another,
		// so should perform different comparison logic
		if (parents[0] == parents[1])
		{
			//LOG_INFO("LessThanInEntityHierarchy call [0x%X] (%s) against [0x%X] (%s): %s", this, GetName().c_str(), e, e->GetName().c_str(), isReverse ? "true" : "false");
			return isReverse;
		}
	}

	// Revert parent references back
	if (isReverse)
	{
		std::swap(parents[0], parents[1]); // Now this depth > other depth
	}

	// Entity levels must be equal depth here
	assert(parents[0]->hierarchyDepth == parents[1]->hierarchyDepth);

	int depth = parents[0]->hierarchyDepth;
	while (depth >= 0)
	{
		auto parent1 = g_entitiesCollection->GetParent(parents[0]->id);
		auto parent2 = g_entitiesCollection->GetParent(parents[1]->id);

		assert(nullptr != parent1 && nullptr != parent2);

		if (parent1 == parent2)
		{
			// Equal parents, compare order
			bool result = parents[0]->orderInParent < parents[1]->orderInParent;
			return result;
		}

		parents[0] = parent1;
		parents[1] = parent2;
		--depth;
	}

	//LOG_ERROR("Failed to compare entities in hierarchy");
	return &lhs < &rhs;
}

int EntityHierarchyManager::GetHierarchyOrderDiff(const EntityId lhsId, const EntityId rhsId) const
{
	auto lhsIt = std::lower_bound(m_hierarchyOrder.begin(), m_hierarchyOrder.end(), lhsId, g_comparator);
	auto rhsIt = std::lower_bound(m_hierarchyOrder.begin(), m_hierarchyOrder.end(), rhsId, g_comparator);
	assert(lhsIt != m_hierarchyOrder.end());
	assert(rhsIt != m_hierarchyOrder.end());
	
	return static_cast<int>(rhsIt - lhsIt);
}

void EntityHierarchyManager::GetEntitiesCountInBranchInternal(const EntityId rootEntityId, std::size_t& result) const
{
	result += g_entitiesCollection->GetChildrenCount(rootEntityId);
	for (const auto& child : g_entitiesCollection->GetChildrenData(rootEntityId))
	{
		GetEntitiesCountInBranchInternal(child.id, result);
	}
}

void EntityHierarchyManager::GetActiveEntitiesCountInBranchInternal(const EntityId rootEntityId, std::size_t& result) const
{
	if (g_entitiesCollection->IsEntityActivated(rootEntityId))
	{
		++result;

		for (const auto& child : g_entitiesCollection->GetChildrenData(rootEntityId))
		{
			GetActiveEntitiesCountInBranchInternal(child.id, result);
		}
	}
}

std::size_t EntityHierarchyManager::GetEntitiesCountInBranch(const EntityId rootEntityId) const
{
	std::size_t result;
	GetEntitiesCountInBranchInternal(rootEntityId, result);

	return result;
}

std::size_t EntityHierarchyManager::GetActiveEntitiesCountInBranch(const EntityId rootEntityId) const
{
	std::size_t result;
	GetActiveEntitiesCountInBranchInternal(rootEntityId, result);

	return result;
}

void EntityHierarchyManager::AddEntity(const EntityId id)
{
	auto it = std::lower_bound(m_hierarchyOrder.begin(), m_hierarchyOrder.end(), id, g_comparator);
	if (it == m_hierarchyOrder.end())
	{
		m_hierarchyOrder.insert(it, id);
	}
}

void EntityHierarchyManager::RemoveEntity(const EntityId id)
{
	auto it = std::find(m_hierarchyOrder.begin(), m_hierarchyOrder.end(), id);
	if (it != m_hierarchyOrder.end())
	{
		m_hierarchyOrder.erase(it);
	}
}

} // namespace ecs
