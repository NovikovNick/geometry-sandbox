/**
 * @file sample.cpp
 * @brief curves and splines
 *
 * @todo implement NURBS
 *
 * @author MetalHeart
 */
#include "sample.h"

#include "curve_math.h"

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
#include "feature/camera_idle_rotation_manager.h"
#include "interaction/ecs_components.h"
#include "interaction/service.h"
#include "render/low_level_service.h"
#include "ui/manager.h"
#include "ui/state_manager.h"
#include "ui/ui.h"

#include "imgui.h"

#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <functional>
#include <utility>
#include <vector>

namespace gs
{
namespace
{
enum class BezierAlgorithm : std::uint8_t
{
	DeCasteljau,
	Bernshtain,
	Count
};
enum class CameraProjection : std::uint8_t
{
	Orthographic,
	Perspective,
	Count
};

/** @brief Parameters that the user configures through the UI update or dragging scene objects */
struct Parameters
{
	std::vector<Vec3> controlPoints;
	std::vector<Vec3> monomials;
	BezierAlgorithm algorithm	= BezierAlgorithm::DeCasteljau;
	CameraProjection projection = CameraProjection::Perspective;
	bool showDerivative			= false;
};

/** @brief Identifiers of scene objects related to the camera */
struct Entities
{
	std::vector<ecs::Entity> controlPoints;
};

/** @brief Create scene objecs and define colors, radiuses, font sizes and etc */
void setupScene(Parameters& params, Entities& entities)
{
	// NOLINTBEGIN(*-magic-numbers)
	auto& ctx					  = di::getContext();
	auto& uiStateManager		  = ctx.create<IUIStateManager&>();
	auto& sceneService			  = ctx.create<ISceneService&>();
	auto& renderer				  = ctx.create<render::IFacade&>();
	auto& graphic				  = ctx.create<render::ILowLevelService&>();
	auto& cameraService			  = ctx.create<ICameraService&>();
	auto& cameraControllerService = ctx.create<ICameraControllerService&>();
	auto& log					  = ctx.create<ILogManager&>();
	auto& settings				  = ctx.create<Settings&>();

	constexpr int count			  = 4;
	constexpr float radius		  = 0.05F;
	constexpr Color color		  = Color::yellow();

	Camera& camera				  = uiStateManager.getState().cameras[0];
	camera.position				  = Vec3{2, 2, 5};
	cameraService.lookAt(camera, Vec3{2, 2, 0});

	params.monomials.resize(count);
	params.controlPoints.resize(count);
	params.controlPoints[0] = Vec3{0.7F, 2, 0};
	params.controlPoints[1] = Vec3{1, 4, 0};
	params.controlPoints[2] = Vec3{4.5F, 2, 0};
	params.controlPoints[3] = Vec3{1, 0, 0};
	// params.controlPoints[4] = Vec3{0.7F, 2, 0};

	params.showDerivative = false;
	params.algorithm	  = BezierAlgorithm::DeCasteljau;

	entities.controlPoints.reserve(count);

	for (int i = 0; i < count; ++i)
	{
		entities.controlPoints.push_back(sceneService.addPoint({.position = params.controlPoints[i],  //
																.color	  = color,
																.radius	  = radius}));
	}

	renderer.onRender(
		[&]
		{
			Camera& cam = uiStateManager.getState().cameras[0];

			if (params.projection == CameraProjection::Orthographic)
			{
				camera.perspective = false;
				camera.fovY		   = 140.0F;
				cameraControllerService.setControllerType(CameraControllerType::Orbit);
				settings.showControlRotation = false;
			}
			else
			{

				camera.perspective = true;
				camera.fovY		   = 55.0F;
				cameraControllerService.setControllerType(CameraControllerType::Free);
				settings.showControlRotation = true;
			}

			constexpr int curveStepCount	 = 64;
			constexpr float curveStepCountF	 = curveStepCount;
			constexpr float curveThickness	 = 0.02F;
			constexpr float controlThickness = 0.01F;

			const auto algo					 = params.algorithm == BezierAlgorithm::Bernshtain ? &calcBezierWithBernsteinPolynomial
																							   : &calcBezierWithDeCasteljauAlgorithm;

			const Vec3 a1					 = params.monomials[1];
			const Vec3 a2					 = params.monomials[2];
			const Vec3 a3					 = params.monomials[3];

			for (int i = 1; i <= curveStepCount; ++i)
			{
				const float t1 = static_cast<float>(i - 1) / curveStepCountF;
				const float t2 = static_cast<float>(i) / curveStepCountF;
				const Vec3 p1  = algo(params.controlPoints, t1);
				const Vec3 p2  = algo(params.controlPoints, t2);

				if (params.showDerivative)
				{
					const Vec3 derivative = a1 + (2 * a2 * t1) + (3 * a3 * t1 * t1);
					graphic.drawArrow(p1, p1 + derivative, controlThickness, Color::white(), 3);
				}
				graphic.drawLine(p1, p2, curveThickness, Color::yellow());
			}

			for (int i = 1; i < params.controlPoints.size(); ++i)
			{
				const Vec3 p1 = params.controlPoints[i - 1];
				const Vec3 p2 = params.controlPoints[i];
				graphic.drawLine(p1, p2, controlThickness, Color::white());
			}
		});

	// NOLINTEND(*-magic-numbers)
}

/** @brief Recalculate data and update positions after UI update or dragging scene objects */
void updateScene(Parameters& params, const Entities& entities)
{
	auto& ctx			 = di::getContext();
	auto& sceneService	 = ctx.create<ISceneService&>();
	auto& uiStateManager = ctx.create<IUIStateManager&>();

	for (int i = 0; i < params.controlPoints.size(); ++i)
	{
		sceneService.setPosition(entities.controlPoints[i], params.controlPoints[i]);
	}
	updateCoefficientMonomial(params.controlPoints, params.monomials);
}

void setupInteractionLogic(Parameters& params, const Entities& entities)
{
	auto& ctx						 = di::getContext();
	auto& registry					 = ctx.create<ecs::Registry&>();

	constexpr float kCollisionRadius = 0.1F;

	for (int i = 0; i < entities.controlPoints.size(); ++i)
	{
		const ecs::Entity id = entities.controlPoints[i];
		registry.emplace_or_replace<ecs::component::SphereCollider>(id, kCollisionRadius);
		registry.emplace_or_replace<ecs::component::OnTranslateCallback>(id,
																		 [&, i](Vec3 position)
																		 {
																			 params.controlPoints[i] = std::move(position);
																			 updateScene(params, entities);
																		 });
	}
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
			gs::Camera& camera	  = ui.cameras.back();

			bool changed		  = false;

			drawCombo("Projection", params.projection, "2D", "3D");

			changed |= drawCombo("BezierAlgorithm", params.algorithm, "de Casteljau", "Bernshtain");

			ImGui::Checkbox("Show derivative", &params.showDerivative);

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
