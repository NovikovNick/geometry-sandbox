/**
 * @file facade.h
 * @brief Manages all specific services a managers to render
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_RENDER_FACADE_H
#define GEOMETRY_SANDBOX_RENDER_FACADE_H

#include "core/base_app_component.h"
#include "core/ecs.h"
#include "core/types.h"

#include "boost/di.hpp"
#include "raylib.h"

#include <functional>
#include <memory>

namespace gs
{
class IResourceManager;
class IUIManager;
class IUIStateManager;
class ITranslateGizmoRenderService;

/** @brief Contains services and managers for rendering */
namespace render
{
class IViewportManager;
class IMeshInstancedDrawService;
class IFrustumDrawService;
class ILowLevelService;

/** @brief facade for all rendering */
class IFacade
{
  public:
	virtual void render()								  = 0;
	virtual void onRender(std::function<void()> callback) = 0;
	virtual ~IFacade()									  = default;
};

/**
 * @brief Manages all specific services a managers to render
 *
 * @todo pointTransforms_ and pointColors_ is a TMP solution. They should be in moved to ecs after implemented rotation
 */
class Facade : public BaseAppComponent, public IFacade
{
	std::shared_ptr<ecs::Registry> registry_;
	std::shared_ptr<IResourceManager> resourceManager_;
	std::shared_ptr<IUIManager> uiManager_;
	std::shared_ptr<IUIStateManager> uiStateManager_;
	std::shared_ptr<ITranslateGizmoRenderService> translateGizmoRenderer_;
	std::shared_ptr<ILowLevelService> graphic_;
	std::shared_ptr<IViewportManager> viewportManager_;
	std::shared_ptr<IMeshInstancedDrawService> meshInstancing_;
	std::shared_ptr<IFrustumDrawService> frustumDrawer_;

	std::vector<Mat4> pointTransforms_;	 // should be moved to ecs after implemented rotation
	std::vector<Color> pointColors_;	 // should be moved to ecs after implemented rotation

	std::function<void()> onRenderCallback_;

  public:
	Facade(const std::shared_ptr<Settings>&,
		   const std::shared_ptr<ILogManager>&,
		   const std::shared_ptr<ecs::Registry>&,
		   const std::shared_ptr<IResourceManager>&,
		   const std::shared_ptr<IUIManager>&,
		   const std::shared_ptr<IUIStateManager>&,
		   const std::shared_ptr<ITranslateGizmoRenderService>&,
		   const std::shared_ptr<ILowLevelService>&,
		   const std::shared_ptr<IViewportManager>&,
		   const std::shared_ptr<IMeshInstancedDrawService>&,
		   const std::shared_ptr<IFrustumDrawService>&);

	virtual void render() override;
	virtual void onRender(std::function<void()> callback) override { onRenderCallback_ = callback; }

  private:
	void drawScene(const Camera&);
	void setupCamera(const Camera&);
	void updateViewports();
	void updateCursorType();
	void debug();
};
}  // namespace render

namespace di
{
inline auto renderFacade() noexcept
{
	return boost::di::bind<render::IFacade>.to<render::Facade>();
}
}  // namespace di
}  // namespace gs
#endif	// GEOMETRY_SANDBOX_RENDER_FACADE_H
