#pragma once

namespace ecs
{

class Manager;

// Dummy class, which should be implemented
class System
{
public:
	System() = delete;
	explicit System(Manager& manager)
		: m_ecsManager(manager)
	{}
	virtual ~System() = default;

	virtual void Init() {};
	virtual void Destroy() {};
	virtual void Update() = 0;

protected:
	Manager& m_ecsManager;
};

} // namespace ecs
