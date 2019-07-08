#pragma once
#include "ECSApiDef.hpp"

namespace ecs
{

class Manager;

// System is a class, that has lifecycle callbacks Init, Destroy and Update,
// and has integer priority, which is used to determine system's order inside systems collection
class System
{
public:
	System() = delete;
	explicit ECS_API System(Manager& manager, const int priority = 100);
	virtual ~System() = default;

	void ECS_API SetPriority(const int priority);
	int ECS_API GetPriority() const;

	virtual void ECS_API Init();
	virtual void ECS_API Destroy();

	virtual void Update() = 0;

	bool ECS_API operator<(const System&) const;

protected:
	Manager& m_ecsManager;

private:
	int m_priority = 100;
};

} // namespace ecs
