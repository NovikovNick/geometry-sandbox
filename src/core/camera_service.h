/**
 * @file camera_service.h
 * @brief various camera operations: orientation, rotation, math
 *
 * @todo calculate dolly effect - change fov with fixed frustum slice
 *
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_CAMERA_SERVICE_H
#define GEOMETRY_SANDBOX_CAMERA_SERVICE_H

#include "core/base_app_component.h"
#include "core/types.h"

#include "boost/di.hpp"

#include <memory>

namespace gs
{
/** @brief various camera operations: orientation, rotation, math */
class ICameraService
{
  public:
	virtual Vec3 getForward(const Camera&) const						 = 0;
	virtual Vec3 getRight(const Camera&) const							 = 0;
	virtual Vec3 getUp(const Camera&) const								 = 0;
	virtual Mat4 getViewMatrix(const Camera&) const						 = 0;
	virtual Mat4 getProjectionMatrix(const Camera&) const				 = 0;
	virtual void rotateYaw(Camera&, float delta) const					 = 0;
	virtual void rotatePitch(Camera&, float delta) const				 = 0;
	virtual Ray getScreenToWorldRay(Vec2 screenPos, const Camera&) const = 0;
	virtual ~ICameraService()											 = default;
};

/** @brief basic CameraService implementation */
class CameraService : public BaseService, public ICameraService
{
  public:
	CameraService(const std::shared_ptr<Settings>& settings, const std::shared_ptr<ILogManager>& log) : BaseService(settings, log) {}

	virtual Vec3 getForward(const Camera&) const override;
	virtual Vec3 getRight(const Camera&) const override;
	virtual Vec3 getUp(const Camera&) const override;
	virtual Mat4 getViewMatrix(const Camera&) const override;
	virtual Mat4 getProjectionMatrix(const Camera&) const override;
	virtual void rotateYaw(Camera&, float angleRad) const override;
	virtual void rotatePitch(Camera&, float angleRad) const override;

	/** @todo add support for orthographic projection */
	virtual Ray getScreenToWorldRay(Vec2 screenPos, const Camera&) const override;
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
