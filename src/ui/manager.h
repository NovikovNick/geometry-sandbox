/**
 * @file manager.h
 * @brief Main UI manager
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_UI_MANAGER_H
#define GEOMETRY_SANDBOX_UI_MANAGER_H

#include "core/base_app_component.h"
#include "core/types.h"

#include "boost/di.hpp"

#include <functional>
#include <memory>

namespace gs
{
class IResourceManager;
class IWindowManager;
class IUIStateManager;
class IInputManager;
class IUIService;

namespace animation
{
class IPlayerManager;
class IService;
}  // namespace animation

/** @brief Render ui widgets */
class IUIManager
{
  public:
	virtual void init()											   = 0;
	virtual void tick()											   = 0;
	virtual void render()										   = 0;
	virtual void onDetailsViewDraw(std::function<void()> callback) = 0;
	virtual bool isMouseCaptured() const						   = 0;
	virtual ~IUIManager()										   = default;
};

/**
 * @brief Implements IUIManager with DearImGui + rlImGui
 * @todo move drawFooter and drawDetailsView to UI service?
 */
class UIManager : public BaseManager, public IUIManager
{
	std::shared_ptr<IResourceManager> resourceManager_;
	std::shared_ptr<IWindowManager> windowManager_;
	std::shared_ptr<animation::IService> animationService_;
	std::shared_ptr<IUIService> uiService_;
	std::shared_ptr<IUIStateManager> stateManager_;
	std::shared_ptr<animation::IPlayerManager> animationPlayerManager_;
	std::shared_ptr<IInputManager> inputManager_;

	std::function<void()> detailsViewDrawCallback_;

	bool mouseCaptured_;

  public:
	UIManager(const std::shared_ptr<Settings>&,
			  const std::shared_ptr<ILogManager>&,
			  const std::shared_ptr<IResourceManager>&,
			  const std::shared_ptr<IWindowManager>&,
			  const std::shared_ptr<animation::IService>&,
			  const std::shared_ptr<IUIService>&,
			  const std::shared_ptr<IUIStateManager>&,
			  const std::shared_ptr<animation::IPlayerManager>&,
			  const std::shared_ptr<IInputManager>&);

	virtual void init() override;
	virtual void tick() override;
	virtual void render() override;
	virtual void onDetailsViewDraw(std::function<void()> callback) override { detailsViewDrawCallback_ = std::move(callback); };
	virtual bool isMouseCaptured() const override { return mouseCaptured_; };

  private:
	void drawFooter(const RectSize&, const Vec2& coord);
	void drawDetailsView(const RectSize&, const Vec2& coord);
	void drawControls(const RectSize&, const Vec2& coord);
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
