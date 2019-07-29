#pragma once
#include "ecs/component/ComponentHandle.hpp"
#include "ecs/Manager.hpp"

namespace ecs
{

template <class T>
struct TypedComponentHandle
	: public ComponentHandle
{
public:
	TypedComponentHandle()
		: ComponentHandle()
	{}

	TypedComponentHandle(const ComponentHandle& genericHandle)
		: ComponentHandle(genericHandle)
	{
		if (GetStdTypeIndex() != typeid(T))
		{
			m_typeId = ComponentHandleInternal::GetInvalidTypeId();
			m_handleIndexPtr = nullptr;
		}
	}

	T* GetValue() const
	{
		return static_cast<T*>(ComponentHandle::GetValue());
	}
};

} // namespace ecs
