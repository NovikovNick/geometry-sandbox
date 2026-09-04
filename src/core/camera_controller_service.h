/**
 * @file camera_controller_service.h
 * @brief Camera movement and rotation logic driven by user input
 * @author MetalHeart
 */
#ifndef CAMERA_CONTROLLER_SYSTEM_H
#define CAMERA_CONTROLLER_SYSTEM_H

#include "core/base_app_component.h"
#include "core/ecs.h"

#include "boost/di.hpp"

#include <memory>

namespace gs
{
class IInputManager;
class ICameraService;

enum class CameraControllerType
{
	Free,	// free movement in 6 degrees of freedom (position + orientation), no restrictions
	Orbit,	// rotates around a target point
};

/** @brief Camera movement and rotation logic driven by user input */
class ICameraControllerService
{
  public:
	virtual void setControllerType(CameraControllerType) = 0;
	virtual void update(Camera&) const					 = 0;
	virtual ~ICameraControllerService()					 = default;
};

/** @brief basic ICameraControllerService implementation */
class CameraControllerService : public BaseService, public ICameraControllerService
{
	std::shared_ptr<IInputManager> inputManager_;
	std::shared_ptr<ICameraService> cameraService_;

	CameraControllerType controllerType_;

  public:
	CameraControllerService(const std::shared_ptr<Settings>& settings,
							const std::shared_ptr<ILogManager>& log,
							const std::shared_ptr<IInputManager>& inputManager,
							const std::shared_ptr<ICameraService>& cameraService);

	/** @todo rename CameraControllerService to manager cause service should be stateless */
	virtual void setControllerType(CameraControllerType type) override { controllerType_ = type; };
	virtual void update(Camera&) const override;

  private:
	void updateFree(Camera&) const;
	void updateOrbit(Camera&) const;
};

namespace di
{
inline auto cameraControllerSystem() noexcept
{
	return boost::di::bind<ICameraControllerService>.to<CameraControllerService>();
}
}  // namespace di
}  // namespace gs

#endif	// CAMERA_CONTROLLER_SYSTEM_H
