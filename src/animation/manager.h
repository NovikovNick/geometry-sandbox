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
#include "core/ecs.h"
#include "core/types.h"

#include "boost/di.hpp"

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace gs
{
struct Settings;
class ILogManager;

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

	virtual void tick(Nanoseconds deltaTime)																  = 0;
	virtual void animate(ecs::Registry& registry, ui::State& uiState)										  = 0;
	~IManager()																								  = default;
};

/**
 * @brief IManager implementation with slot map
 */
class Manager : public IManager
{
	std::shared_ptr<Settings> settings_;
	std::shared_ptr<ILogManager> logManager_;
	Sampler<64> sampler_;
	std::list<Asset> assets_;  // just some storage for assets without iterator invalidation.
	SlotMap<Instance> animations_;

  public:
	Manager(std::shared_ptr<Settings> settings, std::shared_ptr<ILogManager> logManager) : settings_(settings), logManager_(logManager)
	{
	}

	virtual AssetCollection build(KeyframeCollection&) override;
	virtual Handle play(Clip&) override;
	virtual Handle buildAndPlay(KeyframeCollection&, std::function<void(Clip&)> setupCallback) override;
	virtual bool isValid(const Handle&) const override;
	virtual Instance& getAnimationInstance(const Handle&) override;

	virtual void tick(Nanoseconds deltaTime) override;
	virtual void animate(ecs::Registry&, ui::State&) override;

  private:
	static Asset build(std::span<Key> keys);

	/**
	 * @brief update animation's elapsed and pause on markers
	 */
	static void integrate(Instance&, Nanoseconds deltaTime);
	static void apply(ecs::Registry&, ui::State&, const Target&, const std::vector<Asset::Channel>&, const std::span<float>& sample);

	void animate(ecs::Registry&, ui::State&, Instance&);
	void animate(ecs::Registry&, ui::State&, Instance&, float progress);
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
