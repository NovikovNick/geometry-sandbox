#include "interaction/service.h"

#include "core/camera_service.h"
#include "core/ecs.h"
#include "core/input_manager.h"
#include "core/math.h"
#include "core/types.h"
#include "interaction/ecs_components.h"
#include "interaction/translate_gizmo.h"
#include "ui/manager.h"
#include "ui/state_manager.h"

#include <limits>

namespace gs
{
namespace
{
void onTranslate(entt::registry& registry, entt::entity entity)
{
	if (auto* callback = registry.try_get<ecs::component::OnTranslateCallback>(entity); callback)
	{
		auto& translated = registry.get<ecs::component::Translated>(entity);
		callback->callback(translated.position);
	}
};
void onSelect(entt::registry& registry, entt::entity entity)
{
	if (auto* callback = registry.try_get<ecs::component::OnSelectCallback>(entity); callback)
	{
		callback->callback();
	}
};

void onHoverBegin(entt::registry& registry, entt::entity entity)
{
	if (auto* callback = registry.try_get<ecs::component::OnHoverCallback>(entity); callback)
	{
		callback->callback(true);
	}
};
void onHoverEnd(entt::registry& registry, entt::entity entity)
{
	if (auto* callback = registry.try_get<ecs::component::OnHoverCallback>(entity); callback)
	{
		callback->callback(false);
	}
};
}  // namespace

void InteractionService::init()
{
	ui::State& ui		= uiStateManager_->getState();
	auto onMousePressed = [&]
	{
		if (ui.transformGizmo.isHovered())	// first check all vieport widgets
		{
			translateGizmoService_->constrain(ui.mouseRay, ui.transformGizmo);
		}
		else
		{
			if (selected_ != hovered_)
			{
				registry_->remove<ecs::component::Selected>(selected_);
			}

			selected_ = hovered_;

			if (selected_ != entt::null)
			{
				registry_->emplace_or_replace<ecs::component::Selected>(selected_);

				const auto& position	   = registry_->get<ecs::component::Position>(selected_);
				ui.transformGizmo.active   = true;
				ui.transformGizmo.position = position.val;
			}
			else
			{
				ui.transformGizmo.active	 = false;
				ui.transformGizmo.dragOffset = Vec3::Zero();
			}
		}
	};
	auto onMouseReleased = [&]
	{
		if (ui.transformGizmo.isConstrained())
		{
			registry_->remove<ecs::component::Selected>(selected_);
			registry_->remove<ecs::component::Translated>(selected_);

			// reset after drag
			selected_		  = entt::null;
			ui.transformGizmo = TranslateGizmo{};
		}
	};
	inputManager_->onPress(InputKey::MouseLeft, onMousePressed);
	inputManager_->onRelease(InputKey::MouseLeft, onMouseReleased);

	registry_->on_update<ecs::component::Translated>().connect<&onTranslate>();
	registry_->on_construct<ecs::component::Selected>().connect<&onSelect>();
	registry_->on_construct<ecs::component::Hovered>().connect<&onHoverBegin>();
	registry_->on_destroy<ecs::component::Hovered>().connect<&onHoverEnd>();
}

void InteractionService::update()
{
	if (uiManager_->isMouseCaptured())
	{
		return;
	}

	const Vec2 screenPos = inputManager_->getCursorScreenPosition();
	ui::State& ui		 = uiStateManager_->getState();
	ui.mouseRay			 = cameraService_->getScreenToWorldRay(screenPos, ui.cameras[ui.activeCameraIndex]);

	// 1. raycast
	if (ui.transformGizmo.isActive())
	{
		const Camera& camera = ui.cameras[ui.activeCameraIndex];
		const float scale	 = (camera.position - ui.transformGizmo.position).norm();
		translateGizmoService_->hover(ui.mouseRay, ui.transformGizmo, scale);

		if (ui.transformGizmo.isConstrained())
		{
			translateGizmoService_->translate(ui.mouseRay, ui.transformGizmo);

			auto& translate	   = registry_->get_or_emplace<ecs::component::Translated>(selected_);
			translate.position = ui.transformGizmo.position;
			registry_->patch<ecs::component::Translated>(selected_);
		}
	}

	entt::entity hovered = entt::null;
	float distance		 = std::numeric_limits<float>::max();
	for (const auto& [entity, pos, collider] : registry_->view<ecs::component::Position, ecs::component::SphereCollider>().each())
	{
		const Sphere sphere{.center = pos.val, .radius = collider.radius};
		const Intersection intersection = intersect(ui.mouseRay, sphere);
		if (intersection.hit && intersection.distance < distance)
		{
			distance = intersection.distance;
			hovered	 = entity;
		}
	}

	for (const auto& [entity, position, collider] : registry_->view<ecs::component::Position, ecs::component::AABBCollider>().each())
	{
		const AABB aabb{.min = position.val - collider.size / 2, .max = position.val + collider.size / 2};
		const Intersection intersection = intersect(ui.mouseRay, aabb);
		if (intersection.hit && intersection.distance < distance)
		{
			distance = intersection.distance;
			hovered	 = entity;
		}
	}

	if (hovered_ != hovered)
	{
		registry_->remove<ecs::component::Hovered>(hovered_);
		hovered_ = hovered;
		if (hovered_ != entt::null)
		{
			registry_->emplace<ecs::component::Hovered>(hovered_);
		}
	}

	ui.cursorType = hovered_ == entt::null ? CursorType::Arrow : CursorType::PointingHand;
}
}  // namespace gs
