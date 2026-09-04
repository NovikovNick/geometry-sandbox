#include "core/camera_service.h"

#include "core/math.h"
#include "core/settings.h"
#include "core/types.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>

namespace gs
{
namespace
{
Vec3 getUpVector(const Camera& camera)
{
	switch (camera.upAxis)
	{
		case Axis::X: return Vec3::UnitX();
		case Axis::Y: return Vec3::UnitY();
		case Axis::Z: return Vec3::UnitZ();
		default: throw std::runtime_error("invalid up vector");
	}
}
}  // namespace

Vec3 CameraService::getForward(const Camera& camera) const
{
	return (camera.target - camera.position).normalized();
}

Vec3 CameraService::getRight(const Camera& camera) const
{
	const Vec3 upVector = getUpVector(camera);
	const Vec3 forward	= getForward(camera);
	return (camera.handedness == CoordinateHandedness::Left ? upVector.cross(forward) : forward.cross(upVector)).normalized();
}

Vec3 CameraService::getUp(const Camera& camera) const
{
	const Vec3 right   = getRight(camera);
	const Vec3 forward = getForward(camera);
	return (camera.handedness == CoordinateHandedness::Left ? forward.cross(right) : right.cross(forward)).normalized();
}

Mat4 gs::CameraService::getViewMatrix(const Camera& camera) const
{
	// In a left - handed world, the most common convention is to point +Z in the direction that the camera is facing
	const Vec3 vz = camera.handedness == CoordinateHandedness::Left ? getForward(camera) : getForward(camera) * -1;
	const Vec3 vy = getUp(camera);
	const Vec3 vx = getRight(camera);

	// clang-format off
	Mat4 res; 
	res <</* m0 */ vx.x(), /* m4 */ vx.y(), /* m8  */ vx.z(), /* m12 */ -vx.dot(camera.position),
		  /* m1 */ vy.x(), /* m5 */ vy.y(), /* m9  */ vy.z(), /* m13 */ -vy.dot(camera.position),
		  /* m2 */ vz.x(), /* m6 */ vz.y(), /* m10 */ vz.z(), /* m14 */ -vz.dot(camera.position),
	      /* m3 */      0, /* m7 */      0, /* m11 */      0, /* m15 */ 1;
	// clang-format on
	return res;
}

Mat4 gs::CameraService::getProjectionMatrix(const Camera& camera) const
{
	const float aspect = static_cast<float>(camera.width) / static_cast<float>(camera.height);
	const float t	   = camera.zNear * std::tan(degToRad(camera.fov * 0.5F));
	// float b					= -t;
	const float r = t * aspect;
	// float l					= -r;
	const float n = camera.zNear;
	const float f = camera.zFar;

	// simplified formula for a symmetric camera frustum

	// clang-format off

	Mat4 perspectRH;
	perspectRH << /* m0 */ n / r, /* m4 */   0  , /* m8  */          0        , /* m12 */       0,
				  /* m1 */   0  , /* m5 */ n / t, /* m9  */          0        , /* m13 */       0,
		 		  /* m2 */   0  , /* m6 */   0  , /* m10 */ -(f + n) / (f - n), /* m14 */ -(f * n * 2) / (f - n),
		  		  /* m3 */   0  , /* m7 */   0  , /* m11 */        -1         , /* m15 */       0;

		
	Mat4 ortographRH;
	ortographRH << /* m0 */ 1 / r, /* m4 */   0  , /* m8  */          0        , /* m12 */       0,
				   /* m1 */   0  , /* m5 */ 1 / t, /* m9  */          0        , /* m13 */       0,
		 		   /* m2 */   0  , /* m6 */   0  , /* m10 */       -2 / (f - n), /* m14 */ -(f + n) / (f - n),
		  		   /* m3 */   0  , /* m7 */   0  , /* m11 */          0        , /* m15 */       1;
	// clang-format on

	Mat4 perspectLH = perspectRH;
	perspectLH(2, 2) *= -1;	 // m10
	perspectLH(3, 2) *= -1;	 // m11

	Mat4 ortographLH = ortographRH;
	ortographLH(2, 2) *= -1;  // m10

	switch (camera.handedness)
	{
		case CoordinateHandedness::Left: return camera.perspective ? perspectLH : ortographLH;
		case CoordinateHandedness::Right: return camera.perspective ? perspectRH : ortographRH;
		default: throw std::runtime_error("invalid camera.handedness");
	}
}

void CameraService::rotateYaw(Camera& camera, float angleRad) const
{
	const float handedness = camera.handedness == CoordinateHandedness::Right ? -1.0F : 1.0F;
	const Vec3 direction   = rotateVector(getForward(camera), getUp(camera), angleRad * handedness);
	const float distance   = (camera.target - camera.position).norm();
	camera.target		   = camera.position + direction.normalized() * distance;
}

void CameraService::rotatePitch(Camera& camera, float angleRad) const
{
	// Positive rotation in LH coordinates: CW (clockwise, left-hand rule)
	// Positive rotation in RH coordinates : CCW(counterclockwise, right - hand rule)

	const float handedness			 = camera.handedness == CoordinateHandedness::Right ? -1.0F : 1.0F;

	const Vec3 direction			 = rotateVector(getForward(camera), getRight(camera), angleRad * handedness);

	constexpr float kGimbalLockGuard = 0.98F;
	if (std::abs(direction.dot(getUpVector(camera))) > kGimbalLockGuard)
	{
		return;
	}
	const float distance = (camera.target - camera.position).norm();
	camera.target		 = camera.position + direction.normalized() * distance;
}

Ray CameraService::getScreenToWorldRay(Vec2 screenPos, const Camera& camera) const
{  // ndc, y is inverted
	Vec3 ndc;
	ndc.x()			   = ((2 * screenPos.x()) / static_cast<float>(camera.width)) - 1;
	ndc.y()			   = 1 - ((2 * screenPos.y()) / static_cast<float>(camera.height));
	ndc.z()			   = 1.0F;

	const Mat4 matView = getViewMatrix(camera);
	const Mat4 matProj = getProjectionMatrix(camera);

	// Unproject far/near points
	const Vec3 nearPoint = deprojectVector({ndc.x(), ndc.y(), 0.0F}, matView, matProj);
	const Vec3 farPoint	 = deprojectVector({ndc.x(), ndc.y(), 1.0F}, matView, matProj);

	Ray ray;
	ray.origin	  = camera.position;
	ray.direction = (farPoint - nearPoint).normalized();
	/*if (camera.projection == CAMERA_PERSPECTIVE)
		ray.position = camera.position;
	else if (camera.projection == CAMERA_ORTHOGRAPHIC)
		ray.position = cameraPlanePointerPos;*/
	return ray;
}
}  // namespace gs