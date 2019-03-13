#include "ecs/ComponentCollectionImpl.hpp"
#include "ecs/Manager.hpp"

namespace
{
ecs::Manager* gManager = nullptr;
}

namespace ecs
{
namespace detail
{

ComponentCollectionManagerConnection::EntityData ComponentCollectionManagerConnection::GetEntityData(const std::size_t id) const
{
	auto& entitiesCollection = gManager->GetEntitiesCollection();
	return EntityData(entitiesCollection.IsEntityEnabled(id), entitiesCollection.IsEntityActivated(id));
}

void ComponentCollectionManagerConnection::SetManagerInstance(ecs::Manager* manager)
{
	gManager = manager;
}

} // namespace detail
} // namespace ecs
