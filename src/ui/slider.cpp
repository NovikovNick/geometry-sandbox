#include "core/types.h"
#include "ui/ui.h"

#include "imgui.h"

#include <algorithm>
#include <string>

namespace gs
{
bool slider(const char* label, float& value, const RectSize& size, const ui::Slider& slider)
{
	const float width	 = size.width;
	const float height	 = size.height;
	const float currentY = ImGui::GetCursorPosY();
	ImGui::SetCursorPosY(currentY + (height / 2) - (slider.height / 2));  // alignment

	const ImVec2 sliderPos	= ImGui::GetCursorScreenPos();
	const ImVec2 sliderSize = ImVec2(width, slider.height);

	ImGui::PushID(label);

	// invisible button to handle interactions (hover/active/drag) and make it a little higher for convenience
	const std::string interactButtonLabel = std::string{label} + "_interact";
	ImGui::InvisibleButton(interactButtonLabel.c_str(), ImVec2(width, slider.height));
	const bool isHovered  = ImGui::IsItemHovered();
	const bool isActive	  = ImGui::IsItemActive() && (ImGui::IsMouseDragging(0) || ImGui::IsMouseClicked(0));

	ImU32 grabColor		  = 0;	// transparent
	const ImU32 bgColor	  = ImGui::GetColorU32(convert(slider.backgroudColor));
	const ImU32 fillColor = ImGui::GetColorU32(convert(slider.fillColor));

	if (isHovered || isActive)
	{
		grabColor				= ImGui::GetColorU32(convert(slider.grabberColor));
		const float cursorValue = std::clamp((ImGui::GetMousePos().x - sliderPos.x) / width, 0.0F, 1.0F);

		if (isActive)
		{
			value	  = cursorValue;
			grabColor = ImGui::GetColorU32(convert(slider.grabberColorActive));
		}

		// tooltip
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, slider.roundingSize);
		const float tooltipHalfWidth = 55;
		const float tooltipX		 = std::clamp(ImGui::GetMousePos().x - tooltipHalfWidth,
										  sliderPos.x - tooltipHalfWidth,
										  sliderPos.x + width - tooltipHalfWidth);
		const float tooltipY		 = sliderPos.y - (slider.height * 8);
		ImGui::SetNextWindowPos({tooltipX, tooltipY});

		if (ImGui::BeginTooltip())
		{
			const std::string tooltipText = slider.tooltipConverter(cursorValue);
			ImGui::TextUnformatted(tooltipText.c_str());
			ImGui::EndTooltip();
		}
		ImGui::PopStyleVar(1);
	}

	ImDrawList* drawList = ImGui::GetWindowDrawList();

	// background bar
	drawList->AddRectFilled(sliderPos, ImVec2(sliderPos.x + width, sliderPos.y + slider.height), bgColor, slider.roundingSize);

	// fill bar
	const float filledWidth = width * value;
	if (filledWidth > 0)
	{
		drawList->AddRectFilled(sliderPos,
								ImVec2(sliderPos.x + filledWidth, sliderPos.y + slider.height),
								fillColor,
								slider.roundingSize);
	}

	// grabber
	constexpr int kCircleSegments = 12;
	drawList->AddCircleFilled(ImVec2(sliderPos.x + filledWidth, sliderPos.y + (slider.height / 2)),
							  slider.grabberSize / 2,
							  grabColor,
							  kCircleSegments);

	// markers
	const ImU32 markerColor = ImGui::GetColorU32(convert(slider.grabberColorActive));
	for (const float marker : slider.markers)
	{
		const float x = width * marker;
		drawList->AddRectFilled(ImVec2(sliderPos.x + x, sliderPos.y),
								ImVec2(sliderPos.x + x + (slider.height / 2), sliderPos.y + slider.height),
								markerColor,
								1.0F);
	}

	ImGui::SetCursorPosY(currentY);	 // return the cursor back for the next element
	ImGui::PopID();
	return isActive;
}
}  // namespace gs
