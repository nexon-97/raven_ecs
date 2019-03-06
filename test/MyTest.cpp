#include <gtest/gtest.h>

int add(int a, int b)
{
	return a + b;
}

TEST(SomeTestCase, MyTest)
{
	EXPECT_EQ(1, 1);
}
