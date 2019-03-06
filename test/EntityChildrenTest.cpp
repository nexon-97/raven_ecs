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

} // namespace
