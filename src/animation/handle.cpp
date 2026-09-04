#include "handle.h"
#include "animation/handle.h"

#include "animation/manager.h"
#include "animation/types.h"
#include "core/types.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <functional>
#include <ranges>

namespace gs::animation
{
void Handle::setProgress(const float progress) const
{
	assert(isValid());
	Instance& animation = manager->getAnimationInstance(*this);
	animation.elapsed	= std::chrono::duration_cast<Nanoseconds>(animation.duration * progress);
	manager->applyAnimation(animation);
}

void Handle::pause() const
{
	assert(isValid());
	manager->getAnimationInstance(*this).setPaused(true);
}

void Handle::resume() const
{
	assert(isValid());
	Instance& animation = manager->getAnimationInstance(*this);
	animation.setPaused(false);
}

void Handle::stop()
{
	assert(isValid());
	Instance& animation = manager->getAnimationInstance(*this);
	animation.setRemoveOnComplete(true);
	animation.forceToComplete();
	manager = nullptr;
}

void Handle::playForward() const
{
	assert(isValid());
	Instance& animation = manager->getAnimationInstance(*this);
	animation.setReversed(false);
	animation.setPaused(false);
}

void Handle::playBackward() const
{
	assert(isValid());
	Instance& animation = manager->getAnimationInstance(*this);
	animation.setReversed(true);
	animation.setPaused(false);
}

void Handle::stepForwardToNextMarker() const
{
	assert(isValid());
	Instance& animation	 = manager->getAnimationInstance(*this);
	const float progress = animation.getProgress();
	auto marker			 = animation.markers.upper_bound(progress);
	if (marker == animation.markers.end())
	{
		animation.elapsed = animation.duration;
	}
	else
	{
		animation.elapsed = std::chrono::duration_cast<Milliseconds>(animation.duration * *marker);
	}
	manager->applyAnimation(animation);
	animation.setPaused(true);
}

void Handle::stepBackToPrevMarker() const
{
	assert(isValid());
	Instance& animation	 = manager->getAnimationInstance(*this);
	const float progress = animation.getProgress();
	auto marker			 = std::ranges::upper_bound(animation.markers | std::views::reverse, progress, std::greater<>{});
	if (marker == animation.markers.rend())
	{
		animation.elapsed = Nanoseconds{0};
	}
	else
	{
		animation.elapsed = std::chrono::duration_cast<Milliseconds>(animation.duration * *marker);
	}
	manager->applyAnimation(animation);
	animation.setPaused(true);
}

void Handle::setPauseOnMarker(bool val) const
{
	assert(isValid());
	manager->getAnimationInstance(*this).setPauseOnMarker(val);
}

bool Handle::isValid() const
{
	return (manager != nullptr) && manager->isValid(*this);
}
const Instance& Handle::getAnimationInstance() const
{
	assert(isValid());
	return manager->getAnimationInstance(*this);
};

}  // namespace gs::animation
