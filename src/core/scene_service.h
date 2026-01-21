/**
 * @file scene_service.h
 * @brief Creating and updating objects on the scene
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_SCENE_SERVICE_H
#define GEOMETRY_SANDBOX_SCENE_SERVICE_H

#include "core/ecs.h"
#include "core/types.h"

#include "boost/di.hpp"

#include <memory>

namespace gs
{
struct Settings;

/** @brief Creating and updating objects on the scene */
class ISceneService
{
  public:
	virtual ecs::Entity addModel(const scene::Model&)			= 0;
	virtual ecs::Entity addPlane(const scene::Plane&)			= 0;
	virtual ecs::Entity addVector(const scene::Vector&)			= 0;
	virtual ecs::Entity addPoint(const scene::Point&)			= 0;
	virtual ecs::Entity addText(const scene::Text&)				= 0;
	virtual ecs::Entity addDashedLine(const scene::DashedLine&) = 0;
	virtual ecs::Entity addAABB(const scene::AABB&)				= 0;

	virtual AABB getSceneBounds() const							= 0;
	virtual Vec3 getSceneCenter() const							= 0;

	virtual void setPosition(ecs::Entity, const Vec3&)			= 0;
	virtual void setEndPosition(ecs::Entity, const Vec3&)		= 0;
	virtual void setNormal(ecs::Entity, const Vec3&)			= 0;
	virtual void setDistance(ecs::Entity, float)				= 0;
	virtual void setAABBSize(ecs::Entity, float)				= 0;

	virtual ~ISceneService()									= default;
};

/** @brief basic ISceneService implementation */
class SceneService : public ISceneService
{
	std::shared_ptr<Settings> settings_;
	std::shared_ptr<ecs::Registry> registry_;

  public:
	SceneService(std::shared_ptr<Settings> settings, std::shared_ptr<ecs::Registry> registry)
		: settings_(settings), registry_(registry)
	{
	}

	virtual ecs::Entity addModel(const scene::Model&) override;
	virtual ecs::Entity addPlane(const scene::Plane&) override;
	virtual ecs::Entity addVector(const scene::Vector&) override;
	virtual ecs::Entity addPoint(const scene::Point&) override;
	virtual ecs::Entity addDashedLine(const scene::DashedLine&) override;
	virtual ecs::Entity addText(const scene::Text&) override;
	virtual ecs::Entity addAABB(const scene::AABB&) override;

	virtual AABB getSceneBounds() const override;
	virtual Vec3 getSceneCenter() const override;

	virtual void setPosition(ecs::Entity, const Vec3&) override;
	virtual void setEndPosition(ecs::Entity, const Vec3&) override;
	virtual void setNormal(ecs::Entity, const Vec3&) override;
	virtual void setDistance(ecs::Entity, float) override;
	virtual void setAABBSize(ecs::Entity, float) override;
};

namespace di
{
inline auto sceneService() noexcept
{
	return boost::di::bind<ISceneService>.to<SceneService>();
}
}  // namespace di
}  // namespace gs
#endif	// GEOMETRY_SANDBOX_SCENE_SERVICE_H
