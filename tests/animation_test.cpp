

#include <gtest/gtest.h>

#include "animation/dsl.h"
#include "animation/manager.h"
#include "animation/service.h"
#include "core/log_manager.h"
#include "core/scene_service.h"
#include "core/settings.h"
#include "core/types.h"
#include "mock/ui/manager.h"
#include "ui/state_manager.h"

#include "boost/di.hpp"

#include <chrono>
#include <memory>

namespace gs::animation
{
namespace
{
auto& getTestDependencyInjectionContext()
{
	static auto ctx = []()
	{
		auto injector			= boost::di::make_injector(boost::di::bind<Settings>().in(boost::di::singleton),  //
												   di::logManager(),
												   di::animationService(),
												   di::animationManager(),
												   mock::di::uiManager(),
												   di::uiStateManager(),
												   di::sceneService());

		Settings& settings		= injector.create<Settings&>();
		settings.animationSpeed = 1.0F;

		return injector;
	}();
	return ctx;
};

class AnimationTest : public ::testing::Test
{
  protected:
	void SetUp() override
	{
		auto& ctx	   = getTestDependencyInjectionContext();

		registry	   = &ctx.create<ecs::Registry&>();
		settings	   = &ctx.create<Settings&>();
		animManager	   = &ctx.create<animation::IManager&>();
		sceneService   = &ctx.create<ISceneService&>();
		uiStateManager = &ctx.create<IUIStateManager&>();

		pointId		   = sceneService->addPoint({.position = Vec3{0, 0, 0}, .color = Color::blue(), .radius = 0.1F});
	}

	ecs::Registry* registry			 = nullptr;
	Settings* settings				 = nullptr;
	animation::IManager* animManager = nullptr;
	ISceneService* sceneService		 = nullptr;
	IUIStateManager* uiStateManager	 = nullptr;

	ecs::Entity pointId;
};

}  // namespace

TEST_F(AnimationTest, single_property_channel_interpolation)
{
	// arrange
	using namespace animation::dsl;

	KeyframeCollection keyframes;

	keyframes += keyframe(0s) | accessor::point(pointId) / point::position / vec3::z << 1.0F;
	keyframes += keyframe(1s) | accessor::point(pointId) / point::position / vec3::z << 2.0F;
	animManager->buildAndPlay(keyframes);

	// act
	Milliseconds dt{500};
	animManager->updateAndApplyAnimations(dt);

	// assert
	const auto& pos = registry->get<ecs::component::Position>(pointId);
	EXPECT_EQ(pos.val.z(), 1.5F);
}
TEST_F(AnimationTest, mixed_keyframe_single_property_override)
{
	// arrange
	using namespace animation::dsl;

	KeyframeCollection keyframes;
	keyframes += Keyframe{.time = 0s} | accessor::point(pointId) / point::position << Vec3{0.0F, 0.0F, 1.0F};
	keyframes += Keyframe{.time = 1s} | accessor::point(pointId) / point::position / vec3::z << 2.0F;
	animManager->buildAndPlay(keyframes);

	// act
	Milliseconds dt{500};
	animManager->updateAndApplyAnimations(dt);

	// assert
	const auto& pos = registry->get<ecs::component::Position>(pointId);
	EXPECT_EQ(pos.val.z(), 1.5F);
}

TEST_F(AnimationTest, mixed_keyframe_with_whole_object_init)
{
	// arrange
	using namespace animation::dsl;

	KeyframeCollection keyframes;
	keyframes += keyframe(0s) | accessor::point(pointId) << scene::Point{.position = Vec3{0, 0, 1.0F},	//
																		 .color	   = Color::blue(),
																		 .radius   = 0.1F};
	keyframes += keyframe(1s) | accessor::point(pointId) / point::position / vec3::z << 2.0F;
	animManager->buildAndPlay(keyframes);

	// act
	Milliseconds dt{500};
	animManager->updateAndApplyAnimations(dt);

	// assert
	const auto& pos = registry->get<ecs::component::Position>(pointId);
	EXPECT_EQ(pos.val.z(), 1.5F);
}

TEST_F(AnimationTest, complex_channels_and_custom_interpolator)
{
	// arrange
	using namespace animation::dsl;

	const Interpolator<Vec3> cusomInterpolator = [](const Vec3&, const Vec3&, float t) { return Vec3{t, t, t}; };

	KeyframeCollection keyframes;
	keyframes += keyframe(0s) | accessor::point(pointId) / point::position << Vec3{1.0F, 2.0F, 3.F};
	keyframes += keyframe(1s) | accessor::point(pointId) / point::position << key(Vec3{4.0F, 5.0F, 6.F}, cusomInterpolator);
	animManager->buildAndPlay(keyframes);

	// act
	Milliseconds dt{500};
	animManager->updateAndApplyAnimations(dt);

	// assert
	const auto& pos = registry->get<ecs::component::Position>(pointId);
	EXPECT_EQ(pos.val.x(), 0.5F);
	EXPECT_EQ(pos.val.y(), 0.5F);
	EXPECT_EQ(pos.val.z(), 0.5F);
}

TEST_F(AnimationTest, point_radius_test)
{
	// arrange
	using namespace animation::dsl;

	KeyframeCollection keyframes;
	keyframes += keyframe(Seconds{0}) | accessor::point(pointId) / point::radius << 1.0F;
	keyframes += keyframe(Seconds{1}) | accessor::point(pointId) / point::radius << 2.0F;
	animManager->buildAndPlay(keyframes);

	// act
	Milliseconds dt{500};
	animManager->updateAndApplyAnimations(dt);

	// assert
	const auto& r = registry->get<ecs::component::Radius>(pointId);
	EXPECT_EQ(r.val, 1.5F);
}

TEST_F(AnimationTest, interpolator_test)
{
	// arrange
	using namespace animation::dsl;

	float i = 0;
	const Camera beginCamera{.position = Vec3{++i, ++i, ++i}, .target = Vec3{++i, ++i, ++i}, .fov = ++i};
	const Camera endCamera{.position = Vec3{++i, ++i, ++i}, .target = Vec3{++i, ++i, ++i}, .fov = ++i};
	const int cameraId = 0;
	ui::State& state   = uiStateManager->getState();
	state.cameras.resize(1);
	state.cameras[cameraId]							   = beginCamera;

	const animation::Interpolator<Camera> interpolator = [](const Camera& lhs, const Camera& rhs, float progress)
	{
		Camera res = lhs;
		res.fov	   = std::lerp(lhs.fov, rhs.fov, progress);
		return res;
	};

	animation::KeyframeCollection keyframes;
	keyframes += keyframe(0s) | accessor::camera(cameraId) << beginCamera;
	keyframes += keyframe(1s) | accessor::camera(cameraId) << key(endCamera, interpolator);
	animManager->buildAndPlay(keyframes, [](animation::Clip& clip) { clip.removeOnComplete = true; });

	// act
	Milliseconds dt{500};
	animManager->updateAndApplyAnimations(dt);

	// assert
	const Camera& camera = state.cameras[cameraId];
	EXPECT_EQ(camera.fov, std::lerp(beginCamera.fov, endCamera.fov, 0.5));
}

}  // namespace gs::animation
