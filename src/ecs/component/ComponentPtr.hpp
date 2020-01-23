#pragma once
#include "ecs/Manager.hpp"
#include <cstdint>

namespace ecs
{

struct ComponentPtrBlock
{
	int32_t typeId = 0;
	int32_t dataIndex = 0;
	int32_t refCount = 0;
};

class ComponentPtr
{
public:
	ComponentPtr() = default;
	ComponentPtr(ComponentPtrBlock* cblock)
		: m_block(cblock)
	{}

	ComponentPtr(const ComponentPtr& other)
		: m_block(other.m_block)
	{
		++m_block->refCount;
	}

	ComponentPtr& operator=(const ComponentPtr& other)
	{
		m_block = other.m_block;
		++m_block->refCount;

		return *this;
	}

	ComponentPtr(ComponentPtr&& other)
		: m_block(other.m_block)
	{
		other.m_block = nullptr;
	}

	ComponentPtr& operator=(ComponentPtr&& other)
	{
		m_block = other.m_block;
		other.m_block = nullptr;

		return *this;
	}

	~ComponentPtr()
	{
		if (nullptr != m_block && m_block->refCount == 1U)
		{
			Manager::Get()->ReleaseComponentV2(m_block->typeId, m_block->dataIndex);
		}
	}

	bool IsValid() const
	{
		return nullptr != m_block && m_block->refCount > 0;
	}

private:
	ComponentPtrBlock* m_block = nullptr;
};

template <class T>
class TComponentPtr
	: public ComponentPtr
{
public:
	TComponentPtr() = default;
	TComponentPtr(ComponentPtrBlock* cblock)
		: ComponentPtr(cblock)
	{}

	T* Get() const
	{
		if (IsValid())
		{
			return Manager::Get()->GetComponentV2<T>(m_block->typeId, m_block->dataIndex);
		}

		return nullptr;
	}
};

}
