#include "camera_idle_rotation_manager.h"

#include "animation/dsl.h"
#include "animation/manager.h"
#include "animation/types.h"
#include "core/camera_service.h"
#include "core/input_manager.h"
#include "core/log_manager.h"
#include "core/math.h"
#include "core/scene_service.h"
#include "core/settings.h"
#include "core/types.h"
#include "easing.h"
#include "ui/state_manager.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>

namespace gs
{
void CameraIdleRotationAnimationManager::enableIdleRotation(int cameraId)
{
	assert(!activeAnimation_.isValid());

	using namespace animation::dsl;	 // NOLINT(*-using-namespace)

	// todo: callback should receive the frame delta time, not the animation progress. Need to add new abstraction
	const animation::Interpolator<Camera> cameraIdleRotation = [&, cameraId](const Camera&, const Camera&, float)
	{
		const Seconds startDelay			 = settings_->idleRotationAnimationStartDelay;
		const Nanoseconds transitionDuration = settings_->idleRotationTransitionDuration;
		const float distanceMin				 = settings_->idleRotationDistanceMin;
		const float distanceMod				 = settings_->idleRotationDistanceMod;
		const Vec3 rotationAxis				 = Vec3::UnitY();  // should calculated from camera.upAxis

		Nanoseconds elapsedAfterLastUpdate	 = Clock::now() - inputManager_->getLastUpdateAt();
		elapsedAfterLastUpdate -= startDelay;

		Camera camera = uiStateManager_->getState().cameras[cameraId];

		if (elapsedAfterLastUpdate > 0s)
		{
			if (sceneBoundsDirty_)
			{
				sceneBoundsDirty_		   = false;
				const AABB sceneBounds	   = sceneService_->getSceneBounds();
				const Vec3 sceneExtents	   = sceneBounds.max - sceneBounds.min;	 // dimensions along all three axes of the scene
				const float sceneMaxExtent = sceneExtents.maxCoeff();			 // the max value among all elements of the extents.

				sceneCenter_			   = std::lerp(sceneBounds.min, sceneBounds.max, 0.5F);	 // NOLINT(*-magic-numbers)

				beginPosition_			   = camera.position;
				beginRotation_			   = camera.rotation;

				distanceToSceneCenter_	   = std::clamp(sceneMaxExtent * distanceMod, distanceMin, camera.zFar);

				const Quat nextRotation	   = quaternionFromLookAt(camera.position, sceneCenter_, getUpVector(camera.upAxis));
				ccw_					   = beginRotation_.dot(nextRotation) > 0;
			}

			const auto elapsed	 = static_cast<float>(elapsedAfterLastUpdate.count());
			const auto duration	 = static_cast<float>(transitionDuration.count());
			const float progress = std::clamp(elapsed / duration, 0.0F, 1.0F);	// progress is calculated based on elapsed time

			// 1. look at scene center
			const Quat newRotation = quaternionFromLookAt(camera.position, sceneCenter_, getUpVector(camera.upAxis));
			camera.rotation		   = beginRotation_.slerp(expo::in_out(progress), newRotation);

			// 2. move the camera along the ray to the target to maintain distance
			const Vec3 retractedPosition = sceneCenter_ + (camera.position - sceneCenter_).normalized() * distanceToSceneCenter_;
			Vec3 newPosition			 = std::lerp(beginPosition_, retractedPosition, expo::in_out(progress));

			// 3. rotate around scene center
			newPosition -= sceneCenter_;
			float angle = settings_->idleRotationAnglePerFrame;
			angle *= progress;	// start smoothly

			// direction of rotation must align with the quaternion slerp's arc,
			// otherwise, a shorter arc may appear in the middle of animation,
			// disrupting the slerp and make ugle orientation teleport
			angle *= (ccw_ ? -1.0F : 1.0F);

			newPosition = rotateVector(newPosition, rotationAxis, angle);
			newPosition += sceneCenter_;

			camera.position = newPosition;
		}
		else
		{
			sceneBoundsDirty_ = true;
		}
		return camera;
	};

	const Camera& camera = uiStateManager_->getState().cameras[cameraId];

	// since the progress (elapsed/duration) value is not used, keyframes with time should be omitted.
	animation::KeyframeCollection keyframes;
	keyframes += keyframe(0s) | accessor::camera(cameraId) << camera;
	keyframes += keyframe(1s) | accessor::camera(cameraId) << key(camera, cameraIdleRotation);
	activeAnimation_ = animationManager_->buildAndPlay(keyframes, [](animation::Clip& clip) { clip.loop = true; });
}

void CameraIdleRotationAnimationManager::disableIdleRotation()
{
	assert(activeAnimation_.isValid());
	activeAnimation_.pause();  // dirty hack: take out of the animation loop to prevent setting final state
	activeAnimation_.stop();
}

}  // namespace gs
