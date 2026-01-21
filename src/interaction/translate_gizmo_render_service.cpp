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
AABB getPlane(const Vec3& pos, const Axis axis, const float planeSize)
{
	const float halfPlaneSize = planeSize / 2;
	switch (axis)
	{
		case Axis::XY:
			return {
				.min = Vec3{pos.x() + halfPlaneSize, pos.y() + halfPlaneSize, pos.z()},
				.max = Vec3{pos.x() + planeSize, pos.y() + planeSize, pos.z()},
			};
		case Axis::YZ:
			return {
				.min = Vec3{pos.x(), pos.y() + halfPlaneSize, pos.z() + halfPlaneSize},
				.max = Vec3{pos.x(), pos.y() + planeSize, pos.z() + planeSize},
			};
		case Axis::ZX:
			return {
				.min = Vec3{pos.x() + halfPlaneSize, pos.y(), pos.z() + halfPlaneSize},
				.max = Vec3{pos.x() + planeSize, pos.y(), pos.z() + planeSize},
			};
		default: assert(false && "Invalid hover state");
	}
	return {};	// std::unreachable();
}
}  // namespace

void TranslateGizmoRenderService::render(const TranslateGizmo& gizmo, const float scale)
{
	const float axisThickness = settings_->gizmoColliderAxisThickness * scale;
	const float axisLength	  = settings_->gizmoColliderAxisLength * scale;
	const float planeSize	  = settings_->gizmoColliderPlaneSize * scale;
	const float arrowSize	  = settings_->gizmoArrowSize;
	const Vec3 pos			  = gizmo.position;

	graphic_->drawArrow(pos,
						pos + Vec3::UnitX() * axisLength,
						axisThickness,
						gizmo.hoveredAxis == Axis::X ? settings_->gizmoTranslateAxisXSelected : settings_->gizmoTranslateAxisX,
						arrowSize);
	graphic_->drawAABB(getPlane(pos, Axis::XY, planeSize),
					   gizmo.hoveredAxis == Axis::XY ? settings_->gizmoTranslateAxesXYSelected : settings_->gizmoTranslateAxesXY);

	graphic_->drawArrow(pos,
						pos + Vec3::UnitY() * axisLength,
						axisThickness,
						gizmo.hoveredAxis == Axis::Y ? settings_->gizmoTranslateAxisYSelected : settings_->gizmoTranslateAxisY,
						arrowSize);
	graphic_->drawAABB(getPlane(pos, Axis::YZ, planeSize),
					   gizmo.hoveredAxis == Axis::YZ ? settings_->gizmoTranslateAxesYZSelected : settings_->gizmoTranslateAxesYZ);

	graphic_->drawArrow(pos,
						pos + Vec3::UnitZ() * axisLength,
						axisThickness,
						gizmo.hoveredAxis == Axis::Z ? settings_->gizmoTranslateAxisZSelected : settings_->gizmoTranslateAxisZ,
						arrowSize);
	graphic_->drawAABB(getPlane(pos, Axis::ZX, planeSize),
					   gizmo.hoveredAxis == Axis::ZX ? settings_->gizmoTranslateAxesZXSelected : settings_->gizmoTranslateAxesZX);
}
}  // namespace gs