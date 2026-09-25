/**
 * @file mesh_instanced_draw_service.h
 * @brief Implements gpu instancing optimization
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_RENDER_MESH_INSTANCED_DRAW_SERVICE_H
#define GEOMETRY_SANDBOX_RENDER_MESH_INSTANCED_DRAW_SERVICE_H

#include "core/base_app_component.h"
#include "core/ecs.h"
#include "core/types.h"

#include "boost/di.hpp"
#include "raylib.h"

#include <memory>
#include <span>

namespace gs
{
class IResourceManager;

namespace render
{
/** @brief GPU instancing optimization */
class IMeshInstancedDrawService
{
  public:
	virtual void drawMeshInstanced(ResourceId,
								   const Vec3& cameraPosition,
								   const std::span<Mat4>& transforms,
								   const std::span<Color>& colors) const = 0;
	virtual ~IMeshInstancedDrawService()								 = default;
};

/** @brief basic IMeshInstancedDrawService implementation */
class MeshInstancedDrawService : public BaseService, public IMeshInstancedDrawService
{
	std::shared_ptr<IResourceManager> resources_;

	ResourceId shaderId_{};

  public:
	MeshInstancedDrawService(const std::shared_ptr<Settings>& settings,
							 const std::shared_ptr<ILogManager>& log,
							 const std::shared_ptr<IResourceManager>& resources);

	virtual void drawMeshInstanced(ResourceId,
								   const Vec3& cameraPosition,
								   const std::span<Mat4>& transforms,
								   const std::span<Color>& colors) const override;
};
}  // namespace render

namespace di
{
inline auto meshInstancedDrawService() noexcept
{
	return boost::di::bind<render::IMeshInstancedDrawService>.to<render::MeshInstancedDrawService>();
}
}  // namespace di
}  // namespace gs
#endif	// GEOMETRY_SANDBOX_RENDER_MESH_INSTANCED_DRAW_SERVICE_H
