#pragma once
#include "ecs/ECSApiDef.hpp"
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

	static const uint32_t ECS_API GetInvalidId();

	//void ECS_API AddComponent(const ComponentHandle& handle);
	//void ECS_API RemoveComponent(const ComponentHandle& handle);
	//bool ECS_API HasComponent(const uint8_t componentType);
	//ECS_API void* GetComponent(const uint8_t componentType);
	//ECS_API Entity* GetParent();

	//template <typename ComponentType>
	//ComponentType* GetComponent() const
	//{
	//	uint8_t componentTypeId = s_collection->GetComponentTypeIdByTypeIndex(typeid(ComponentType));
	//	return static_cast<ComponentType*>(GetComponent(componentTypeId));
	//}
};

} // namespace ecs
