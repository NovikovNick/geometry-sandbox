#include <benchmark/benchmark.h>

#include "animation/dsl.h"
#include "animation/manager.h"
#include "core/log_manager.h"
#include "core/math.h"
#include "core/scene_service.h"
#include "core/settings.h"

#include "boost/di.hpp"

#include <chrono>
#include <cmath>

namespace gs
{
static void BM_animationBuild(benchmark::State& state)
{
	// arrange
	auto ctx			  = boost::di::make_injector(di::animationManager(), di::logManager());
	auto& animationManger = ctx.create<animation::IManager&>();
	scene::Point point{.position = {0.f, 2.f, 0.f}, .color = Color::black(), .radius = 0.1f};

	// act
	for (auto _ : state)
	{
		using namespace animation::dsl;
		animation::KeyframeCollection keyframes;

		for (int i = 0; i < state.range(0); ++i)
		{
			keyframes += keyframe(0s) | accessor::point(0) << point;
			keyframes += keyframe(1s) | accessor::point(0) / point::position << Vec3{0.0f, 0.0f, 0.f};
		}
		animationManger.build(keyframes);
	}
}
BENCHMARK(BM_animationBuild)->Arg(100)->Arg(1000)->Arg(10000);

static void BM_animationUpdate(benchmark::State& state)
{
	// arrange
	auto ctx			  = boost::di::make_injector(di::animationManager(), di::sceneService(), di::logManager());
	auto& animationManger = ctx.create<animation::IManager&>();
	auto& registry		  = ctx.create<ecs::Registry&>();
	auto& scene			  = ctx.create<SceneService&>();
	ui::State ui{};
	scene::Point point{.position = {0.f, 2.f, 0.f}, .color = Color::black(), .radius = 0.1f};

	using namespace animation::dsl;
	animation::KeyframeCollection keyframes;

	for (int i = 0; i < state.range(0); ++i)
	{
		auto id = scene.addPoint(point);
		keyframes += keyframe(0s) | accessor::point(id) << point;
		keyframes += keyframe(1s) | accessor::point(id) / point::position << Vec3{0.0f, 0.0f, 0.f};
	}
	animationManger.buildAndPlay(keyframes);

	// act
	for (auto _ : state)
	{
		animationManger.tick(Nanoseconds{1});
		animationManger.animate(registry, ui);
	}
}
BENCHMARK(BM_animationUpdate)->Arg(100)->Arg(1000)->Arg(10000);

static void BM_animationUpdateCustomInterpolator(benchmark::State& state)
{
	// arrange
	auto ctx			  = boost::di::make_injector(di::animationManager(), di::sceneService(), di::logManager());
	auto& animationManger = ctx.create<animation::IManager&>();
	auto& registry		  = ctx.create<ecs::Registry&>();
	auto& scene			  = ctx.create<SceneService&>();
	ui::State ui{};
	scene::Point point{.position = {0.f, 2.f, 0.f}, .color = Color::black(), .radius = 0.1f};

	animation::Interpolator<Vec3> circularMovement = [angle = 360.0F](const Vec3&, const Vec3&, float t) -> Vec3
	{ return Vec3{std::sin(degToRad(angle * t)), 0, std::cos(degToRad(angle * t))}; };

	using namespace animation::dsl;
	animation::KeyframeCollection keyframes;

	for (int i = 0; i < state.range(0); ++i)
	{
		auto id = scene.addPoint(point);
		keyframes += keyframe(0s) | accessor::point(id) << point;
		keyframes += keyframe(1s) | accessor::point(id) / point::position << key(Vec3{0.0f, 0.0f, 0.f}, circularMovement);
	}
	animationManger.buildAndPlay(keyframes);

	// act
	for (auto _ : state)
	{
		animationManger.tick(Nanoseconds{1});
		animationManger.animate(registry, ui);
	}
}
BENCHMARK(BM_animationUpdateCustomInterpolator)->Arg(100)->Arg(1000)->Arg(10000);

static void BM_animationUpdateLongAnimation(benchmark::State& state)
{
	// arrange
	auto ctx			  = boost::di::make_injector(di::animationManager(), di::sceneService(), di::logManager());
	auto& animationManger = ctx.create<animation::IManager&>();
	auto& registry		  = ctx.create<ecs::Registry&>();
	auto& scene			  = ctx.create<SceneService&>();
	ui::State ui{};
	scene::Point point{.position = {0.f, 2.f, 0.f}, .color = Color::black(), .radius = 0.1f};
	constexpr int pointCount = 100;

	using namespace animation::dsl;
	animation::KeyframeCollection keyframes;
	for (int i = 0; i < pointCount; ++i)
	{
		auto id = scene.addPoint(point);

		for (int j = 0; j < state.range(0); ++j)
		{
			keyframes += keyframe(Milliseconds{j}) | accessor::point(id) << point;
		}
	}
	animationManger.buildAndPlay(keyframes);

	// act
	for (auto _ : state)
	{
		animationManger.tick(Nanoseconds{1});
		animationManger.animate(registry, ui);
	}
}
BENCHMARK(BM_animationUpdateLongAnimation)->Arg(10)->Arg(100);
}  // namespace gs
