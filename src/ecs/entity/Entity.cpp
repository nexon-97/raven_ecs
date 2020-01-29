#include "ecs/entity/Entity.hpp"
#include "ecs/entity/EntitiesCollection.hpp"
#include "ecs/Manager.hpp"

#include <functional>

namespace
{
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

void Entity::AddComponent(const ComponentPtr& handle)
{
	if (!handle.IsValid())
		return;

	auto& mappingEntry = Manager::Get()->GetEntitiesCollection().CreateComponentMappingEntry(*m_data);

	mappingEntry.componentPtr = handle;
	m_data->componentsMask.set(handle.GetTypeId());
	mappingEntry.componentPtr.m_block->entityId = m_data->id;

	// Invoke global callback
	//if (nullptr != Manager::Get()->m_globalEntityComponentAddedCallback)
	//{
	//	std::invoke(Manager::Get()->m_globalEntityComponentAddedCallback, *this, handle);
	//}
}

void Entity::RemoveComponent(const ComponentPtr& handle)
{
	if (!handle.IsValid())
		return;

	ComponentTypeId componentType = handle.GetTypeId();
	if (HasComponent(componentType))
	{
		Manager::Get()->GetEntitiesCollection().RemoveComponentMappingEntry(*m_data, handle.GetTypeId());
		handle.m_block->entityId = Entity::GetInvalidId();

		m_data->componentsMask.reset(componentType);

		// Invoke global callback
		//if (nullptr != Manager::Get()->m_globalEntityComponentRemovedCallback)
		//{
		//	std::invoke(Manager::Get()->m_globalEntityComponentRemovedCallback, *this, handle);
		//}
	}
}

bool Entity::HasComponent(const ComponentTypeId componentType) const
{
	return m_data->componentsMask.test(componentType);
}

ComponentPtr Entity::GetComponent(const ComponentTypeId componentType) const
{
	if (HasComponent(componentType))
	{
		auto mappingEntry = Manager::Get()->GetEntitiesCollection().FindComponentMappingEntry(*m_data, componentType);
		if (nullptr != mappingEntry)
		{
			return mappingEntry->componentPtr;
		}
	}

	return ComponentPtr();
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
		return Manager::Get()->GetEntitiesCollection().GetEntityById(m_data->parentId);
	}

	return Entity();
}

uint16_t Entity::GetOrderInParent() const
{
	return m_data->orderInParent;
}

Entity Entity::Clone()
{
	if (IsValid())
	{
		Entity clone = Manager::Get()->GetEntitiesCollection().CreateEntity();

		// Clone components
		for (const ComponentPtr& componentPtr : GetComponents())
		{
			ComponentPtr componentClone = Manager::Get()->CloneComponent(componentPtr);
			clone.AddComponent(componentClone);
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
	auto& storage = Manager::Get()->GetEntitiesCollection().GetComponentsMapStorage();
	return EntityComponentsCollection(storage, m_data->componentsDataOffset);
}

void Entity::AddChild(Entity& child)
{
	EntityData* childData = child.GetData();
	childData->parentId = m_data->id;
	childData->orderInParent = static_cast<uint16_t>(m_data->children.size());

	m_data->children.push_back(child);

	// Invoke global callback
	//if (nullptr != Manager::Get()->m_globalEntityChildAddedCallback)
	//{
	//	std::invoke(Manager::Get()->m_globalEntityChildAddedCallback, *this, child.GetId());
	//}
}

void Entity::RemoveChild(Entity& child)
{
	auto it = std::find(m_data->children.begin(), m_data->children.end(), child);
	if (it != m_data->children.end())
	{
		EntityData* childData = child.GetData();
		childData->parentId = Entity::GetInvalidId();
		childData->orderInParent = k_invalidOrderInParent;

		// Invoke global callback
		//if (nullptr != Manager::Get()->m_globalEntityChildRemovedCallback)
		//{
		//	std::invoke(Manager::Get()->m_globalEntityChildRemovedCallback, *this, child.GetId());
		//}
	}
}

void Entity::ClearChildren()
{
	for (auto it = m_data->children.begin(); it != m_data->children.end();)
	{
		ecs::Entity child = *it;

		EntityData* childData = child.GetData();
		childData->parentId = Entity::GetInvalidId();
		childData->orderInParent = k_invalidOrderInParent;

		it = m_data->children.erase(it);

		// Invoke global callback
		//if (nullptr != Manager::Get()->m_globalEntityChildRemovedCallback)
		//{
		//	std::invoke(Manager::Get()->m_globalEntityChildRemovedCallback, *this, child.GetId());
		//}
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

		if (m_data->refCount == 0U && nullptr != Manager::Get())
		{
			Manager::Get()->GetEntitiesCollection().OnEntityDataDestroy(m_data->id);
		}
	}
}

EntityData* Entity::GetData() const
{
	return m_data;
}

ComponentTypeId Entity::GetComponentTypeIdByIndex(const std::type_index& index) const
{
	return Manager::Get()->GetComponentTypeIdByIndex(index);
}

} // namespace ecs
