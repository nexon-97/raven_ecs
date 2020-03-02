#include "ecs/System.hpp"
#include "ecs/Manager.hpp"

namespace ecs
{

System::System(const int priority)
	: m_priority(priority)
{}

void System::DispatchInit()
{
	assert(!m_hasBeenInitialized);

	Init();
	m_hasBeenInitialized = true;
}

void System::DispatchDestroy()
{
	Destroy();
}

void System::Init()
{}

void System::Destroy()
{}

void System::SetPriority(const int priority)
{
	if (priority != m_priority)
	{
		m_priority = priority;

		Manager::Get()->NotifySystemPriorityChanged();
	}
}

int System::GetPriority() const
{
	return m_priority;
}

void System::MarkUpdateThreadSafe(bool isThreadSafe)
{
	// It's prohibited to affect thread safety flag after initialization of system
	assert(!m_hasBeenInitialized);

	m_updateThreadSafe = isThreadSafe;
}

bool System::IsUpdateThreadSafe() const
{
	return m_updateThreadSafe;
}

void System::AddUpdateDependency(const std::type_index& systemTypeIndex)
{
	m_updateDependencies.push_back(systemTypeIndex);
}

const std::vector<std::type_index>& System::GetUpdateDependenciesList() const
{
	return m_updateDependencies;
}

bool System::operator<(const System& other) const
{
	return GetPriority() > other.GetPriority();
}

} // namespace ecs
