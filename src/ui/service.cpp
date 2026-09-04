#include "service.h"
#include "ui/service.h"

#include "core/log_manager.h"
#include "core/math.h"
#include "core/resource_manager.h"
#include "core/settings.h"
#include "core/types.h"
#include "ui/ui.h"

#include "imgui.h"

namespace gs
{
void UIService::overlayConsole(const RectSize& size, const Vec2& coord)
{
	ImGui::SetNextWindowSize(ImVec2(size.width, size.height));
	ImGui::SetNextWindowPos(ImVec2(coord.x(), coord.y()));
	ImGui::Begin("##console",
				 nullptr,
				 ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	ImGui::BeginChild("Messages", ImVec2(0, 0));

	for (const auto& msg : logManager_->getFrameLog())
	{
		ImGui::TextUnformatted(msg.c_str());
	}

	if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
	{
		ImGui::SetScrollHereY(1.0F);
	}
	ImGui::EndChild();
	ImGui::End();
}

void UIService::animationPlayer(const ui::AnimationPlayer& player)
{
	button(player.prevButton);

	ImGui::SameLine();
	button(player.playButton);

	ImGui::SameLine();
	button(player.nextButton);

	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (settings_->uiPlayerHeight / 2) - (settings_->uiFontSize / 2));
	ImGui::TextUnformatted(player.currentTime.c_str());

	ImGui::SameLine();
	const float sliderWidth = ImGui::GetContentRegionAvail().x - (settings_->uiPlayerControlButtonWidth * 4) -
							  (settings_->uiPlayerControlTextWidth * 2);
	const float sliderHeight = settings_->uiPlayerHeight;

	float value				 = player.timelineSlider.value;
	if (slider("##timeline", value, {.width = sliderWidth, .height = sliderHeight}, player.timelineSlider))
	{
		player.timelineSlider.onUpdate(value);
	}

	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (settings_->uiPlayerHeight / 2) - (settings_->uiFontSize / 2));
	ImGui::TextUnformatted(player.endTime.c_str());
}

bool UIService::button(const ui::Button& btn)
{
	constexpr ImVec4 transparent{0, 0, 0, 0};

	bool pressed	 = false;
	ImFont* iconFont = resourceManager_->iconFont();

	ImGui::PushFont(iconFont, btn.fontSize);
	ImGui::PushStyleColor(ImGuiCol_Button, transparent);
	ImGui::PushStyleColor(ImGuiCol_Border, convert(btn.props.color));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, transparent);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, transparent);
	ImGui::PushStyleColor(ImGuiCol_Text, convert(btn.props.color));
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5F, 0.5F));	 // NOLINT(*-magic-numbers)
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, btn.border);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, settings_->buttonRounding);

	ImGui::BeginDisabled(!btn.active);
	if (ImGui::Button(std::string{btn.icon}.c_str(), ImVec2{btn.size.width, btn.size.height}))
	{
		pressed = true;
		if (btn.onClick)
		{
			btn.onClick();
		}
	}
	ImGui::EndDisabled();

	btn.pressed = ImGui::IsItemActive();

	if (btn.hovered != ImGui::IsItemHovered())
	{
		btn.hovered = ImGui::IsItemHovered();
		if (btn.onHover)
		{
			btn.onHover(btn.hovered);
		}
	}

	ImGui::PopStyleVar(4);	  // NOLINT(*-magic-numbers)
	ImGui::PopStyleColor(5);  // NOLINT(*-magic-numbers)
	ImGui::PopFont();

	return pressed;
}

void UIService::settingsButton(const ui::Button& btn)
{
	if (button(btn))
	{
		ImGui::OpenPopup("settings_popup_menu");
	}

	if (ImGui::BeginPopup("settings_popup_menu"))
	{
		if (ImGui::BeginTabBar("##bar", ImGuiTabBarFlags_None))
		{
			if (ImGui::BeginTabItem("Settings"))
			{
				ImGui::SeparatorText("Animation player");  // ------------
				{
					ImGui::SliderFloat("speed",
									   &settings_->animationSpeed,
									   Settings::kAnimationSpeedMin,
									   Settings::kAnimationSpeedMax);
				}

				ImGui::SeparatorText("UI Layout");	// -----------------------------
				{
					ImGui::Checkbox("show details", &settings_->showDetailsView);
					ImGui::Checkbox("show footer", &settings_->showFooter);
					ImGui::Checkbox("show controls", &settings_->showControls);
				}

				ImGui::SeparatorText("Console");  // -----------------------------
				{
					ImGui::Checkbox("show console", &settings_->showConsole);

					ImGui::BeginDisabled(!settings_->showConsole);
					ImGui::Checkbox("show performance in console", &settings_->showPerformance);
					ImGui::Checkbox("show translate gizmo in console", &settings_->showTranslateGizmo);
					ImGui::EndDisabled();
				}

				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Debug"))
			{

				ImGui::SeparatorText("Collisions");	 // -----------------------------
				ImGui::Checkbox("show collisions", &settings_->showCollisions);
				ImGui::Checkbox("show cursor ray", &settings_->showCursorRay);

				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::EndPopup();
	}
}

void UIService::text(const std::string& text, const Color& color)
{
	ImGui::PushStyleColor(ImGuiCol_Text, convert(color));
	ImGui::TextUnformatted(text.c_str());
	ImGui::PopStyleColor(1);
}

}  // namespace gs