/**
 * @file viewport_manager.h
 * @brief Manages viewports for all scene cameras
 *
 * A viewport will be created for each camera in the scene. Only width and height
 * will be taken from the camera, and the viewport will be created with FoV = 90 and zNear = 1
 * This is done so that scale in world will be the same as zoom and can be calculated as:
 *
 * \f$ Scale = Zoom = \frac{zNear}{\tan(\frac{FoV}{2})} \f$
 *
 * The viewport for the active camera (the one you are looking from) will be used for rendering
 * vieport widgets (like translate gizmo)
 *
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_RENDER_VIEWPORT_MANAGER_H
#define GEOMETRY_SANDBOX_RENDER_VIEWPORT_MANAGER_H

#include "core/base_app_component.h"
#include "core/types.h"

#include "boost/di.hpp"
#include "raylib.h"

#include <memory>
#include <vector>

namespace gs
{
class IWindowManager;
class IUIStateManager;

namespace render
{
/** @brief Manages viewports for all scene cameras */
class IViewportManager
{
  public:
	virtual void init() = 0;

	/** @brief setup viewport as render target */
	virtual void setupViewport(std::size_t viewportIndex)		   = 0;
	virtual void cleanupViewport()								   = 0;
	virtual Model getViewport3D(std::size_t viewportIndex) const   = 0;
	virtual Texture getViewport2D(std::size_t viewportIndex) const = 0;
	virtual ~IViewportManager()									   = default;
};

/** @brief basic IViewportManager implementation */
class ViewportManager : public BaseManager, public IViewportManager
{
	struct Viewport
	{
		Model plane;
		RenderTexture renderTarget;
	};

	std::vector<Viewport> viewports_;
	std::shared_ptr<IWindowManager> windowManager_;
	std::shared_ptr<IUIStateManager> uiStateManager_;

  public:
	ViewportManager(const std::shared_ptr<Settings>& settings,
					const std::shared_ptr<ILogManager>& log,
					const std::shared_ptr<IWindowManager>& windowManager,
					const std::shared_ptr<IUIStateManager>& uiStateManager)
		: BaseManager(settings, log),	  //
		  windowManager_(windowManager),  //
		  uiStateManager_(uiStateManager) {};

	virtual void init() override;
	virtual void setupViewport(std::size_t viewportIndex) override;
	virtual void cleanupViewport() override;
	virtual Model getViewport3D(std::size_t viewportIndex) const override;
	virtual Texture getViewport2D(std::size_t viewportIndex) const override;

  private:
	static Viewport createViewport(const RectSize&);
	void buildViewports();
	void rebuildViewports(const RectSize&);
};
}  // namespace render

namespace di
{
inline auto viewportManager() noexcept
{
	return boost::di::bind<render::IViewportManager>.to<render::ViewportManager>();
}
}  // namespace di
}  // namespace gs

#endif	// GEOMETRY_SANDBOX_RENDER_VIEWPORT_MANAGER_H
