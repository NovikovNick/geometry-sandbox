/**
 * @file ortho_projection_view_manager.h
 * @brief Animates switch between perspective and front, back, right, left, top, bottom views
 *
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_ORTHO_PROJECTION_VIEW_MANAGER_H
#define GEOMETRY_SANDBOX_ORTHO_PROJECTION_VIEW_MANAGER_H

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

enum class ViewProjection
{
	Perspective,
	OrthoFront,
	OrthoBack,
	OrthoRight,
	OrthoLeft,
	OrthoTop,
	OrthoBottom
};

/**
 * @brief manager to animate switch between perspective and front, back, right, left, top, bottom views
 *
 * usage: di::getContext().create<IOrthoProjectionViewManager&>().switchTo(0, ViewProjection::kOrthoFront);
 */
class IOrthoProjectionViewManager
{
  public:
	virtual void switchTo(int cameraId, ViewProjection) = 0;
	~IOrthoProjectionViewManager()						= default;
};

/** @brief basic IOrthoProjectionViewManager implementation */
class OrthoProjectionViewManager : public IOrthoProjectionViewManager
{
	std::shared_ptr<Settings> settings_;
	std::shared_ptr<IUIManager> uiManager_;
	std::shared_ptr<IInputManager> inputManager_;
	std::shared_ptr<ISceneService> sceneService_;
	std::shared_ptr<animation::IManager> animationManager_;

	ViewProjection view_;
	animation::Handle activeAnimation_;

  public:
	OrthoProjectionViewManager(std::shared_ptr<Settings> settings,
							   std::shared_ptr<IUIManager> uiManager,
							   std::shared_ptr<IInputManager> inputManager,
							   std::shared_ptr<ISceneService> sceneService,
							   std::shared_ptr<animation::IManager> animationManager)
		: settings_(settings),					//
		  uiManager_(uiManager),				//
		  inputManager_(inputManager),			//
		  sceneService_(sceneService),			//
		  animationManager_(animationManager),	//
		  activeAnimation_({})
	{
	}

	virtual void switchTo(int cameraId, ViewProjection) override;
};

namespace di
{
inline auto orthoProjectionViewManager() noexcept
{
	return boost::di::bind<IOrthoProjectionViewManager>.to<OrthoProjectionViewManager>();
}
}  // namespace di

}  // namespace gs

#endif	// GEOMETRY_SANDBOX_ORTHO_PROJECTION_VIEW_MANAGER_H