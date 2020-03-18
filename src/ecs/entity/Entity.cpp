#include "ecs/entity/Entity.hpp"
#include "ecs/entity/EntitiesCollection.hpp"
#include "ecs/Manager.hpp"

#include <functional>
#include <algorithm>

namespace
{
const uint32_t k_invalidStorageLocation = uint32_t(-1);
const uint16_t k_invalidOrderInParent = uint16_t(-1);
const ecs::EntityId k_invalidEntityId = std::numeric_limits<ecs::EntityId>::max();
const std::string k_emptyName;

struct FindComponentByTypePredicate
{
	ecs::ComponentTypeId typeId;

	FindComponentByTypePredicate(ecs::ComponentTypeId inTypeId)
		: typeId(inTypeId)
	{}

	bool operator()(const ecs::ComponentPtr& component)
	{
		return component.GetTypeId() == typeId;
	}
};
}

namespace ecs
{

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

	assert(handle.m_block->entityId == k_invalidEntityId);

	// Register component inside entity data
	m_data->components.push_back(handle);
	m_data->componentsMask.set(handle.GetTypeId());

	// Register entity id inside component handle
	handle.m_block->entityId = m_data->id;
	
	// Invoke global callback
	Manager::Get()->GetComponentAttachedDelegate().Broadcast(*this, handle);
}

void Entity::RemoveComponent(const ComponentPtr& handle)
{
	if (!handle.IsValid())
		return;

	if (handle.m_block->entityId != m_data->id)
	{
		// Trying to remove component from entity that it's not attached to
		// Put warning here
		return;
	}

	auto it = std::find(m_data->components.begin(), m_data->components.end(), handle);
	if (it != m_data->components.end())
	{
		m_data->components.erase(it);
		handle.m_block->entityId = k_invalidEntityId;
	}

	m_data->componentsMask.reset(handle.GetTypeId());

	// Invoke component detach delegate
	Manager::Get()->GetComponentDetachedDelegate().Broadcast(*this, handle);
}

bool Entity::HasComponent(const ComponentTypeId componentType) const
{
	if (componentType == Manager::GetInvalidComponentTypeId())
		return false;

	return m_data->componentsMask.test(componentType);
}

bool Entity::HasComponents(ComponentTypeId* componentTypes, const std::size_t count) const
{
	for (std::size_t i = 0U; i < count; ++i)
	{
		if (!m_data->componentsMask.test(componentTypes[i]))
		{
			return false;
		}
	}

	return true;
}

ComponentPtr Entity::GetComponent(const ComponentTypeId componentType) const
{
	if (HasComponent(componentType))
	{
		for (const ComponentPtr& component : m_data->components)
		{
			if (component.GetTypeId() == componentType)
			{
				return component;
			}
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

Entity Entity::Clone() const
{
	if (IsValid())
	{
		Entity clone = Manager::Get()->GetEntitiesCollection().CreateEntity();

		// Clone name
		clone.SetName(GetName());

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

const EntityComponentsContainer& Entity::GetComponents() const
{
	return m_data->components;
}

void Entity::GetComponentsOfTypes(ComponentPtr* outComponents, ComponentTypeId* componentTypes, const std::size_t count) const
{
	for (std::size_t i = 0; i < count; ++i)
	{
		if (HasComponent(componentTypes[i]))
		{
			FindComponentByTypePredicate predicate(componentTypes[i]);

			auto it = std::find_if(m_data->components.begin(), m_data->components.end(), predicate);
			if (it != m_data->components.end())
			{
				// Set component ptr value and go to next
				outComponents[i] = *it;
				continue;
			}
		}

		// Set empty component ptr
		outComponents[i] = ComponentPtr();
	}
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
	return k_invalidEntityId;
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

void Entity::SetName(const std::string& name)
{
	if (name.empty())
	{
		m_data->name.reset();
	}
	else
	{
		m_data->name = std::make_unique<std::string>(name);
	}
}

void Entity::SetName(std::string&& name)
{
	if (name.empty())
	{
		m_data->name.reset();
	}
	else
	{
		m_data->name = std::make_unique<std::string>(std::move(name));
	}
}

const std::string& Entity::GetName() const
{
	if (m_data->name)
	{
		return *m_data->name;
	}
	else
	{
		return k_emptyName;
	}
}

} // namespace ecs
