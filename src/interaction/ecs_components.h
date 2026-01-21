/**
 * @file ecs_components.h
 * @brief ecs components for interaction system
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_INTERACTION_ECS_COMPONENTS_H
#define GEOMETRY_SANDBOX_INTERACTION_ECS_COMPONENTS_H

#include "core/math.h"

#include <functional>

namespace gs::ecs::component
{

/** @brief Part of interaction system. Empty class to mark selected */
struct Selected
{
};

/** @brief Part of interaction system. void() on select callback */
struct OnSelectCallback
{
	std::function<void()> callback;
};

/** @brief Part of interaction system. Empty class to mark hovered */
struct Hovered
{
};

/** @brief Part of interaction system. void(is hovered flag) on hover callback */
struct OnHoverCallback
{
	std::function<void(bool)> callback;
};

/** @brief Part of interaction system. Component containing position vector */
struct Translated
{
	Vec3 position;
};

/** @brief Part of interaction system. void(new position vector) on translate callback */
struct OnTranslateCallback
{
	std::function<void(Vec3)> callback;
};
}  // namespace gs::ecs::component
#endif	// GEOMETRY_SANDBOX_INTERACTION_ECS_COMPONENTS_H
