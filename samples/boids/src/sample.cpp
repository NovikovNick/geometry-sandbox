/**
 * @file sample.cpp
 * @brief simulates the flocking behaviour of birds, and related group motion.
 *
 * check https://en.wikipedia.org/wiki/Boids
 *
 * @author MetalHeart
 */
#include "sample.h"

#include "animation/dsl.h"
#include "animation/handle.h"
#include "animation/manager.h"
#include "animation/player_manager.h"
#include "core/application.h"
#include "core/camera_controller_service.h"
#include "core/configuration.h"
#include "core/ecs.h"
#include "core/input_manager.h"
#include "core/math.h"
#include "core/scene_service.h"
#include "core/types.h"
#include "feature/camera_idle_rotation_manager.h"
#include "interaction/ecs_components.h"
#include "interaction/service.h"
#include "render/low_level_service.h"
#include "ui/manager.h"
#include "ui/state_manager.h"
#include "ui/ui.h"

#include "imgui.h"

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <functional>
#include <utility>
#include <vector>

namespace gs
{
namespace
{
/** @brief Parameters that the user configures through the UI update or dragging scene objects */
struct Parameters
{
};

/** @brief Identifiers of scene objects related to the camera */
struct Entities
{
};

/** @brief Create scene objecs and define colors, radiuses, font sizes and etc */
void setupScene(Parameters& params, Entities& entities) {}

void setupPlaybackAnimation() {}

void updateScene(const Parameters& params, const Entities& entities) {}

void setupInteractionLogic(Parameters& params, const Entities& entities) {}

void setupUI(Parameters& params, const Entities& entities) {}

Application& getApp()
{
	static auto instance = di::getContext().create<Application>();
	return instance;
}
}  // namespace

void init()
{
	static Entities entities;
	static Parameters params;

	setupScene(params, entities);
	setupInteractionLogic(params, entities);
	setupUI(params, entities);
	setupPlaybackAnimation();

	updateScene(params, entities);
}

void drawNextFrame()
{
	static Timepoint prev		= Clock::now();
	const Timepoint now			= Clock::now();
	const Nanoseconds timeDelta = now - prev;
	prev						= now;

	getApp().drawNextFrame(timeDelta);
}

void startInfiniteLoop()
{
	getApp().run();
}

}  // namespace gs
