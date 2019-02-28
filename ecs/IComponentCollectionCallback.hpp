#pragma once
#include "ComponentHandle.hpp"

namespace ecs
{

/**
* @brief Component collection callback is used for collection state changes notifications
*
* There are several supported callback actions:
* - Component created (called right after component has been created and its constructor was called)
* - Component destroyed (called right before the object is actually destroyed, handle and pointer are still valid)
* - Component enabled (component awaken from disabled state, or just has been created and auto-activated)
* - Component disabled (component marked as disabled, though it still not destroyed, so takes up place in collection)
* 
* Class members description:
* @member m_priority - priority in callbacks execution list. The greater the priority, the earlier it is invoked.
* When the priorities are equal - execution order is random.
* @member m_wantCreateNotifications - Declares if the callback method will be invoked, when the component is created.
* @member m_wantDestroyNotifications - Declares if the callback method will be invoked, when the component is destroyed.
* @member m_wantEnableNotifications - Declares if the callback method will be invoked, when the component is enabled.
* @member m_wantDisableNotifications - Declares if the callback method will be invoked, when the component is disabled.
*/
template <typename ComponentType>
class IComponentCollectionCallback
{
public:
	IComponentCollectionCallback() = delete;
	explicit IComponentCollectionCallback(const int priority = 0
		, const bool wantCreateNotifications = true
		, const bool wantDestroyNotifications = true
		, const bool wantEnableNotifications = true
		, const bool wantDisableNotifications = true)
		: m_priority(priority)
		, m_wantCreateNotifications(wantCreateNotifications)
		, m_wantDestroyNotifications(wantDestroyNotifications)
		, m_wantEnableNotifications(wantEnableNotifications)
		, m_wantDisableNotifications(wantDisableNotifications)
	{}
	virtual ~IComponentCollectionCallback() = default;

	virtual void OnComponentCreated(ComponentType* component, const ComponentHandle& handle) {}
	virtual void OnComponentDestroyed(ComponentType* component, const ComponentHandle& handle) {}
	virtual void OnComponentEnabled(ComponentType* component, const ComponentHandle& handle) {}
	virtual void OnComponentDisabled(ComponentType* component, const ComponentHandle& handle) {}

	const bool WantsCreateNotifications() const
	{
		return m_wantCreateNotifications;
	}

	const bool WantsDestroyNotifications() const
	{
		return m_wantDestroyNotifications;
	}

	const bool WantsEnableNotifications() const
	{
		return m_wantEnableNotifications;
	}

	const bool WantsDisableNotifications() const
	{
		return m_wantDisableNotifications;
	}

	const int GetPriority() const
	{
		return m_priority;
	}

private:
	const int m_priority = 0;
	const bool m_wantCreateNotifications : 1;
	const bool m_wantDestroyNotifications : 1;
	const bool m_wantEnableNotifications : 1;
	const bool m_wantDisableNotifications : 1;
};

} // namespace ecs
