#include "scene_service.h"

#include "core/scene_service.h"

#include "core/ecs.h"
#include "core/math.h"
#include "core/settings.h"
#include "core/types.h"

#include <cmath>
#include <limits>

namespace gs
{

ecs::Entity SceneService::addModel(const scene::Model& model)
{
	const ecs::Entity entity = registry_->create();
	auto& mesh				 = registry_->emplace<ecs::component::Mesh>(entity);
	mesh.origin				 = model.origin;
	mesh.scale				 = model.scale;
	mesh.type				 = model.type;
	registry_->emplace<ecs::component::Position>(entity, model.position);
	return entity;
}

ecs::Entity SceneService::addPlane(const scene::Plane& plane)
{
	const ecs::Entity entity = registry_->create();
	registry_->emplace<ecs::component::MainColor>(entity, plane.color);
	registry_->emplace<ecs::component::Normal>(entity, plane.normal);
	registry_->emplace<ecs::component::Distance>(entity, plane.distance);
	return entity;
}

ecs::Entity SceneService::addVector(const scene::Vector& vec)
{
	const ecs::Entity entity = registry_->create();
	registry_->emplace<ecs::component::Arrow>(entity, vec.arrowSize);
	registry_->emplace<ecs::component::MainColor>(entity, vec.color);
	registry_->emplace<ecs::component::Position>(entity, vec.begin);
	registry_->emplace<ecs::component::EndPosition>(entity, vec.end);
	registry_->emplace<ecs::component::Thickness>(entity, vec.thickness);
	return entity;
}

AABB SceneService::getSceneBounds() const
{
	AABB bounds{
		.min = Vec3::Constant(+std::numeric_limits<float>::infinity()),
		.max = Vec3::Constant(-std::numeric_limits<float>::infinity()),
	};

	for (const auto& [_, point] : registry_->view<ecs::component::Position>().each())
	{
		bounds.min = bounds.min.cwiseMin(point.val);  // coefficient-wise minimum
		bounds.max = bounds.max.cwiseMax(point.val);  // coefficient-wise maximum
	}

	for (const auto& [_, point] : registry_->view<ecs::component::EndPosition>().each())
	{
		bounds.min = bounds.min.cwiseMin(point.val);  // coefficient-wise minimum
		bounds.max = bounds.max.cwiseMax(point.val);  // coefficient-wise maximum
	}

	// if empty or to small
	if (bounds.min.x() >= bounds.max.x()	 //
		&& bounds.min.y() >= bounds.max.y()	 //
		&& bounds.min.z() >= bounds.max.z())
	{
		bounds.min = Vec3::Constant(-settings_->grid.gridSize);
		bounds.max = Vec3::Constant(+settings_->grid.gridSize);
	}

	return bounds;
}

Vec3 SceneService::getSceneCenter() const
{
	const AABB bounds = getSceneBounds();
	// scene center is exactly in the middle between min and max
	return std::lerp(bounds.min, bounds.max, 0.5F);	 // NOLINT(*-magic-numbers)
}

ecs::Entity SceneService::addPoint(const scene::Point& point)
{
	const ecs::Entity entity = registry_->create();
	registry_->emplace<ecs::component::Position>(entity, point.position);
	registry_->emplace<ecs::component::MainColor>(entity, point.color);
	registry_->emplace<ecs::component::Radius>(entity, point.radius);
	return entity;
}

ecs::Entity SceneService::addDashedLine(const scene::DashedLine& line)
{
	const ecs::Entity entity = registry_->create();
	registry_->emplace<ecs::component::Dashed>(entity, line.dashLength, line.gapLength);
	registry_->emplace<ecs::component::Position>(entity, line.begin);
	registry_->emplace<ecs::component::EndPosition>(entity, line.end);
	registry_->emplace<ecs::component::MainColor>(entity, line.color);
	registry_->emplace<ecs::component::Thickness>(entity, line.thickness);
	return entity;
}

ecs::Entity SceneService::addText(const scene::Text& txt)
{
	const ecs::Entity entity = registry_->create();
	registry_->emplace<ecs::component::Position>(entity, txt.position);
	registry_->emplace<ecs::component::Label>(entity, txt.text, txt.fontSize);
	registry_->emplace<ecs::component::MainColor>(entity, txt.color);
	return entity;
}

ecs::Entity SceneService::addAABB(const scene::AABB& aabb)
{
	const ecs::Entity entity = registry_->create();
	registry_->emplace<ecs::component::Position>(entity, aabb.position);
	registry_->emplace<ecs::component::MainColor>(entity, aabb.color);
	registry_->emplace<ecs::component::AABBSize>(entity, aabb.size);
	registry_->emplace<ecs::component::Thickness>(entity, aabb.thickness);
	return entity;
}

void SceneService::setPosition(ecs::Entity entity, const Vec3& val)
{
	registry_->get<ecs::component::Position>(entity).val = val;
}

void SceneService::setEndPosition(ecs::Entity entity, const Vec3& val)
{
	registry_->get<ecs::component::EndPosition>(entity).val = val;
}

void SceneService::setNormal(ecs::Entity entity, const Vec3& val)
{
	registry_->get<ecs::component::Normal>(entity).val = val;
}

void SceneService::setDistance(ecs::Entity entity, float val)
{
	registry_->get<ecs::component::Distance>(entity).val = val;
}

void SceneService::setAABBSize(ecs::Entity entity, float val)
{
	registry_->get<ecs::component::AABBSize>(entity).val = val;
}

}  // namespace gs