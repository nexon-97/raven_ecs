#pragma once
#include "ecs/entity/Entity.hpp"
#include <set>
#include <vector>

namespace ecs
{

//class Manager;
//
//class EntityHierarchyManager
//{
//public:
//	explicit EntityHierarchyManager(Manager* manager);
//
//	// Compares two entities in hierarchy (make sure they are in the same hierarchy tree, otherwise it will throw an assertion failure)
//	//bool CompareEntitiesInHierarchy(const Entity& lhs, const Entity& rhs) const;
//
//	// Method to count child entities in some entity (including the root entity)
//	//std::size_t GetEntitiesCountInBranch(const EntityId rootEntityId) const;
//	// Method to count only activated child entities in some entity (including the root entity)
//	//std::size_t GetActiveEntitiesCountInBranch(const EntityId rootEntityId) const;
//
//	int GetHierarchyOrderDiff(const EntityId lhsId, const EntityId rhsId) const;
//
//	void AddEntity(const EntityId id);
//	void RemoveEntity(const EntityId id);
//
//private:
//	void GetEntitiesCountInBranchInternal(const EntityId rootEntityId, std::size_t& result) const;
//	void GetActiveEntitiesCountInBranchInternal(const EntityId rootEntityId, std::size_t& result) const;
//
//private:
//	std::vector<EntityId> m_hierarchyOrder;
//};

} // namespace ecs