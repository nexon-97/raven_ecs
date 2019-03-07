#include <ecs/Manager.hpp>
#include <gtest/gtest.h>
#include <gtest/gtest_prod.h>
#include "TestEntitiesCollection.hpp"

namespace test
{

class EntityChildrenTest
	: public ::testing::Test
{
protected:
	EntityChildrenTest()
		: entityCollection(manager)
	{
		manager.Init();
	}

	~EntityChildrenTest() override
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

	void CreateMultipleChildren(ecs::Entity& entity, int count, uint32_t* ids)
	{
		for (int i = 0; i < count; ++i)
		{
			auto& child = entityCollection.CreateEntity();
			entityCollection.AddChild(entity, child);
			ids[i] = child.id;
		}
	}

	ecs::Manager manager;
	TestEntitiesCollection entityCollection;
};

TEST_F(EntityChildrenTest, AddChildTest)
{
	auto& entity = entityCollection.CreateEntity();
	auto& child = entityCollection.CreateEntity();
	entityCollection.AddChild(entity, child);
	
	EXPECT_EQ(entity.parentId, ecs::Entity::k_invalidId) << "Parent entity has invalid id";
	EXPECT_EQ(child.parentId, entity.id) << "Child entity parent id not equals to actual parent id";
	EXPECT_EQ(entityCollection.GetChildrenCount(entity), 1) << "Invalid children count set for parent";
}

TEST_F(EntityChildrenTest, RemoveChildTest)
{
	// Add test child
	auto& entity = entityCollection.CreateEntity();
	auto& child = entityCollection.CreateEntity();
	entityCollection.AddChild(entity, child);

	// Check child added successfully
	EXPECT_EQ(entity.parentId, ecs::Entity::k_invalidId);
	EXPECT_EQ(child.parentId, entity.id) << "Child entity parent id not equals to actual parent id";
	EXPECT_EQ(entityCollection.GetChildrenCount(entity), 1) << "Invalid children count in parent";

	// Remove created child
	entityCollection.RemoveChild(entity, child);

	// Check child removed successfully
	EXPECT_EQ(entity.parentId, ecs::Entity::k_invalidId);
	EXPECT_EQ(child.parentId, ecs::Entity::k_invalidId);
	EXPECT_EQ(entityCollection.GetChildrenCount(entity), 0) << "Invalid children count in parent!";
}

TEST_F(EntityChildrenTest, ChildrenIteratorTest)
{
	// Add multiple child entities
	auto& entity = entityCollection.CreateEntity();
	const int k_testChildrenCount = 3;
	uint32_t childIds[k_testChildrenCount];

	CreateMultipleChildren(entity, k_testChildrenCount, childIds);

	int iteratedCount = 0;
	for (auto& child : entityCollection.GetChildrenData(entity))
	{
		ASSERT_EQ(child.id, childIds[iteratedCount]);
		++iteratedCount;
	}

	// Check iterated children count
	ASSERT_EQ(iteratedCount, k_testChildrenCount);
}

TEST_F(EntityChildrenTest, MiddleChildRemovalTest)
{
	// Add multiple child entities
	auto& entity = entityCollection.CreateEntity();
	const int k_testChildrenCount = 3;
	uint32_t childIds[k_testChildrenCount];

	CreateMultipleChildren(entity, k_testChildrenCount, childIds);

	auto& middleChild = entityCollection.GetEntity(childIds[1]);
	entityCollection.RemoveChild(entity, middleChild);

	std::swap(childIds[1], childIds[2]);

	int iteratedCount = 0;
	for (auto& child : entityCollection.GetChildrenData(entity))
	{
		ASSERT_EQ(child.id, childIds[iteratedCount]);
		++iteratedCount;
	}

	// Check iterated children count
	ASSERT_EQ(iteratedCount, k_testChildrenCount - 1);
}

TEST_F(EntityChildrenTest, FirstChildRemovalTest)
{
	// Add multiple child entities
	auto& entity = entityCollection.CreateEntity();
	const int k_testChildrenCount = 3;
	uint32_t childIds[k_testChildrenCount];

	CreateMultipleChildren(entity, k_testChildrenCount, childIds);

	auto& firstChild = entityCollection.GetEntity(childIds[0]);
	entityCollection.RemoveChild(entity, firstChild);

	std::swap(childIds[0], childIds[1]);
	std::swap(childIds[1], childIds[2]);

	int iteratedCount = 0;
	for (auto& child : entityCollection.GetChildrenData(entity))
	{
		ASSERT_EQ(child.id, childIds[iteratedCount]);
		++iteratedCount;
	}

	// Check iterated children count
	ASSERT_EQ(iteratedCount, k_testChildrenCount - 1);
}

} // namespace
