#include "ecs/Entity.hpp"
#include "ecs/ComponentHandle.hpp"
#include "ecs/EntitiesCollection.hpp"
#include "ecs/Manager.hpp"

namespace ecs
{

const EntityId k_invalidId = std::numeric_limits<EntityId>::max();
const HierarchyDepth k_invalidHierarchyDepth = std::numeric_limits<HierarchyDepth>::max();
Manager* Entity::s_ecsManager = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////////////

Entity::Entity()
	: id(Entity::GetInvalidId())
	, parentId(Entity::GetInvalidId())
	, hierarchyDataOffset(0U)
	, componentsDataOffset(0U)
	, hierarchyDepth(Entity::GetInvalidHierarchyDepth())
	, orderInParent(Entity::GetInvalidHierarchyDepth())
	, refCount(0U)
	, childrenCount(0U)
	, isEnabled(true)
	, isActivated(false)
{}

Entity::Entity(Entity&& other) noexcept
	: id(other.id)
	, parentId(other.parentId)
	, hierarchyDataOffset(other.hierarchyDataOffset)
	, componentsDataOffset(other.componentsDataOffset)
	, hierarchyDepth(other.hierarchyDepth)
	, orderInParent(other.orderInParent)
	, refCount(other.refCount)
	, childrenCount(other.childrenCount)
	, isEnabled(other.isEnabled)
	, isActivated(other.isActivated)
{
	other.id = Entity::GetInvalidId();
	other.parentId = Entity::GetInvalidId();
	other.refCount = 0U;
	other.hierarchyDataOffset = 0U;
	other.componentsDataOffset = 0U;
	other.hierarchyDepth = Entity::GetInvalidHierarchyDepth();
	other.orderInParent = Entity::GetInvalidHierarchyDepth();
	other.childrenCount = 0U;
	other.isEnabled = true;
	other.isActivated = false;
}

Entity& Entity::operator=(Entity&& other) noexcept
{
	id = other.id;
	parentId = other.parentId;
	refCount = other.refCount;
	hierarchyDataOffset = other.hierarchyDataOffset;
	componentsDataOffset = other.componentsDataOffset;
	hierarchyDepth = other.hierarchyDepth;
	orderInParent = other.orderInParent;
	childrenCount = other.childrenCount;
	isEnabled = other.isEnabled;
	isActivated = other.isActivated;

	other.id = Entity::GetInvalidId();
	other.parentId = Entity::GetInvalidId();
	other.refCount = 0U;
	other.hierarchyDataOffset = 0U;
	other.componentsDataOffset = 0U;
	other.hierarchyDepth = Entity::GetInvalidHierarchyDepth();
	other.orderInParent = Entity::GetInvalidHierarchyDepth();
	other.childrenCount = 0U;
	other.isEnabled = true;
	other.isActivated = false;

	return *this;
}

void Entity::AddComponent(const ComponentHandle& handle)
{
	if (handle.IsValid())
	{
		auto& entitiesCollection = s_ecsManager->GetEntitiesCollection();
		entitiesCollection.AddComponent(*this, handle);
	}
}

void Entity::RemoveComponent(const ComponentHandle& handle)
{
	if (handle.IsValid())
	{
		auto& entitiesCollection = s_ecsManager->GetEntitiesCollection();
		entitiesCollection.RemoveComponent(*this, handle);
	}
}

bool Entity::HasComponent(const ComponentTypeId componentType) const
{
	auto& entitiesCollection = s_ecsManager->GetEntitiesCollection();
	return entitiesCollection.HasComponent(*this, componentType);
}

ComponentHandle Entity::GetComponentHandle(const ComponentTypeId componentType) const
{
	auto& entitiesCollection = s_ecsManager->GetEntitiesCollection();
	return entitiesCollection.GetComponentHandle(*this, componentType);
}

void* Entity::DoGetComponentPtr(const ComponentHandle handle) const
{
	return s_ecsManager->GetComponent(handle);
}

EntityId Entity::GetId() const
{
	return id;
}

std::size_t Entity::GetChildrenCount() const
{
	return childrenCount;
}

EntityId Entity::GetParentId() const
{
	return parentId;
}

bool Entity::IsEnabled() const
{
	return isEnabled;
}

HierarchyDepth Entity::GetHierarchyDepth() const
{
	return hierarchyDepth;
}

void Entity::SetEnabled(const bool enable)
{
	if (enable != isEnabled)
	{
		isEnabled = enable;

		// Notify entity enable state changed
		s_ecsManager->GetEntitiesCollection().OnEntityEnabled(id, isEnabled);
	}
}

const EntityId Entity::GetInvalidId()
{
	return k_invalidId;
}

const HierarchyDepth Entity::GetInvalidHierarchyDepth()
{
	return k_invalidHierarchyDepth;
}

void Entity::AddRef()
{
	++refCount;
}

void Entity::RemoveRef()
{
	if (refCount > 0U)
	{
		--refCount;

		if (refCount == 0U)
		{
			s_ecsManager->GetEntitiesCollection().OnEntityDataDestroy(id);
		}
	}
}

void Entity::SetManagerInstance(Manager* manager)
{
	s_ecsManager = manager;
}

Manager* Entity::GetManagerInstance()
{
	return s_ecsManager;
}

} // namespace ecs
