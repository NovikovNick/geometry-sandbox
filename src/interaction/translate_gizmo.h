/**
 * @file translate_gizmo.h
 * @brief services to handle translate gizmo
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_TRANSLATE_GIZMO_H
#define GEOMETRY_SANDBOX_TRANSLATE_GIZMO_H

#include "core/base_app_component.h"
#include "core/types.h"

#include "boost/di.hpp"

namespace gs
{
struct Settings;

namespace render
{
class ILowLevelService;
}

/**
 * @brief Draws translate gizmo
 *
 * @todo rename as ITranslateGizmoDrawService?
 */
class ITranslateGizmoRenderService
{
  public:
	virtual void render(const TranslateGizmo&, float scale = 1.0f) = 0;
	virtual ~ITranslateGizmoRenderService()						   = default;
};

/** @brief basic ITranslateGizmoRenderService implementation */
class TranslateGizmoRenderService : public BaseService, public ITranslateGizmoRenderService
{
	std::shared_ptr<render::ILowLevelService> graphic_;

  public:
	TranslateGizmoRenderService(const std::shared_ptr<Settings>& settings,
								const std::shared_ptr<ILogManager>& log,
								const std::shared_ptr<render::ILowLevelService>& graphic)
		: BaseService(settings, log), graphic_(graphic) {};

	virtual void render(const TranslateGizmo&, float scale) override;
};

/** @brief Provides translate gizmo operations: hover, constrain and translate */
class ITranslateGizmoUpdateService
{
  public:
	virtual void hover(const Ray&, TranslateGizmo&, float scale) const = 0;
	virtual void constrain(const Ray&, TranslateGizmo&) const		   = 0;
	virtual void translate(const Ray&, TranslateGizmo&) const		   = 0;
	virtual ~ITranslateGizmoUpdateService()							   = default;
};

/** @brief basic ITranslateGizmoUpdateService implementation */
class TranslateGizmoUpdateService : public ITranslateGizmoUpdateService
{
	std::shared_ptr<Settings> settings_;

  public:
	TranslateGizmoUpdateService(std::shared_ptr<Settings> settings) : settings_(settings) {};
	virtual void hover(const Ray&, TranslateGizmo&, float scale) const override;
	virtual void constrain(const Ray&, TranslateGizmo&) const override;
	virtual void translate(const Ray&, TranslateGizmo&) const override;
};

namespace di
{
inline auto translateGizmoRenderService() noexcept
{
	return boost::di::bind<ITranslateGizmoRenderService>.to<TranslateGizmoRenderService>();
}

inline auto translateGizmoUpdateService() noexcept
{
	return boost::di::bind<ITranslateGizmoUpdateService>.to<TranslateGizmoUpdateService>();
}
}  // namespace di
}  // namespace gs

#endif	// GEOMETRY_SANDBOX_TRANSLATE_GIZMO_H
