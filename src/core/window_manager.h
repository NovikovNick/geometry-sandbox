/**
 * @file window_manager.h
 * @brief Provides methods for window initialization and callbacks
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_WINDOW_MANAGER_H
#define GEOMETRY_SANDBOX_WINDOW_MANAGER_H

#include "boost/di.hpp"

#include <functional>
#include <memory>
#include <utility>
#include <vector>

namespace gs
{
struct Settings;
class IResourceManager;

/** @brief Provides methods for window initialization and callbacks */
class IWindowManager
{
  public:
	using OnResizeCallback					  = std::function<void(int, int)>;
	using OnCloseCallback					  = std::function<void()>;

	virtual void initWindow()				  = 0;
	virtual void tick()						  = 0;
	virtual void onClose(OnCloseCallback&&)	  = 0;
	virtual void onResize(OnResizeCallback&&) = 0;
	virtual ~IWindowManager()				  = default;
};

/** @brief basic IWindowManager implementation */
class WindowManager : public IWindowManager
{
	std::shared_ptr<Settings> settings_;
	std::vector<OnResizeCallback> onResizeCallbacks_;
	std::vector<OnCloseCallback> onCloseCallbacks_;

  public:
	WindowManager(std::shared_ptr<Settings> settings) : settings_(settings) {}

	virtual void initWindow() override;
	virtual void tick() override;
	virtual void onClose(OnCloseCallback&&) override;
	virtual void onResize(OnResizeCallback&&) override;
	virtual ~WindowManager() override;
};

namespace di
{
inline auto windowManager() noexcept
{
	return boost::di::bind<IWindowManager>.to<WindowManager>();
}
}  // namespace di
}  // namespace gs

#endif	// GEOMETRY_SANDBOX_WINDOW_MANAGER_H
