/**
 * @file state_manager.h
 * @brief Contains ui state
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_UI_STATE_MANAGER_H
#define GEOMETRY_SANDBOX_UI_STATE_MANAGER_H

#include "core/base_app_component.h"
#include "core/types.h"

#include "boost/di.hpp"

#include <memory>

namespace gs
{
/** @brief Contains ui state */
class IUIStateManager
{
  public:
	virtual ui::State& getState() = 0;
	virtual ~IUIStateManager()	  = default;
};

/** @brief basic IUIStateManager implementation */
class UIStateManager : public BaseManager, public IUIStateManager
{
	ui::State state_;

  public:
	UIStateManager(const std::shared_ptr<Settings>&, const std::shared_ptr<ILogManager>&);
	virtual ui::State& getState() override { return state_; };
};

namespace di
{
inline auto uiStateManager() noexcept
{
	return boost::di::bind<IUIStateManager>.to<UIStateManager>();
}
}  // namespace di
}  // namespace gs
#endif	// GEOMETRY_SANDBOX_UI_MANAGER_H
