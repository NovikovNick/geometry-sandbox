/**
 * @file input_manager.h
 * @brief tracks key states and handles press/release callbacks
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_INPUT_H
#define GEOMETRY_SANDBOX_INPUT_H

#include "core/base_app_component.h"
#include "core/math.h"
#include "core/types.h"

#include "boost/di.hpp"

#include <array>
#include <bitset>
#include <functional>

namespace gs
{
enum class InputKey : std::size_t
{
	Forward,
	Backward,
	Left,
	Right,
	MouseLeft,
	MouseRight,
	Test,
	Count
};

enum class InputSource : std::size_t
{
	Keybord,
	Mouse,
	UI,
	Count
};

constexpr std::size_t kInputKeyCount	= static_cast<std::size_t>(InputKey::Count);
constexpr std::size_t kInputSourceCount = static_cast<std::size_t>(InputSource::Count);

/** @brief tracks key states and handles press/release callbacks */
class IInputManager
{
  public:
	virtual bool isKeyPressed(InputKey) const						   = 0;
	virtual void onPress(InputKey, std::function<void()>&& callback)   = 0;
	virtual void onRelease(InputKey, std::function<void()>&& callback) = 0;
	virtual Vec2 getCursorScreenPosition() const					   = 0;
	virtual Vec2 getCursorDelta() const								   = 0;
	virtual Timepoint getLastUpdateAt() const						   = 0;
	virtual void press(InputKey, InputSource = InputSource::Keybord)   = 0;
	virtual void release(InputKey, InputSource = InputSource::Keybord) = 0;

	virtual void tick()												   = 0;

	virtual ~IInputManager()										   = default;
};

/** @brief basic IInputManager implementation */
class InputManager : public BaseManager, public IInputManager
{
	using Callback = std::function<void()>;

	std::array<std::bitset<kInputSourceCount>, kInputKeyCount> pressed_;
	std::array<std::vector<Callback>, kInputKeyCount> onPressCallbacks_;
	std::array<std::vector<Callback>, kInputKeyCount> onReleaseCallbacks_;
	Vec2 currCursorScreenPosition_;
	Vec2 prevCursorScreenPosition_;
	Timepoint lastUpdateAt_{Seconds{0}};

  public:
	InputManager(const std::shared_ptr<Settings>& settings, const std::shared_ptr<ILogManager>& log) : BaseManager(settings, log) {}

	virtual bool isKeyPressed(InputKey key) const override;
	virtual void onPress(InputKey, Callback&&) override;
	virtual void onRelease(InputKey, Callback&&) override;
	virtual Vec2 getCursorScreenPosition() const override;
	virtual Vec2 getCursorDelta() const override;
	virtual Timepoint getLastUpdateAt() const override { return lastUpdateAt_; };
	virtual void press(InputKey, InputSource) override;
	virtual void release(InputKey, InputSource) override;

	virtual void tick() override;

  private:
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
