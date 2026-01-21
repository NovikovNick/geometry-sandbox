

#include <gtest/gtest.h>

#include "animation/dsl.h"
#include "animation/reflection.h"
#include "core/types.h"

#include <format>
#include <iostream>

namespace gs
{
TEST(ReflectionTest, each_float_property)
{
	// arrange
	int actualCount	  = 0;
	int expectedCount = 5;

	// act
	for (const reflection::Property& property : reflection::nextFloatProperties(0, expectedCount))
	{
		++actualCount;

		// assert
		EXPECT_EQ(property.type, reflection::PropertyType::Float);
	}

	EXPECT_EQ(actualCount, expectedCount);
}

TEST(ReflectionTest, each_float_property_one_count)
{
	// arrange
	int actualCount	  = 0;
	int expectedCount = 1;

	// act
	for (const reflection::Property& property : reflection::nextFloatProperties(5, expectedCount))
	{
		++actualCount;
		// assert
		EXPECT_EQ(property.type, reflection::PropertyType::Float);
	}

	EXPECT_EQ(actualCount, expectedCount);
}

}  // namespace gs
