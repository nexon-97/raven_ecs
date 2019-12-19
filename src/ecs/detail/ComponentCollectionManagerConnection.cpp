#include "ecs/component/ComponentCollectionImpl.hpp"
#include "ecs/Manager.hpp"

namespace
{
ecs::Manager* gManager = nullptr;
}

namespace ecs
{
namespace detail
{

ComponentCollectionManagerConnection::EntityActivationData ComponentCollectionManagerConnection::GetEntityActivationData(const EntityId id) const
{
	auto& entitiesCollection = gManager->GetEntitiesCollection();
	Entity entity = entitiesCollection.GetEntityById(id);

	return EntityActivationData(entity.IsEnabled(), entitiesCollection.IsEntityActivated(entity));
}

void ComponentCollectionManagerConnection::InvokeComponentActivationEvent(const ComponentHandle& handle, bool activated)
{
	if (activated)
	{
		if (nullptr != gManager->m_globalComponentActivatedCallback)
		{
			std::invoke(gManager->m_globalComponentActivatedCallback, handle);
		}
	}
	else
	{
		if (nullptr != gManager->m_globalComponentDeactivatedCallback)
		{
			std::invoke(gManager->m_globalComponentDeactivatedCallback, handle);
		}
	}
}

void ComponentCollectionManagerConnection::SetManagerInstance(ecs::Manager* manager)
{
	gManager = manager;
}

} // namespace detail
} // namespace ecs
