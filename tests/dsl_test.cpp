

#include <gtest/gtest.h>

#include "animation/dsl.h"
#include "animation/reflection.h"
#include "core/ecs.h"
#include "core/types.h"

namespace gs::animation::dsl
{
namespace
{
reflection::Property getProperty(const auto& path)
{
	return reflection::getProperty(path.target.propertyId);
}
}  // namespace

TEST(DSLTest, dsl_camera_root_offset_test)
{
	// arrange
	const auto camera = accessor::camera(0);

	PropertyPath<Camera, Camera> path{Target{.entityId=0, .propertyId=0}};

	auto p = path / camera::position / vec3::x;

	// act
	// assert
	EXPECT_TRUE(std::is_standard_layout<Camera>::value);
	EXPECT_EQ(getProperty(camera / camera::position).offset, offsetof(Camera, position));
	EXPECT_EQ(getProperty(camera / camera::position / vec3::x).offset, offsetof(Camera, position) + sizeof(float) * 0);
	EXPECT_EQ(getProperty(camera / camera::position / vec3::y).offset, offsetof(Camera, position) + sizeof(float) * 1);
	EXPECT_EQ(getProperty(camera / camera::position / vec3::z).offset, offsetof(Camera, position) + sizeof(float) * 2);
}

TEST(DSLTest, vec3_adapter_test)
{
	// arrange
	scene::Point point{.position=Vec3{1, 2, 3}, .color=Color{.r=4, .g=5, .b=6, .a=7}, .radius=8};
	std::vector<float> seq(FloatSpanAdapter<scene::Point>::channelCount);
	FloatSpanAdapter<scene::Point>::toFloatSpan(point, std::span{seq});

	// act
	// assert
	EXPECT_EQ(point.position.x(), seq[0]);
	EXPECT_EQ(point.position.y(), seq[1]);
	EXPECT_EQ(point.position.z(), seq[2]);
	EXPECT_EQ(point.color.r, seq[3]);
	EXPECT_EQ(point.color.g, seq[4]);
	EXPECT_EQ(point.color.b, seq[5]);
	EXPECT_EQ(point.color.a, seq[6]);
	EXPECT_EQ(point.radius, seq[7]);
}
}  // namespace gs::animation::dsl
