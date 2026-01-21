

#include <gtest/gtest.h>

#include "common/slot_map.h"
#include <iostream>

TEST(SlotMapTest, basic_test)
{
	// arrange
	SlotMap<int> map;

	// act
	map.insert(1);
	map.insert(2);
	map.remove(0);
	map.insert(3);

	// assert
	EXPECT_TRUE(map.data()[0].occupied);
	EXPECT_EQ(map.data()[0].data, 3);

	EXPECT_TRUE(map.data()[1].occupied);
	EXPECT_EQ(map.data()[1].data, 2);

	EXPECT_FALSE(map.data()[2].occupied);
}

TEST(SlotMapTest, grouth_test)
{
	// arrange
	SlotMap<int> map{3};

	// act
	map.insert(1);
	map.insert(2);
	map.remove(0);
	map.insert(3);
	map.insert(4);

	// assert
	EXPECT_EQ(map.size(), 6);

	EXPECT_TRUE(map.data()[0].occupied);
	EXPECT_TRUE(map.data()[1].occupied);
	EXPECT_TRUE(map.data()[2].occupied);
	EXPECT_FALSE(map.data()[3].occupied);
	EXPECT_FALSE(map.data()[4].occupied);
	EXPECT_FALSE(map.data()[5].occupied);
}

TEST(SlotMapTest, foreach_test)
{
	// arrange
	SlotMap<int> map;

	// act
	map.insert(1);
	map.insert(2);

	// assert
	for (auto&& [i, data] : map.forEach())
	{
		EXPECT_TRUE(map.data()[i].occupied);
		EXPECT_EQ(&map.data()[i].data, &data);
	}
}

TEST(SlotMapTest, reuseSlot_test)
{
	// arrange
	SlotMap<int> map;
	const std::size_t expectedSize = map.data().size();

	// act
	for (int i = 0; i < 1024; ++i)
	{
		auto key = map.insert(0);
		std::cout << key.slotIndex << std::endl;
		map.remove(key.slotIndex);
	}

	// assert
	EXPECT_EQ(map.data().size(), expectedSize);
}