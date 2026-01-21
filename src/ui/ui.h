/**
 * @file ui.h
 * @brief Custom Imgui widgets
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_UI_H
#define GEOMETRY_SANDBOX_UI_H

#include "imgui.h"

#include "core/types.h"

#include <array>
#include <string>

namespace gs
{
inline ImVec4 convert(Color src)
{
	return ImVec4{
		src.r / 255.0f,
		src.g / 255.0f,
		src.b / 255.0f,
		src.a / 255.0f,
	};
}

/** @brief Custom slider for animation player */
bool slider(const char* label, float& value, const RectSize&, const ui::Slider&);

/**
 * @brief Draws an ImGui combo box with items provided as variadic arguments.
 *
 * Creates a dropdown combo box with the specified label and items. The current
 * selection is stored in currentIndex. When the user selects a new item, the
 * function updates currentIndex and returns true.
 *
 * @tparam T Type of the currentIndex value (must be enum or integral type)
 * @tparam Items Types of the variadic items (must be convertible to std::string)
 * @param label Text label displayed next to the combo box
 * @param currentIndex Reference to the current selection value, updated on user selection
 * @param args Variadic list of items to display in the combo box
 * @return true if the user changed the selection, false otherwise
 */
template <typename T, typename... Items>
bool drawCombo(const char* label, T& currentIndex, Items&&... args)
{
	static_assert(std::is_enum_v<T> || std::is_integral_v<T>, "T must be enum or integral type");
	static_assert((std::is_convertible_v<Items, std::string> && ...), "All items must be convertible to std::string");

	constexpr int itemCount = static_cast<int>(sizeof...(Items));

	if constexpr (std::is_enum_v<T>)
	{
		static_assert(itemCount <= static_cast<int>(T::Count),
					  "provided items count is not in the valid range of values for the T enum");
	}

	bool changed								   = false;
	const int currentItem						   = static_cast<int>(currentIndex);

	const std::array<std::string, itemCount> items = {std::forward<Items>(args)...};  // NOLINT(*-array-to-pointer-decay)
	if (ImGui::BeginCombo(label, items.at(currentItem).c_str()))
	{
		for (int i = 0; i < itemCount; i++)
		{
			const bool isSelected = (currentItem == i);
			if (ImGui::Selectable(items.at(i).c_str(), isSelected))
			{
				currentIndex = static_cast<T>(i);
				changed		 = true;
			}
			if (isSelected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	return changed;
}
}  // namespace gs
#endif	// GEOMETRY_SANDBOX_UI_H
