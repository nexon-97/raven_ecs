#pragma once
#include "ecs/detail/Types.hpp"
#include <cstdint>

namespace ecs
{

class ECS_API ComponentPtr
{
	friend class Manager;
	friend struct Entity;

public:
	ComponentPtr() = default;
	ComponentPtr(ComponentPtrBlock* cblock);
	~ComponentPtr();

	ComponentPtr(const ComponentPtr& other);
	ComponentPtr& operator=(const ComponentPtr& other);
	ComponentPtr(ComponentPtr&& other);
	ComponentPtr& operator=(ComponentPtr&& other);

	ComponentTypeId GetTypeId() const;
	Entity GetEntity() const;
	bool IsValid() const;

	std::size_t GetHash() const;

	bool operator==(const ComponentPtr& other) const;
	bool operator!=(const ComponentPtr& other) const;

protected:
	void* GetRawData() const;

private:
	ComponentPtrBlock* m_block = nullptr;
};

template <class T>
class TComponentPtr
	: public ComponentPtr
{
public:
	TComponentPtr() = default;

	TComponentPtr(const ComponentPtr& ptr)
		: ComponentPtr(ptr)
	{}

	TComponentPtr(ComponentPtrBlock* cblock)
		: ComponentPtr(cblock)
	{}

	T* Get() const
	{
		if (IsValid())
		{
			return static_cast<T*>(GetRawData());
		}

		return nullptr;
	}

	T* operator->() const
	{
		return static_cast<T*>(GetRawData());
	}
};

}

namespace std
{

template <>
struct hash<ecs::ComponentPtr>
{
	std::size_t operator()(const ecs::ComponentPtr& ptr) const
	{
		return ptr.GetHash();
	}
};

template <class T>
struct hash<ecs::TComponentPtr<T>>
{
	std::size_t operator()(const ecs::TComponentPtr<T>& ptr) const
	{
		return ptr.GetHash();
	}
};

}
