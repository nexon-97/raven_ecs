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

	ecs::ComponentHandleInternal dummyHandleInternal = { 50, 1 };
	ecs::ComponentHandleInternal dummyHandleInternal2 = { 55, 1 };
	ecs::ComponentHandleInternal dummyHandleInternal3 = { 60, 1 };
	ecs::ComponentHandle dummyHandle(&dummyHandleInternal);
	ecs::ComponentHandle dummyHandle2(&dummyHandleInternal2);
	ecs::ComponentHandle dummyHandle3(&dummyHandleInternal3);
	
	ecsManager.DestroyComponent(dummyHandle);
	ecsManager.DestroyComponent(dummyHandle2);
	ecsManager.DestroyComponent(dummyHandle3);

	auto transformHandle = ecsManager.CreateComponent<Transform>();
	transformHandle = ecsManager.CreateComponent<Transform>();

	int a = 0;
}

void RenderSystem::Update()
{
	
}
