/**
 * @file service.h
 * @brief Provides factory methods for creating common animation
 *
 * @todo it looks like it can be named as IFactor or LibraryService...
 *
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_ANIMATION_SERVICE_H
#define GEOMETRY_SANDBOX_ANIMATION_SERVICE_H

#include "animation/manager.h"
#include "animation/types.h"
#include "core/ecs.h"
#include "core/types.h"

#include "boost/di.hpp"

#include <functional>

namespace gs
{
class Settings;

namespace animation
{
/**
 * @brief A callable wrapper that toggles an animation between forward and reverse playback.
 *
 * Designed primarily for hover interactions: when the mouse enters the hover area,
 * the animation plays forward; when it leaves, the animation plays backward.
 * The underlying animation is paused upon construction and only advances when invoked.
 */
class ReversableAnimation
{
	animation::Handle animationHandle_;

  public:
	explicit ReversableAnimation(animation::Handle animationHandle) : animationHandle_(animationHandle)
	{
		if (animationHandle_.isValid())
		{
			animationHandle_.pause();
		}
	}

	void operator()(bool hover)
	{
		if (animationHandle_.isValid())
		{
			if (hover)
			{
				animationHandle_.playForward();
			}
			else
			{
				animationHandle_.playBackward();
			}
		}
	}
};

/** @brief Provides factory methods for creating common animation interactions */
class IService
{
  public:
	/** @brief color-based hover */
	virtual ReversableAnimation createHoverPointAnimation(entt::entity entity) = 0;

	/** @brief bounce selection pulse */
	virtual std::function<void()> createSelectPointAnimation(entt::entity entity) = 0;

	/** @brief opacity-based hover */
	virtual ReversableAnimation createHoverPlaneAnimation(entt::entity entity) = 0;

	/** @brief color-based hover */
	virtual ReversableAnimation createHoverButtonAnimation(const ui::Element& elem) = 0;

	/** @brief  fade-in for UI layout */
	virtual void playAppearUIAnimation() = 0;
	~IService()							 = default;
};

/** @brief basic IService implementation */
class Service : public IService
{
	std::shared_ptr<Settings> settings_;
	std::shared_ptr<IManager> animationManager_;
	std::shared_ptr<ecs::Registry> registry_;

  public:
	Service(std::shared_ptr<Settings> settings, std::shared_ptr<IManager> animationManager, std::shared_ptr<ecs::Registry> registry)
		: settings_(settings), animationManager_(animationManager), registry_(registry)
	{
	}

	virtual ReversableAnimation createHoverPointAnimation(entt::entity entity) override;
	virtual std::function<void()> createSelectPointAnimation(entt::entity entity) override;
	virtual ReversableAnimation createHoverPlaneAnimation(entt::entity entity) override;
	virtual ReversableAnimation createHoverButtonAnimation(const ui::Element& elem) override;
	virtual void playAppearUIAnimation() override;
};
}  // namespace animation

namespace di
{
inline auto animationService() noexcept
{
	return boost::di::bind<animation::IService>.to<animation::Service>();
}
}  // namespace di

}  // namespace gs

#endif	// GEOMETRY_SANDBOX_ANIMATION_SERVICE_H