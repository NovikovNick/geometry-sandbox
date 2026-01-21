/**
 * @file camera_idle_rotation_manager.h
 * @brief Rotates a camera around the scene in idle
 *
 * In theory, it's part of the camera controller, but since it
 * is an additional functionality, I moved it to a separate manager.
 *
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_CAMERA_IDLE_ROTATION_MANAGER_H
#define GEOMETRY_SANDBOX_CAMERA_IDLE_ROTATION_MANAGER_H

#include "animation/handle.h"
#include "core/types.h"

#include "boost/di.hpp"

#include <memory>

namespace gs
{
class Settings;
class IUIManager;
class IInputManager;
class ISceneService;

namespace animation
{
class IManager;
}  // namespace animation

/** @brief Rotates a camera around the scene in idle */
class ICameraIdleRotationManager
{
  public:
	virtual void enableIdleRotation(int cameraId) = 0;
	virtual void disableIdleRotation()			  = 0;
	~ICameraIdleRotationManager()				  = default;
};

/** @brief basic ICameraIdleRotationManager implementation */
class CameraIdleRotationAnimationManager : public ICameraIdleRotationManager
{
	std::shared_ptr<Settings> settings_;
	std::shared_ptr<IUIManager> uiManager_;
	std::shared_ptr<IInputManager> inputManager_;
	std::shared_ptr<ISceneService> sceneService_;
	std::shared_ptr<animation::IManager> animationManager_;

	bool sceneBoundsDirty_;

	Vec3 beginPosition_;
	Vec3 beginTarget_;

	Vec3 sceneCenter_;
	float distanceToSceneCenter_;

	animation::Handle activeAnimation_;

  public:
	CameraIdleRotationAnimationManager(std::shared_ptr<Settings> settings,
									   std::shared_ptr<IUIManager> uiManager,
									   std::shared_ptr<IInputManager> inputManager,
									   std::shared_ptr<ISceneService> sceneService,
									   std::shared_ptr<animation::IManager> animationManager)
		: settings_(settings),					//
		  uiManager_(uiManager),				//
		  inputManager_(inputManager),			//
		  sceneService_(sceneService),			//
		  animationManager_(animationManager),	//
		  sceneBoundsDirty_(true),				//
		  activeAnimation_({})
	{
	}

	virtual void enableIdleRotation(int cameraId) override;
	virtual void disableIdleRotation() override;

  private:
};

namespace di
{
inline auto cameraIdleRotationManager() noexcept
{
	return boost::di::bind<ICameraIdleRotationManager>.to<CameraIdleRotationAnimationManager>();
}
}  // namespace di

}  // namespace gs

#endif	// GEOMETRY_SANDBOX_CAMERA_IDLE_ROTATION_MANAGER_H