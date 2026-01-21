#include <emscripten/bind.h>
#include <emscripten/emscripten.h>

#include "closest_point_on_plane.h"

EMSCRIPTEN_BINDINGS()
{
	gs::init();
	emscripten_set_main_loop(&gs::drawNextFrame, /*fps*/ 0, /*simulate_infinite_loop*/ false);
}
