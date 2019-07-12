#include "ecs/entity/EntityData.hpp"
#include "ecs/entity/Entity.hpp"

namespace
{
const uint32_t k_invalidStorageLocation = uint32_t(-1);
}

namespace ecs
{

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
	, storageLocation(k_invalidStorageLocation)
{}

EntityData::~EntityData()
{

}

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
	, storageLocation(other.storageLocation)
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
	other.storageLocation = k_invalidStorageLocation;
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
	storageLocation = other.storageLocation;

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
	other.storageLocation = k_invalidStorageLocation;

	return *this;
}

} // namespace ecs
