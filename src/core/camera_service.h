/**
 * @file camera_service.h
 * @brief various camera operations: orientation, rotation, math
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_CAMERA_SERVICE_H
#define GEOMETRY_SANDBOX_CAMERA_SERVICE_H

#include "core/types.h"

#include "boost/di.hpp"

#include <memory>

namespace gs
{
struct Settings;

/** @brief various camera operations: orientation, rotation, math */
class ICameraService
{
  public:
	virtual Vec3 getForward(const Camera&)						   = 0;
	virtual Vec3 getRight(const Camera&)						   = 0;
	virtual Vec3 getUp(const Camera&)							   = 0;
	virtual Mat4 getViewMatrix(const Camera&)					   = 0;
	virtual Mat4 getProjectionMatrix(const Camera&)				   = 0;
	virtual void rotateYaw(Camera&, float delta)				   = 0;
	virtual void rotatePitch(Camera&, float delta)				   = 0;
	virtual Ray getScreenToWorldRay(Vec2 screenPos, const Camera&) = 0;
	virtual ~ICameraService()									   = default;
};

/** @brief basic CameraService implementation */
class CameraService : public ICameraService
{
	std::shared_ptr<Settings> settings_;

  public:
	CameraService(std::shared_ptr<Settings> settings) : settings_(settings) {}

	virtual Vec3 getForward(const Camera&) override;
	virtual Vec3 getRight(const Camera&) override;
	virtual Vec3 getUp(const Camera&) override;
	virtual Mat4 getViewMatrix(const Camera&) override;
	virtual Mat4 getProjectionMatrix(const Camera&) override;
	virtual void rotateYaw(Camera&, float angleRad) override;
	virtual void rotatePitch(Camera&, float angleRad) override;

	/** @todo add support for orthographic projection */
	virtual Ray getScreenToWorldRay(Vec2 screenPos, const Camera&) override;
};

namespace di
{
inline auto cameraService() noexcept
{
	return boost::di::bind<ICameraService>.to<CameraService>();
}
}  // namespace di
}  // namespace gs
#endif	// GEOMETRY_SANDBOX_CAMERA_SERVICE_H
