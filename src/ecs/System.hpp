#pragma once
#include "raven_ecs_export.h"
#include <vector>
#include <typeindex>

namespace ecs
{

// System is a class, that has lifecycle callbacks Init, Destroy and Update,
// and has integer priority, which is used to determine system's order inside systems collection
class System
{
	friend class Manager;

public:
	System() = default;
	explicit ECS_API System(const int priority);
	virtual ~System() = default;

	void ECS_API SetPriority(const int priority);
	int ECS_API GetPriority() const;

	virtual void ECS_API Init();
	virtual void ECS_API Destroy();

	virtual void Update() = 0;

	// Less operator compares priorities
	bool ECS_API operator<(const System&) const;

	// System update thread safety indicator (if system is thread safe, it can be run on any thread other than main)
	void ECS_API MarkUpdateThreadSafe(bool isThreadSafe);
	bool ECS_API IsUpdateThreadSafe() const;

	// Update dependencies management (for thread safe systems)
	void ECS_API AddUpdateDependency(const std::type_index& systemTypeIndex);

	template <class SystemT>
	void AddUpdateDependency()
	{
		static_assert(std::is_base_of_v<System, SystemT>, "Update dependecy must be a system derived class!");
		AddUpdateDependency(typeid(SystemT));
	}

	ECS_API const std::vector<std::type_index>& GetUpdateDependenciesList() const;

private:
	void DispatchInit();
	void DispatchDestroy();

private:
	int m_priority = 100;
	std::vector<std::type_index> m_updateDependencies;
	bool m_hasBeenInitialized = false;
	bool m_updateThreadSafe = false;
};

} // namespace ecs
