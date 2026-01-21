/**
 * @file camera_sample.cpp
 * @brief Implementation of camera sample
 *
 * - Since the code example provided is not intended to be tested or run in
 * a multi-threaded environment, the data is static for simplicity.
 * - The file contains many magic literals, as this is just a visualization example.
 *
 * @author MetalHeart
 */
#include "camera_sample.h"

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
	Vec3 cameraPosition;
	Vec3 cameraTarget;

	float cameraFov{};
	// todo etc...
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
	auto& ctx			  = di::getContext();
	auto& sceneService	  = ctx.create<ISceneService&>();
	auto& uiManager		  = ctx.create<IUIManager&>();
	auto& settings		  = ctx.create<Settings&>();

	params.cameraPosition = Vec3{3, 3, -1};
	params.cameraTarget	  = Vec3{0, 0, 0};

	ui::State& ui		  = uiManager.getState();

	// Camera& mainCamera	= ui.cameras[ui.activeCameraIndex];
	Camera sampleCamera	  = settings.defaultCamera;
	sampleCamera.position = params.cameraPosition;
	sampleCamera.target	  = params.cameraTarget;
	sampleCamera.zFar	  = 10.0F;
	ui.cameras.push_back(sampleCamera);

	entities.position = sceneService.addPoint({.color = Color::transparent(), .radius = 0.0F});
	entities.target	  = sceneService.addModel({.origin = Vec3{0.2F, 0.9F, 0.0F}, .scale = 1.0F, .type = ModelType::Duck});
	// NOLINTEND(*-magic-numbers)
}

/** @brief Recalculate data and update positions after UI update or dragging scene objects */
void updateScene(const Parameters& params, const Entities& entities)
{
	auto& ctx			  = di::getContext();
	auto& sceneService	  = ctx.create<ISceneService&>();
	auto& uiManager		  = ctx.create<IUIManager&>();

	Camera& sampleCamera  = uiManager.getState().cameras.back();

	sampleCamera.position = params.cameraPosition;
	sceneService.setPosition(entities.position, params.cameraPosition);

	sampleCamera.target = params.cameraTarget;
	sceneService.setPosition(entities.target, params.cameraTarget);
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
															  params.cameraPosition = std::move(position);
															  updateScene(params, entities);
														  });

	registry.emplace<ecs::component::SphereCollider>(entities.target, kObjectCollisionRadius);
	registry.emplace<ecs::component::OnTranslateCallback>(entities.target,
														  [&](Vec3 position)
														  {
															  params.cameraTarget = std::move(position);
															  updateScene(params, entities);
														  });
}

void setupUI(Parameters& params, const Entities& entities)
{
	auto& ctx		= di::getContext();
	auto& uiManager = ctx.create<IUIManager&>();
	auto& settings	= ctx.create<Settings&>();

	uiManager.onDetailsViewDraw(
		[&]()
		{
			const float sceneSize = settings.grid.gridSize;

			ui::State& ui		  = uiManager.getState();
			gs::Camera& camera	  = ui.cameras.back();

			drawCombo("Camera", ui.activeCameraIndex, "Main", "Second");

			ImGui::SliderFloat("fov", &camera.fov, 0, 180);	 // NOLINT(*-magic-numbers)
			ImGui::SliderFloat("zNear", &camera.zNear, 0, sceneSize * 2);
			ImGui::SliderFloat("zFar", &camera.zFar, 0, sceneSize * 2);

			drawCombo("Handedness", camera.handedness, "Left-hand", "Right-hand");

			bool changed = false;
			changed |= ImGui::SliderFloat("pos.x", &params.cameraPosition.x(), -sceneSize, sceneSize);
			changed |= ImGui::SliderFloat("pos.y", &params.cameraPosition.y(), -sceneSize, sceneSize);
			changed |= ImGui::SliderFloat("pos.z", &params.cameraPosition.z(), -sceneSize, sceneSize);
			changed |= ImGui::SliderFloat("target.x", &params.cameraTarget.x(), -sceneSize, sceneSize);
			changed |= ImGui::SliderFloat("target.y", &params.cameraTarget.y(), -sceneSize, sceneSize);
			changed |= ImGui::SliderFloat("target.z", &params.cameraTarget.z(), -sceneSize, sceneSize);
			if (changed)
			{
				updateScene(params, entities);
			};
		});
}

void setupPlaybackAnimation()
{
	auto& ctx		  = di::getContext();
	auto& uiManager	  = ctx.create<IUIManager&>();
	auto& animManager = ctx.create<animation::IManager&>();
	auto& player	  = ctx.create<animation::IPlayerManager&>();

	// NOLINTBEGIN(*-using-namespace, *-magic-numbers)
	using namespace animation::dsl;

	Camera beginCamera									   = uiManager.getState().cameras[0];
	Camera endCamera									   = beginCamera;
	endCamera.position									   = Vec3{1, 1, 1};

	const animation::Interpolator<Camera> circularMovement = [](const Camera& begin, const Camera& end, float progress)
	{
		Camera res	 = begin;
		res.position = std::lerp(begin.position, end.position, progress);
		res.target	 = Vec3{0, 0, 0};
		return res;
	};

	animation::KeyframeCollection keyframes;
	keyframes += keyframe(0s)  //
				 | accessor::camera(0) << beginCamera;
	keyframes += keyframe(2s)  //
				 | accessor::camera(0) << key(endCamera, circularMovement);

	const animation::Handle handle = animManager.buildAndPlay(keyframes,
															  [](animation::Clip& clip)
															  {
																  clip.markers		 = {1s};
																  clip.pauseOnMarker = true;
															  });
	player.setAnimation(handle);
	// NOLINTEND(*-using-namespace, *-magic-numbers)
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
	auto& uiManager		 = ctx.create<IUIManager&>();
	auto& cameraService	 = ctx.create<ICameraService&>();

	const Camera& camera = uiManager.getState().cameras.back();
	const Mat4 view		 = cameraService.getViewMatrix(camera);
	return convert(view);
};

Matrix4x4DTO getView()
{
	auto& ctx			 = di::getContext();
	auto& uiManager		 = ctx.create<IUIManager&>();
	auto& cameraService	 = ctx.create<ICameraService&>();

	const Camera& camera = uiManager.getState().cameras.back();
	const Mat4 view		 = cameraService.getViewMatrix(camera);
	return convert(view);
};

Matrix4x4DTO getProjection()
{
	auto& ctx			  = di::getContext();
	auto& uiManager		  = ctx.create<IUIManager&>();
	auto& cameraService	  = ctx.create<ICameraService&>();

	const Camera& camera  = uiManager.getState().cameras.back();
	const Mat4 projection = cameraService.getProjectionMatrix(camera);
	return convert(projection);
};

void init()
{
	static Parameters params;
	static Entities entities;

	setupScene(params, entities);
	setupInteractionLogic(params, entities);
	setupUI(params, entities);
	setupPlaybackAnimation();

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
