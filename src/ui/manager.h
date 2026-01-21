/**
 * @file manager.h
 * @brief Main UI manager
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_UI_MANAGER_H
#define GEOMETRY_SANDBOX_UI_MANAGER_H

#include "core/types.h"

#include "boost/di.hpp"

#include <functional>
#include <memory>

namespace gs
{
struct Settings;
class IResourceManager;
class IWindowManager;
class IUIService;

namespace animation
{
class IManager;
class IPlayerManager;
class IService;
}  // namespace animation

/** @brief Manages all UI */
class IUIManager
{
  public:
	virtual void init()											   = 0;
	virtual void tick()											   = 0;
	virtual ui::State& getState()								   = 0;
	virtual void render()										   = 0;
	virtual void onDetailsViewDraw(std::function<void()> callback) = 0;
	virtual bool isMouseCaptured() const						   = 0;
	virtual ~IUIManager()										   = default;
};

/**
 * @brief Implements IUIManager with DearImGui + rlImGui
 * @todo move drawFooter and drawDetailsView to UI service?
 */
class UIManager : public IUIManager
{
	std::shared_ptr<Settings> settings_;
	std::shared_ptr<IResourceManager> resourceManager_;
	std::shared_ptr<animation::IManager> animationManager_;
	std::shared_ptr<IWindowManager> windowManager_;
	std::shared_ptr<animation::IService> animationService_;
	std::shared_ptr<IUIService> uiService_;
	std::shared_ptr<animation::IPlayerManager> animationPlayerManager_;

	ui::State state_;
	std::function<void()> detailsViewDrawCallback_;

	bool mouseCapturedByDetailsView_;
	bool mouseCapturedByFooter_;

  public:
	UIManager(const std::shared_ptr<Settings>& settings,
			  const std::shared_ptr<IResourceManager>& resourceManager,
			  const std::shared_ptr<animation::IManager>& animationManager,
			  const std::shared_ptr<IWindowManager>& windowManager,
			  const std::shared_ptr<animation::IService>& animationService,
			  const std::shared_ptr<IUIService>& uiService,
			  const std::shared_ptr<animation::IPlayerManager>& animationPlayerManager);

	virtual void init() override;
	virtual void tick() override;
	virtual ui::State& getState() override { return state_; };
	virtual void render() override;
	virtual void onDetailsViewDraw(std::function<void()> callback) override { detailsViewDrawCallback_ = std::move(callback); };
	virtual bool isMouseCaptured() const override { return mouseCapturedByDetailsView_ || mouseCapturedByFooter_; };

  private:
	void drawFooter(const RectSize&, const Vec2& coord);	   // todo: move to UI service
	void drawDetailsView(const RectSize&, const Vec2& coord);  // todo: move to UI service
};

namespace di
{
inline auto uiManager() noexcept
{
	return boost::di::bind<IUIManager>.to<UIManager>();
}
}  // namespace di
}  // namespace gs
#endif	// GEOMETRY_SANDBOX_UI_MANAGER_H
