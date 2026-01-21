#include "feature/ortho_projection_view_manager.h"

#include "animation/dsl.h"
#include "animation/manager.h"
#include "animation/types.h"
#include "core/math.h"
#include "core/scene_service.h"
#include "core/settings.h"
#include "core/types.h"
#include "easing.h"
#include "ui/manager.h"

#include <chrono>
#include <cmath>
#include <stdexcept>

namespace gs
{
namespace
{
Vec3 getAxis(ViewProjection view)
{
	switch (view)
	{
		case ViewProjection::OrthoFront: return Vec3::UnitZ();
		case ViewProjection::OrthoBack: return -Vec3::UnitZ();
		case ViewProjection::OrthoRight: return Vec3::UnitX();
		case ViewProjection::OrthoLeft: return -Vec3::UnitX();
		case ViewProjection::OrthoTop: return Vec3::UnitY();
		case ViewProjection::OrthoBottom: return -Vec3::UnitY();
		default: throw std::runtime_error("Illegal view projection"); return {};
	}
}
}  // namespace

void OrthoProjectionViewManager::switchTo(int cameraId, ViewProjection view)
{
	constexpr float dollyDist = 1000.0F;

	if (!activeAnimation_.isValid())
	{
		using namespace animation::dsl;	 // NOLINT(*-using-namespace)

		const Vec3 target		   = sceneService_->getSceneCenter();
		const Vec3 orthoProjection = getAxis(view);

		const Camera beginCamera   = uiManager_->getState().cameras[cameraId];

		Camera positionedCamera	   = beginCamera;
		positionedCamera.target	   = target;
		positionedCamera.position  = target + orthoProjection * settings_->grid.gridSize;

		Camera endCamera		   = positionedCamera;
		endCamera.perspective	   = false;

		// Moving the camera back while zooming in(reducing the FoV)

		endCamera.target   = target;
		endCamera.position = target + Vec3::UnitZ() * dollyDist;
		// Near and far clip planes should remain in place.
		endCamera.zNear = positionedCamera.zNear + dollyDist - settings_->grid.gridSize;
		endCamera.zFar	= positionedCamera.zFar + dollyDist;

		// Reducing the FoV. Target should have a fixed scale
		const float beginDist							   = (endCamera.target - positionedCamera.position).norm();
		const float endDist								   = (endCamera.target - endCamera.position).norm();

		const float beginTop							   = beginDist * std::tan(degToRad(positionedCamera.fov / 2));
		endCamera.fov									   = radToDeg(std::atan(beginTop / endDist) * 2);

		const animation::Interpolator<Camera> interpolator = [](const Camera& lhs, const Camera& rhs, float progress)
		{
			progress			   = lhs.perspective ? expo::in(progress) : expo::out(progress);
			Camera currCamera	   = lhs;
			currCamera.perspective = true;	// always perspective for this animation

			currCamera.position	   = std::lerp(lhs.position, rhs.position, progress);
			currCamera.zNear	   = std::lerp(lhs.zNear, rhs.zNear, progress);
			currCamera.zFar		   = std::lerp(lhs.zFar, rhs.zFar, progress);

			// it is not enough to simply lerp FoV because target's scale should be fixed.
			const float beginDist = (rhs.target - lhs.position).norm();
			const float currDist  = (rhs.target - currCamera.position).norm();

			const float beginTop  = beginDist * std::tan(degToRad(lhs.fov / 2));
			currCamera.fov		  = radToDeg(std::atan(beginTop / currDist) * 2);
			return currCamera;
		};

		animation::KeyframeCollection keyframes;
		keyframes += keyframe(0s) | accessor::camera(cameraId) << beginCamera;
		keyframes += keyframe(1s) | accessor::camera(cameraId) << key(positionedCamera, expo::in_out);
		keyframes += keyframe(2s) | accessor::camera(cameraId) << key(endCamera, interpolator);
		activeAnimation_ = animationManager_->buildAndPlay(keyframes);
	}
	else if (const animation::Instance& animation = activeAnimation_.getAnimationInstance();
			 animation.isComplete() && animation.isForward())
	{
		activeAnimation_.playBackward();
		animationManager_->getAnimationInstance(activeAnimation_).setRemoveOnComplete(true);
	}
}
}  // namespace gs
