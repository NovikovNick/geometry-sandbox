#include "animation/manager.h"

#include "animation/dsl.h"
#include "animation/reflection.h"
#include "animation/types.h"
#include "core/types.h"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <span>
#include <unordered_map>
#include <vector>

namespace gs::animation
{
Asset Manager::build(std::span<Key> keys)
{
	// init
	using ChannelIndex	  = std::size_t;
	auto millisecondsHash = [](const Milliseconds& d) { return std::hash<decltype(d.count())>{}(d.count()); };

	Asset anim;
	anim.interpolations.resize(keys.size());

	std::ranges::sort(keys, std::less{}, &Key::time);

	int keyframeCount = 0;
	Milliseconds duration{-1};
	std::unordered_map<reflection::Property::Id, ChannelIndex> propertyIdToChannelIndex;  // find active channels
	std::unordered_map<Milliseconds, int, decltype(millisecondsHash)> keyframeIndexByTime;
	std::unordered_map<reflection::Property::Id, std::vector<Milliseconds>> timesByProperty;
	for (const Key& key : keys)
	{
		if (duration < key.time)
		{
			duration					  = key.time;
			keyframeIndexByTime[duration] = keyframeCount;
			++keyframeCount;
		}

		for (const reflection::Property& property : reflection::nextFloatProperties(key.target.propertyId, key.values.size()))
		{
			timesByProperty[property.id].push_back(key.time);

			if (!propertyIdToChannelIndex.contains(property.id))
			{
				propertyIdToChannelIndex[property.id] = anim.channels.size();  // channelIndex
				anim.channels.emplace_back(property.id, /*keyBeginIndex*/ -1, /*keyCount*/ 0);
			}
		}
	}
	anim.keys.resize(anim.channels.size() * keyframeCount);

	// Col-major traverse
	for (int keyIdx = 0; keyIdx < keys.size(); ++keyIdx)
	{
		const Key& key							 = keys[keyIdx];
		anim.interpolations[keyIdx].easing		 = key.easing;
		anim.interpolations[keyIdx].interpolator = key.interpolator;

		int i									 = 0;
		for (const reflection::Property& property : reflection::nextFloatProperties(key.target.propertyId, key.values.size()))
		{

			const std::size_t channelIdx = propertyIdToChannelIndex[property.id];
			anim.channels[channelIdx].keyCount += 1;

			const int keyframeIndex = keyframeIndexByTime[key.time];
			const std::size_t idx	= (anim.channels.size() * keyframeIndex) + channelIdx;
			anim.keys[idx]			= Asset::Key{.interpolationIndex = keyIdx, .propertyId = property.id, .value = key.values[i]};

			++i;
		}
	}

	// Row-major traverse
	for (Asset::Channel& channel : anim.channels)
	{
		if (channel.keyCount != 0)
		{
			channel.keyBeginIndex = static_cast<int>(anim.keyframeTimes.size());
			for (const Milliseconds time : timesByProperty[channel.propertyId])
			{
				const int keyframeIndex = keyframeIndexByTime[time];
				const float t			= static_cast<float>(time.count()) / static_cast<float>(duration.count());
				anim.keyframeTimes.push_back({.index = keyframeIndex, .time = t});
			}
		}
	}

	anim.duration = duration;
	return anim;
};

}  // namespace gs::animation
