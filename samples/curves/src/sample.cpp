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
#include "core/settings.h"
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
#include <numbers>
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

struct Spline
{
	std::vector<Vec3> poles;  // control points
	std::vector<float> knots;
	int degree = 3;

	// todo: implement in future:
	// std::vector<float> weights;
	// std::vector<int> multiplicities;
	// bool rational;
	// bool periodic;
};

/** @brief Parameters that the user configures through the UI update or dragging scene objects */
struct Parameters
{
	Spline spline;
	BezierAlgorithm algorithm	= BezierAlgorithm::DeCasteljau;
	CameraProjection projection = CameraProjection::Perspective;
	bool showDerivative			= false;
	bool addPoint				= false;

	int curveSegemntsCount		= 64;
	float progress				= 0;
};

/** @brief Identifiers of scene objects related to the camera */
struct Entities
{
	std::vector<ecs::Entity> poles;
};

/** @brief Recalculate data and update positions after UI update or dragging scene objects */
void updateScene(Parameters& params, const Entities& entities)
{
	auto& ctx			 = di::getContext();
	auto& sceneService	 = ctx.create<ISceneService&>();
	auto& uiStateManager = ctx.create<IUIStateManager&>();

	for (int i = 0; i < params.spline.poles.size(); ++i)
	{
		sceneService.setPosition(entities.poles[i], params.spline.poles[i]);
	}
	// updateCoefficientMonomial(params.poles, params.monomials);
}

/** @brief Create scene objecs and define colors, radiuses, font sizes and etc */
void setupScene(Parameters& params, Entities& entities)
{
	// NOLINTBEGIN(*-magic-numbers)
	auto& ctx					  = di::getContext();
	auto& registry				  = ctx.create<ecs::Registry&>();
	auto& uiStateManager		  = ctx.create<IUIStateManager&>();
	auto& sceneService			  = ctx.create<ISceneService&>();
	auto& renderer				  = ctx.create<render::IFacade&>();
	auto& graphic				  = ctx.create<render::ILowLevelService&>();
	auto& cameraService			  = ctx.create<ICameraService&>();
	auto& cameraControllerService = ctx.create<ICameraControllerService&>();
	auto& input					  = ctx.create<IInputManager&>();
	auto& log					  = ctx.create<ILogManager&>();
	auto& settings				  = ctx.create<Settings&>();

	constexpr int count			  = 4;
	constexpr float radius		  = 0.05F;
	constexpr Color color		  = Color::yellow();

	settings.showCursorRay		  = false;

	Camera& camera				  = uiStateManager.getState().cameras[0];
	camera.position				  = Vec3{2, 2, 5};
	cameraService.lookAt(camera, Vec3{2, 2, 0});

	input.onPress(InputKey::MouseLeft,
				  [&]()
				  {
					  if (!params.addPoint)
					  {
						  return;
					  }

					  params.addPoint				  = params.spline.poles.size() < 3;

					  const Ray mouseRay			  = cameraService.getScreenToWorldRay(input.getCursorScreenPosition(), camera);
					  const Intersection intersection = intersect(mouseRay, Plane{Vec3{0, 0, 1}, 0.0F});
					  const Vec3 newPoint			  = intersection.point;
					  const std::size_t idx			  = params.spline.poles.size();

					  params.spline.poles.push_back(newPoint);

					  const ecs::Entity id = sceneService.addPoint({.position = newPoint, .color = color, .radius = radius});
					  entities.poles.push_back(id);
					  registry.emplace_or_replace<ecs::component::SphereCollider>(id, .1F);
					  registry.emplace_or_replace<ecs::component::OnTranslateCallback>(id,
																					   [&, idx](Vec3 position)
																					   {
																						   params.spline.poles[idx] = position;
																						   updateScene(params, entities);
																					   });

					  log.log("added new point at {}, {}, {}", newPoint.x(), newPoint.y(), newPoint.z());
				  });

	params.showDerivative = false;
	params.algorithm	  = BezierAlgorithm::DeCasteljau;
	params.projection	  = CameraProjection::Orthographic;

	renderer.onRender(
		[&]
		{
			auto drawCircle = [&](const Vec3& center, const float radius, const Color color, const float thickness)
			{
				if (params.showDerivative)
				{
					Vec3 prevPoint{radius, 0, 0};
					for (int j = 0; j <= params.curveSegemntsCount; ++j)
					{
						const float t	 = static_cast<float>(j) / params.curveSegemntsCount;
						const float t2Pi = 2 * std::numbers::pi * t;
						Vec3 currPoint	 = Vec3{std::cos(t2Pi), std::sin(t2Pi), 0} * radius;

						graphic.drawLine(center + prevPoint, center + currPoint, thickness, color);

						prevPoint = currPoint;
					}
				}
			};

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

			const float controlThickness = settings.lineThickness / 2;
			const float arrowSize		 = settings.gizmoArrowSize;

			for (int i = 0; (i + params.spline.degree) < params.spline.poles.size(); i += params.spline.degree)
			{

				const std::span range{params.spline.poles.data() + i, static_cast<std::size_t>(params.spline.degree + 1)};

				const auto [_, a, b, c] = polinomialCoefficients(range);

				Vec3 prevPoint			= range[0];
				for (int j = 1; j <= params.curveSegemntsCount; ++j)
				{
					const float t = static_cast<float>(j) / params.curveSegemntsCount;

					// const Vec3 currPoint = evalBezierByBernstein(range, t);
					// const Vec3 currPoint = evalBezierByDeCasteljau(range, t);
					const Vec3 currPoint = evalBezierByPolinomialCoefficients(range, t);

					graphic.drawLine(prevPoint, currPoint, controlThickness, Color::white());

					prevPoint = currPoint;
				}

				if (params.showDerivative)
				{
					const float t					  = params.progress;
					const Vec3 d0					  = evalBezierByDeCasteljau(range, t);	// point on curve
					const Vec3 d1					  = a + 2 * b * t + 3 * c * t * t;		// tangent
					const Vec3 d2					  = 2 * b + 6 * c * t;
					const Vec3 d3					  = 6 * c;

					const float curvatory			  = d1.cross(d2).norm() / std::pow(d1.norm(), 3);
					const Vec3 normal				  = rotateVector(d1, Vec3::UnitZ(), degToRad(90.0F)).normalized();
					const Vec3 binormal				  = d1.cross(normal).normalized();

					const float curvatoryCircleRadius = 1.0F / curvatory;
					const Vec3 curvatoryCircleCenter  = normal * curvatoryCircleRadius;

					graphic.drawLine(d0, d0 + curvatoryCircleCenter, controlThickness, Color::white());
					drawCircle(d0 + curvatoryCircleCenter, curvatoryCircleRadius, Color::white(), controlThickness);

					graphic.drawArrow(d0, d0 + d1, controlThickness, Color::yellow(), arrowSize);
					graphic.drawArrow(d0, d0 + binormal, controlThickness, Color::red(), arrowSize);
				}
			}
		});

	// NOLINTEND(*-magic-numbers)
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
			ImGui::Checkbox("Add point", &params.addPoint);

			ImGui::SliderInt("Segments count", &params.curveSegemntsCount, 1, 128);
			ImGui::SliderFloat("Progress", &params.progress, 0.0F, 1.0F);

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
