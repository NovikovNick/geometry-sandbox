/**
 * @file viewport_manager.h
 * @brief Manages viewports for all scene cameras
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_RENDER_VIEWPORT_MANAGER_H
#define GEOMETRY_SANDBOX_RENDER_VIEWPORT_MANAGER_H

#include "core/types.h"

#include "boost/di.hpp"
#include "raylib.h"

#include <memory>
#include <vector>

namespace gs
{
struct Settings;
class IWindowManager;
class IUIManager;

namespace render
{
/** @brief Manages viewports for all scene cameras */
class IViewportManager
{
  public:
	virtual void init()											   = 0;
	virtual void setupViewport(std::size_t viewportIndex)		   = 0;
	virtual void cleanupViewport()								   = 0;
	virtual Model getViewport3D(std::size_t viewportIndex) const   = 0;
	virtual Texture getViewport2D(std::size_t viewportIndex) const = 0;
	virtual ~IViewportManager()									   = default;
};

/** @brief basic IViewportManager implementation */
class ViewportManager : public IViewportManager
{
	struct Viewport
	{
		Model plane;
		RenderTexture renderTarget;
	};

	std::vector<Viewport> viewports_;

	std::shared_ptr<Settings> settings_;
	std::shared_ptr<IWindowManager> windowManager_;
	std::shared_ptr<IUIManager> uiManager_;

  public:
	ViewportManager(const std::shared_ptr<Settings>& settings,			   //
					const std::shared_ptr<IWindowManager>& windowManager,  //
					const std::shared_ptr<IUIManager>& uiManager)
		: settings_(settings),			  //
		  windowManager_(windowManager),  //
		  uiManager_(uiManager) {};

	virtual void init() override;
	virtual void setupViewport(std::size_t viewportIndex) override;
	virtual void cleanupViewport() override;
	virtual Model getViewport3D(std::size_t viewportIndex) const override;
	virtual Texture getViewport2D(std::size_t viewportIndex) const override;

  private:
	static Viewport createViewport(const RectSize&, float fov);
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
