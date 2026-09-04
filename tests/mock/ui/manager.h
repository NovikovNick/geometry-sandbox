#ifndef GEOMETRY_SANDBOX_UI_MANAGER_MOCK_H
#define GEOMETRY_SANDBOX_UI_MANAGER_MOCK_H

#include "ui/manager.h"

#include "boost/di.hpp"
#include <gmock/gmock.h>

namespace gs::mock
{

class UIManager : public IUIManager
{
  public:
	MOCK_METHOD(void, init, (), (override));
	MOCK_METHOD(void, tick, (), (override));
	MOCK_METHOD(void, render, (), (override));
	MOCK_METHOD(void, onDetailsViewDraw, (std::function<void()> callback), (override));
	MOCK_METHOD(bool, isMouseCaptured, (), (const, override));
};

namespace di
{
inline auto uiManager() noexcept
{
	return boost::di::bind<IUIManager>().to<UIManager>();
}
}  // namespace di
}  // namespace gs::mock

#endif	// GEOMETRY_SANDBOX_UI_MANAGER_MOCK_H