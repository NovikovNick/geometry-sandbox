/**
 * @file frustum_draw_service.h
 * @brief Draws camera frustum
 *
 * @todo std::size_t cameraIndex should be used as alias CameraId or something...
 *
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_RENDER_FRUSTUM_DRAW_SERVICE_H
#define GEOMETRY_SANDBOX_RENDER_FRUSTUM_DRAW_SERVICE_H

#include "core/types.h"

#include "boost/di.hpp"
#include "raylib.h"

#include <initializer_list>
#include <span>
#include <string>

namespace gs
{
struct Settings;
class IResourceManager;
class ICameraService;

namespace render
{
class IViewportManager;
class ILowLevelService;

/** @brief Draws camera frustum */
class IFrustumDrawService
{
  public:
	virtual void drawFrustum(std::size_t cameraIndex, const Camera&) const = 0;
	~IFrustumDrawService()												   = default;
};

/** @brief Draws camera frustum with viewport and camera model */
class FrustumDrawService : public IFrustumDrawService
{
	std::shared_ptr<Settings> settings_;
	std::shared_ptr<IResourceManager> resources_;
	std::shared_ptr<ICameraService> cameraService_;
	std::shared_ptr<IViewportManager> viewports_;
	std::shared_ptr<ILowLevelService> graphic_;

  public:
	FrustumDrawService(std::shared_ptr<Settings> settings,
					   std::shared_ptr<IResourceManager> resources,
					   std::shared_ptr<ICameraService> cameraService,
					   std::shared_ptr<IViewportManager> viewports,
					   std::shared_ptr<ILowLevelService> graphic)
		: settings_(settings), resources_(resources), cameraService_(cameraService), viewports_(viewports), graphic_(graphic)
	{
	}

	virtual void drawFrustum(std::size_t cameraIndex, const Camera&) const override;

  private:
	void drawFrustumWires(const Camera&) const;
};
}  // namespace render

namespace di
{
inline auto frustumDrawService() noexcept
{
	return boost::di::bind<render::IFrustumDrawService>.to<render::FrustumDrawService>();
}
}  // namespace di
}  // namespace gs

#endif	// GEOMETRY_SANDBOX_RENDER_FRUSTUM_DRAW_SERVICE_H
