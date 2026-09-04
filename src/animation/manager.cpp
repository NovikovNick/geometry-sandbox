#include "animation/manager.h"

#include "animation/dsl.h"
#include "animation/handle.h"
#include "animation/sampler.h"
#include "animation/types.h"
#include "core/settings.h"
#include "core/types.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <functional>
#include <iterator>
#include <memory>
#include <span>
#include <utility>
#include <vector>

namespace gs::animation
{
AssetCollection Manager::build(KeyframeCollection& keyframes)
{
	AssetCollection res{.duration = Milliseconds{0}};

	const auto rootPropertyIdCmp = [](const Key& lhs, const Key& rhs) { return lhs.rootPropertyId < rhs.rootPropertyId; };
	std::ranges::sort(keyframes.keys, rootPropertyIdCmp);

	for (auto it = keyframes.keys.begin(); it != keyframes.keys.end();)
	{
		const Target target{.entityId = it->target.entityId, .propertyId = it->rootPropertyId};

		auto last = std::upper_bound(it, keyframes.keys.end(), *it, rootPropertyIdCmp);
		assets_.push_back(build(std::span<Key>(it, last)));
		Asset& asset	= assets_.back();
		res.duration	= std::max(res.duration, std::chrono::duration_cast<Milliseconds>(asset.duration));
		res.map[target] = &asset;

		it				= last;
	}
	return res;
}

bool Manager::isValid(const Handle& handle) const
{
	return animations_.isValid(handle.animationId);
}

Instance& Manager::getAnimationInstance(const Handle& handle)
{
	return animations_.at(handle.animationId);
}

Handle Manager::play(Clip& clip)
{
	Instance animation{};

	animation.duration = std::chrono::duration_cast<Nanoseconds>(clip.duration);
	animation.elapsed  = Nanoseconds{0};
	animation.setRemoveOnComplete(clip.removeOnComplete);
	animation.setLooping(clip.loop);

	// setup animation
	const auto durationMs = static_cast<float>(clip.duration.count());
	for (const Milliseconds marker : clip.markers)
	{
		animation.markers.insert(static_cast<float>(marker.count()) / durationMs);
	}
	animation.animations		 = std::move(clip.animations);
	const InstanceId animationId = animations_.insert(std::move(animation));

	Handle handle{};
	handle.manager	   = this;
	handle.animationId = animationId;
	return handle;
}

Handle Manager::buildAndPlay(KeyframeCollection& keyframes, std::function<void(Clip&)> setupCallback)
{
	AssetCollection assets = build(keyframes);
	Clip clip{.duration = assets.duration};
	std::ranges::copy(assets.map, std::back_inserter(clip.animations));
	setupCallback(clip);
	return play(clip);
}

void Manager::updateAndApplyAnimations(Nanoseconds deltaTime)
{
	deltaTime = std::chrono::duration_cast<Nanoseconds>(deltaTime * settings_->animationSpeed);
	for (auto&& [i, animation] : animations_.forEach())
	{
		if (animation.isComplete() && animation.isRemoveOnComplete())
		{
			animations_.remove(i);
			continue;
		}

		if (animation.isPaused())
		{
			continue;
		}

		integrate(animation, deltaTime);
		applyAnimation(animation);
	}
}

void Manager::applyAnimation(const Instance& animation)
{
	const auto totalDuration = static_cast<float>(animation.duration.count());
	const auto totalProgress = static_cast<float>(animation.elapsed.count()) / totalDuration;

	for (const std::pair<Target, Asset*>& entry : animation.animations)
	{
		const Target& target = entry.first;
		const Asset& asset	 = *entry.second;
		assert(sampler_.isBufferSufficientForChannels(asset.channels.size()));

		// correct the asset's duration relative to the duration of the entire animation
		const auto duration			  = static_cast<float>(asset.duration.count());
		const float progress		  = totalProgress * totalDuration / duration;

		const std::span<float> sample = sampler_.sample(progress, asset);

		apply(target, asset.channels, sample);
	}
}

}  // namespace gs::animation
