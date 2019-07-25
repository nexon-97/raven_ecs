#include "ecs/entity/EntityData.hpp"
#include "ecs/entity/Entity.hpp"

namespace
{
const uint32_t k_invalidStorageLocation = uint32_t(-1);
const uint16_t k_invalidOrderInParent = uint16_t(-1);
}

namespace ecs
{

EntityData::EntityData()
	: id(Entity::GetInvalidId())
	, parentId(Entity::GetInvalidId())
	, componentsDataOffset(0U)
	, orderInParent(std::numeric_limits<uint16_t>::max())
	, refCount(0U)
	, isEnabled(true)
	, isActivated(false)
	, storageLocation(k_invalidStorageLocation)
{}

EntityData::EntityData(EntityData&& other) noexcept
	: id(other.id)
	, parentId(other.parentId)
	, componentsDataOffset(other.componentsDataOffset)
	, orderInParent(other.orderInParent)
	, refCount(other.refCount)
	, isEnabled(other.isEnabled)
	, isActivated(other.isActivated)
	, storageLocation(other.storageLocation)
	, children(std::move(other.children))
	, componentsMask(other.componentsMask)
{
	other.id = Entity::GetInvalidId();
	other.parentId = Entity::GetInvalidId();
	other.refCount = 0U;
	other.componentsDataOffset = 0U;
	other.orderInParent = std::numeric_limits<uint16_t>::max();
	other.isEnabled = true;
	other.isActivated = false;
	other.storageLocation = k_invalidStorageLocation;
	other.componentsMask.reset();
}

EntityData& EntityData::operator=(EntityData&& other) noexcept
{
	id = other.id;
	parentId = other.parentId;
	refCount = other.refCount;
	componentsDataOffset = other.componentsDataOffset;
	orderInParent = other.orderInParent;
	isEnabled = other.isEnabled;
	isActivated = other.isActivated;
	storageLocation = other.storageLocation;
	children = std::move(other.children);
	componentsMask = other.componentsMask;

	other.id = Entity::GetInvalidId();
	other.parentId = Entity::GetInvalidId();
	other.refCount = 0U;
	other.componentsDataOffset = 0U;
	other.componentsMask.reset();
	other.orderInParent = std::numeric_limits<uint16_t>::max();
	other.isEnabled = true;
	other.isActivated = false;
	other.storageLocation = k_invalidStorageLocation;

	return *this;
}

} // namespace ecs
