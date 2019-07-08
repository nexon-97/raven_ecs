#include "ecs/System.hpp"
#include "ecs/Manager.hpp"

namespace ecs
{

System::System(Manager& manager)
	: m_ecsManager(manager)
{}

void System::Init()
{}

void System::Destroy()
{}

void System::SetPriority(const int priority)
{
	if (priority != m_priority)
	{
		m_priority = priority;

		m_ecsManager.NotifySystemPriorityChanged();
	}
}

int System::GetPriority() const
{
	return m_priority;
}

bool System::operator<(const System& other) const
{
	return GetPriority() > other.GetPriority();
}

} // namespace ecs
