#pragma once

namespace ecs
{

// System is a class, that has lifecycle callbacks Init, Destroy and Update,
// and has integer priority, which is used to determine system's order inside systems collection
class ECS_API System
{
public:
	System() = default;
	explicit System(const int priority);
	virtual ~System() = default;

	void SetPriority(const int priority);
	int GetPriority() const;

	virtual void Init();
	virtual void Destroy();

	virtual void Update() = 0;

	bool operator<(const System&) const;

private:
	int m_priority = 100;
};

} // namespace ecs
