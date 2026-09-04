#include "camera_idle_rotation_manager.h"

#include "animation/dsl.h"
#include "animation/manager.h"
#include "animation/types.h"
#include "core/input_manager.h"
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
		const float rotationAnglePerFrame	 = settings_->idleRotationAnglePerFrame;
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
				const AABB sceneBounds	   = sceneService_->getSceneBounds();
				const Vec3 sceneExtents	   = sceneBounds.max - sceneBounds.min;	 // dimensions along all three axes of the scene
				const float sceneMaxExtent = sceneExtents.maxCoeff();			 // the max value among all elements of the extents.

				beginPosition_			   = camera.position;
				beginTarget_			   = camera.target;

				sceneCenter_			   = std::lerp(sceneBounds.min, sceneBounds.max, 0.5F);	 // NOLINT(*-magic-numbers)
				distanceToSceneCenter_	   = std::clamp(sceneMaxExtent * distanceMod, distanceMin, camera.zFar);

				sceneBoundsDirty_		   = false;
			}

			const auto elapsed	 = static_cast<float>(elapsedAfterLastUpdate.count());
			const auto duration	 = static_cast<float>(transitionDuration.count());
			const float progress = std::clamp(elapsed / duration, 0.0F, 1.0F);

			// look at scene center
			camera.target = std::lerp(beginTarget_, sceneCenter_, expo::in_out(progress));

			// move the camera along the ray to the target to maintain distance
			const Vec3 retractedPosition = sceneCenter_ + (camera.position - sceneCenter_).normalized() * distanceToSceneCenter_;
			camera.position				 = std::lerp(beginPosition_, retractedPosition, expo::in_out(progress));

			// rotate around scene center
			camera.position -= sceneCenter_;
			camera.position = rotateVector(camera.position, rotationAxis, rotationAnglePerFrame * progress);
			camera.position += sceneCenter_;
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
