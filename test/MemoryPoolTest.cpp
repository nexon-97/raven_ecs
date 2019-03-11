#include <storage/MemoryPool.hpp>
#include <gtest/gtest.h>

namespace test
{

struct TestObject
{
	int a = 5;
	float b = 15.f;
	char c = 'W';
};

const std::size_t k_defaultChunkSize = 64U;

class MemoryPoolTest
	: public ::testing::Test
{
protected:
	MemoryPoolTest()
	{}

	~MemoryPoolTest() override
	{}

	void SetUp() override
	{
		
	}

	void TearDown() override
	{
		
	}
};

// Empty pool must not allocate any space
TEST_F(MemoryPoolTest, EmptyPoolAllocatedSizeTest)
{
	auto pool = ecs::detail::MemoryPool<TestObject>(k_defaultChunkSize);
	EXPECT_EQ(pool.GetAllocatedCount(), 0U);
}

// When there is no space in pool, a single chunk of space should be allocated
TEST_F(MemoryPoolTest, AllocatedChunkSizeTest)
{
	auto pool = ecs::detail::MemoryPool<TestObject>(k_defaultChunkSize);
	pool.CreateItem();
	EXPECT_EQ(pool.GetAllocatedCount(), k_defaultChunkSize);
}

// When the collection is cleared, no capacity changes should occur
TEST_F(MemoryPoolTest, ClearNotAffectCapacityTest)
{
	auto pool = ecs::detail::MemoryPool<TestObject>(k_defaultChunkSize);

	for (int i = 0; i < 16; ++i)
	{
		pool.CreateItem();
	}
	
	EXPECT_EQ(pool.GetAllocatedCount(), k_defaultChunkSize);

	pool.Clear();

	EXPECT_EQ(pool.GetAllocatedCount(), k_defaultChunkSize);
}

TEST_F(MemoryPoolTest, MultipleChunksAllocationTest)
{
	auto pool = ecs::detail::MemoryPool<TestObject>(k_defaultChunkSize);

	for (int i = 0; i < k_defaultChunkSize * 3; ++i)
	{
		pool.CreateItem();
	}

	pool.CreateItem();

	EXPECT_EQ(pool.GetAllocatedCount(), k_defaultChunkSize * 4U);
	EXPECT_EQ(pool.GetItemsCount(), k_defaultChunkSize * 3U + 1U);
}

TEST_F(MemoryPoolTest, SimpleIteratorTest)
{
	auto pool = ecs::detail::MemoryPool<TestObject>(k_defaultChunkSize);

	for (int i = 0; i < k_defaultChunkSize; ++i)
	{
		pool.CreateItem();
	}

	int iteratedCount = 0;
	for (auto item : pool)
	{
		++iteratedCount;
	}

	EXPECT_EQ(iteratedCount, k_defaultChunkSize);
}

// Removes multiple random elements during collection iteration, and check the iterated items count is
// equal to expected value, and deleted elements were not iterated through
TEST_F(MemoryPoolTest, IteratorWithRemovalTest)
{
	auto pool = ecs::detail::MemoryPool<TestObject>(k_defaultChunkSize);

	for (int i = 0; i < k_defaultChunkSize; ++i)
	{
		auto result = pool.CreateItem();
		result.second->a = i;
	}

	int iteratedCount = 0;
	for (auto it = pool.begin(), end = pool.end(); it != end;)
	{
		if ((*it)->a == 2 || (*it)->a == 10)
		{
			auto destructionResult = pool.erase(it);
			it = destructionResult.first;
			end = destructionResult.second;
		}
		else
		{
			++it;
			++iteratedCount;
		}
	}

	EXPECT_EQ(iteratedCount, k_defaultChunkSize - 2U);
	EXPECT_EQ(iteratedCount, pool.GetItemsCount());
}

} // namespace
