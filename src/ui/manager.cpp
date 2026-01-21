#include "ui/manager.h"

#include "animation/manager.h"
#include "animation/player_manager.h"
#include "animation/service.h"
#include "core/resource_manager.h"
#include "core/settings.h"
#include "core/types.h"
#include "core/window_manager.h"
#include "ui/service.h"

#include "imgui.h"
#include "rlImGui.h"

#include <cassert>
#include <functional>
#include <memory>
#include <string_view>

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

UIManager::UIManager(const std::shared_ptr<Settings>& settings,
					 const std::shared_ptr<IResourceManager>& resourceManager,
					 const std::shared_ptr<animation::IManager>& animationManager,
					 const std::shared_ptr<IWindowManager>& windowManager,
					 const std::shared_ptr<animation::IService>& animationService,
					 const std::shared_ptr<IUIService>& uiService,
					 const std::shared_ptr<animation::IPlayerManager>& animationPlayerManager)
	: settings_(settings), resourceManager_(resourceManager), animationManager_(animationManager), windowManager_(windowManager),
	  animationService_(animationService), uiService_(uiService), animationPlayerManager_(animationPlayerManager), state_({}),
	  mouseCapturedByDetailsView_(false), mouseCapturedByFooter_(false)
{

	state_.width									= settings->width;
	state_.height									= settings->height;

	state_.player.prevButton.icon					= settings->iconPlayerBackwardStep;
	state_.player.prevButton.props.color			= settings->uiButtonColor;
	state_.player.prevButton.size					= settings->uiFontSize;
	state_.player.prevButton.active					= true;

	state_.player.playButton.icon					= settings->iconPlayerPlay;
	state_.player.playButton.props.color			= settings->uiButtonColor;
	state_.player.playButton.size					= settings->uiFontSize;
	state_.player.playButton.active					= true;

	state_.player.nextButton.icon					= settings->iconPlayerForwardStep;
	state_.player.nextButton.props.color			= settings->uiButtonColor;
	state_.player.nextButton.size					= settings->uiFontSize;
	state_.player.nextButton.active					= true;

	state_.settingsButton.icon						= settings->iconSettings;
	state_.settingsButton.props.color				= settings->uiButtonColor;
	state_.settingsButton.size						= settings->uiFontSize;
	state_.settingsButton.active					= true;

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

	Camera camera			 = settings->defaultCamera;
	camera.width			 = settings->width;
	camera.height			 = settings->height;
	state_.activeCameraIndex = static_cast<int>(state_.cameras.size());
	state_.cameras.push_back(camera);

	windowManager_->onResize(
		[&](const int width, const int height)
		{
			state_.width  = width;
			state_.height = height;

			for (Camera& camera : state_.cameras)
			{
				camera.width  = width;
				camera.height = height;
			}
		});
}
void UIManager::init()
{
	ImGuiIO& imGuiIO						  = ImGui::GetIO();
	imGuiIO.ConfigWindowsMoveFromTitleBarOnly = true;

	animationPlayerManager_->init(state_.player);
	state_.settingsButton.onHover = animationService_->createHoverButtonAnimation(state_.settingsButton);

	animationService_->playAppearUIAnimation();
}

void UIManager::tick()
{
	animationPlayerManager_->update(state_.player);
}

void UIManager::render()
{
	rlImGuiBegin();

	ImGuiStyle& style = ImGui::GetStyle();
	style.Alpha		  = state_.layout.opacity;

	ImFont* myFont	  = resourceManager_->defaultUIFont();
	ImGui::PushFont(myFont, settings_->uiFontSize);

	const auto footerWidth	  = static_cast<float>(state_.width);
	const float footerHeight  = settings_->footerHeight;
	const float detailsWidth  = settings_->detailsWidth;
	const float detailsHeight = static_cast<float>(state_.height) - footerHeight;
	const float consoleWidth  = static_cast<float>(state_.width) - detailsWidth;
	const float consoleHeight = static_cast<float>(state_.height) - footerHeight;

	if (settings_->showConsole)
	{
		uiService_->overlayConsole({.width = consoleWidth, .height = consoleHeight}, Vec2::Zero());
	}
	drawDetailsView({.width = detailsWidth, .height = detailsHeight}, Vec2{consoleWidth, 0.0F});
	drawFooter({.width = footerWidth, .height = footerHeight}, Vec2{0.0F, consoleHeight});

	ImGui::PopFont();

	rlImGuiEnd();
}

void UIManager::drawFooter(const RectSize& size, const Vec2& coord)
{
	ImGui::SetNextWindowSize(ImVec2(size.width, size.height));
	ImGui::SetNextWindowPos(ImVec2(coord.x(), coord.y()));
	const auto flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
					   ImGuiWindowFlags_NoResize;
	ImGui::Begin("##footer", nullptr, flags);

	mouseCapturedByFooter_ = ImGui::IsWindowHovered();

	if (animationPlayerManager_->hasAnimation())
	{
		uiService_->animationPlayer(state_.player);
	}

	constexpr int margin = 10;	// todo move to settings
	ImGui::SameLine(ImGui::GetWindowWidth() - settings_->uiPlayerControlButtonWidth - margin);
	uiService_->settingsButton(state_.settingsButton);

	ImGui::End();
}

void UIManager::drawDetailsView(const RectSize& size, const Vec2& coord)
{
	if (detailsViewDrawCallback_)
	{
		ImGui::SetNextWindowSize(ImVec2(size.width, size.height));
		ImGui::SetNextWindowPos(ImVec2(coord.x(), coord.y()));
		auto flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
		if (ImGui::Begin("##details", nullptr, flags))
		{
			mouseCapturedByDetailsView_ = ImGui::IsWindowHovered();
			detailsViewDrawCallback_();
		}
		ImGui::End();
	}
	else
	{
		mouseCapturedByDetailsView_ = false;
	}
}
}  // namespace gs