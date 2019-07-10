#include "ecs/Entity.hpp"
#include "ecs/ComponentHandle.hpp"
#include "ecs/EntitiesCollection.hpp"
#include "ecs/Manager.hpp"

namespace ecs
{

const EntityId k_invalidId = std::numeric_limits<EntityId>::max();
const HierarchyDepth k_invalidHierarchyDepth = std::numeric_limits<HierarchyDepth>::max();
Manager* EntityData::s_ecsManager = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////////////

Entity::Entity(EntityData* data)
	: data(data)
{
	assert(nullptr != data);
	data->AddRef();
}

Entity::~Entity()
{
	data->RemoveRef();
}

Entity::Entity(Entity&& other) noexcept
	: data(other.data)
{
	other.data = nullptr;

	assert(nullptr != data);
}

Entity& Entity::operator=(Entity&& other) noexcept
{
	data = other.data;
	other.data = nullptr;

	assert(nullptr != data);

	return *this;
}

void Entity::AddComponent(const ComponentHandle& handle)
{
	if (handle.IsValid())
	{
		auto& entitiesCollection = EntityData::GetManagerInstance()->GetEntitiesCollection();
		entitiesCollection.AddComponent(*data, handle);
	}
}

void Entity::RemoveComponent(const ComponentHandle& handle)
{
	if (handle.IsValid())
	{
		auto& entitiesCollection = EntityData::GetManagerInstance()->GetEntitiesCollection();
		entitiesCollection.RemoveComponent(*data, handle);
	}
}

bool Entity::HasComponent(const ComponentTypeId componentType) const
{
	auto& entitiesCollection = EntityData::GetManagerInstance()->GetEntitiesCollection();
	return entitiesCollection.HasComponent(*data, componentType);
}

ComponentHandle Entity::GetComponentHandle(const ComponentTypeId componentType) const
{
	auto& entitiesCollection = EntityData::GetManagerInstance()->GetEntitiesCollection();
	return entitiesCollection.GetComponentHandle(*data, componentType);
}

void* Entity::DoGetComponentPtr(const ComponentHandle handle) const
{
	return EntityData::GetManagerInstance()->GetComponent(handle);
}

EntityId Entity::GetId() const
{
	return data->id;
}

std::size_t Entity::GetChildrenCount() const
{
	return data->childrenCount;
}

EntityId Entity::GetParentId() const
{
	return data->parentId;
}

bool Entity::IsEnabled() const
{
	return data->isEnabled;
}

void Entity::SetEnabled(const bool enable)
{
	if (enable != data->isEnabled)
	{
		data->isEnabled = enable;

		// Notify entity enable state changed
		EntityData::GetManagerInstance()->GetEntitiesCollection().OnEntityEnabled(data->id, data->isEnabled);
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

/////////////////////////////////////////////////////////////////////////////////////////////////

EntityData::EntityData()
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

EntityData::EntityData(EntityData&& other) noexcept
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

EntityData& EntityData::operator=(EntityData&& other) noexcept
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

void EntityData::AddRef()
{
	++refCount;
}

void EntityData::RemoveRef()
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

void EntityData::SetManagerInstance(Manager* manager)
{
	s_ecsManager = manager;
}

Manager* EntityData::GetManagerInstance()
{
	return s_ecsManager;
}

} // namespace ecs
