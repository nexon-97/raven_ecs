#include "ecs/entity/Entity.hpp"
#include "ecs/entity/EntitiesCollection.hpp"
#include "ecs/Manager.hpp"

namespace
{
ecs::Manager* s_ecsManager = nullptr;
}

namespace ecs
{

const EntityId k_invalidId = std::numeric_limits<EntityId>::max();
const HierarchyDepth k_invalidHierarchyDepth = std::numeric_limits<HierarchyDepth>::max();

/////////////////////////////////////////////////////////////////////////////////////////////////

Entity::Entity()
	: m_data(nullptr)
{}

Entity::Entity(EntityData* data)
	: m_data(data)
{
	if (IsValid())
	{
		AddRef();
	}
}

Entity::~Entity()
{
	if (IsValid())
	{
		RemoveRef();
		m_data = nullptr;
	}
}

Entity::Entity(const Entity& other) noexcept
	: m_data(other.m_data)
{
	if (IsValid())
	{
		AddRef();
	}
}

Entity& Entity::operator=(const Entity& other) noexcept
{
	m_data = other.m_data;

	if (IsValid())
	{
		AddRef();
	}

	return *this;
}

Entity::Entity(Entity&& other) noexcept
	: m_data(other.m_data)
{
	other.m_data = nullptr;
}

Entity& Entity::operator=(Entity&& other) noexcept
{
	m_data = other.m_data;
	other.m_data = nullptr;

	return *this;
}

void Entity::Reset()
{
	if (IsValid())
	{
		RemoveRef();
		m_data = nullptr;
	}
}

bool Entity::IsValid() const
{
	return nullptr != m_data;
}

Entity::operator bool() const
{
	return IsValid();
}

void Entity::AddComponent(const ComponentHandle& handle)
{
	if (!handle.IsValid())
		return;

	auto& mappingEntry = s_ecsManager->GetEntitiesCollection().CreateComponentMappingEntry(*m_data);

	mappingEntry.handle = handle;
	m_data->componentsMask.set(handle.GetTypeId());
	s_ecsManager->SetComponentEntityId(handle, m_data->id);

	s_ecsManager->RefreshComponentActivation(handle, m_data->isEnabled, m_data->isActivated);
}

void Entity::RemoveComponent(const ComponentHandle& handle)
{
	if (!handle.IsValid())
		return;

	ComponentTypeId componentType = handle.GetTypeId();
	if (HasComponent(componentType))
	{
		s_ecsManager->GetEntitiesCollection().RemoveComponentMappingEntry(*m_data, handle.GetTypeId());
		s_ecsManager->SetComponentEntityId(handle, Entity::GetInvalidId());
	}
}

bool Entity::HasComponent(const ComponentTypeId componentType) const
{
	return m_data->componentsMask.test(componentType);
}

ComponentHandle Entity::GetComponentHandle(const ComponentTypeId componentType) const
{
	if (HasComponent(componentType))
	{
		auto mappingEntry = s_ecsManager->GetEntitiesCollection().FindComponentMappingEntry(*m_data, componentType);
		if (nullptr != mappingEntry)
		{
			return mappingEntry->handle;
		}
	}

	return ComponentHandle();
}

void* Entity::DoGetComponentPtr(const ComponentHandle handle) const
{
	return s_ecsManager->GetComponent(handle);
}

EntityId Entity::GetId() const
{
	if (nullptr != m_data)
	{
		return m_data->id;
	}
	
	return Entity::GetInvalidId();
}

std::size_t Entity::GetChildrenCount() const
{
	return m_data->childrenCount;
}

Entity Entity::GetParent() const
{
	if (Entity::GetInvalidId() != m_data->parentId)
	{
		return GetManagerInstance()->GetEntitiesCollection().GetEntityById(m_data->parentId);
	}

	return Entity();
}

bool Entity::IsEnabled() const
{
	return m_data->isEnabled;
}

HierarchyDepth Entity::GetHierarchyDepth() const
{
	return m_data->hierarchyDepth;
}

void Entity::SetEnabled(const bool enable)
{
	if (enable != m_data->isEnabled)
	{
		m_data->isEnabled = enable;

		// Notify entity enable state changed
		s_ecsManager->GetEntitiesCollection().OnEntityEnabled(m_data->id, m_data->isEnabled);
	}
}

Entity Entity::Clone()
{
	if (IsValid())
	{
		return s_ecsManager->GetEntitiesCollection().CloneEntity(*this);
	}
	
	return Entity();
}

EntityComponentsCollection Entity::GetComponents() const
{
	auto& storage = s_ecsManager->GetEntitiesCollection().GetComponentsMapStorage();
	return EntityComponentsCollection(storage, m_data->componentsDataOffset);
}

void Entity::AddChild(Entity& child)
{
	auto& entitiesCollection = s_ecsManager->GetEntitiesCollection();

	EntityHierarchyData& childEntry = entitiesCollection.CreateEntityHierarchyDataEntry(*m_data);
	childEntry.childId = child.GetId();

	child.GetData()->parentId = m_data->id;
	child.GetData()->orderInParent = m_data->childrenCount;
	++m_data->childrenCount;

	entitiesCollection.RefreshHierarchyDepth(*child.GetData(), m_data->id, false);
	entitiesCollection.RefreshActivation(*m_data);
}

void Entity::RemoveChild(Entity& child)
{
	s_ecsManager->GetEntitiesCollection().RemoveEntityHierarchyDataEntry(*m_data, child);
}

void Entity::ClearChildren()
{
	auto& entitiesCollection = s_ecsManager->GetEntitiesCollection();

	for (ecs::Entity& child : GetChildren())
	{
		EntityData& childData = *child.GetData();
		childData.orderInParent = Entity::GetInvalidHierarchyDepth();

		entitiesCollection.RefreshHierarchyDepth(childData, Entity::GetInvalidId(), false);
		entitiesCollection.RefreshActivation(childData);
	}

	m_data->childrenCount = 0;

	EntityHierarchyDataStorageType& data = s_ecsManager->GetEntitiesCollection().GetEntityHierarchyData();
	EntityHierarchyData* hierarchyDataEntry = data[m_data->hierarchyDataOffset];
	*hierarchyDataEntry = EntityHierarchyData();
}

Entity Entity::GetChildByIdx(const std::size_t idx) const
{
	EntityChildrenCollection collection = GetChildren();
	return collection[idx];
}

EntityChildrenCollection Entity::GetChildren() const
{
	EntityHierarchyDataStorageType& data = s_ecsManager->GetEntitiesCollection().GetEntityHierarchyData();
	return EntityChildrenCollection(data, m_data->hierarchyDataOffset);
}

bool Entity::operator==(const Entity& other) const
{
	return GetId() == other.GetId();
}

bool Entity::operator!=(const Entity& other) const
{
	return !(*this == other);
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
	++m_data->refCount;
}

void Entity::RemoveRef()
{
	if (m_data->refCount > 0U)
	{
		--m_data->refCount;

		if (m_data->refCount == 0U)
		{
			s_ecsManager->GetEntitiesCollection().OnEntityDataDestroy(m_data->id);
		}
	}
}

EntityData* Entity::GetData() const
{
	return m_data;
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
