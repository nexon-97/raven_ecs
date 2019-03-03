#include "test/RenderSystem.hpp"
#include "test/App.hpp"
#include "test/Transform.hpp"
#include "test/SpriteRender.hpp"

void RenderSystem::Init()
{
	auto& ecsManager = App::GetInstance()->GetECSManager();
	auto& entitiesCollection = ecsManager.GetEntitiesCollection();
	auto& transformsCollection = *ecsManager.GetComponentCollection<Transform>();
	auto& spritesCollection = *ecsManager.GetComponentCollection<SpriteRender>();

	// Create 100 entities with transform and sprite render
	for (int i = 0; i < 100; ++i)
	{
		auto& entity = entitiesCollection.CreateEntity();
		auto transformHandle = ecsManager.CreateComponent<Transform>();
		auto spriteHandle = ecsManager.CreateComponent<SpriteRender>();

		entitiesCollection.AddComponent(entity, transformHandle);
		entitiesCollection.AddComponent(entity, spriteHandle);
	}
}

void RenderSystem::Update()
{
	
}
