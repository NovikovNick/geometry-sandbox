#include "ui/manager.h"

#include "animation/player_manager.h"
#include "animation/service.h"
#include "core/base_app_component.h"
#include "core/input_manager.h"
#include "core/resource_manager.h"
#include "core/settings.h"
#include "core/types.h"
#include "core/window_manager.h"
#include "ui/service.h"
#include "ui/state_manager.h"

#include "imgui.h"
#include "rlImGui.h"

#include <cassert>
#include <functional>
#include <memory>
#include <string>

namespace gs
{
UIManager::UIManager(const std::shared_ptr<Settings>& settings,
					 const std::shared_ptr<ILogManager>& log,
					 const std::shared_ptr<IResourceManager>& resourceManager,
					 const std::shared_ptr<IWindowManager>& windowManager,
					 const std::shared_ptr<animation::IService>& animationService,
					 const std::shared_ptr<IUIService>& uiService,
					 const std::shared_ptr<IUIStateManager>& stateManager,
					 const std::shared_ptr<animation::IPlayerManager>& animationPlayerManager,
					 const std::shared_ptr<IInputManager>& inputManager)
	: BaseManager(settings, log), resourceManager_(resourceManager), windowManager_(windowManager),
	  animationService_(animationService), uiService_(uiService), stateManager_(stateManager),
	  animationPlayerManager_(animationPlayerManager), inputManager_(inputManager), mouseCaptured_(false)
{
	windowManager_->onResize(
		[&](const int width, const int height)
		{
			ui::State& state = stateManager_->getState();
			state.width		 = width;
			state.height	 = height;

			for (Camera& camera : state.cameras)
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

	ui::State& state						  = stateManager_->getState();
	animationPlayerManager_->init(state.player);
	state.settingsButton.onHover = animationService_->createHoverButtonAnimation(state.settingsButton);
	state.forwardButton.onHover	 = animationService_->createHoverButtonAnimation(state.forwardButton);
	state.leftButton.onHover	 = animationService_->createHoverButtonAnimation(state.leftButton);
	state.backwardButton.onHover = animationService_->createHoverButtonAnimation(state.backwardButton);
	state.rightButton.onHover	 = animationService_->createHoverButtonAnimation(state.rightButton);

	animationService_->playAppearUIAnimation();
}

void UIManager::tick()
{
	ui::State& ui = stateManager_->getState();
	animationPlayerManager_->update(ui.player);

	const Color hovered			  = settings_->buttonColorHover;
	const Color base			  = settings_->buttonColor;

	ui.forwardButton.props.color  = inputManager_->isKeyPressed(InputKey::Forward) || ui.forwardButton.hovered ? hovered : base;
	ui.leftButton.props.color	  = inputManager_->isKeyPressed(InputKey::Left) || ui.leftButton.hovered ? hovered : base;
	ui.backwardButton.props.color = inputManager_->isKeyPressed(InputKey::Backward) || ui.backwardButton.hovered ? hovered : base;
	ui.rightButton.props.color	  = inputManager_->isKeyPressed(InputKey::Right) || ui.rightButton.hovered ? hovered : base;
}

void UIManager::render()
{
	const ui::State& state = stateManager_->getState();

	rlImGuiBegin();

	ImGuiStyle& style = ImGui::GetStyle();
	style.Alpha		  = state.layout.opacity;

	ImFont* myFont	  = resourceManager_->defaultUIFont();
	ImGui::PushFont(myFont, settings_->uiFontSize);

	const auto footerWidth			 = static_cast<float>(state.width);
	const float footerHeight		 = settings_->footerHeight;
	const float controlsWidth		 = settings_->controlsWidthOffset;
	const float controlsHeight		 = settings_->controlsHeightOffset;
	const float controlsWidthOffset	 = static_cast<float>(state.width) - controlsWidth;
	const float controlsHeightOffset = static_cast<float>(state.height) - footerHeight - controlsHeight;
	const float detailsWidth		 = settings_->detailsWidth;
	const float detailsHeight = static_cast<float>(state.height) - footerHeight - settings_->detailsHeightOffset - controlsHeight;
	const float consoleWidth  = static_cast<float>(state.width) - detailsWidth;
	const float consoleHeight = static_cast<float>(state.height) - footerHeight;

	if (settings_->showConsole)
	{
		uiService_->overlayConsole({.width = consoleWidth, .height = consoleHeight}, Vec2::Zero());
	}

	if (settings_->showDetailsView)
	{

		drawDetailsView({.width = detailsWidth, .height = detailsHeight}, Vec2{consoleWidth, settings_->detailsHeightOffset});
	}

	if (settings_->showControls)
	{
		drawControls({.width = controlsWidth, .height = controlsHeight}, Vec2{controlsWidthOffset, controlsHeightOffset});
	}

	if (settings_->showFooter)
	{
		drawFooter({.width = footerWidth, .height = footerHeight}, Vec2{0.0F, consoleHeight});
	}

	ImGui::PopFont();

	rlImGuiEnd();
}

void UIManager::drawFooter(const RectSize& size, const Vec2& coord)
{
	const ui::State& state = stateManager_->getState();

	ImGui::SetNextWindowSize(ImVec2(size.width, size.height));
	ImGui::SetNextWindowPos(ImVec2(coord.x(), coord.y()));
	const auto flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
					   ImGuiWindowFlags_NoResize;
	ImGui::Begin("##footer", nullptr, flags);

	mouseCaptured_ = ImGui::IsWindowHovered();

	if (animationPlayerManager_->hasAnimation())
	{
		uiService_->animationPlayer(state.player);
	}

	constexpr int margin = 10;	// todo move to settings
	ImGui::SameLine(ImGui::GetWindowWidth() - settings_->uiPlayerControlButtonWidth - margin);
	uiService_->settingsButton(state.settingsButton);

	ImGui::End();
}

void UIManager::drawDetailsView(const RectSize& size, const Vec2& coord)
{
	ImGui::SetNextWindowSize(ImVec2(size.width, size.height));
	ImGui::SetNextWindowPos(ImVec2(coord.x(), coord.y()));
	auto flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
	if (ImGui::Begin("##details", nullptr, flags))
	{
		detailsViewDrawCallback_();
	}
	ImGui::End();
}
void gs::UIManager::drawControls(const RectSize& size, const Vec2& coord)
{
	static const std::string rotateTip	= "hold right mouse button to rotate";
	constexpr float controlsMarginRight = 100.0F;  // it depends entirely on the length of the rotateTip

	ImGui::SetNextWindowSize(ImVec2(size.width, size.height));
	ImGui::SetNextWindowPos(ImVec2(coord.x(), coord.y()));

	const ui::State& state = stateManager_->getState();

	auto flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
	if (ImGui::Begin("##cameraControl", nullptr, flags))
	{
		auto drawButton = [&](const ui::Button& btn, InputKey key)
		{
			uiService_->button(btn);
			if (btn.pressed)
			{
				inputManager_->press(key, InputSource::UI);
			}
			else
			{
				inputManager_->release(key, InputSource::UI);
			}
		};

		// draw WASD buttons
		ImGui::Dummy(ImVec2(controlsMarginRight, settings_->controlButtonHeight));	// margin
		ImGui::SameLine();
		ImGui::Dummy(ImVec2(settings_->controlButtonWidth, settings_->controlButtonHeight));  // skip Q placeholder
		ImGui::SameLine();
		drawButton(state.forwardButton, InputKey::Forward);

		// next line
		ImGui::Dummy(ImVec2(controlsMarginRight, settings_->controlButtonHeight));	//   margin

		ImGui::SameLine();
		drawButton(state.leftButton, InputKey::Left);

		ImGui::SameLine();
		drawButton(state.backwardButton, InputKey::Backward);

		ImGui::SameLine();
		drawButton(state.rightButton, InputKey::Right);
	}

	if (settings_->showControlRotation && !inputManager_->isKeyPressed(InputKey::MouseRight))
	{
		uiService_->text(rotateTip, settings_->buttonColor);
	}

	ImGui::End();
}
}  // namespace gs