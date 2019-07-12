#include "ecs/entity/Entity.hpp"
#include "ecs/entity/EntitiesCollection.hpp"
#include "ecs/Manager.hpp"

namespace ecs
{

const EntityId k_invalidId = std::numeric_limits<EntityId>::max();
const HierarchyDepth k_invalidHierarchyDepth = std::numeric_limits<HierarchyDepth>::max();
Manager* Entity::s_ecsManager = nullptr;

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
	return m_data->id;
}

std::size_t Entity::GetChildrenCount() const
{
	return m_data->childrenCount;
}

EntityId Entity::GetParentId() const
{
	return m_data->parentId;
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
