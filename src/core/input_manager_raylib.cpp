#include "core/input_manager.h"

#include "core/math.h"
#include "core/types.h"

#include "raylib.h"

#include <array>
#include <bitset>
#include <utility>
#include <vector>

namespace gs
{
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

void InputManager::press(InputKey key)
{
	const auto index = static_cast<int>(key);
	if (!pressed_[index])
	{
		pressed_[index] = true;
		for (const auto& callback : onPressCallbacks_.at(index))
		{
			callback();
		}
	}
}
void InputManager::release(InputKey key)
{
	const auto index = static_cast<int>(key);
	if (pressed_[index])
	{
		pressed_[index] = false;
		for (const auto& callback : onReleaseCallbacks_.at(index))
		{
			callback();
		}
	}
	lastUpdateAt_ = Clock::now();
}

void InputManager::tick()
{
	const Vector2 mousePos = GetMousePosition();
	if (Vec2{mousePos.x, mousePos.y} != prevCursorScreenPosition_)
	{
		lastUpdateAt_ = Clock::now();
	}
	prevCursorScreenPosition_ = std::exchange(currCursorScreenPosition_, {mousePos.x, mousePos.y});

	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	{
		press(InputKey::MouseLeft);
	}
	if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
	{
		release(InputKey::MouseLeft);
	}

	if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
	{
		press(InputKey::MouseRight);
	}
	if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT))
	{
		release(InputKey::MouseRight);
	}

	if (IsKeyPressed(KEY_W))
	{
		press(InputKey::Forward);
	}
	if (IsKeyReleased(KEY_W))
	{
		release(InputKey::Forward);
	}

	if (IsKeyPressed(KEY_S))
	{
		press(InputKey::Backward);
	}
	if (IsKeyReleased(KEY_S))
	{
		release(InputKey::Backward);
	}

	if (IsKeyPressed(KEY_A))
	{
		press(InputKey::Left);
	}
	if (IsKeyReleased(KEY_A))
	{
		release(InputKey::Left);
	}

	if (IsKeyPressed(KEY_D))
	{
		press(InputKey::Right);
	}
	if (IsKeyReleased(KEY_D))
	{
		release(InputKey::Right);
	}

	if (IsKeyPressed(KEY_Q))
	{
		press(InputKey::Test);
	}
	if (IsKeyReleased(KEY_Q))
	{
		release(InputKey::Test);
	}

	if (pressed_.any())
	{
		lastUpdateAt_ = Clock::now();
	}
}
}  // namespace gs
