#include "interaction/translate_gizmo.h"

#include "core/math.h"
#include "core/settings.h"
#include "core/types.h"
#include "render/low_level_service.h"

#include <cassert>

namespace gs
{
namespace
{
AABB getPlaneCollider(const Vec3& pos, const Axis axis, const float planeSize, const float planeThickness)
{
	switch (axis)
	{
		case Axis::XY: return {.min = pos, .max = {pos.x() + planeSize, pos.y() + planeSize, pos.z() + planeThickness}};
		case Axis::YZ: return {.min = pos, .max = {pos.x() + planeThickness, pos.y() + planeSize, pos.z() + planeSize}};
		case Axis::ZX: return {.min = pos, .max = {pos.x() + planeSize, pos.y() + planeThickness, pos.z() + planeSize}};
		default: assert(false && "Invalid hover state");
	}
	return {};	// std::unreachable();
}
AABB getAxisCollider(const Vec3& pos, const Axis axis, const float axisLength, const float axisThickness)
{
	switch (axis)
	{
		case Axis::X: return {.min = pos, .max = {pos.x() + axisLength, pos.y() + axisThickness, pos.z() + axisThickness}};
		case Axis::Y: return {.min = pos, .max = {pos.x() + axisThickness, pos.y() + axisLength, pos.z() + axisThickness}};
		case Axis::Z: return {.min = pos, .max = {pos.x() + axisThickness, pos.y() + axisThickness, pos.z() + axisLength}};
		default: assert(false && "Invalid hover state");
	}
	return {};	// std::unreachable();
}
}  // namespace

void TranslateGizmoUpdateService::hover(const Ray& ray, TranslateGizmo& gizmo, const float scale) const
{
	const float axisLenth  = settings_->gizmoColliderAxisLength * scale;
	const float axisThick  = settings_->gizmoColliderAxisThickness * scale;
	const float planeSize  = settings_->gizmoColliderPlaneSize * scale;
	const float planeThick = settings_->gizmoColliderPlaneThickness * scale;

	gizmo.hoveredAxis	   = Axis::None;

	for (auto axis : {Axis::X, Axis::Y, Axis::Z})
	{
		if (intersect(ray, getAxisCollider(gizmo.position, axis, axisLenth, axisThick)).hit)
		{
			gizmo.hoveredAxis = axis;
			return;
		}
	}

	for (auto plane : {Axis::XY, Axis::YZ, Axis::ZX})
	{
		if (intersect(ray, getPlaneCollider(gizmo.position, plane, planeSize, planeThick)).hit)
		{
			gizmo.hoveredAxis = plane;
			return;
		}
	}
}
void TranslateGizmoUpdateService::constrain(const Ray& ray, TranslateGizmo& gizmo) const
{
	gizmo.constrained = true;

	switch (gizmo.hoveredAxis)
	{
		case Axis::XY:
			gizmo.constraintAxis		  = Vec3::UnitX();
			gizmo.constrainedOnSingleAxis = false;
			gizmo.constraintPlane.normal  = Vec3::UnitZ();
			break;
		case Axis::YZ:
			gizmo.constraintAxis		  = Vec3::UnitZ();
			gizmo.constrainedOnSingleAxis = false;
			gizmo.constraintPlane.normal  = Vec3::UnitX();
			break;
		case Axis::ZX:
			gizmo.constraintAxis		  = Vec3::UnitX();
			gizmo.constrainedOnSingleAxis = false;
			gizmo.constraintPlane.normal  = Vec3::UnitY();
			break;
		case Axis::X:
			gizmo.constraintAxis		  = Vec3::UnitX();
			gizmo.constrainedOnSingleAxis = true;
			gizmo.constraintPlane.normal  = gizmo.constraintAxis.cross(gizmo.constraintAxis.cross(ray.direction)).normalized();
			break;
		case Axis::Y:
			gizmo.constraintAxis		  = Vec3::UnitY();
			gizmo.constrainedOnSingleAxis = true;
			gizmo.constraintPlane.normal  = gizmo.constraintAxis.cross(gizmo.constraintAxis.cross(ray.direction)).normalized();
			break;
		case Axis::Z:
			gizmo.constraintAxis		  = Vec3::UnitZ();
			gizmo.constrainedOnSingleAxis = true;
			gizmo.constraintPlane.normal  = gizmo.constraintAxis.cross(gizmo.constraintAxis.cross(ray.direction)).normalized();
			break;
		default: assert(false && "Invalid hover state");
	}
	gizmo.constraintPlane.distance = -gizmo.constraintPlane.normal.dot(gizmo.position);

	// offset
	if (const Intersection intersection = intersect(ray, gizmo.constraintPlane); intersection.hit)
	{
		const float projection = (intersection.point - gizmo.position).dot(gizmo.constraintAxis);
		const Vec3 res	 = gizmo.constrainedOnSingleAxis ? gizmo.position + gizmo.constraintAxis * projection : intersection.point;
		gizmo.dragOffset = res - gizmo.position;
	}
}
void TranslateGizmoUpdateService::translate(const Ray& ray, TranslateGizmo& gizmo) const
{
	if (const Intersection intersection = intersect(ray, gizmo.constraintPlane); intersection.hit)
	{
		const float projection = (intersection.point - gizmo.position).dot(gizmo.constraintAxis);
		const Vec3 res = gizmo.constrainedOnSingleAxis ? gizmo.position + gizmo.constraintAxis * projection : intersection.point;

		gizmo.position = res - gizmo.dragOffset;
	}
}

}  // namespace gs