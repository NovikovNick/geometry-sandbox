/**
 * @file configuration.h
 * @brief define all settings and setup dependency injection context
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_CONFIGURATION_H
#define GEOMETRY_SANDBOX_CONFIGURATION_H

#include "animation/manager.h"
#include "animation/player_manager.h"
#include "animation/service.h"
#include "core/application.h"
#include "core/camera_controller_service.h"
#include "core/camera_service.h"
#include "core/input_manager.h"
#include "core/log_manager.h"
#include "core/math.h"
#include "core/resource_manager.h"
#include "core/scene_service.h"
#include "core/settings.h"
#include "core/window_manager.h"
#include "feature/camera_idle_rotation_manager.h"
#include "feature/ortho_projection_view_manager.h"
#include "interaction/service.h"
#include "interaction/translate_gizmo.h"
#include "render/facade.h"
#include "render/frustum_draw_service.h"
#include "render/low_level_service.h"
#include "render/mesh_instanced_draw_service.h"
#include "render/viewport_manager.h"
#include "ui/manager.h"
#include "ui/service.h"
#include "ui/state_manager.h"

#include "IconsFontAwesome7.h"
#include "boost/di.hpp"

#include <memory>

namespace gs
{
inline Settings createDefaultSettings()
{
	const Camera defaultCamera{
		.handedness	 = CoordinateHandedness::Right,
		.position	 = Vec3{5, 5, 5},
		.target		 = Vec3{2, 2, 2},
		.upAxis		 = Axis::Y,
		.width		 = 1024,
		.height		 = 768,
		.fov		 = 50.f,
		.zNear		 = 1.0f,
		.zFar		 = 100.f,
		.perspective = true,  //  perspective or orthographic
	};

	const scene::Grid2D grid{.majorLineColor = Color{125.0F, 125.0F, 125.0F, 255.0F},
							 .minorLineColor = Color{90.0F, 90.0F, 90.0F, 255.0F},
							 .majorLineStep	 = 10,
							 .cellSize		 = 1.0F,
							 .gridSize		 = 10};

	return Settings{
		.title								= "Geometry Sandbox",
		.width								= 1024,
		.height								= 768,
		.fullscreen							= false,
		.multiSampleAntiAliasing4X			= true,
		.vSync								= true,

		.footerHeight						= 50.0F,
		.detailsWidth						= 350.0f,
		.detailsHeightOffset				= 15.0F,
		.buttonRounding						= 8.0F,
		.controlButtonWidth					= 64.0F,
		.controlButtonHeight				= 64.0F,
		.controlsWidthOffset				= 350.0F,
		.controlsHeightOffset				= 175.0F,

		.showConsole						= true,
		.showPerformance					= false,
		.showTranslateGizmo					= false,
		.showCollisions						= false,
		.showCursorRay						= false,

		.showControls						= true,
		.showControlRotation				= true,

		.showDetailsView					= true,
		.showFooter							= true,

		.animationSpeed						= 1.00F,

		.iconPlayerBackwardFast				= ICON_FA_BACKWARD_FAST,
		.iconPlayerBackwardStep				= ICON_FA_BACKWARD_STEP,
		.iconPlayerPlay						= ICON_FA_PLAY,
		.iconPlayerPause					= ICON_FA_PAUSE,
		.iconPlayerForwardStep				= ICON_FA_FORWARD_STEP,
		.iconPlayerForwardFast				= ICON_FA_FORWARD_FAST,
		.iconSettings						= ICON_FA_GEAR,
		.iconCameraForward					= ICON_FA_W,
		.iconCameraLeft						= ICON_FA_A,
		.iconCameraBackward					= ICON_FA_S,
		.iconCameraRight					= ICON_FA_D,

		.gizmoArrowSize						= 7.0F,
		.gizmoTranslateAxisX				= Color::red(),
		.gizmoTranslateAxisXSelected		= Color::yellow(),
		.gizmoTranslateAxesXY				= Color::blue(),
		.gizmoTranslateAxesXYSelected		= Color::yellow(),
		.gizmoTranslateAxisY				= Color::green(),
		.gizmoTranslateAxisYSelected		= Color::yellow(),
		.gizmoTranslateAxesYZ				= Color::red(),
		.gizmoTranslateAxesYZSelected		= Color::yellow(),
		.gizmoTranslateAxisZ				= Color::blue(),
		.gizmoTranslateAxisZSelected		= Color::yellow(),
		.gizmoTranslateAxesZX				= Color::green(),
		.gizmoTranslateAxesZXSelected		= Color::yellow(),
		.gizmoColliderPlaneThickness		= 0,
		.gizmoColliderAxisThickness			= 0.005F,
		.gizmoColliderAxisLength			= 0.2,
		.gizmoColliderPlaneSize				= 0.1,

		.canvasFontSize						= 32.0F,
		.uiFontSize							= 20.0F,
		.controlButtonFontSize				= 24.0F,
		.controlButtonBorderThickness		= 2.0F,
		.buttonColor						= Color::white(),
		.buttonColorHover					= Color::blue(),
		.canvasBackgroundColor				= Color{63.0F, 63.0F, 63.0F, 255.0F},

		.uiPlayerTimelineBackgroundColor	= Color::gray(),
		.uiPlayerTimelineFillColor			= Color::blue(),
		.uiPlayerTimelineGrabberColor		= Color::white(),
		.uiPlayerTimelineGrabberColorActive = Color::yellow(),
		.uiPlayerTimelineGrabberSize		= 10.0F,
		.uiPlayerTimelineRoundingSize		= 5.0F,
		.uiPlayerTimelineHeight				= 6.0F,
		.uiPlayerControlButtonWidth			= 25.0F,
		.uiPlayerControlTextWidth			= 15.0F,
		.uiPlayerHeight						= 34.0F,

		.lineThickness						= 0.03F,
		.dashLength							= 0.2F,

		.defaultCamera						= defaultCamera,
		.grid								= grid,

		.idleRotationAnglePerFrame			= 0.002F,
		.idleRotationAnimationStartDelay	= Seconds{1},
		.idleRotationTransitionDuration		= Seconds{5},
		.idleRotationDistanceMin			= 4.0F,
		.idleRotationDistanceMod			= 1.5F,

		.cameraRotateSensitivity			= 0.002F,
		.cameraMoveSensitivity				= 0.15F,
	};
}

/** @brief Dependency injection */
namespace di
{
inline auto& getContext()
{
	static auto ctx = []()
	{
		auto injector	   = boost::di::make_injector(boost::di::bind<Settings>().in(boost::di::singleton),
												  boost::di::bind<Application>().in(boost::di::singleton),
												  boost::di::bind<ecs::Registry>().in(boost::di::singleton),
												  inputManager(),
												  windowManager(),
												  resourceManager(),
												  animationManager(),
												  uiManager(),
												  uiStateManager(),
												  uiService(),
												  cameraControllerSystem(),
												  interactionService(),
												  cameraService(),
												  translateGizmoUpdateService(),
												  translateGizmoRenderService(),
												  sceneService(),
												  animationService(),
												  animationPlayerManager(),
												  cameraIdleRotationManager(),
												  orthoProjectionViewManager(),
												  renderFacade(),
												  frustumDrawService(),
												  lowLevelRenderService(),
												  meshInstancedDrawService(),
												  viewportManager(),
												  logManager());

		Settings& settings = injector.create<Settings&>();
		settings		   = createDefaultSettings();

		return injector;
	}();

	return ctx;
};
}  // namespace di

}  // namespace gs
#endif	// GEOMETRY_SANDBOX_CONFIGURATION_H
