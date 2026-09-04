/**
 * @file types.h
 * @brief Core data types and structures for the animation system
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_ANIMATION_TYPES_H
#define GEOMETRY_SANDBOX_ANIMATION_TYPES_H

#include "easing.h"

#include "animation/reflection.h"
#include "common/slot_map.h"
#include "core/ecs.h"
#include "core/types.h"

#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <chrono>
#include <functional>
#include <set>
#include <span>
#include <string_view>
#include <type_traits>
#include <vector>

namespace gs::animation
{
/**
 * @brief Uniquely identifies an animation target by entity and property.
 *
 * Complex properties contains primitive properties.
 * Primitive properties (number, bool) in animation system acts as channel.
 */
struct Target
{
	std::uint32_t entityId;
	std::uint32_t propertyId;

	auto operator<=>(const Target&) const = default;
};

/**
 * @brief Callback type for custom multi-channel interpolation.
 *
 * Receives left-hand-side values, right-hand-side values, and writes interpolated
 * results into the output span. The float parameter is the normalized progress
 * between the two keyframes in [0.0, 1.0].
 *
 * @param lhs   Span of left keyframe values.
 * @param rhs   Span of right keyframe values.
 * @param out   Span to write interpolated results into.
 * @param t     Interpolation factor in [0.0, 1.0].
 */
using SampleInterpolator = std::function<void(const std::span<float>, const std::span<float>, std::span<float>, float)>;

/**
 * @brief Immutable animation asset.
 *
 * An asset is a baked collection of channels, keyframes, and interpolation rules.
 * Multiple instances can share the same asset.
 */
struct Asset
{
	/** @brief Keyframe index and normalized time */
	struct KeyframeTime
	{
		int index;
		float time;
	};

	/** @brief Keyframe range for individual float property */
	struct Channel
	{
		reflection::Property::Id propertyId;
		int keyBeginIndex;
		std::size_t keyCount;

		std::span<const KeyframeTime> keyframeTimes(const KeyframeTime* times) const { return {times + keyBeginIndex, keyCount}; }
	};

	/** @brief Individual float property, float value and interpoation index*/
	struct Key
	{
		int interpolationIndex;
		reflection::Property::Id propertyId;
		float value;
	};

	/** @brief Interpolation data: easing or custom interpolator */
	struct Interpolation
	{
		float (*easing)(float);
		SampleInterpolator interpolator;
	};

	Nanoseconds duration;
	std::vector<Channel> channels;
	std::vector<KeyframeTime> keyframeTimes;
	std::vector<Key> keys;
	std::vector<Interpolation> interpolations;

	std::span<const KeyframeTime> channelKeyframeTimes(int channelIndex) const
	{
		return channels[channelIndex].keyframeTimes(keyframeTimes.data());
	}
};

/**
 * @brief Runtime state of a playing animation instance.
 */
struct Instance
{
	Timepoint startedAt;
	Nanoseconds duration;
	Nanoseconds elapsed;
	std::set<float> markers;
	std::vector<std::pair<Target, Asset*>> animations;

	bool isComplete() const
	{
		bool completeAsBackwardPlaying = isReversed() && elapsed == Nanoseconds{0};
		bool completeAsForwardPlaying  = !isReversed() && elapsed == duration;
		return !isLooping() && (completeAsBackwardPlaying || completeAsForwardPlaying);
	}

	void forceToComplete()
	{
		setLooping(false);
		elapsed = isReversed() ? Nanoseconds{0} : duration;
	}

	bool atBegin() const { return elapsed.count() == 0; }
	bool atEnd() const { return elapsed == duration; }
	bool isLooping() const { return flags[looping]; }
	bool isReversed() const { return flags[reversed]; }
	bool isForward() const { return !isReversed(); }
	bool isPaused() const { return flags[paused]; }
	bool isPlaying() const { return !isPaused(); }
	bool isRemoveOnComplete() const { return flags[removeOnComplete]; }
	bool isPauseOnMarker() const { return !markers.empty() && flags[pauseOnMarker]; }

	float getProgress() const
	{
		assert(duration.count() > 0);
		return std::clamp(static_cast<float>(elapsed.count()) / static_cast<float>(duration.count()), 0.0F, 1.0F);
	};

	void setLooping(bool v) { flags[looping] = v; }
	void setReversed(bool v) { flags[reversed] = v; }
	void setPaused(bool v) { flags[paused] = v; }
	void setRemoveOnComplete(bool v) { flags[removeOnComplete] = v; }
	void setPauseOnMarker(bool v) { flags[pauseOnMarker] = v; }

  private:
	static constexpr std::size_t looping		  = 0;
	static constexpr std::size_t reversed		  = 1;
	static constexpr std::size_t paused			  = 2;
	static constexpr std::size_t removeOnComplete = 3;
	static constexpr std::size_t pauseOnMarker	  = 4;

	std::bitset<5> flags;
};

/**
 * @brief Opaque key type for referencing animation instances in the animation::Manager
 *
 * Extracted to separate header to break circular dependency between Handle and Manager:
 * Handle needs InstanceId for its member, while Manager includes Handle for its interface.
 */
using InstanceId = SlotMap<Instance>::Key;

/**
 * @brief A collection of refs to animation assets grouped by their target.
 *
 * To be able to redefine the target after build
 */
struct AssetCollection
{
	struct AnimationTargetHash
	{
		size_t operator()(const Target& target) const noexcept
		{
			const uint64_t combined = (static_cast<uint64_t>(target.entityId) << 32) | target.propertyId;
			return std::hash<uint64_t>{}(combined);
		}
	};
	std::unordered_map<Target, Asset*, AnimationTargetHash> map;
	Milliseconds duration;
};

/**
 * @brief Animation clip for playback.
 *
 * A clip bundles together all the information needed to play an animation
 * instance: which entities/properties to animate, the total duration, markers,
 * and default playback flags. Passed to the animation manager for play.
 */
struct Clip
{
	std::vector<std::pair<Target, Asset*>> animations;
	std::vector<Milliseconds> markers;
	Milliseconds duration;
	bool loop			  = false;
	bool removeOnComplete = false;
};
}  // namespace gs::animation

#endif	// GEOMETRY_SANDBOX_ANIMATION_TYPES_H
