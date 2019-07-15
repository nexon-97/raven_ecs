#include "ecs/component/ComponentHandle.hpp"
#include "ecs/Manager.hpp"

namespace
{
ecs::Manager* gManager = nullptr;
}

namespace ecs
{

ComponentHandle::ComponentHandle()
	: m_typeId(ComponentHandleInternal::GetInvalidTypeId())
	, m_handleIndexPtr(nullptr)
{}

ComponentHandle::ComponentHandle(const ComponentTypeId typeId, HandleIndex* handleIndexPtr)
	: m_typeId(typeId)
	, m_handleIndexPtr(handleIndexPtr)
{}

bool ComponentHandle::operator==(const ComponentHandle& other) const
{
	return m_typeId == other.m_typeId && m_handleIndexPtr == other.m_handleIndexPtr;
}

bool ComponentHandle::operator!=(const ComponentHandle& other) const
{
	return !(*this == other);
}

bool ComponentHandle::IsValid() const
{
	return (m_typeId != ComponentHandleInternal::GetInvalidTypeId());
}

ComponentTypeId ComponentHandle::GetTypeId() const
{
	return m_typeId;
}

ComponentHandle::HandleIndex ComponentHandle::GetOffset() const
{
	return *m_handleIndexPtr;
}

EntityId ComponentHandle::GetEntityId() const
{
	return gManager->GetComponentEntityId(*this);
}

void ComponentHandle::SetManagerInstance(ecs::Manager* manager)
{
	gManager = manager;
}

ecs::Manager* ComponentHandle::GetManagerInstance()
{
	return gManager;
}

bool ComponentHandle::IsOfTypeImpl(const std::type_index& typeIndex) const
{
	return gManager->GetComponentTypeIdByIndex(typeIndex) == m_typeId;
}

std::type_index ComponentHandle::GetStdTypeIndex() const
{
	return gManager->GetComponentTypeIndexByTypeId(m_typeId);
}

void* ComponentHandle::GetValue() const
{
	return gManager->GetComponent(*this);
}

} // namespace ecs
