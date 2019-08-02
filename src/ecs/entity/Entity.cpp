#include "ecs/entity/Entity.hpp"
#include "ecs/entity/EntitiesCollection.hpp"
#include "ecs/Manager.hpp"

namespace
{
ecs::Manager* s_ecsManager = nullptr;
const uint32_t k_invalidStorageLocation = uint32_t(-1);
const uint16_t k_invalidOrderInParent = uint16_t(-1);
}

namespace ecs
{

const EntityId k_invalidId = std::numeric_limits<EntityId>::max();

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

		m_data->componentsMask.reset(componentType);
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
	return m_data->children.size();
}

Entity Entity::GetParent() const
{
	if (Entity::GetInvalidId() != m_data->parentId)
	{
		return GetManagerInstance()->GetEntitiesCollection().GetEntityById(m_data->parentId);
	}

	return Entity();
}

uint16_t Entity::GetOrderInParent() const
{
	return m_data->orderInParent;
}

bool Entity::IsEnabled() const
{
	return m_data->isEnabled;
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
		Entity clone = s_ecsManager->GetEntitiesCollection().CreateEntity();

		// Clone components
		for (const ecs::ComponentHandle& componentHandle : GetComponents())
		{
			ecs::ComponentHandle componentCloneHandle = s_ecsManager->CloneComponent(componentHandle);
			clone.AddComponent(componentCloneHandle);
		}

		// Clone children
		for (ecs::Entity& child : GetChildren())
		{
			ecs::Entity childClone = child.Clone();
			clone.AddChild(childClone);
		}

		return clone;
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

	EntityData* childData = child.GetData();
	childData->parentId = m_data->id;
	childData->orderInParent = static_cast<uint16_t>(m_data->children.size());

	m_data->children.push_back(child);

	entitiesCollection.RefreshActivation(*childData);
}

void Entity::RemoveChild(Entity& child)
{
	auto it = std::find(m_data->children.begin(), m_data->children.end(), child);
	if (it != m_data->children.end())
	{
		EntityData* childData = child.GetData();
		childData->parentId = Entity::GetInvalidId();
		childData->orderInParent = k_invalidOrderInParent;

		auto& entitiesCollection = s_ecsManager->GetEntitiesCollection();
		entitiesCollection.RefreshActivation(*childData);
	}
}

void Entity::ClearChildren()
{
	auto& entitiesCollection = s_ecsManager->GetEntitiesCollection();

	for (auto it = m_data->children.begin(); it != m_data->children.end();)
	{
		ecs::Entity child = *it;

		EntityData* childData = child.GetData();
		childData->parentId = Entity::GetInvalidId();
		childData->orderInParent = k_invalidOrderInParent;

		entitiesCollection.RefreshActivation(*childData);

		it = m_data->children.erase(it);
	}
}

Entity& Entity::GetChildByIdx(const std::size_t idx) const
{
	std::size_t i = 0;
	for (Entity& entity : m_data->children)
	{
		if (i == idx)
			return entity;

		++i;
	}

	static Entity k_empty;
	return k_empty;
}

EntityChildrenContainer& Entity::GetChildren() const
{
	return m_data->children;
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
