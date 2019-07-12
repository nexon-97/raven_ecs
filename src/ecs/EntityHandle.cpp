#include "ecs/EntityHandle.hpp"
#include "ecs/Manager.hpp"

namespace
{
ecs::Manager* gManager = nullptr;
}

namespace ecs
{

EntityHandle::EntityHandle() noexcept
	: m_handleIndexPtr(nullptr)
{}

EntityHandle::~EntityHandle() noexcept
{
	if (IsValid())
	{
		Entity* entityData = gManager->GetEntitiesCollection().GetEntity(*this);
		entityData->RemoveRef();
	}
}

EntityHandle::EntityHandle(HandleIndex* handleIndexPtr) noexcept
	: m_handleIndexPtr(handleIndexPtr)
{
	if (IsValid())
	{
		Entity* entityData = gManager->GetEntitiesCollection().GetEntity(*this);
		entityData->AddRef();
	}
}

EntityHandle::EntityHandle(const EntityHandle& other) noexcept
	: m_handleIndexPtr(other.m_handleIndexPtr)
{
	if (IsValid())
	{
		Entity* entityData = gManager->GetEntitiesCollection().GetEntity(*this);
		entityData->AddRef();
	}
}

EntityHandle& EntityHandle::operator=(const EntityHandle& other) noexcept
{
	m_handleIndexPtr = other.m_handleIndexPtr;

	return *this;
}

EntityHandle::EntityHandle(EntityHandle&& other) noexcept
	: m_handleIndexPtr(other.m_handleIndexPtr)
{
	other.m_handleIndexPtr = nullptr;
}

EntityHandle& EntityHandle::operator=(EntityHandle&& other) noexcept
{
	m_handleIndexPtr = other.m_handleIndexPtr;
	other.m_handleIndexPtr = nullptr;

	return *this;
}

bool EntityHandle::operator==(const EntityHandle& other) const
{
	return m_handleIndexPtr == other.m_handleIndexPtr;
}

bool EntityHandle::operator!=(const EntityHandle& other) const
{
	return !(*this == other);
}

bool EntityHandle::IsValid() const
{
	return nullptr != m_handleIndexPtr;
}

EntityHandle::HandleIndex EntityHandle::GetIndex() const
{
	return *m_handleIndexPtr;
}

void EntityHandle::SetManagerInstance(ecs::Manager* manager)
{
	gManager = manager;
}

} // namespace ecs
