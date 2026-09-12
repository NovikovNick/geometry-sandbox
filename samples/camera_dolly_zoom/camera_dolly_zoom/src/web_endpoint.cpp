#include <emscripten/bind.h>
#include <emscripten/emscripten.h>

#include "camera_dolly_zoom_sample.h"

EMSCRIPTEN_BINDINGS()
{
	gs::init();
	emscripten_set_main_loop(&gs::drawNextFrame, /*fps*/ 0, /*simulate_infinite_loop*/ false);
}
