#include "animation/manager.h"

#include "animation/types.h"
#include "core/types.h"

#include <algorithm>
#include <cassert>
#include <chrono>

namespace gs::animation
{
void Manager::integrate(Instance& instance, Nanoseconds deltaTime)
{
	assert(instance.duration.count() > 0);

	Nanoseconds elapsed = instance.elapsed + (instance.isReversed() ? -deltaTime : deltaTime);

	if (elapsed >= instance.duration)
	{
		if (instance.isLooping())
		{
			elapsed = Nanoseconds{elapsed.count() % instance.duration.count()};
		}
		else
		{
			elapsed = instance.duration;  // complete
			instance.setPaused(true);
		}
	}
	else if (elapsed < Nanoseconds{0})
	{
		if (instance.isLooping())
		{
			auto count = elapsed.count() % instance.duration.count();
			elapsed	   = Nanoseconds{count < 0 ? instance.duration.count() + count : count};
		}
		else
		{
			elapsed = Nanoseconds{0};  // complete
			instance.setPaused(true);
		}
	}
	else if (instance.isPauseOnMarker())
	{
		// The idea is to check whether a marker is in the interval between the current
		// elapsed and next elapsed values. If so, then no matter whether the playing was
		// forward or backward, we've crossed the marker.
		auto [lhs, rhs]			= std::minmax(instance.elapsed, elapsed);

		const auto duration		= static_cast<float>(instance.duration.count());
		const float lhsProgress = static_cast<float>(lhs.count()) / duration;
		const float rhsProgress = static_cast<float>(rhs.count()) / duration;
		auto marker				= instance.markers.upper_bound(lhsProgress);

		if (marker != instance.markers.end() && *marker < rhsProgress)
		{
			elapsed = std::chrono::duration_cast<Nanoseconds>(instance.duration * *marker);
			instance.setPaused(true);
		}
	}

	instance.elapsed = elapsed;
}

}  // namespace gs::animation