#include "core/input_manager.h"

#include "core/math.h"
#include "core/types.h"

#include "raylib.h"

#include <algorithm>
#include <array>
#include <bitset>
#include <cstddef>
#include <utility>
#include <vector>

namespace gs
{
bool InputManager::isKeyPressed(InputKey key) const
{
	return pressed_.at(static_cast<std::size_t>(key)).any();
}

void InputManager::onPress(InputKey key, Callback&& callback)
{
	const auto index = static_cast<int>(key);
	onPressCallbacks_.at(index).emplace_back(std::move(callback));
}

void InputManager::onRelease(InputKey key, Callback&& callback)
{
	const auto index = static_cast<int>(key);
	onReleaseCallbacks_.at(index).emplace_back(std::move(callback));
}

Vec2 InputManager::getCursorScreenPosition() const
{
	return currCursorScreenPosition_;
}

Vec2 InputManager::getCursorDelta() const
{
	return currCursorScreenPosition_ - prevCursorScreenPosition_;
}

void InputManager::press(InputKey key, InputSource src)
{
	const auto keyIndex = static_cast<int>(key);
	const auto srcIndex = static_cast<int>(src);

	if (pressed_.at(keyIndex).none())
	{
		for (const auto& callback : onPressCallbacks_.at(keyIndex))
		{
			callback();
		}
	}

	pressed_.at(keyIndex).set(srcIndex, true);
}

void InputManager::release(InputKey key, InputSource src)
{
	const auto keyIndex	  = static_cast<int>(key);
	const auto srcIndex	  = static_cast<int>(src);

	const bool wasPressed = pressed_.at(keyIndex).any();
	pressed_.at(keyIndex).set(srcIndex, false);
	const bool isReleased = pressed_.at(keyIndex).none();

	if (wasPressed && isReleased)
	{
		for (const auto& callback : onReleaseCallbacks_.at(keyIndex))
		{
			callback();
		}
		lastUpdateAt_ = Clock::now();
	}
}

void InputManager::tick()
{
	const Vector2 mousePos = GetMousePosition();
	if (Vec2{mousePos.x, mousePos.y} != prevCursorScreenPosition_)
	{
		lastUpdateAt_ = Clock::now();
	}
	prevCursorScreenPosition_ = std::exchange(currCursorScreenPosition_, {mousePos.x, mousePos.y});

	static const std::array mouseButtonMapping{std::make_pair(MOUSE_BUTTON_LEFT, InputKey::MouseLeft),
											   std::make_pair(MOUSE_BUTTON_RIGHT, InputKey::MouseRight)};

	for (const auto [raylibKey, appKey] : mouseButtonMapping)
	{
		if (IsMouseButtonPressed(raylibKey))
		{
			press(appKey, InputSource::Mouse);
		}
		if (IsMouseButtonReleased(raylibKey))
		{
			release(appKey, InputSource::Mouse);
		}
	}

	static const std::array keyboardMapping{std::make_pair(KEY_W, InputKey::Forward),
											std::make_pair(KEY_A, InputKey::Left),
											std::make_pair(KEY_S, InputKey::Backward),
											std::make_pair(KEY_D, InputKey::Right),
											std::make_pair(KEY_Q, InputKey::Test)};

	for (const auto [raylibKey, appKey] : keyboardMapping)
	{
		if (IsKeyPressed(raylibKey))
		{
			press(appKey, InputSource::Keybord);
		}
		if (IsKeyReleased(raylibKey))
		{
			release(appKey, InputSource::Keybord);
		}
	}

	if (std::ranges::any_of(pressed_, [](const auto& state) { return state.any(); }))
	{
		lastUpdateAt_ = Clock::now();
	}
}
}  // namespace gs
