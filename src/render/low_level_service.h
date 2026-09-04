/**
 * @file low_level_service.h
 * @brief Encapsulates raylib logic
 *
 * Ideally raylib should not be included in .h files. But at the moment I don't have a clear idea of ​​
 * how to organize work with Mesh/Shader/Model, so for now raylib is appears in the headers and its
 * logic is distributed among services and managers.
 *
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_RENDER_LOW_LEVEL_SERVICE_H
#define GEOMETRY_SANDBOX_RENDER_LOW_LEVEL_SERVICE_H

#include "core/base_app_component.h"
#include "core/types.h"

#include "boost/di.hpp"

#include <initializer_list>
#include <span>
#include <string>

namespace gs
{
class IResourceManager;
class ICameraService;

namespace render
{
/** @brief Encapsulates low level graphic logic */
class ILowLevelService
{
  public:
	virtual void setupCamera(const Camera& camera) /*const*/															  = 0;
	virtual void cleanupCamera() /*const*/																				  = 0;

	virtual void clearBackground(const Color&) const																	  = 0;
	virtual void drawLine(const Vec3& startWorld, const Vec3& endWorld, float thickness, const Color& color) const		  = 0;
	virtual void drawDashedLine(const Vec3& startWorld,
								const Vec3& endWorld,
								float thickness,
								const Color& color,
								float dashLength,
								float gapLength) const																	  = 0;
	virtual void drawArrow(const Vec3& startWorld, const Vec3& endWorld, float thickness, const Color&, float size) const = 0;
	virtual void drawAABB(const AABB&, const Color&) const																  = 0;
	virtual void drawAABBFramed(const AABB&, const Color&, float thickness) const										  = 0;
	virtual void drawPlane(const Plane&, const Color&) const															  = 0;
	virtual void drawText(const Camera&, const std::string&, const Vec3&, const Color&, const float fontSize) const		  = 0;

	virtual void drawAxesGrid(Axis up, const scene::Grid2D&) const														  = 0;

	virtual void drawSphereWires(const Vec3& position, const Color&, float radius) const								  = 0;

	virtual void drawBoundingBox(const Vec3& min, const Vec3& max, const Color&) const									  = 0;
	virtual void drawModel(ModelType, const Vec3& position, float scale) const											  = 0;

	~ILowLevelService()																									  = default;
};

/** @brief Implements ILowLevelService with Raylib */
class LowLevelService : public BaseService, public ILowLevelService
{
	std::shared_ptr<IResourceManager> resources_;
	std::shared_ptr<ICameraService> cameraService_;

  public:
	LowLevelService(const std::shared_ptr<Settings>& settings,
					const std::shared_ptr<ILogManager>& log,
					const std::shared_ptr<IResourceManager>& resources,
					const std::shared_ptr<ICameraService>& cameraService)
		: BaseService(settings, log), resources_(resources), cameraService_(cameraService)
	{
	}

	virtual void setupCamera(const Camera& camera) override;
	virtual void cleanupCamera() override;

	virtual void clearBackground(const Color&) const override;
	virtual void drawLine(const Vec3& startWorld, const Vec3& endWorld, float thickness, const Color&) const override;

	/**
	 * @brief Draw dashed line (naive implementation, don't use it for large lines)
	 *
	 * @todo Currently it has naive implementation with draw call for each dash. Definitely needs improvement
	 */
	virtual void drawDashedLine(const Vec3& startWorld,
								const Vec3& endWorld,
								float thickness,
								const Color&,
								float dashLength,
								float gapLength) const override;
	virtual void drawArrow(const Vec3& startWorld, const Vec3& endWorld, float thickness, const Color&, float size) const override;
	virtual void drawAABB(const AABB&, const Color&) const override;
	virtual void drawAABBFramed(const AABB&, const Color&, float thickness) const override;
	virtual void drawPlane(const Plane&, const Color&) const override;
	virtual void drawText(const Camera&, const std::string&, const Vec3&, const Color&, const float fontSize) const override;
	virtual void drawAxesGrid(Axis up, const scene::Grid2D&) const override;

	virtual void drawSphereWires(const Vec3& position, const Color&, float radius) const override;
	virtual void drawBoundingBox(const Vec3& min, const Vec3& max, const Color&) const override;
	virtual void drawModel(ModelType, const Vec3& position, float scale) const override;
};
}  // namespace render

namespace di
{
inline auto lowLevelRenderService() noexcept
{
	return boost::di::bind<render::ILowLevelService>.to<render::LowLevelService>();
}
}  // namespace di
}  // namespace gs

#endif	// GEOMETRY_SANDBOX_RENDER_LOW_LEVEL_SERVICE_H
