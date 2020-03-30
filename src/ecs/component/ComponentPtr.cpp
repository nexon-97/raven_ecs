#include "ecs/component/ComponentPtr.hpp"
#include "ecs/Manager.hpp"

namespace ecs
{

ComponentPtrBlock::ComponentPtrBlock()
	: typeId(Manager::GetInvalidComponentTypeId())
	, dataIndex(-1)
	, entityId(Entity::GetInvalidId())
	, refCount(0)
{}

ComponentPtrBlock::ComponentPtrBlock(ComponentTypeId inTypeId, int32_t inDataIndex, EntityId inEntityId, int32_t inRefCount)
	: typeId(inTypeId)
	, dataIndex(inDataIndex)
	, entityId(inEntityId)
	, refCount(inRefCount)
{}

ComponentPtr::ComponentPtr(ComponentPtrBlock* cblock)
	: m_block(cblock)
{}

ComponentPtr::ComponentPtr(const ComponentPtr& other)
	: m_block(other.m_block)
{
	if (nullptr != m_block)
	{
		++m_block->refCount;
	}
}

ComponentPtr& ComponentPtr::operator=(const ComponentPtr& other)
{
	m_block = other.m_block;

	if (nullptr != m_block)
	{
		++m_block->refCount;
	}

	return *this;
}

ComponentPtr::ComponentPtr(ComponentPtr&& other)
	: m_block(other.m_block)
{
	other.m_block = nullptr;
}

ComponentPtr& ComponentPtr::operator=(ComponentPtr&& other)
{
	m_block = other.m_block;
	other.m_block = nullptr;

	return *this;
}

ComponentPtr::~ComponentPtr()
{
	Reset();
}

void ComponentPtr::Reset()
{
	Manager* manager = Manager::Get();
	if (nullptr != manager && nullptr != m_block && m_block->refCount == 1U)
	{
		manager->ReleaseComponent(m_block->typeId, m_block->dataIndex);
	}

	m_block = nullptr;
}

bool ComponentPtr::IsValid() const
{
	return nullptr != m_block && m_block->refCount > 0;
}

ComponentTypeId ComponentPtr::GetTypeId() const
{
	if (IsValid())
	{
		return m_block->typeId;
	}

	return Manager::Get()->GetInvalidComponentTypeId();
}

Entity ComponentPtr::GetEntity() const
{
	if (IsValid())
	{
		return Manager::Get()->GetEntityById(m_block->entityId);
	}

	return Entity();
}

void* ComponentPtr::GetRawData() const
{
	return Manager::Get()->GetComponentRaw(m_block->typeId, m_block->dataIndex);
}

ComponentTypeId ComponentPtr::TypeIndexToTypeId(const std::type_index& typeIndex)
{
	return Manager::Get()->GetComponentTypeIdByIndex(typeIndex);
}

ComponentPtr ComponentPtr::GetSibling(const ComponentTypeId componentType) const
{
	return GetEntity().GetComponent(componentType);
}

std::size_t ComponentPtr::GetHash() const
{
	return std::hash<ComponentPtrBlock*>()(m_block);
}

bool ComponentPtr::operator==(const ComponentPtr& other) const
{
	return m_block == other.m_block;
}

bool ComponentPtr::operator!=(const ComponentPtr& other) const
{
	return !(*this == other);
}

ComponentPtr::operator bool() const
{
	return IsValid();
}

}
