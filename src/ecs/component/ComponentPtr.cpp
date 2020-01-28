#include "ecs/component/ComponentPtr.hpp"
#include "ecs/Manager.hpp"

namespace ecs
{

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
	if (nullptr != m_block && m_block->refCount == 1U)
	{
		Manager::Get()->ReleaseComponent(m_block->typeId, m_block->dataIndex);
	}
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

	return -1;
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

}
