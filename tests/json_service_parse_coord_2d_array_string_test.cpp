

#include <gtest/gtest.h>

#include "core/json_service.h"

#include <span>
#include <vector>

namespace gs
{
TEST(parseCoord2dArrayString, parse_ratio_numbers)
{
	// arrange
	JsonService serviceImpl{};
	const IJsonService& service = serviceImpl;
	std::string input			= "[[3,42],[-4,0.5]]";
	std::vector<Vec2> points;

	// act
	service.parseCoord2dArrayString(input, points);

	// assert
	EXPECT_EQ(2, points.size());
	EXPECT_EQ(3, points[0].x());
	EXPECT_EQ(42, points[0].y());
	EXPECT_EQ(-4, points[1].x());
	EXPECT_EQ(0.5, points[1].y());
}

TEST(parseCoord2dArrayString, parse_with_spaces)
{
	// arrange
	JsonService serviceImpl{};
	const IJsonService& service = serviceImpl;
	std::string input			= "  [  [  3  ,  0  ]  ,  [  -4  ,  0.5  ]  ]  ";
	std::vector<Vec2> points;

	// act
	service.parseCoord2dArrayString(input, points);

	// assert
	EXPECT_EQ(2, points.size());
	EXPECT_EQ(3, points[0].x());
	EXPECT_EQ(0, points[0].y());
	EXPECT_EQ(-4, points[1].x());
	EXPECT_EQ(0.5, points[1].y());
}
}  // namespace gs
