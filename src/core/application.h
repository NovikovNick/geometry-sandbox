/**
 * @file application.h
 *
 * @brief main entry class
 *
 * @todo [docs]: ★★★ Cleanup .bat and .sh scripts
 * @todo [improvement]: ★★★★ Real 3D text, not a 2D projection
 * @todo [improvement]: ★★★ Template accessor instead of std::span<std::byte>?
 * @todo [improvement]: ★★★★ Template <typename T> using Animator = std::function<T(Nanoseconds)>
 * @todo [improvement]: ★★★★ Move platform specific code to separate class
 * @todo [docs]: ★★★★ Dark/light theme toggle in docs doesn't work correctly
 * @todo [linter]: ★★★ Check names in clang-tidy (active, hover, constrained...)
 *
 * @todo [sample]: ★★★ Add samples for all https://gamemath.com/book/geomtests.html
 * @todo [sample]: ★★★ Boids simulation for benchmark: https://github.com/SkitzFist/Boids
 * @todo [R&D]: ★★★ Dual contouring vs marching cubes
 * @todo [R&D]: ★★★★ Motion matching sample
 * @todo [improvement]: ★★★★ ASAN for unit-tests on clang
 * @todo [performance]: ★★★★	 Animation build should be at compile-time
 * @todo [performance]: ★★★★ Arena allocator should improve animation build's performance
 * @todo [performance]: ★★★★ Concurrency for animation system. Does -pthread work on github pages?
 * @todo [performance]: ★★★ Frustum culling? scene manager and octree
 * @todo [feature]: ★★★★ Rotation gizmo example: https://github.com/CedricGuillemet/ImGuizmo
 * @todo [feature]: ★★★ Select scene objects with rectangle
 * @todo [feature]: ★★★★ Ru/En lang switcher? Asset manager to load i18n?
 * @todo [feature]: ★★★★ Add https://github.com/NanoMichael/MicroTeX
 * @todo [feature]: ★★★ Add color chooser and setup app's color theme
 *
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_APP_H
#define GEOMETRY_SANDBOX_APP_H

#include "animation/types.h"
#include "core/ecs.h"
#include "core/types.h"

#include "boost/di.hpp"

#include <memory>

/** @brief geometry sandbox */
namespace gs
{
struct Settings;
class IInputManager;
class IWindowManager;
class IResourceManager;
class IUIManager;
class IUIStateManager;
class IViewportManager;
class ILogManager;
class IRenderService;
class ICameraControllerService;
class IInteractionService;
class ITranslateGizmoUpdateService;

namespace animation
{
class IManager;
}
namespace render
{
class IFacade;
class IViewportManager;
}  // namespace render

/** @brief main entry class */
class Application
{
	std::shared_ptr<ecs::Registry> registry_;
	std::shared_ptr<IInputManager> inputManager_;
	std::shared_ptr<IWindowManager> windowManager_;
	std::shared_ptr<IResourceManager> resourceManager_;
	std::shared_ptr<animation::IManager> animationManager_;
	std::shared_ptr<IUIManager> uiManager_;
	std::shared_ptr<IUIStateManager> uiStateManager_;
	std::shared_ptr<render::IFacade> renderer_;
	std::shared_ptr<ICameraControllerService> cameraController_;
	std::shared_ptr<IInteractionService> interationService_;
	std::shared_ptr<render::IViewportManager> viewportManager_;
	std::shared_ptr<ILogManager> logManager_;

  public:
	Application(const std::shared_ptr<ecs::Registry>&,
				const std::shared_ptr<IInputManager>&,
				const std::shared_ptr<IWindowManager>&,
				const std::shared_ptr<IResourceManager>&,
				const std::shared_ptr<animation::IManager>&,
				const std::shared_ptr<IUIManager>&,
				const std::shared_ptr<IUIStateManager>&,
				const std::shared_ptr<render::IFacade>&,
				const std::shared_ptr<ICameraControllerService>&,
				const std::shared_ptr<IInteractionService>&,
				const std::shared_ptr<render::IViewportManager>&,
				const std::shared_ptr<ILogManager>&);
	void run();
	void drawNextFrame(Nanoseconds timeDelta);
};
}  // namespace gs
#endif	// GEOMETRY_SANDBOX_APP_H
