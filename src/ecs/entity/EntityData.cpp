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
	, orderInParent(std::numeric_limits<uint16_t>::max())
	, refCount(0U)
	, storageLocation(k_invalidStorageLocation)
{}

EntityData::EntityData(EntityData&& other) noexcept
	: id(other.id)
	, parentId(other.parentId)
	, components(std::move(other.components))
	, orderInParent(other.orderInParent)
	, refCount(other.refCount)
	, storageLocation(other.storageLocation)
	, children(std::move(other.children))
	, componentsMask(other.componentsMask)
	, name(std::move(other.name))
{
	other.id = Entity::GetInvalidId();
	other.parentId = Entity::GetInvalidId();
	other.refCount = 0U;
	other.components.clear();
	other.orderInParent = std::numeric_limits<uint16_t>::max();
	other.storageLocation = k_invalidStorageLocation;
	other.componentsMask.reset();
}

EntityData& EntityData::operator=(EntityData&& other) noexcept
{
	id = other.id;
	parentId = other.parentId;
	refCount = other.refCount;
	components = std::move(other.components);
	orderInParent = other.orderInParent;
	storageLocation = other.storageLocation;
	children = std::move(other.children);
	componentsMask = other.componentsMask;
	name = std::move(other.name);

	other.id = Entity::GetInvalidId();
	other.parentId = Entity::GetInvalidId();
	other.refCount = 0U;
	other.components.clear();
	other.componentsMask.reset();
	other.orderInParent = std::numeric_limits<uint16_t>::max();
	other.storageLocation = k_invalidStorageLocation;

	return *this;
}

} // namespace ecs
