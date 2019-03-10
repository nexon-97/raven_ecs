#pragma once
#include "ILifecycleCallback.hpp"

namespace ecs
{

class Manager;

// Dummy class, which should be implemented
class System
	: public ILifecycleCallback
{
public:
	System() = delete;
	explicit System(Manager& manager)
		: m_ecsManager(manager)
	{}
	virtual ~System() = default;

protected:
	Manager& m_ecsManager;
};

} // namespace ecs
