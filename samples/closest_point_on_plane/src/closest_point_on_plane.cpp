/**
 * @file closest_point_on_plane.cpp
 * @brief Implementation of finding the closest point on a plane algorithm
 *
 * When dragging, the plane normal always points away from the origin. To
 * change the sign of the normal need to use UI slider.
 *
 * @todo plane has AABB collider, need to add a rotatable box collider.
 * @todo FPS drop cause of large dashed line.
 *
 * @author MetalHeart
 */
#include "closest_point_on_plane.h"

#include "animation/manager.h"
#include "animation/service.h"
#include "core/application.h"
#include "core/configuration.h"
#include "core/ecs.h"
#include "core/math.h"
#include "core/scene_service.h"
#include "core/types.h"
#include "feature/camera_idle_rotation_manager.h"
#include "interaction/ecs_components.h"
#include "interaction/service.h"
#include "ui/state_manager.h"

#include "imgui.h"

#include <cassert>
#include <functional>
#include <vector>

namespace gs
{
namespace
{
/**
 * @brief Parameters that the user configures through the UI update or dragging scene objects
 */
struct Parameters
{
	Vec3 point;
	Vec3 planeNormal;
	float planeDistance{};
};

/**
 * @brief Result of calculations of the algorithm for finding the closest point with intermediate data.
 */
struct Results
{
	Vec3 planePoint;
	float pointToPlaneNormalProjection{};
	Vec3 pointToPlaneNormal;
	Vec3 closestPoint;
};

struct Entities
{
	struct
	{
		ecs::Entity sphere;	 // point visual representation
		ecs::Entity label;
		ecs::Entity planeNormalComponentVector;	 // projection point onto the plane normal
		ecs::Entity planeNormalComponentLine;	 // dotted line
	} point;

	struct
	{
		ecs::Entity plane;	// plane visual representation
		ecs::Entity normalVector;
		ecs::Entity normalLabel;
		ecs::Entity distanceVector;
		ecs::Entity distanceLabel;
	} plane;

	struct
	{
		ecs::Entity sphere;	 // closest point visual representation
		ecs::Entity label;
		ecs::Entity connectionVector;  // vector q → q'
	} closestPoint;
};

struct UIContext
{
	bool idleRotationEnabled;
};

/**
 * @brief Main algorithm
 */
Results findClosestPoint(const Parameters& input)
{
	const Vec3 q  = input.point;
	const Vec3 n  = input.planeNormal;
	const float d = input.planeDistance;

	return Results{.planePoint					 = d * n,
				   .pointToPlaneNormalProjection = q.dot(n),
				   .pointToPlaneNormal			 = q.dot(n) * n,
				   .closestPoint				 = q + (d - q.dot(n)) * n};
}

/**
 * @brief Create scene objecs and define colors, radiuses, font sizes and etc, setup camera
 */
void setupScene(Parameters& params, Entities& entities)
{
	// NOLINTBEGIN(*-magic-numbers)
	auto& ctx			  = di::getContext();
	auto& sceneService	  = ctx.create<ISceneService&>();
	auto& uiStateManager  = ctx.create<IUIStateManager&>();
	auto& settings		  = ctx.create<Settings&>();

	params.point		  = Vec3{0.6F, 0.5F, 0.5F};
	params.planeNormal	  = Vec3{0.7F, 0.6F, -0.3F}.normalized();
	params.planeDistance  = 2.0F;

	ui::State& ui		  = uiStateManager.getState();
	Camera& camera		  = ui.cameras[ui.activeCameraIndex];
	camera.target		  = Vec3{0.0F, 1.5F, 0.0F};

	entities.point.sphere = sceneService.addPoint({.color = Color::blue(), .radius = 0.08F});
	entities.point.label  = sceneService.addText({.text = "q", .color = Color::white(), .fontSize = settings.canvasFontSize});
	entities.point.planeNormalComponentVector = sceneService.addVector({.color	   = Color::white(),  //
																		.thickness = 0.03F,
																		.arrowSize = 5.F});
	entities.point.planeNormalComponentLine	  = sceneService.addDashedLine({.color		= Color::white(),  //
																			.thickness	= 0.01F,
																			.dashLength = settings.dashLength,
																			.gapLength	= settings.dashLength});

	entities.plane.plane					  = sceneService.addPlane({.color = Color::green(64.0F)});
	entities.plane.normalVector				  = sceneService.addVector({.color = Color::red(), .thickness = 0.02F, .arrowSize = 5.F});
	entities.plane.normalLabel	  = sceneService.addText({.text = "n", .color = Color::white(), .fontSize = settings.canvasFontSize});
	entities.plane.distanceVector = sceneService.addVector({.color = Color::white(), .thickness = 0.01F, .arrowSize = 10.F});
	entities.plane.distanceLabel  = sceneService.addText({.text = "d", .color = Color::white(), .fontSize = settings.canvasFontSize});

	entities.closestPoint.sphere  = sceneService.addPoint({.color = Color::blue(), .radius = 0.025F});
	entities.closestPoint.label	  = sceneService.addText({.text = "q'", .color = Color::white(), .fontSize = settings.canvasFontSize});
	entities.closestPoint.connectionVector = sceneService.addVector({.color = Color::white(), .thickness = 0.02F, .arrowSize = 5.F});
	// NOLINTEND(*-magic-numbers)
}

/**
 * @brief Recalculate data and update positions of all geometry after UI update or dragging scene objects
 */
void updateScene(const Parameters& params, Results& res, const Entities& entities)
{
	auto& ctx		   = di::getContext();
	auto& sceneService = ctx.create<ISceneService&>();

	res				   = findClosestPoint(params);

	// point
	sceneService.setPosition(entities.point.sphere, params.point);
	sceneService.setPosition(entities.point.label, params.point);
	sceneService.setPosition(entities.point.planeNormalComponentVector, Vec3::Zero());
	sceneService.setEndPosition(entities.point.planeNormalComponentVector, res.pointToPlaneNormal);
	sceneService.setPosition(entities.point.planeNormalComponentLine, params.point);
	sceneService.setEndPosition(entities.point.planeNormalComponentLine, res.pointToPlaneNormal);

	// plane
	sceneService.setNormal(entities.plane.plane, params.planeNormal);
	sceneService.setDistance(entities.plane.plane, params.planeDistance);
	sceneService.setPosition(entities.plane.plane, res.planePoint);	 // ! for interaction
	sceneService.setPosition(entities.plane.normalVector, res.planePoint);
	sceneService.setEndPosition(entities.plane.normalVector, res.planePoint + params.planeNormal);
	sceneService.setPosition(entities.plane.normalLabel, res.planePoint + params.planeNormal);
	sceneService.setPosition(entities.plane.distanceVector, Vec3::Zero());
	sceneService.setEndPosition(entities.plane.distanceVector, res.planePoint);
	sceneService.setPosition(entities.plane.distanceLabel, res.planePoint * 0.5);  // NOLINT(*-magic-numbers)

	// closest point
	sceneService.setPosition(entities.closestPoint.sphere, res.closestPoint);
	sceneService.setPosition(entities.closestPoint.label, res.closestPoint);
	sceneService.setPosition(entities.closestPoint.connectionVector, params.point);
	sceneService.setEndPosition(entities.closestPoint.connectionVector, res.closestPoint);
}

/**
 * @brief Setup ecs components and add callbacks to handle dragging and hover/select animations
 */
void setupInteractionLogic(Parameters& params, Results& res, const Entities& entities)
{
	auto& ctx		  = di::getContext();
	auto& registry	  = ctx.create<ecs::Registry&>();
	auto& animService = ctx.create<animation::IService&>();

	// q
	constexpr float qRadius = 0.2F;
	registry.emplace<ecs::component::SphereCollider>(entities.point.sphere, qRadius);
	registry.emplace<ecs::component::OnHoverCallback>(entities.point.sphere,
													  animService.createHoverPointAnimation(entities.point.sphere));
	registry.emplace<ecs::component::OnSelectCallback>(entities.point.sphere,
													   animService.createSelectPointAnimation(entities.point.sphere));
	registry.emplace<ecs::component::OnTranslateCallback>(entities.point.sphere,
														  [&](const Vec3& pos)
														  {
															  params.point = pos;
															  updateScene(params, res, entities);
														  });

	// plane
	const float aabbSize = params.planeDistance / 2;
	registry.emplace<ecs::component::Position>(entities.plane.plane, params.planeNormal * params.planeDistance);
	registry.emplace<ecs::component::AABBCollider>(entities.plane.plane, Vec3{aabbSize, aabbSize, aabbSize});
	registry.emplace<ecs::component::OnHoverCallback>(entities.plane.plane,
													  animService.createHoverPlaneAnimation(entities.plane.plane));
	registry.emplace<ecs::component::OnTranslateCallback>(entities.plane.plane,
														  [&](const Vec3& pos)
														  {
															  params.planeNormal   = pos.normalized();
															  params.planeDistance = pos.norm();
															  updateScene(params, res, entities);
														  });
}

/**
 * @brief Setup ui to configure Parameters
 */
void setupUI(UIContext& uiContext, Parameters& params, Results& res, const Entities& entities)
{
	auto& ctx						= di::getContext();
	auto& uiStateManager			= ctx.create<IUIStateManager&>();
	auto& uiManager					= ctx.create<IUIManager&>();
	auto& sceneService				= ctx.create<ISceneService&>();
	auto& idleManager				= ctx.create<ICameraIdleRotationManager&>();
	auto& settings					= ctx.create<Settings&>();

	const int idleRotationCameraIdx = uiStateManager.getState().activeCameraIndex;
	idleManager.enableIdleRotation(idleRotationCameraIdx);
	uiContext.idleRotationEnabled = true;

	uiManager.onDetailsViewDraw(
		[&, idleRotationCameraIdx]()
		{
			bool changed			= false;
			const float sceneBounds = settings.grid.gridSize / 2;
			ImGui::SeparatorText("Point");
			changed |= ImGui::SliderFloat("q.x", &params.point.x(), -sceneBounds, sceneBounds);
			changed |= ImGui::SliderFloat("q.y", &params.point.y(), -sceneBounds, sceneBounds);
			changed |= ImGui::SliderFloat("q.z", &params.point.z(), -sceneBounds, sceneBounds);

			ImGui::SeparatorText("Plane");
			changed |= ImGui::SliderFloat("d", &params.planeDistance, -sceneBounds, sceneBounds);
			changed |= ImGui::SliderFloat("n.x", &params.planeNormal.x(), -1.0F, 1.0F);
			changed |= ImGui::SliderFloat("n.y", &params.planeNormal.y(), -1.0F, 1.0F);
			changed |= ImGui::SliderFloat("n.z", &params.planeNormal.z(), -1.0F, 1.0F);

			ImGui::SeparatorText("Camera");
			if (ImGui::Checkbox("Enable idle rotation", &uiContext.idleRotationEnabled))
			{
				if (uiContext.idleRotationEnabled)
				{
					idleManager.enableIdleRotation(idleRotationCameraIdx);
				}
				else
				{
					idleManager.disableIdleRotation();
				}
			}

			if (changed)
			{
				// correct the plane normal updated through the UI
				// it makes impossible to set the value accurately, but I just like the "elastic" effect itself
				assert(params.planeNormal != Vec3::Zero());
				params.planeNormal = params.planeNormal.normalized();

				updateScene(params, res, entities);
			};
		});
}

Application& getApp()
{
	static auto instance = di::getContext().create<gs::Application>();
	return instance;
}
}  // namespace

void init()
{
	static UIContext uiContext;
	static Entities entities;
	static Results res;
	static Parameters params;

	setupScene(params, entities);
	setupInteractionLogic(params, res, entities);
	setupUI(uiContext, params, res, entities);

	updateScene(params, res, entities);
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
