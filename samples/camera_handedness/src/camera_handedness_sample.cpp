/**
 * @file camera_handedness_sample.cpp
 * @brief Handedness of coordinate systems
 *
 * For now, this is just a camera with a handedness switch. Perhaps I'll expand
 * the example later to demonstrate how the handedness of coordinate systems is
 * related to the rotation direction and vertex order.
 *
 * @author MetalHeart
 */
#include "camera_handedness_sample.h"

#include "animation/dsl.h"
#include "animation/handle.h"
#include "animation/manager.h"
#include "animation/player_manager.h"
#include "animation/service.h"
#include "animation/types.h"
#include "core/application.h"
#include "core/configuration.h"
#include "core/ecs.h"
#include "core/math.h"
#include "core/scene_service.h"
#include "core/types.h"
#include "feature/camera_idle_rotation_manager.h"
#include "interaction/ecs_components.h"
#include "interaction/service.h"
#include "ui/manager.h"
#include "ui/state_manager.h"
#include "ui/ui.h"

#include "imgui.h"
#include <Eigen/src/Core/Map.h>

#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <functional>
#include <numbers>
#include <string>
#include <utility>
#include <vector>

namespace gs
{
namespace
{
/** @brief Parameters that the user configures through the UI update or dragging scene objects */
struct Parameters
{
	Camera camera;
};

/** @brief Identifiers of scene objects related to the camera */
struct Entities
{
	ecs::Entity position;
	ecs::Entity target;
};

/** @brief Create scene objecs and define colors, radiuses, font sizes and etc */
void setupScene(Parameters& params, Entities& entities)
{
	// NOLINTBEGIN(*-magic-numbers)
	auto& ctx			   = di::getContext();
	auto& sceneService	   = ctx.create<ISceneService&>();
	auto& uiStateManager   = ctx.create<IUIStateManager&>();
	auto& settings		   = ctx.create<Settings&>();
	auto& facade		   = ctx.create<render::IFacade&>();
	auto& graphic		   = ctx.create<render::ILowLevelService&>();
	auto& cameraService	   = ctx.create<ICameraService&>();
	auto& log			   = ctx.create<ILogManager&>();

	settings.showConsole   = false;

	ui::State& ui		   = uiStateManager.getState();

	Camera& mainCamera	   = ui.cameras[ui.activeCameraIndex];
	mainCamera.position	   = Vec3{5.0F, 5.0F, 2.0F};
	mainCamera.target	   = Vec3::Zero();

	params.camera		   = settings.defaultCamera;
	params.camera.position = Vec3{2, 2, 2};
	params.camera.target   = Vec3::Zero();
	params.camera.fov	   = 55.0F;
	params.camera.zNear	   = 1.0F;
	params.camera.zFar	   = 15.0F;

	ui.cameras.push_back(params.camera);

	entities.position = sceneService.addPoint({.color = Color::transparent(), .radius = 0.0F});
	entities.target	  = sceneService.addModel({.origin = Vec3{0.2F, 0.0F, 0.0F}, .scale = 1.0F, .type = ModelType::Duck});

	// NOLINTEND(*-magic-numbers)
}

/** @brief Recalculate data and update positions after UI update or dragging scene objects */
void updateScene(const Parameters& params, const Entities& entities)
{
	auto& ctx			  = di::getContext();
	auto& sceneService	  = ctx.create<ISceneService&>();
	auto& uiStateManager  = ctx.create<IUIStateManager&>();

	Camera& sampleCamera  = uiStateManager.getState().cameras.back();

	sampleCamera.position = params.camera.position;
	sceneService.setPosition(entities.position, params.camera.position);

	sampleCamera.target = params.camera.target;
	sceneService.setPosition(entities.target, params.camera.target);
}

void setupInteractionLogic(Parameters& params, const Entities& entities)
{
	auto& ctx							   = di::getContext();
	auto& sceneService					   = ctx.create<ISceneService&>();
	auto& uiManager						   = ctx.create<IUIManager&>();
	auto& registry						   = ctx.create<ecs::Registry&>();
	auto& animService					   = ctx.create<animation::IService&>();

	constexpr float kObjectCollisionRadius = 1.0F;
	constexpr float kCameraCollisionRadius = 0.6F;

	registry.emplace<ecs::component::SphereCollider>(entities.position, kCameraCollisionRadius);
	registry.emplace<ecs::component::OnTranslateCallback>(entities.position,
														  [&](Vec3 position)
														  {
															  params.camera.position = std::move(position);
															  updateScene(params, entities);
														  });

	registry.emplace<ecs::component::SphereCollider>(entities.target, kObjectCollisionRadius);
	registry.emplace<ecs::component::OnTranslateCallback>(entities.target,
														  [&](Vec3 position)
														  {
															  params.camera.target = std::move(position);
															  updateScene(params, entities);
														  });
}

void setupUI(Parameters& params, const Entities& entities)
{
	auto& ctx			 = di::getContext();
	auto& uiStateManager = ctx.create<IUIStateManager&>();
	auto& uiManager		 = ctx.create<IUIManager&>();
	auto& settings		 = ctx.create<Settings&>();

	uiManager.onDetailsViewDraw(
		[&]()
		{
			const float sceneSize = settings.grid.gridSize;

			ui::State& ui		  = uiStateManager.getState();
			gs::Camera& camera	  = ui.cameras.back();

			bool changed		  = false;

			drawCombo("Camera", ui.activeCameraIndex, "Main", "Second");

			changed |= drawCombo("Handedness", camera.handedness, "Left-hand", "Right-hand");
			changed |= ImGui::SliderFloat("pos.x", &params.camera.position.x(), -sceneSize, sceneSize);
			changed |= ImGui::SliderFloat("pos.y", &params.camera.position.y(), -sceneSize, sceneSize);
			changed |= ImGui::SliderFloat("pos.z", &params.camera.position.z(), -sceneSize, sceneSize);

			if (changed)
			{
				updateScene(params, entities);
			};
		});
}

Matrix4x4DTO convert(const Mat4& m)
{
	constexpr std::size_t matrixSize = 16;
	Matrix4x4DTO res{.data = std::vector<float>(matrixSize)};

	Eigen::Map<Mat4>(res.data.data()) = m;

	return res;
}

Application& getApp()
{
	static auto instance = di::getContext().create<Application>();
	return instance;
}
}  // namespace

Matrix4x4DTO getModel()
{
	auto& ctx			 = di::getContext();
	auto& uiStateManager = ctx.create<IUIStateManager&>();
	auto& cameraService	 = ctx.create<ICameraService&>();

	const Camera& camera = uiStateManager.getState().cameras.back();
	const Mat4 view		 = cameraService.getViewMatrix(camera).inverse();
	return convert(view);
};

Matrix4x4DTO getView()
{
	auto& ctx			 = di::getContext();
	auto& uiStateManager = ctx.create<IUIStateManager&>();
	auto& cameraService	 = ctx.create<ICameraService&>();

	const Camera& camera = uiStateManager.getState().cameras.back();
	const Mat4 view		 = cameraService.getViewMatrix(camera);
	return convert(view);
};

Matrix4x4DTO getProjection()
{
	auto& ctx			  = di::getContext();
	auto& uiStateManager  = ctx.create<IUIStateManager&>();
	auto& cameraService	  = ctx.create<ICameraService&>();

	const Camera& camera  = uiStateManager.getState().cameras.back();
	const Mat4 projection = cameraService.getProjectionMatrix(camera);
	return convert(projection);
};

void init()
{
	static Entities entities;
	static Parameters params;

	setupScene(params, entities);
	setupInteractionLogic(params, entities);
	setupUI(params, entities);

	updateScene(params, entities);
}

void drawNextFrame()
{
	static Timepoint prev		= Clock::now();
	const Timepoint now			= Clock::now();
	const Nanoseconds timeDelta = now - prev;
	prev						= now;

	getApp().drawNextFrame(timeDelta);
}

void startInfiniteLoop()
{
	getApp().run();
}

}  // namespace gs
