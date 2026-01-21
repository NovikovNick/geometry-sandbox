#include "animation/player_manager.h"

#include "animation/service.h"

#include "animation/types.h"
#include "core/settings.h"
#include "core/types.h"

#include <chrono>
#include <cstdint>
#include <format>
#include <functional>
#include <string>

namespace gs::animation
{
namespace
{
std::string toString(const Nanoseconds duration)
{
	const std::int64_t nano		= duration.count();
	const std::int64_t totalMs	= nano / 1'000'000;
	const std::int64_t totalSec = totalMs / 1000;
	const std::int64_t min		= totalSec / 60;
	const std::int64_t sec		= totalSec % 60;
	const std::int64_t ms		= totalMs % 1000;
	return std::format("{:02d}:{:02d}.{:03d}", min, sec, ms);
}
}  // namespace

void PlayerManager::init(ui::AnimationPlayer& player)
{
	player.prevButton.onHover			   = animationService_->createHoverButtonAnimation(player.prevButton);
	player.playButton.onHover			   = animationService_->createHoverButtonAnimation(player.playButton);
	player.nextButton.onHover			   = animationService_->createHoverButtonAnimation(player.nextButton);

	player.prevButton.onClick			   = [this] { backward(); };
	player.nextButton.onClick			   = [this] { forward(); };
	player.playButton.onClick			   = [this] { togglePlayPause(); };
	player.timelineSlider.onUpdate		   = [this](float progress) { setProgress(progress); };
	player.timelineSlider.tooltipConverter = [this](float progress) { return convertProgressToString(progress); };
}

void PlayerManager::update(ui::AnimationPlayer& player) const
{
	if (animationHandle_.isValid())
	{
		const Instance& animation = animationHandle_.getAnimationInstance();

		if (!animation.atBegin() || animation.isLooping())
		{
			player.prevButton.active = true;
			player.prevButton.icon	 = animation.isForward() || animation.isPaused() ? settings_->iconPlayerBackwardStep
																					 : settings_->iconPlayerBackwardFast;
		}
		else
		{
			player.prevButton.active = false;
		}

		player.playButton.icon = animation.isPlaying() ? settings_->iconPlayerPause : settings_->iconPlayerPlay;

		if (!animation.atEnd() || animation.isLooping())
		{
			player.nextButton.icon = animation.isForward() || animation.isPaused() ? settings_->iconPlayerForwardStep
																				   : settings_->iconPlayerForwardFast;
		}
		else
		{
			player.nextButton.active = false;
		}

		player.currentTime			  = toString(animation.elapsed);
		player.endTime				  = toString(animation.duration);
		player.isPlaying			  = animation.isPlaying();
		player.isForward			  = animation.isForward();

		player.timelineSlider.markers = animation.markers;
		player.timelineSlider.value	  = animation.getProgress();
	}
}

bool PlayerManager::hasAnimation() const
{
	return animationHandle_.isValid();
}

void PlayerManager::backward()
{
	if (animationHandle_.isValid())
	{
		const Instance& animation = animationHandle_.getAnimationInstance();
		if (animation.isPlaying() && animation.isReversed())
		{
			animationHandle_.stepBackToPrevMarker();
			if (animation.isPauseOnMarker())
			{
				animationHandle_.pause();
			}
		}
		else if (!animation.atBegin() || animation.isLooping())
		{
			animationHandle_.playBackward();
		}
	}
}

void PlayerManager::forward()
{
	if (animationHandle_.isValid())
	{
		const Instance& animation = animationHandle_.getAnimationInstance();
		if (animation.isPlaying() && animation.isForward())
		{
			animationHandle_.stepForwardToNextMarker();
			if (animation.isPauseOnMarker())
			{
				animationHandle_.pause();
			}
		}
		else if (!animation.atEnd() || animation.isLooping())
		{
			animationHandle_.playForward();
		}
	}
}

void PlayerManager::togglePlayPause()
{
	if (animationHandle_.isValid())
	{
		const Instance& animation = animationHandle_.getAnimationInstance();
		if (animation.isPaused())
		{
			animationHandle_.resume();
		}
		else
		{
			animationHandle_.pause();
		}
	}
}

void PlayerManager::setProgress(float progress)
{
	if (animationHandle_.isValid())
	{
		animationHandle_.setProgress(progress);
	}
}

std::string PlayerManager::convertProgressToString(float progress)
{
	if (animationHandle_.isValid())
	{
		const Instance& animation = animationHandle_.getAnimationInstance();
		return toString(std::chrono::duration_cast<Nanoseconds>(animation.duration * progress));
	}
	return "";
}
}  // namespace gs::animation
