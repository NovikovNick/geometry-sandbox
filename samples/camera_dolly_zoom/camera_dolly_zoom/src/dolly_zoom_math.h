/**
 * @file dolly_zoom_service.h
 * @brief helper functions for camera sample
 * @author MetalHeart
 */
#ifndef CAMERA_SAMPLE_DOLLY_ZOOM_SERVICE_H
#define CAMERA_SAMPLE_DOLLY_ZOOM_SERVICE_H

#include "core/math.h"
#include "core/types.h"

#include <cmath>

namespace gs
{
/**
 * @brief Update FoV while maintaining the scale at a certain distance
 * @param cam Camera to modify
 * @param nextFov Target field of view in degrees
 * @param fixedDistance Distance between near and far clipping planes where scale remains unchanged
 */
void adjustFOVWithDistanceCompensation(Camera& cam, const float nextFov, const float fixedDistance)
{
	const float r	  = (cam.zNear + fixedDistance) * std::tan(degToRad(cam.fov / 2));
	const float d	  = r / std::tan(degToRad(nextFov / 2));
	const float delta = (cam.zNear + fixedDistance) - d;

	cam.position	  = cam.position - (cam.position - cam.target).normalized() * delta;  // distance compensation
	cam.zNear		  = cam.zNear - delta;	// shift clipping planes along with the camera,
	cam.zFar		  = cam.zFar - delta;	// so that the frustum stays fixed to the scene
	cam.fov			  = nextFov;
}

/** @brief Recalculates FOV for a new projection distance while maintaining the scale at a certain distance */
float calculateFOVAtDistance(const float oldDistance, const float oldFov, const float newDistance)
{
	const float r = oldDistance * std::tan(degToRad(oldFov / 2));
	return radToDeg(std::atan(r / newDistance)) * 2;
}

/** @brief  */
float calculateDistanceAtFOV(const float oldDistance, const float oldFov, const float newFov)
{
	const float r = oldDistance * std::tan(degToRad(oldFov / 2));
	return r / std::tan(degToRad(newFov / 2));
}

}  // namespace gs

#endif	// CAMERA_SAMPLE_DOLLY_ZOOM_SERVICE_H
