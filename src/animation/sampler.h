/**
 * @file sampler.h
 * @brief Samples animation keyframe data and produces interpolated channel values.
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_ANIMATION_SAMPLER_H
#define GEOMETRY_SANDBOX_ANIMATION_SAMPLER_H

#include "animation/dsl.h"

#include <array>
#include <cstring>	// for std::memcpy
#include <span>

namespace gs::animation
{
/**
 * @brief Samples animation keyframe data and produces interpolated channel values.
 *
 * Operates on a fixed-size internal buffer (known at compile time) to avoid dynamic
 * memory allocation during sampling. Supports both per-channel scalar interpolation
 * (with configurable easing) and batched custom interpolation through user-supplied
 * interpolator functions.
 */
template <std::size_t Size>
class Sampler
{
  public:
	/**
	 * @brief Checks whether the internal buffer is large enough for a given channel count
	 */
	static bool isBufferSufficientForChannels(int channelCount)
	{
		constexpr int slotCount = 3;  // res, lhs and rhs sample slots in buffer
		return channelCount * slotCount <= Size;
	}

	/**
	 * @brief Samples the animation asset at the given progress value.
	 *
	 * @todo think about an Accessor abstraction to avoid using ordered properties invariant
	 */
	std::span<float> sample(const float progress, const Asset& asset)
	{
		const std::size_t channelCount		 = asset.channels.size();
		resSample							 = std::span<float>{buffer.data() + channelCount * 0, channelCount};
		lhsSample							 = std::span<float>{buffer.data() + channelCount * 1, channelCount};
		rhsSample							 = std::span<float>{buffer.data() + channelCount * 2, channelCount};

		const Asset::Key* animationKeyframes = asset.keys.data();

		for (int channelIdx = 0; channelIdx < channelCount; ++channelIdx)
		{
			auto [lhs, rhs] = findBoundingKeys(progress, asset.channelKeyframeTimes(channelIdx));

			std::span<const Asset::Key> rhsKeyframe{animationKeyframes + channelCount * rhs->index, channelCount};
			const Asset::Key& rhsKey = rhsKeyframe[channelIdx];

			if (lhs == rhs)
			{
				resSample[channelIdx] = rhsKey.value;
			}
			else
			{
				std::span<const Asset::Key> lhsKeyframe{animationKeyframes + channelCount * lhs->index, channelCount};
				const Asset::Key& lhsKey		 = lhsKeyframe[channelIdx];

				const float keyProgress			 = (progress - lhs->time) / static_cast<float>(rhs->time - lhs->time);

				const Asset::Interpolation& data = asset.interpolations[rhsKey.interpolationIndex];
				if (data.interpolator == nullptr)
				{
					resSample[channelIdx] = std::lerp(lhsKey.value, rhsKey.value, data.easing(keyProgress));
				}
				else
				{
					// based on the invariant that properties go in order
					int idx					 = channelIdx;
					const int targetMetadata = rhsKey.interpolationIndex;
					while (idx < channelCount && rhsKeyframe[idx].interpolationIndex == targetMetadata)
					{
						rhsSample[idx] = rhsKeyframe[idx].value;
						lhsSample[idx] = lhsKeyframe[idx].value;
						++idx;
					}
					const int count = idx - channelIdx;
					data.interpolator(lhsSample.subspan(channelIdx, count),
									  rhsSample.subspan(channelIdx, count),
									  resSample.subspan(channelIdx, count),
									  keyProgress);

					channelIdx = idx - 1;  // skip channels because they have already been calculated
				}
			}
		}

		return resSample;
	}

  private:
	std::array<float, Size> buffer;
	std::span<float> resSample;
	std::span<float> lhsSample;
	std::span<float> rhsSample;

	/**
	 * @brief Finds the two bounding keyframes surrounding a given progress value.
	 * @return A pair of iterators `(lhs, rhs)` where `lhs <= progress <= rhs`.
	 *         If progress falls before the first key or after the last, both
	 *         iterators point to the nearest boundary key.
	 */
	using Keys = std::span<const Asset::KeyframeTime>;
	static std::pair<Keys::iterator, Keys::iterator> findBoundingKeys(const float progress, const Keys& keys)
	{
		auto cmp = [](const float t, const Asset::KeyframeTime& key) { return t < key.time; };
		auto it	 = std::upper_bound(keys.begin(), keys.end(), progress, cmp);
		if (it == keys.end())
		{
			it = std::prev(keys.end());
			return std::make_pair(it, it);
		}
		else if (it == keys.begin())
		{
			return std::make_pair(it, it);
		}
		else
		{
			return std::make_pair(std::prev(it), it);
		}
	}
};
}  // namespace gs::animation

#endif	// GEOMETRY_SANDBOX_ANIMATION_SAMPLER_H