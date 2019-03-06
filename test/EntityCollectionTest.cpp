#include <ecs/Manager.hpp>
#include <gtest/gtest.h>
#include <gtest/gtest_prod.h>
#include "TestEntitiesCollection.hpp"

namespace test
{

class EntityCollectionTest
	: public ::testing::Test
{
protected:
	EntityCollectionTest()
		: entityCollection(manager)
	{
		manager.Init();
	}

	~EntityCollectionTest() override
	{
		manager.Destroy();
	}

	void SetUp() override
	{
		
	}

	void TearDown() override
	{
		
	}

	bool IsEntityDestoyed(ecs::Entity& entity)
	{
		return (entity.componentsMask == 0
			&& entity.id == ecs::Entity::k_invalidId);
	}

	ecs::Manager manager;
	TestEntitiesCollection entityCollection;
};

TEST_F(EntityCollectionTest, CreateEntityTest)
{
	auto& entity = entityCollection.CreateEntity();
	EXPECT_NE(entity.id, ecs::Entity::k_invalidId);
}

TEST_F(EntityCollectionTest, CreateMultipleEntitiesTest)
{
	auto& entitiesData = entityCollection.GetEntitiesData();
	ecs::Entity* lastCreatedEntity = nullptr;
	const int k_testEntitiesCount = 5;

	for (int i = 0; i < k_testEntitiesCount; ++i)
	{
		lastCreatedEntity = &entityCollection.CreateEntity();
	}
	
	EXPECT_EQ(lastCreatedEntity->id, k_testEntitiesCount - 1);
	EXPECT_EQ(entitiesData.size(), k_testEntitiesCount);
}

TEST_F(EntityCollectionTest, DestroyEntityTest)
{
	auto& entity = entityCollection.CreateEntity();
	entityCollection.DestroyEntity(entity.id);
	auto& entitiesData = entityCollection.GetEntitiesData();
	
	EXPECT_TRUE(IsEntityDestoyed(entity));
}

TEST_F(EntityCollectionTest, DestroyParticularEntityTest)
{
	auto& entitiesData = entityCollection.GetEntitiesData();
	ecs::Entity* lastCreatedEntity = nullptr;
	const int k_testEntitiesCount = 5;

	for (int i = 0; i < k_testEntitiesCount; ++i)
	{
		lastCreatedEntity = &entityCollection.CreateEntity();
	}

	entityCollection.DestroyEntity(3);
	auto& testEntity = entityCollection.GetEntity(3);

	EXPECT_TRUE(IsEntityDestoyed(testEntity));
}

TEST_F(EntityCollectionTest, GetEntityTest)
{
	auto& entitiesData = entityCollection.GetEntitiesData();
	ecs::Entity* lastCreatedEntity = nullptr;
	const int k_testEntitiesCount = 5;

	for (int i = 0; i < k_testEntitiesCount; ++i)
	{
		lastCreatedEntity = &entityCollection.CreateEntity();
	}

	auto& testEntity = entityCollection.GetEntity(3);
	EXPECT_EQ(testEntity.id, 3);
}

} // namespace
