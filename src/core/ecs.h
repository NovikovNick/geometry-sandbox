/**
 * @file ecs.h
 * @brief defines types and components for ECS
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_ESC_H
#define GEOMETRY_SANDBOX_ESC_H

#include "core/math.h"
#include "core/types.h"
#include "entt/entt.hpp"

#include <unordered_map>

/** @brief Entity Component System */
namespace gs::ecs
{
using Entity   = entt::entity;
using Registry = entt::basic_registry<Entity>;

namespace component
{
/** @brief Arrow component with configurable float head size */
struct Arrow
{
	float size;	 // arrow head size
};

/** @brief Component containing Color value */
struct MainColor
{
	Color val;
};

/** @brief Component containing normal vector */
struct Normal
{
	Vec3 val;
};

/** @brief Component containing distance float value */
struct Distance
{
	float val;
};

/** @brief Component containing position vector */
struct Position
{
	Vec3 val;
};

/** @brief Component containing end position vector */
struct EndPosition
{
	Vec3 val;
};

/** @brief Component containing radius float value */
struct Radius
{
	float val;
};

/** @brief AABB is not good name for this cause it contains only float size */
struct AABBSize
{
	float val;
};

/** @brief Component containing thickness float value */
struct Thickness
{
	float val;
};

/** @brief Component containing string label with font float size */
struct Label
{
	std::string text;
	float fontSize;
};

/** @brief Component representing sphere collider with float radius */
struct SphereCollider
{
	float radius;
};

/** @brief Component representing AABB collider with vector of axis sizes */
struct AABBCollider
{
	Vec3 size;
};

/** @brief Component containing float length of dash and gaps segments */
struct Dashed
{
	float dashLength;
	float gapLength;
};

/** @brief Component containing mesh data with transformation and model type */
struct Mesh
{
	Vec3 origin;
	float scale;
	ModelType type;
};
}  // namespace component

}  // namespace gs::ecs
#endif	// GEOMETRY_SANDBOX_ESC_H
