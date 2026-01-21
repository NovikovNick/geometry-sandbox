/**
 * @file input_manager.h
 * @brief tracks key states and handles press/release callbacks
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_INPUT_H
#define GEOMETRY_SANDBOX_INPUT_H

#include "core/math.h"
#include "core/types.h"

#include "boost/di.hpp"

#include <array>
#include <bitset>
#include <functional>

namespace gs
{
enum class InputKey : int
{
	Test,
	Forward,
	Backward,
	Left,
	Right,
	MouseLeft,
	MouseRight,
	Count
};

constexpr size_t kInputKeyCount = static_cast<size_t>(InputKey::Count);

/** @brief tracks key states and handles press/release callbacks */
class IInputManager
{
  public:
	virtual bool isKeyPressed(InputKey key) const						   = 0;
	virtual void onPress(InputKey key, std::function<void()>&& callback)   = 0;
	virtual void onRelease(InputKey key, std::function<void()>&& callback) = 0;
	virtual Vec2 getCursorScreenPosition() const						   = 0;
	virtual Vec2 getCursorDelta() const									   = 0;
	virtual Timepoint getLastUpdateAt() const							   = 0;

	virtual void tick()													   = 0;

	virtual ~IInputManager()											   = default;
};

/** @brief basic IInputManager implementation */
class InputManager final : public IInputManager
{
	using Callback = std::function<void()>;

	std::bitset<kInputKeyCount> pressed_;
	std::array<std::vector<Callback>, kInputKeyCount> onPressCallbacks_;
	std::array<std::vector<Callback>, kInputKeyCount> onReleaseCallbacks_;
	Vec2 currCursorScreenPosition_;
	Vec2 prevCursorScreenPosition_;
	Timepoint lastUpdateAt_{Seconds{0}};

  public:
	virtual bool isKeyPressed(InputKey key) const override { return pressed_[static_cast<size_t>(key)]; }
	virtual void onPress(InputKey key, Callback&& callback) override;
	virtual void onRelease(InputKey key, Callback&& callback) override;
	virtual Vec2 getCursorScreenPosition() const override;
	virtual Vec2 getCursorDelta() const override;
	virtual Timepoint getLastUpdateAt() const override { return lastUpdateAt_; };

	virtual void tick() override;

  private:
	void press(InputKey key);
	void release(InputKey key);
};

namespace di
{
inline auto inputManager() noexcept
{
	return boost::di::bind<IInputManager>.to<InputManager>();
}
}  // namespace di
}  // namespace gs
#endif	// GEOMETRY_SANDBOX_INPUT_H
