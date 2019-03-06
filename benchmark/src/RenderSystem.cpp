#include "RenderSystem.hpp"
#include "App.hpp"
#include "Transform.hpp"
#include "SpriteRender.hpp"
#include "MovementBehavior.hpp"

void RenderSystem::Init()
{
	auto& ecsManager = App::GetInstance()->GetECSManager();
	auto& entitiesCollection = ecsManager.GetEntitiesCollection();
	auto& transformsCollection = *ecsManager.GetComponentCollection<Transform>();
	auto& spritesCollection = *ecsManager.GetComponentCollection<SpriteRender>();

	// Create 100 entities with transform and sprite render
	for (int i = 0; i < 250; ++i)
	{
		auto& entity = entitiesCollection.CreateEntity();

		Transform* transform;
		auto transformHandle = ecsManager.CreateComponent<Transform>(transform);

		transform->positionX = static_cast<float>(rand() % 1600 + 100);
		transform->positionY = static_cast<float>(rand() % 600 + 100);
		transform->scale = static_cast<float>(rand() % 75 + 25);

		// Create random sprite
		SpriteRender* sprite;
		auto spriteHandle = ecsManager.CreateComponent<SpriteRender>(sprite);
		sprite->colorR = rand() % 255;
		sprite->colorG = rand() % 255;
		sprite->colorB = rand() % 255;

		// Create behavior
		MovementBehavior* movement;
		auto movementHandle = ecsManager.CreateComponent<MovementBehavior>(movement);
		movement->velocityX = static_cast<float>(rand() % 100 - 50);
		movement->velocityY = static_cast<float>(rand() % 100 - 50);

		entitiesCollection.AddComponent(entity, transformHandle);
		entitiesCollection.AddComponent(entity, spriteHandle);
		entitiesCollection.AddComponent(entity, movementHandle);
	}
}

void RenderSystem::Update()
{
	auto& ecsManager = App::GetInstance()->GetECSManager();
	auto& spritesCollection = *ecsManager.GetComponentCollection<SpriteRender>();

	auto renderer = App::GetInstance()->GetRenderer();

	for (auto& sprite : spritesCollection)
	{
		auto& entity = ecsManager.GetEntitiesCollection().GetEntity(sprite.entityId);
		auto transform = ecsManager.GetEntitiesCollection().GetComponent<Transform>(entity);

		SDL_SetRenderDrawColor(renderer, sprite.component.colorR, sprite.component.colorG, sprite.component.colorB, 255);
		
		SDL_Rect rect;
		rect.x = static_cast<int>(transform->positionX);
		rect.y = static_cast<int>(transform->positionY);
		rect.w = static_cast<int>(transform->scale);
		rect.h = static_cast<int>(transform->scale);
		SDL_RenderFillRect(renderer, &rect);
	}
}
