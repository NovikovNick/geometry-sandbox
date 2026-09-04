/**
 * @file manager.h
 * @brief Core animation manager - builds assets, spawns instances, and drives playback
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_ANIMATION_MANAGER_H
#define GEOMETRY_SANDBOX_ANIMATION_MANAGER_H

#include "animation/dsl.h"
#include "animation/handle.h"
#include "animation/sampler.h"
#include "animation/types.h"
#include "core/base_app_component.h"
#include "core/ecs.h"
#include "core/settings.h"
#include "core/types.h"

#include "boost/di.hpp"

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace gs
{
class IUIStateManager;

/** @brief animation system */
namespace animation
{
/**
 * @brief Provides the complete lifecycle for animations: building and playing
 *
 * @todo Handle play(Clip&) smells because it partially moved in the implementation
 */
class IManager
{
  public:
	virtual AssetCollection build(KeyframeCollection&)														  = 0;
	virtual Handle play(Clip&)																				  = 0;
	virtual Handle buildAndPlay(KeyframeCollection&, std::function<void(Clip&)> setupCallback = [](Clip&) {}) = 0;
	virtual bool isValid(const Handle&) const																  = 0;
	virtual Instance& getAnimationInstance(const Handle&)													  = 0;
	virtual void updateAndApplyAnimations(Nanoseconds deltaTime)											  = 0;
	virtual void applyAnimation(const Instance&)															  = 0;
	~IManager()																								  = default;
};

/**
 * @brief IManager implementation with slot map
 */
class Manager : public BaseManager, public IManager
{
	Sampler<Settings::kSamplerChannelCount> sampler_;
	std::list<Asset> assets_;  // just some storage for assets without iterator invalidation.
	SlotMap<Instance> animations_;
	std::shared_ptr<ecs::Registry> registry_;
	std::shared_ptr<IUIStateManager> uiStateManager_;

  public:
	Manager(const std::shared_ptr<Settings>& settings,	//
			const std::shared_ptr<ILogManager>& log,
			const std::shared_ptr<ecs::Registry>& registry,
			const std::shared_ptr<IUIStateManager>& uiStateManager)
		: BaseManager(settings, log), registry_(registry), uiStateManager_(uiStateManager)
	{
	}

	virtual AssetCollection build(KeyframeCollection&) override;
	virtual Handle play(Clip&) override;
	virtual Handle buildAndPlay(KeyframeCollection&, std::function<void(Clip&)> setupCallback) override;
	virtual bool isValid(const Handle&) const override;
	virtual Instance& getAnimationInstance(const Handle&) override;

	virtual void updateAndApplyAnimations(Nanoseconds deltaTime) override;
	virtual void applyAnimation(const Instance&) override;

  private:
	void apply(const Target&, const std::vector<Asset::Channel>&, const std::span<float>& sample);

	/**
	 * @brief update animation's elapsed and pause on markers
	 */
	static void integrate(Instance&, Nanoseconds deltaTime);

	static Asset build(std::span<Key> keys);
};

}  // namespace animation

namespace di
{
inline auto animationManager() noexcept
{
	return boost::di::bind<animation::IManager>.to<animation::Manager>();
}
}  // namespace di
}  // namespace gs

#endif	// GEOMETRY_SANDBOX_ANIMATION_MANAGER_H
