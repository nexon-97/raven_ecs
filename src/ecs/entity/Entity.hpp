#pragma once
#include "ecs/entity/EntityData.hpp"
#include "ecs/detail/Types.hpp"

#include <typeindex>

namespace ecs
{

class EntitiesCollection;
class Manager;

typedef void(*EntityChildAddedCallback)(Entity&, EntityId);
typedef void(*EntityChildRemovedCallback)(Entity&, EntityId);

/*
* @brief Entity is a wrapper around entity data, that gives the interface for manipuling the data using ecs infrastructure.
* Entity is a kind of reference couting wrapper, each entity adds to the alive references count.
* Entity can contain no entity data, it is a valid situation, but this object cannot be used to access the data, and the
* object can be tested using simple operator bool or IsValid() method.
*
* Entity has interface to manipulate entity components
* Entity has interface to manipulate its allowed properties
* User can copy and move entity instances
* User has no access to underlying entity data
*/
struct ECS_API Entity
{
	static const EntityId GetInvalidId();

	Entity();
	~Entity();

	bool IsValid() const;
	operator bool() const;

	void Reset();

	Entity Clone() const;

	void AddComponent(const ComponentPtr& handle);
	void RemoveComponent(const ComponentPtr& handle);
	bool HasComponent(const ComponentTypeId componentType) const;
	bool HasComponents(ComponentTypeId* componentTypes, const std::size_t count) const;
	ComponentPtr GetComponent(const ComponentTypeId componentType) const;
	const EntityComponentsContainer& GetComponents() const;
	void GetComponentsOfTypes(ComponentPtr* outComponents, ComponentTypeId* componentTypes, const std::size_t count) const;

	void AddChild(Entity& child);
	void RemoveChild(Entity& child);
	void ClearChildren();
	Entity& GetChildByIdx(const std::size_t idx) const;
	EntityChildrenContainer& GetChildren() const;
	std::size_t GetChildrenCount() const;
	Entity GetParent() const;
	uint16_t GetOrderInParent() const;

	EntityId GetId() const;

	void SetName(const std::string& name);
	void SetName(std::string&& name);
	const std::string& GetName() const;

	template <typename ComponentType>
	bool HasComponent() const
	{
		ComponentTypeId componentTypeId = GetComponentTypeIdByIndex(typeid(ComponentType));
		return HasComponent(componentTypeId);
	}

	template <typename ComponentType>
	TComponentPtr<ComponentType> GetComponent() const
	{
		ComponentTypeId componentTypeId = GetComponentTypeIdByIndex(typeid(ComponentType));
		return static_cast<TComponentPtr<ComponentType>>(GetComponent(componentTypeId));
	}

	// Copy is available
	Entity(const Entity&) noexcept;
	Entity& operator=(const Entity&) noexcept;

	// Move is available
	Entity(Entity&&) noexcept;
	Entity& operator=(Entity&&) noexcept;

	bool operator==(const Entity& other) const;
	bool operator!=(const Entity& other) const;

private:
	ComponentTypeId GetComponentTypeIdByIndex(const std::type_index& index) const;

	// Must be constructed only by EntitiesCollection class
	friend class EntitiesCollection;
	friend class EntityChildrenCollection;

	Entity(EntityData* data);

	void AddRef();
	void RemoveRef();

	EntityData* GetData() const;

private:
	EntityData* m_data = nullptr;
};

} // namespace ecs
