#include "test/MovementSystem.hpp"
#include "test/App.hpp"
#include "test/MovementBehavior.hpp"
#include "test/Transform.hpp"

void MovementSystem::Update()
{
	auto& ecsManager = App::GetInstance()->GetECSManager();
	auto& movementsCollection = *ecsManager.GetComponentCollection<MovementBehavior>();
	auto& transformCollection = *ecsManager.GetComponentCollection<Transform>();
	float timeDelta = App::GetInstance()->GetTimeDelta();

	auto end = movementsCollection.end();
	for (auto it = movementsCollection.begin(); it != end;)
	{
		auto& movement = *it;

		auto& entity = ecsManager.GetEntitiesCollection().GetEntity(movement.entityId);
		auto transform = ecsManager.GetEntitiesCollection().GetComponent<Transform>(entity);

		transform->positionX += movement.component.velocityX * timeDelta;
		transform->positionY += movement.component.velocityY * timeDelta;

		if (transform->positionX < 0 || transform->positionX > 1800
			|| transform->positionY < 0 || transform->positionY > 800)
		{
			// Destroy entity
			ecsManager.GetEntitiesCollection().DestroyEntity(entity.id);
			end = movementsCollection.end();
		}
		else
		{
			++it;
		}
	}
}
