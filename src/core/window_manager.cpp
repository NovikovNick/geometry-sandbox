#include "core/window_manager.h"

#include "core/settings.h"

#include "raylib.h"
#include "rlImGui.h"
#include "rlgl.h"

#include <utility>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

namespace
{
EM_BOOL onWebResize(int eventType, const EmscriptenUiEvent* uiEvent, void* userData)
{
	int width  = (int)uiEvent->windowInnerWidth;
	int height = (int)uiEvent->windowInnerHeight;
	SetWindowSize(width, height);  // Tells Raylib to update its internal screen size
	return EM_TRUE;
}
}  // namespace
#endif

namespace gs
{
void WindowManager::initWindow()
{
	unsigned flags = FLAG_WINDOW_RESIZABLE;
	flags |= settings_->fullscreen ? FLAG_FULLSCREEN_MODE : 0;
	flags |= settings_->multiSampleAntiAliasing4X ? FLAG_MSAA_4X_HINT : 0;
	flags |= settings_->vSync ? FLAG_VSYNC_HINT : 0;

	SetConfigFlags(flags);
	InitWindow(settings_->width, settings_->height, settings_->title.data());

#ifdef __EMSCRIPTEN__
	double width, height;
	emscripten_get_element_css_size("#canvas", &width, &height);
	emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, onWebResize);
	SetWindowSize(width, height);
#else
	// SetTargetFPS(60);  // do not set fps when used in browser for better web performance
#endif

	rlImGuiSetup(true);
}

void WindowManager::tick()
{

#ifndef __EMSCRIPTEN__
	if (WindowShouldClose())
	{
		for (auto& callback : onCloseCallbacks_)
		{
			callback();
		}
	}
#endif

	if (IsWindowResized())
	{
		for (auto& callback : onResizeCallbacks_)
		{
			callback(GetScreenWidth(), GetScreenHeight());
		}
	}
}

void WindowManager::onClose(OnCloseCallback&& callback)
{
	onCloseCallbacks_.push_back(std::move(callback));
}

void WindowManager::onResize(OnResizeCallback&& callback)
{
	onResizeCallbacks_.push_back(std::move(callback));
}

WindowManager::~WindowManager()
{
	rlImGuiShutdown();
	CloseWindow();
}
}  // namespace gs
