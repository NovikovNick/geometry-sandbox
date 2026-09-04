#include "ui/state_manager.h"

#include "core/base_app_component.h"
#include "core/settings.h"
#include "core/types.h"

#include <memory>

namespace gs
{
namespace
{
constexpr void registerElement(ui::State& state, ui::Element& element)
{
	element.id = static_cast<int>(state.elements.size());
	state.elements.push_back(&element);
}
}  // namespace

UIStateManager::UIStateManager(const std::shared_ptr<Settings>& settings, const std::shared_ptr<ILogManager>& log)
	: BaseManager(settings, log), state_({})
{
	const RectSize footerButtonSize{.width = settings_->uiPlayerControlButtonWidth, .height = settings_->uiPlayerHeight};
	const RectSize controlButtonSize{.width = settings->controlButtonWidth, .height = settings->controlButtonWidth};
	state_.width  = settings->width;
	state_.height = settings->height;

	ui::Button defaultButton{};
	defaultButton.props.color						= settings->buttonColor;
	defaultButton.active							= true;

	state_.forwardButton							= defaultButton;
	state_.forwardButton.icon						= settings->iconCameraForward;
	state_.forwardButton.fontSize					= settings->controlButtonFontSize;
	state_.forwardButton.size						= controlButtonSize;
	state_.forwardButton.border						= settings->controlButtonBorderThickness;

	state_.leftButton								= defaultButton;
	state_.leftButton.icon							= settings->iconCameraLeft;
	state_.leftButton.fontSize						= settings->controlButtonFontSize;
	state_.leftButton.size							= controlButtonSize;
	state_.leftButton.border						= settings->controlButtonBorderThickness;

	state_.backwardButton							= defaultButton;
	state_.backwardButton.icon						= settings->iconCameraBackward;
	state_.backwardButton.props.color				= settings->buttonColor;
	state_.backwardButton.fontSize					= settings->controlButtonFontSize;
	state_.backwardButton.size						= controlButtonSize;
	state_.backwardButton.border					= settings->controlButtonBorderThickness;

	state_.rightButton								= defaultButton;
	state_.rightButton.icon							= settings->iconCameraRight;
	state_.rightButton.fontSize						= settings->controlButtonFontSize;
	state_.rightButton.size							= controlButtonSize;
	state_.rightButton.border						= settings->controlButtonBorderThickness;

	state_.player.prevButton						= defaultButton;
	state_.player.prevButton.icon					= settings->iconPlayerBackwardStep;
	state_.player.prevButton.fontSize				= settings->uiFontSize;
	state_.player.prevButton.size					= footerButtonSize;
	state_.player.prevButton.border					= 0.0F;

	state_.player.playButton						= defaultButton;
	state_.player.playButton.icon					= settings->iconPlayerPlay;
	state_.player.playButton.fontSize				= settings->uiFontSize;
	state_.player.playButton.size					= footerButtonSize;
	state_.player.playButton.border					= 0.0F;

	state_.player.nextButton						= defaultButton;
	state_.player.nextButton.icon					= settings->iconPlayerForwardStep;
	state_.player.nextButton.fontSize				= settings->uiFontSize;
	state_.player.nextButton.size					= footerButtonSize;
	state_.player.nextButton.border					= 0.0F;

	state_.settingsButton							= defaultButton;
	state_.settingsButton.icon						= settings->iconSettings;
	state_.settingsButton.fontSize					= settings->uiFontSize;
	state_.settingsButton.size						= footerButtonSize;
	state_.settingsButton.border					= 0.0F;

	state_.player.timelineSlider.backgroudColor		= settings->uiPlayerTimelineBackgroundColor;
	state_.player.timelineSlider.fillColor			= settings->uiPlayerTimelineFillColor;
	state_.player.timelineSlider.grabberColor		= settings->uiPlayerTimelineGrabberColor;
	state_.player.timelineSlider.grabberColorActive = settings->uiPlayerTimelineGrabberColorActive;
	state_.player.timelineSlider.grabberSize		= settings->uiPlayerTimelineGrabberSize;
	state_.player.timelineSlider.roundingSize		= settings->uiPlayerTimelineRoundingSize;
	state_.player.timelineSlider.height				= settings->uiPlayerTimelineHeight;

	registerElement(state_, state_.player);
	registerElement(state_, state_.player.prevButton);
	registerElement(state_, state_.player.playButton);
	registerElement(state_, state_.player.nextButton);
	registerElement(state_, state_.player.timelineSlider);
	registerElement(state_, state_.settingsButton);
	registerElement(state_, state_.forwardButton);
	registerElement(state_, state_.leftButton);
	registerElement(state_, state_.backwardButton);
	registerElement(state_, state_.rightButton);

	Camera camera			 = settings->defaultCamera;
	camera.width			 = settings->width;
	camera.height			 = settings->height;
	state_.activeCameraIndex = static_cast<int>(state_.cameras.size());
	state_.cameras.push_back(camera);
}

}  // namespace gs
