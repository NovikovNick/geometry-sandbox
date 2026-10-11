/**
 * @file sample.cpp
 * @brief quaternion sample
 *
 * check https://en.wikipedia.org/wiki/Boids
 *
 * @author MetalHeart
 */
#include "sample.h"

#include "animation/dsl.h"
#include "animation/handle.h"
#include "animation/manager.h"
#include "animation/player_manager.h"
#include "core/application.h"
#include "core/camera_controller_service.h"
#include "core/configuration.h"
#include "core/ecs.h"
#include "core/input_manager.h"
#include "core/log_manager.h"
#include "core/math.h"
#include "core/scene_service.h"
#include "core/types.h"
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

/** @brief Parameters that the user configures through the UI update or dragging scene objects */
struct Parameters
{
	Vec3 position;
	Vec3 rotation;
	Quat quaternion;
	bool rotationDirectionCCW;
};

/** @brief Identifiers of scene objects related to the camera */
struct Entities
{
};

/** @brief Create scene objecs and define colors, radiuses, font sizes and etc */
void setupScene(Parameters& params, Entities& entities)
{
	// NOLINTBEGIN(*-magic-numbers)
	auto& ctx			 = di::getContext();
	auto& uiStateManager = ctx.create<IUIStateManager&>();
	auto& sceneService	 = ctx.create<ISceneService&>();
	auto& renderer		 = ctx.create<render::IFacade&>();
	auto& graphic		 = ctx.create<render::ILowLevelService&>();
	auto& cameraService	 = ctx.create<ICameraService&>();
	auto& settings		 = ctx.create<Settings&>();
	auto& log			 = ctx.create<ILogManager&>();

	Camera& mainCamera	 = uiStateManager.getState().cameras[0];
	mainCamera.position	 = Vec3{3, 3, 3};
	cameraService.lookAt(mainCamera, Vec3::Zero());

	renderer.onRender(
		[&]
		{
			const float thickness	 = settings.lineThickness / 2;
			const float theta		 = std::acos(params.quaternion.w()) * 2;
			const float sinHalfTheta = std::sin(theta / 2);

			const Vec3 vq			 = Vec3{params.quaternion.x() / sinHalfTheta,  //
									params.quaternion.y() / sinHalfTheta,
									params.quaternion.z() / sinHalfTheta}
								.normalized();

			log.frameLog("{:8.2f} {:8.2f} {:8.2f} ", vq.x(), vq.y(), vq.z());
			graphic.drawArrow(Vec3::Zero(), vq, settings.lineThickness, Color::yellow(), settings.gizmoArrowSize);

			const static std::array points{Vec3{1, 0, 0}.normalized(),
										   Vec3{0, 1, 0}.normalized(),
										   Vec3{0, 0, 1}.normalized(),
										   Vec3{1, 1, 0}.normalized(),
										   Vec3{0, 1, 1}.normalized(),
										   Vec3{1, 0, 1}.normalized(),
										   Vec3{1, 1, 1}.normalized()};

			for (const auto& point : points)
			{
				graphic.drawSphereWires(point, Color::yellow(), 0.05F);
			}

			constexpr float kN = 16;
			Quat q1;
			Quat q2;
			for (int i = 0; i < kN; ++i)
			{
				if (params.rotationDirectionCCW)
				{
					q1 = params.quaternion.slerp(i / kN, Quat::Identity());
					q2 = params.quaternion.slerp((i + 1) / kN, Quat::Identity());
				}
				else
				{
					q1 = Quat::Identity().slerp(i / kN, params.quaternion);
					q2 = Quat::Identity().slerp((i + 1) / kN, params.quaternion);
				}

				for (const auto& point : points)
				{
					graphic.drawLine(q1 * point, q2 * point, thickness, Color::yellow());
				}
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

	// NOLINTBEGIN(*-using-namespace, *-magic-numbers)
	using namespace animation::dsl;	 // NOLINT(*-using-namespace)

	/*

	animation::KeyframeCollection keyframes;

	keyframes += keyframe(0s) | accessor::camera(kSampleCameraIndex) << ortographic;
	keyframes += keyframe(1s) | accessor::camera(kSampleCameraIndex) << key(almostOrthographicPerspective, ortographicLerp);
	keyframes += keyframe(2s) | accessor::camera(kSampleCameraIndex) << key(perspective, perspectiveLerp);

	const animation::Handle handle = animManager.buildAndPlay(keyframes, [&](animation::Clip& clip) { clip.markers = {999ms}; });
	handle.pause();
	player.setAnimation(handle);

	*/

	// NOLINTEND(*-using-namespace, *-magic-numbers)
}

/** @brief Recalculate data and update positions after UI update or dragging scene objects */
void updateScene(const Parameters& params, const Entities& entities)
{
	auto& ctx			 = di::getContext();
	auto& sceneService	 = ctx.create<ISceneService&>();
	auto& uiStateManager = ctx.create<IUIStateManager&>();
}

void setupInteractionLogic(Parameters& params, const Entities& entities)
{
	auto& ctx							   = di::getContext();
	auto& registry						   = ctx.create<ecs::Registry&>();

	constexpr float kObjectCollisionRadius = 1.0F;
	constexpr float kCameraCollisionRadius = 0.6F;

	/*
	registry.emplace_or_replace<ecs::component::SphereCollider>(entities.position, kCameraCollisionRadius);
	registry.emplace_or_replace<ecs::component::OnTranslateCallback>(entities.position,
																	 [&](Vec3 position)
																	 {
																		 params.camera.position = std::move(position);
																		 updateScene(params, entities);
																	 });
	*/
}

void setupUI(Parameters& params, const Entities& entities)
{
	auto& ctx				= di::getContext();
	auto& uiStateManager	= ctx.create<IUIStateManager&>();
	auto& uiManager			= ctx.create<IUIManager&>();
	auto& settings			= ctx.create<Settings&>();
	auto& input				= ctx.create<IInputManager&>();
	auto& controllerService = ctx.create<ICameraControllerService&>();

	uiManager.onDetailsViewDraw(
		[&]()
		{
			const float sceneSize = settings.grid.gridSize;

			ui::State& ui		  = uiStateManager.getState();

			{
				bool changed = false;
				changed |= ImGui::SliderFloat("pitch (X)", &params.rotation.x(), -179.9F, 179.9F);
				changed |= ImGui::SliderFloat("yaw   (Y)", &params.rotation.y(), -179.9F, 179.9F);
				changed |= ImGui::SliderFloat("roll  (Z)", &params.rotation.z(), -179.9F, 179.9F);
				if (changed)
				{
					const float yaw		  = degToRad(params.rotation.x());
					const float pitch	  = degToRad(params.rotation.y());
					const float roll	  = degToRad(params.rotation.z());
					const Quat qFromEuler = Eigen::AngleAxisf(yaw, Vec3::UnitX())	   //
											* Eigen::AngleAxisf(pitch, Vec3::UnitY())  //
											* Eigen::AngleAxisf(roll, Vec3::UnitZ());
					params.quaternion = qFromEuler.normalized();
					updateScene(params, entities);
				};
			}

			ImGui::Separator();

			{
				bool changed = false;
				changed |= ImGui::SliderFloat("quat (X)", &params.quaternion.x(), -1, 1);
				changed |= ImGui::SliderFloat("quat (Y)", &params.quaternion.y(), -1, 1);
				changed |= ImGui::SliderFloat("quat (Z)", &params.quaternion.z(), -1, 1);
				changed |= ImGui::SliderFloat("quat (W)", &params.quaternion.w(), -1, 1);
				if (changed)
				{
					params.quaternion.normalize();

					Vec3 eulerXYZ		= params.quaternion.toRotationMatrix().eulerAngles(0, 1, 2);
					params.rotation.x() = radToDeg(eulerXYZ.x());
					params.rotation.y() = radToDeg(eulerXYZ.y());
					params.rotation.z() = radToDeg(eulerXYZ.z());

					updateScene(params, entities);
				};
			}

			ImGui::Checkbox("CCW", &params.rotationDirectionCCW);
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
	setupInteractionLogic(params, entities);
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
