#include "render/facade.h"

#include "core/ecs.h"
#include "core/log_manager.h"
#include "core/math.h"
#include "core/resource_manager.h"
#include "core/settings.h"
#include "core/types.h"
#include "core/window_manager.h"
#include "interaction/translate_gizmo.h"
#include "render/frustum_draw_service.h"
#include "render/low_level_service.h"
#include "render/mesh_instanced_draw_service.h"
#include "render/viewport_manager.h"
#include "ui/manager.h"

#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>	// for std::uint16_t
#include <cstring>	// for std::memcpy
#include <memory>
#include <span>

namespace gs::render
{
Facade::Facade(const std::shared_ptr<Settings>& settings,
			   const std::shared_ptr<ecs::Registry>& registry,
			   const std::shared_ptr<IResourceManager>& resourceManager,
			   const std::shared_ptr<IUIManager>& uiManager,
			   const std::shared_ptr<ITranslateGizmoRenderService>& translateGizmoRenderer,
			   const std::shared_ptr<ILowLevelService>& graphic,
			   const std::shared_ptr<IViewportManager>& viewportManager,
			   const std::shared_ptr<IMeshInstancedDrawService>& meshInstancing,
			   const std::shared_ptr<IFrustumDrawService>& frustumDrawer,
			   const std::shared_ptr<ILogManager>& logService)
	: settings_(settings),								//
	  registry_(registry),								//
	  resourceManager_(resourceManager),				//
	  uiManager_(uiManager),							//
	  translateGizmoRenderer_(translateGizmoRenderer),	//
	  graphic_(graphic),								//
	  viewportManager_(viewportManager),				//
	  meshInstancing_(meshInstancing),					//
	  frustumDrawer_(frustumDrawer),					//
	  logService_(logService)
{
}

void Facade::render()
{
	const ui::State& ui				 = uiManager_->getState();
	const Timepoint measurementStart = Clock::now();

	// 1. update viewports
	for (std::size_t i = 0; i < ui.cameras.size(); ++i)
	{
		const Camera& camera = ui.cameras[i];

		viewportManager_->setupViewport(i);
		graphic_->setupCamera(camera);

		if (i == ui.activeCameraIndex && ui.transformGizmo.active)
		{
			const float scale = (camera.position - ui.transformGizmo.position).norm();

			graphic_->clearBackground(Color::transparent());
			translateGizmoRenderer_->render(ui.transformGizmo, scale);
		}
		else
		{
			graphic_->clearBackground(Color::black());
			drawScene(camera);
		}

		graphic_->cleanupCamera();
		viewportManager_->cleanupViewport();
	}

	// 2. handle cursor type
	switch (ui.cursorType)
	{
		case CursorType::PointingHand: SetMouseCursor(MOUSE_CURSOR_POINTING_HAND); break;
		default: SetMouseCursor(MOUSE_CURSOR_ARROW); break;
	}

	// 3. draw
	BeginDrawing();

	const Camera& activeCamera = ui.cameras[ui.activeCameraIndex];
	graphic_->setupCamera(activeCamera);

	graphic_->clearBackground(settings_->canvasBackgroundColor);
	drawScene(activeCamera);

	if (settings_->showCursorRay)
	{
		graphic_->drawArrow(ui.mouseRay.origin,
							ui.mouseRay.origin + ui.mouseRay.direction * settings_->grid.gridSize * settings_->grid.cellSize,
							settings_->lineThick,
							Color::yellow(),
							settings_->gizmoArrowSize);
	}

	// camera frustums
	for (std::size_t i = 0; i < ui.cameras.size(); ++i)
	{
		if (i != ui.activeCameraIndex)
		{
			frustumDrawer_->drawFrustum(i, ui.cameras[i]);
		}
	}

	if (settings_->showCollisions)
	{
		registry_->view<ecs::component::Position, ecs::component::SphereCollider>().each(
			[&](entt::entity, ecs::component::Position& position, ecs::component::SphereCollider& collider)
			{ graphic_->drawSphereWires(position.val, Color::yellow(), collider.radius); });

		registry_->view<ecs::component::Position, ecs::component::AABBCollider>().each(
			[&](entt::entity, ecs::component::Position& position, ecs::component::AABBCollider& collider)
			{
				const Vec3 aabbMin = position.val - collider.size / 2;
				const Vec3 aabbMax = position.val + collider.size / 2;
				graphic_->drawBoundingBox(aabbMin, aabbMax, Color::yellow());
			});
	}

	graphic_->cleanupCamera();

	// viewport widgets
	if (ui.transformGizmo.active)
	{
		Texture texture = viewportManager_->getViewport2D(ui.activeCameraIndex);

		// NOTE: Render texture must be y-flipped due to default OpenGL coordinates (left-bottom)
		const Rectangle src{0.0F, 0.0F, static_cast<float>(texture.width), static_cast<float>(-texture.height)};

		const Camera& camera = ui.cameras[ui.activeCameraIndex];
		const Rectangle dst{0.0F, 0.0F, static_cast<float>(camera.width), static_cast<float>(camera.height)};

		const Vector2 origin{0.0F, 0.0F};
		const float rotation					= 0.0F;
		const ::Color tint						= ::WHITE;

		const Shader fxaaShader					= resourceManager_->getShader(ShaderType::FXAA);

		const int viewportSizeLoc				= GetShaderLocation(fxaaShader, "uViewportSize");

		const std::array<float, 2> viewportSize = {static_cast<float>(texture.width), static_cast<float>(texture.height)};
		SetShaderValue(fxaaShader, viewportSizeLoc, viewportSize.data(), SHADER_UNIFORM_VEC2);

		BeginShaderMode(fxaaShader);
		DrawTexturePro(texture, src, dst, origin, rotation, tint);
		EndShaderMode();
	}

	// 3d text
	{
		for (const auto& [entity, label, pos, color] :
			 registry_->view<ecs::component::Label, ecs::component::Position, ecs::component::MainColor>().each())
		{
			graphic_->drawText(activeCamera, label.text, pos.val, color.val, label.fontSize);
		}
	}

	if (settings_->showConsole)
	{
		uiManager_->getState().performance.render = (Clock::now() - measurementStart);
		debug();
	}

	// GUI
	uiManager_->render();

	EndDrawing();
}

void Facade::debug()
{
	ui::State& ui = uiManager_->getState();
	{
		logService_->frameLog("FPS {:>3}, tick {:3}, animation {:3}, interaction {:3}, render {:3}, total {:3}",
							  GetFPS(),
							  std::chrono::duration_cast<Milliseconds>(ui.performance.tick).count(),
							  std::chrono::duration_cast<Milliseconds>(ui.performance.animation).count(),
							  std::chrono::duration_cast<Milliseconds>(ui.performance.interaction).count(),
							  std::chrono::duration_cast<Milliseconds>(ui.performance.render).count(),
							  std::chrono::duration_cast<Milliseconds>(ui.performance.total).count());
	}

	{
		const Camera& camera = ui.cameras[ui.activeCameraIndex];
		logService_->frameLog("{} camera, fov {}", camera.perspective ? "Perspective" : "Orthographic", camera.fov);
		logService_->frameLog("\tpositon [{:>8.2f},{:>8.2f},{:>8.2f}]", camera.position.x(), camera.position.y(), camera.position.z());
		logService_->frameLog("\ttarget [{:>8.2f},{:>8.2f},{:>8.2f}]", camera.target.x(), camera.target.y(), camera.target.z());
	}

	if (ui.transformGizmo.isActive())
	{
		logService_->frameLog("Translate Gizmo [{:>8.2f},{:>8.2f},{:>8.2f}]",
							  ui.transformGizmo.position.x(),
							  ui.transformGizmo.position.y(),
							  ui.transformGizmo.position.z());
	}
}

void Facade::drawScene(const Camera& camera)
{
	graphic_->drawAxesGrid(camera.upAxis, settings_->grid);

	// Points
	{
		const Shader& intansingShader = resourceManager_->getShader(ShaderType::MeshInstancing);
		const Model& point			  = resourceManager_->getModel(ModelType::Point);

		int meshInstanceIdx			  = 0;	// index for mesh instancing
		for (const auto& [_, position, radius, color] :
			 registry_->view<ecs::component::Position, ecs::component::Radius, ecs::component::MainColor>().each())
		{
			// increase if not enogh
			if (meshInstanceIdx >= pointTransforms_.size())
			{
				pointTransforms_.emplace_back(Mat4::Identity());
				pointColors_.push_back(Color::white());
			}

			// clang-format off
			pointTransforms_[meshInstanceIdx] << radius.val,      0.0F,	      0.0F,	 position.val.x(),
												 	  0.0F,	 radius.val,      0.0F,	 position.val.y(),
												 	  0.0F,		  0.0F,  radius.val, position.val.z(),
													  0.0F,		  0.0F,		  0.0F,		    1.0F;
			// clang-format on

			pointColors_[meshInstanceIdx] = color.val.normalized();

			meshInstanceIdx++;	// next
		}

		std::array<float, 3> cameraPos = {camera.position.x(), camera.position.y(), camera.position.z()};

		// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		SetShaderValue(intansingShader, intansingShader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos.data(), SHADER_UNIFORM_VEC3);

		meshInstancing_->drawMeshInstanced(point.meshes[0], point.materials[0], std::span{pointTransforms_}, std::span{pointColors_});
		// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
	}

	// Models
	{
		for (const auto& [_, mesh, pos] : registry_->view<ecs::component::Mesh, ecs::component::Position>().each())
		{
			Vec3 position = pos.val - mesh.origin;
			graphic_->drawModel(mesh.type, position, mesh.scale);
		}
	}

	// Arrow
	{
		for (const auto& [_, arrow, pos, endPos, color, thickness] : registry_
																		 ->view<ecs::component::Arrow,
																				ecs::component::Position,
																				ecs::component::EndPosition,
																				ecs::component::MainColor,
																				ecs::component::Thickness>()
																		 .each())
		{
			graphic_->drawArrow(pos.val, endPos.val, thickness.val, color.val, arrow.size);
		}
	}

	// AABB
	{
		for (const auto& [_, position, size, color, thickness] : registry_
																	 ->view<ecs::component::Position,  //
																			ecs::component::AABBSize,
																			ecs::component::MainColor,
																			ecs::component::Thickness>()
																	 .each())
		{

			AABB const aabb{.min = position.val - Vec3::Constant(size.val / 2), .max = position.val + Vec3::Constant(size.val / 2)};
			graphic_->drawAABBFramed(aabb, color.val, thickness.val);
		}
	}

	// Dashed line
	{
		for (const auto& [_, dashed, pos, endPos, color, thickness] : registry_
																		  ->view<ecs::component::Dashed,
																				 ecs::component::Position,
																				 ecs::component::EndPosition,
																				 ecs::component::MainColor,
																				 ecs::component::Thickness>()
																		  .each())
		{
			graphic_->drawDashedLine(pos.val, endPos.val, thickness.val, color.val, dashed.dashLength, dashed.gapLength);
		}
	}

	// Plane is drawn last, as it is usually partially transparent.
	{
		for (const auto& [_, color, normal, distance] :
			 registry_->view<ecs::component::MainColor, ecs::component::Normal, ecs::component::Distance>().each())
		{
			graphic_->drawPlane(Plane{.normal = normal.val, .distance = distance.val}, color.val);
		}
	}
}
}  // namespace gs::render
