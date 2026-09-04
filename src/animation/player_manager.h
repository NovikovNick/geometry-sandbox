/**
 * @file player_manager.h
 * @brief Manages animation playback for gs::ui::AnimationPlayer
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_ANIMATION_PLAYER_MANAGER_H
#define GEOMETRY_SANDBOX_ANIMATION_PLAYER_MANAGER_H

#include "animation/handle.h"
#include "animation/manager.h"
#include "core/base_app_component.h"
#include "core/types.h"

#include "boost/di.hpp"

namespace gs
{
namespace animation
{
class IService;

/** @brief Manages animation playback for gs::ui::AnimationPlayer */
class IPlayerManager
{
  public:
	virtual void init(ui::AnimationPlayer&)					   = 0;
	virtual void update(ui::AnimationPlayer&) const			   = 0;
	virtual void setAnimation(const animation::Handle& handle) = 0;
	virtual animation::Handle getAnimation() const			   = 0;
	virtual bool hasAnimation() const						   = 0;
	~IPlayerManager()										   = default;
};

/** @brief basic IPlayerManager implementation */
class PlayerManager : public BaseManager, public IPlayerManager
{
	std::shared_ptr<animation::IManager> animationManager_;
	std::shared_ptr<IService> animationService_;

	animation::Handle animationHandle_;

  public:
	PlayerManager(const std::shared_ptr<Settings>& settings,
				  const std::shared_ptr<ILogManager>& log,
				  const std::shared_ptr<IService>& animationService)
		: BaseManager(settings, log), animationService_(animationService), animationHandle_({})
	{
	}

	virtual void init(ui::AnimationPlayer&) override;
	virtual void update(ui::AnimationPlayer&) const override;
	virtual void setAnimation(const animation::Handle& handle) override { animationHandle_ = handle; };
	virtual animation::Handle getAnimation() const override { return animationHandle_; };
	virtual bool hasAnimation() const override;

  private:
	void backward();
	void forward();
	void togglePlayPause();
	void setProgress(float progress);
	std::string convertProgressToString(float progress);
};
}  // namespace animation

namespace di
{
inline auto animationPlayerManager() noexcept
{
	return boost::di::bind<animation::IPlayerManager>.to<animation::PlayerManager>();
}
}  // namespace di
}  // namespace gs

#endif	// GEOMETRY_SANDBOX_ANIMATION_PLAYER_MANAGER_H
