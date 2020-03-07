#pragma once
#include "ecs/detail/Types.hpp"
#include <cstdint>
#include <typeindex>

namespace ecs
{

// Forward declarations
class ComponentPtr;
template <class T>
class TComponentPtr;
template <class T>
TComponentPtr<T> Cast(const ComponentPtr& component);

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
	ComponentPtr GetSibling(const ComponentTypeId componentType) const;

	template <typename SiblingT>
	TComponentPtr<SiblingT> GetSibling() const
	{
		return Cast<SiblingT>(GetSibling(TypeIndexToTypeId(typeid(SiblingT))));
	}

	std::size_t GetHash() const;

	bool operator==(const ComponentPtr& other) const;
	bool operator!=(const ComponentPtr& other) const;
	operator bool() const;

protected:
	void* GetRawData() const;
	ComponentTypeId TypeIndexToTypeId(const std::type_index& typeIndex) const;

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

template <class T>
TComponentPtr<T> Cast(const ComponentPtr& component)
{
	if (component.GetTypeId() == Manager::Get()->GetComponentTypeId<T>())
	{
		return TComponentPtr<T>(component);
	}

	return TComponentPtr<T>();
}

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
