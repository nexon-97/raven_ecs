#pragma once
#include <cstdint>

namespace ecs
{

class EntitiesCollection;
struct ComponentHandle;

struct Entity
{
	uint32_t id;
	uint32_t parentId;
	uint32_t hierarchyDataOffset;
	uint32_t componentsMask;
	uint32_t componentsDataOffset;

	static EntitiesCollection* s_collection;
	static const uint32_t k_invalidId;

	void AddComponent(const ComponentHandle& handle);
	void RemoveComponent(const ComponentHandle& handle);
	bool HasComponent(const uint8_t componentType);
	void* GetComponent(const uint8_t componentType);
	Entity* GetParent();

	template <typename ComponentType>
	ComponentType* GetComponent() const
	{
		uint8_t componentTypeId = s_collection->GetComponentTypeIdByTypeIndex(typeid(ComponentType));
		return static_cast<ComponentType*>(GetComponent(componentTypeId));
	}
};

} // namespace ecs
