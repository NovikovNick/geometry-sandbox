/**
 * @file service.h
 * @brief Contains various UI components: overlay console, animation player, buttons
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_UI_SERVICE_H
#define GEOMETRY_SANDBOX_UI_SERVICE_H

#include "core/types.h"

#include "boost/di.hpp"

#include <memory>

namespace gs
{
struct Settings;
class IResourceManager;
class ILogManager;

/** @brief Contains various UI components: overlay console, animation player, buttons */
class IUIService
{
  public:
	virtual void overlayConsole(const RectSize&, const Vec2& coord) = 0;
	virtual void animationPlayer(const ui::AnimationPlayer&)		= 0;
	virtual bool button(const ui::Button&)							= 0;
	virtual void settingsButton(const ui::Button&)					= 0;
	virtual ~IUIService()											= default;
};

/** @brief basic IUIService implementation */
class UIService : public IUIService
{
	std::shared_ptr<Settings> settings_;
	std::shared_ptr<IResourceManager> resourceManager_;
	std::shared_ptr<ILogManager> logManager_;

  public:
	UIService(std::shared_ptr<Settings> settings,
			  std::shared_ptr<IResourceManager> resourceManager,
			  std::shared_ptr<ILogManager> logManager)
		: settings_(settings), resourceManager_(resourceManager), logManager_(logManager) {};

	virtual void overlayConsole(const RectSize&, const Vec2& coord) override;
	virtual void animationPlayer(const ui::AnimationPlayer&) override;
	virtual bool button(const ui::Button&) override;
	virtual void settingsButton(const ui::Button&) override;
};

namespace di
{
inline auto uiService() noexcept
{
	return boost::di::bind<IUIService>.to<UIService>();
}
}  // namespace di
}  // namespace gs
#endif	// GEOMETRY_SANDBOX_UI_SERVICE_H
