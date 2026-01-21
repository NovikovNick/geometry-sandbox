/**
 * @file camera_controller_service.h
 * @brief Camera movement and rotation logic driven by user input
 * @author MetalHeart
 */
#ifndef CAMERA_CONTROLLER_SYSTEM_H
#define CAMERA_CONTROLLER_SYSTEM_H

#include "core/ecs.h"

#include "boost/di.hpp"

#include <memory>

namespace gs
{

struct Settings;
class IInputManager;
class ICameraService;

/** @brief Camera movement and rotation logic driven by user input */
class ICameraControllerService
{
  public:
	virtual void update(Camera&) const	= 0;
	virtual ~ICameraControllerService() = default;
};

/** @brief basic ICameraControllerService implementation */
class CameraControllerService : public ICameraControllerService
{
	std::shared_ptr<Settings> settings_;
	std::shared_ptr<IInputManager> inputManager_;
	std::shared_ptr<ICameraService> cameraService_;

	bool rotateCamera_ = false;	 // should be in input manager, service should be stateless

  public:
	CameraControllerService(const std::shared_ptr<Settings>& settings,
							const std::shared_ptr<IInputManager>& inputManager,
							const std::shared_ptr<ICameraService>& cameraService);

	virtual void update(Camera&) const override;
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
