#include <emscripten/bind.h>
#include <emscripten/emscripten.h>

#include "camera_handedness_sample.h"

EMSCRIPTEN_BINDINGS()
{
	emscripten::register_vector<float>("VectorFloat");

	emscripten::value_object<gs::Matrix4x4DTO>("Matrix4x4DTO").field("data", &gs::Matrix4x4DTO::data);

	emscripten::function("getModel", &gs::getModel);
	emscripten::function("getView", &gs::getView);
	emscripten::function("getProjection", &gs::getProjection);

	gs::init();
	emscripten_set_main_loop(&gs::drawNextFrame, /*fps*/ 0, /*simulate_infinite_loop*/ false);
}
