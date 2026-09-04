/**
 * @file camera_dolly_zoom_sample.cpp
 * @brief Dolly zoom and interpolation between perspective and orthographic projections
 *
 * To smoothly change the projection type, it is first necessary to transform it into
 * an almost orthographic perspective - with a large distance to the camera and FoV approaching to zero.
 *
 * @author MetalHeart
 */
#include "camera_dolly_zoom_sample.h"

#include "animation/dsl.h"
#include "animation/handle.h"
#include "animation/manager.h"
#include "animation/player_manager.h"
#include "core/application.h"
#include "core/camera_controller_service.h"
#include "core/configuration.h"
#include "core/ecs.h"
#include "core/input_manager.h"
#include "core/math.h"
#include "core/scene_service.h"
#include "core/types.h"
#include "dolly_zoom_math.h"
#include "feature/camera_idle_rotation_manager.h"
#include "interaction/ecs_components.h"
#include "interaction/service.h"
#include "render/low_level_service.h"
#include "ui/manager.h"
#include "ui/state_manager.h"
#include "ui/ui.h"

#include "imgui.h"

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <functional>
#include <utility>
#include <vector>

namespace gs
{
namespace
{
constexpr std::size_t kMainCameraIndex	 = 0;
constexpr std::size_t kSampleCameraIndex = 1;

/** @brief Parameters that the user configures through the UI update or dragging scene objects */
struct Parameters
{
	Camera camera;
	float zFixed = 0.0F;
};

/** @brief Identifiers of scene objects related to the camera */
struct Entities
{
	ecs::Entity position;
	ecs::Entity target;
	ecs::Entity dLabel;
	ecs::Entity rLabel;
	ecs::Entity tLabel;
};

/** @brief Create scene objecs and define colors, radiuses, font sizes and etc */
void setupScene(Parameters& params, Entities& entities)
{

	// NOLINTBEGIN(*-magic-numbers)
	auto& ctx			   = di::getContext();
	auto& sceneService	   = ctx.create<ISceneService&>();
	auto& uiStateManager   = ctx.create<IUIStateManager&>();
	auto& facade		   = ctx.create<render::IFacade&>();
	auto& graphic		   = ctx.create<render::ILowLevelService&>();
	auto& cameraService	   = ctx.create<ICameraService&>();
	auto& input			   = ctx.create<IInputManager&>();
	auto& settings		   = ctx.create<Settings&>();
	auto& log			   = ctx.create<ILogManager&>();

	params.zFixed		   = 2.0F;

	ui::State& ui		   = uiStateManager.getState();

	Camera& mainCamera	   = ui.cameras[kMainCameraIndex];
	mainCamera.position	   = Vec3{10.0F, 10.0F, 10.0F};
	mainCamera.target	   = Vec3::Zero();

	params.camera		   = settings.defaultCamera;
	params.camera.position = Vec3{0, 0, 3};
	params.camera.target   = Vec3::Zero();
	params.camera.fov	   = 90.0F;
	params.camera.zNear	   = 1.0F;
	params.camera.zFar	   = 15.0F;

	ui.cameras.push_back(params.camera);

	entities.position = sceneService.addPoint({.color = Color::transparent(), .radius = 0.0F});
	entities.target	  = sceneService.addModel({.origin = Vec3{0.2F, 0.0F, 0.0F}, .scale = 1.0F, .type = ModelType::Duck});
	entities.dLabel	  = sceneService.addText({.text = "d", .color = Color::white(), .fontSize = settings.canvasFontSize});
	entities.rLabel	  = sceneService.addText({.text = "r", .color = Color::white(), .fontSize = settings.canvasFontSize});
	entities.tLabel	  = sceneService.addText({.text = "t", .color = Color::white(), .fontSize = settings.canvasFontSize});

	facade.onRender(
		[&]()
		{
			const auto frameLog = [&](std::string_view title, const Mat4& m)
			{
				log.frameLog("{}\n"
							 "{:8.2f} {:8.2f} {:8.2f} {:8.2f}\n"
							 "{:8.2f} {:8.2f} {:8.2f} {:8.2f}\n"
							 "{:8.2f} {:8.2f} {:8.2f} {:8.2f}\n"
							 "{:8.2f} {:8.2f} {:8.2f} {:8.2f}",
							 title,
							 m(0, 0),
							 m(0, 1),
							 m(0, 2),
							 m(0, 3),
							 m(1, 0),
							 m(1, 1),
							 m(1, 2),
							 m(1, 3),
							 m(2, 0),
							 m(2, 1),
							 m(2, 2),
							 m(2, 3),
							 m(3, 0),
							 m(3, 1),
							 m(3, 2),
							 m(3, 3));
			};

			const Camera& cam	   = uiStateManager.getState().cameras[1];
			params.camera.position = cam.position;

			const Vec3 direction   = (cam.target - cam.position).normalized();

			const float aspect	   = static_cast<float>(cam.width) / static_cast<float>(cam.height);
			const float d		   = cam.zNear + (cam.perspective ? params.zFixed : 0);
			const float t		   = d * std::tan(degToRad(cam.fov / 2));
			const float r		   = t * aspect;

			const Vec3 top		   = cameraService.getUp(cam) * t;
			const Vec3 right	   = cameraService.getRight(cam) * r;

			frameLog("Projection Matrix", cameraService.getProjectionMatrix(cam));
			log.frameLog("aspect = width / height = {} / {} ={:8.2f}", cam.width, cam.height, aspect);
			log.frameLog("     d = zNear + zFixed ={:8.2f}", d);
			log.frameLog("     t = d * tg(fov/2)  ={:8.2f}", t);
			log.frameLog("     r = d * aspect     ={:8.2f}", r);

			{
				const Vec3 p0 = cam.position + direction * (cam.zNear + params.zFixed) + top - right;
				const Vec3 p1 = cam.position + direction * (cam.zNear + params.zFixed) + top + right;
				const Vec3 p2 = cam.position + direction * (cam.zNear + params.zFixed) - top + right;
				const Vec3 p3 = cam.position + direction * (cam.zNear + params.zFixed) - top - right;

				graphic.drawLine(p0, p1, settings.lineThickness, Color::yellow());
				graphic.drawLine(p1, p2, settings.lineThickness, Color::yellow());
				graphic.drawLine(p2, p3, settings.lineThickness, Color::yellow());
				graphic.drawLine(p3, p0, settings.lineThickness, Color::yellow());
			}

			if (ui.activeCameraIndex == kMainCameraIndex)
			{
				const float thick = settings.lineThickness + 0.01F;	 // a little bit thicker then the default thickness
				const float thin  = settings.lineThickness / 2;		 // much thinner then the default thickness

				const Vec3 p0	  = cam.position;
				const Vec3 p1	  = cam.position + direction * d;
				const Vec3 p2	  = cam.position + direction * d + right;
				const Vec3 p3	  = cam.position + direction * d + top;
				const Vec3 dPos	  = p1 - direction * params.zFixed * 0.5;
				const Vec3 rPos	  = std::lerp(p1, p2, 0.5);
				const Vec3 tPos	  = std::lerp(p1, p3, 0.5) - Vec3::UnitX() * 0.5F;	// offset along the X axis to avoid overlapping

				sceneService.setColor(entities.dLabel, Color::white());
				sceneService.setColor(entities.rLabel, Color::white());
				sceneService.setColor(entities.tLabel, Color::white());
				sceneService.setPosition(entities.dLabel, dPos);
				sceneService.setPosition(entities.rLabel, rPos);
				sceneService.setPosition(entities.tLabel, tPos);

				graphic.drawArrow(p0, p1, thick, Color::white(), settings.gizmoArrowSize);
				graphic.drawArrow(p1, p2, thick, Color::white(), settings.gizmoArrowSize);
				graphic.drawArrow(p1, p3, thick, Color::white(), settings.gizmoArrowSize);
				graphic.drawLine(p0, p2, thin, Color::gray());
				graphic.drawLine(p0, p3, thin, Color::gray());
			}
			else
			{
				sceneService.setColor(entities.dLabel, Color::transparent());
				sceneService.setColor(entities.rLabel, Color::transparent());
				sceneService.setColor(entities.tLabel, Color::transparent());
			}
		});
	// NOLINTEND(*-magic-numbers)
}

void setupPlaybackAnimation(const Parameters& params)
{
	auto& ctx			 = di::getContext();
	auto& uiStateManager = ctx.create<IUIStateManager&>();
	auto& animManager	 = ctx.create<animation::IManager&>();
	auto& player		 = ctx.create<animation::IPlayerManager&>();
	auto& cameraService	 = ctx.create<ICameraService&>();
	auto& facade		 = ctx.create<render::IFacade&>();
	auto& graphic		 = ctx.create<render::ILowLevelService&>();
	auto& log			 = ctx.create<ILogManager&>();
	auto& settings		 = ctx.create<Settings&>();

	if (player.hasAnimation())
	{
		player.getAnimation().stop();
	}

	const animation::Interpolator<Camera> perspectiveLerp = [&](const Camera& lhs, const Camera& rhs, float progress)
	{
		Camera res		= lhs;
		res.perspective = true;
		adjustFOVWithDistanceCompensation(res, std::lerp(lhs.fov, rhs.fov, progress), params.zFixed);
		return res;
	};

	const animation::Interpolator<Camera> ortographicLerp = [](const Camera& lhs, const Camera& rhs, float progress)
	{
		Camera res		= lhs;
		res.perspective = false;
		adjustFOVWithDistanceCompensation(res, std::lerp(lhs.fov, rhs.fov, progress), 0.0F);
		return res;
	};

	const Camera beginCamera = uiStateManager.getState().cameras[kSampleCameraIndex];

	Camera perspective;
	Camera almostOrthographicPerspective;  // perspective projection with FOV approaching zero
	Camera ortographic;

	// NOLINTBEGIN(*-using-namespace, *-magic-numbers)
	using namespace animation::dsl;	 // NOLINT(*-using-namespace)

	animation::KeyframeCollection keyframes;

	if (beginCamera.perspective)
	{
		perspective					  = beginCamera;

		almostOrthographicPerspective = beginCamera;
		adjustFOVWithDistanceCompensation(almostOrthographicPerspective, 0.01F, params.zFixed);

		ortographic				= beginCamera;
		ortographic.perspective = false;
		ortographic.fov			= calculateFOVAtDistance(/*oldDistance*/ beginCamera.zNear + params.zFixed,
												 /*oldFov*/ beginCamera.fov,
												 /*newDistance*/ beginCamera.zNear);

		keyframes += keyframe(0s) | accessor::camera(kSampleCameraIndex) << perspective;
		keyframes += keyframe(1s) | accessor::camera(kSampleCameraIndex) << key(almostOrthographicPerspective, perspectiveLerp);
		keyframes += keyframe(2s) | accessor::camera(kSampleCameraIndex) << key(ortographic, ortographicLerp);
	}
	else
	{
		ortographic								  = beginCamera;

		almostOrthographicPerspective			  = beginCamera;
		almostOrthographicPerspective.perspective = false;
		const float d							  = calculateDistanceAtFOV(/*oldDistance*/ beginCamera.zNear,
											   /*oldFov*/ beginCamera.fov,
											   /*newFov*/ 0.01);
		almostOrthographicPerspective.fov		  = 0.01;
		almostOrthographicPerspective.position += (beginCamera.position - beginCamera.target).normalized() * d;
		almostOrthographicPerspective.zNear += d;
		almostOrthographicPerspective.zFar += d;

		perspective				= beginCamera;
		perspective.perspective = true;
		perspective.fov			= calculateFOVAtDistance(/*oldDistance*/ beginCamera.zNear,
												 /*oldFov*/ beginCamera.fov,
												 /*newDistance*/ beginCamera.zNear + params.zFixed);

		keyframes += keyframe(0s) | accessor::camera(kSampleCameraIndex) << ortographic;
		keyframes += keyframe(1s) | accessor::camera(kSampleCameraIndex) << key(almostOrthographicPerspective, ortographicLerp);
		keyframes += keyframe(2s) | accessor::camera(kSampleCameraIndex) << key(perspective, perspectiveLerp);
	}

	const animation::Handle handle = animManager.buildAndPlay(keyframes, [&](animation::Clip& clip) { clip.markers = {999ms}; });
	handle.pause();
	player.setAnimation(handle);

	// NOLINTEND(*-using-namespace, *-magic-numbers)
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

	setupPlaybackAnimation(params);
}

void enableInteractionLogic(Parameters& params, const Entities& entities)
{
	auto& ctx							   = di::getContext();
	auto& registry						   = ctx.create<ecs::Registry&>();

	constexpr float kObjectCollisionRadius = 1.0F;
	constexpr float kCameraCollisionRadius = 0.6F;

	registry.emplace_or_replace<ecs::component::SphereCollider>(entities.position, kCameraCollisionRadius);
	registry.emplace_or_replace<ecs::component::OnTranslateCallback>(entities.position,
																	 [&](Vec3 position)
																	 {
																		 params.camera.position = std::move(position);
																		 updateScene(params, entities);
																	 });

	registry.emplace_or_replace<ecs::component::SphereCollider>(entities.target, kObjectCollisionRadius);
	registry.emplace_or_replace<ecs::component::OnTranslateCallback>(entities.target,
																	 [&](Vec3 position)
																	 {
																		 params.camera.target = std::move(position);
																		 updateScene(params, entities);
																	 });
}

void disableInteractionLogic(const Entities& entities)
{
	auto& ctx	   = di::getContext();
	auto& registry = ctx.create<ecs::Registry&>();

	registry.remove<ecs::component::SphereCollider>(entities.position);
	registry.remove<ecs::component::OnTranslateCallback>(entities.position);
	registry.remove<ecs::component::SphereCollider>(entities.target);
	registry.remove<ecs::component::OnTranslateCallback>(entities.target);
}

void setupUI(Parameters& params, const Entities& entities)
{
	auto& ctx									 = di::getContext();
	auto& uiStateManager						 = ctx.create<IUIStateManager&>();
	auto& uiManager								 = ctx.create<IUIManager&>();
	auto& settings								 = ctx.create<Settings&>();
	auto& input									 = ctx.create<IInputManager&>();
	auto& controllerService						 = ctx.create<ICameraControllerService&>();

	const auto updateSceneOnSampleCameraMovement = [&]()
	{
		if (uiStateManager.getState().activeCameraIndex == kSampleCameraIndex)
		{
			updateScene(params, entities);
		}
	};

	input.onRelease(InputKey::Forward, updateSceneOnSampleCameraMovement);
	input.onRelease(InputKey::Backward, updateSceneOnSampleCameraMovement);
	input.onRelease(InputKey::Left, updateSceneOnSampleCameraMovement);
	input.onRelease(InputKey::Right, updateSceneOnSampleCameraMovement);

	uiManager.onDetailsViewDraw(
		[&]()
		{
			const float sceneSize = settings.grid.gridSize;

			ui::State& ui		  = uiStateManager.getState();
			gs::Camera& camera	  = ui.cameras.back();
			bool changed		  = false;

			if (drawCombo("Camera", ui.activeCameraIndex, "Main camera", "Sample camera"))
			{
				switch (ui.activeCameraIndex)
				{
					case kMainCameraIndex:
					{
						settings.showControlRotation = true;
						enableInteractionLogic(params, entities);
						controllerService.setControllerType(CameraControllerType::Free);
						break;
					}
					case kSampleCameraIndex:
					{
						settings.showControlRotation = false;
						disableInteractionLogic(entities);
						controllerService.setControllerType(CameraControllerType::Orbit);
						break;
					}
					default: assert(false && "Unsupported controller type");
				}
			}

			ImGui::BeginDisabled(ui.player.isPlaying);
			changed |= drawCombo("Projection", camera.perspective, "Orthographic", "Perspective");

			changed |= ImGui::SliderFloat("fov", &camera.fov, 0, 180);	// NOLINT(*-magic-numbers)
			changed |= ImGui::SliderFloat("zNear", &camera.zNear, 0, sceneSize * 2);
			changed |= ImGui::SliderFloat("zFixed", &params.zFixed, 0, camera.zFar - camera.zNear);
			changed |= ImGui::SliderFloat("zFar", &camera.zFar, 0, sceneSize * 2);

			changed |= ImGui::SliderFloat("pos.x", &params.camera.position.x(), -sceneSize, sceneSize);
			changed |= ImGui::SliderFloat("pos.y", &params.camera.position.y(), -sceneSize, sceneSize);
			changed |= ImGui::SliderFloat("pos.z", &params.camera.position.z(), -sceneSize, sceneSize);
			ImGui::EndDisabled();

			if (changed)
			{
				updateScene(params, entities);
			};
		});
}

Application& getApp()
{
	static auto instance = di::getContext().create<Application>();
	return instance;
}
}  // namespace

void init()
{
	static Entities entities;
	static Parameters params;

	setupScene(params, entities);
	enableInteractionLogic(params, entities);
	setupUI(params, entities);
	setupPlaybackAnimation(params);

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
