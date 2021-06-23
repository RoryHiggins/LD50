#include <od/platform/window.hpp>

#if OD_BUILD_EMSCRIPTEN
#include <emscripten.h>

static void odClient_step_emscripten(void* window_raw) {
	odWindow* window = static_cast<odWindow*>(window_raw);

	if (!odWindow_get_open(window)) {
		emscripten_cancel_main_loop();
	}

	odWindow_step(window);
}
static void odClient_run(odWindow* window) {
	emscripten_set_main_loop_arg(odClient_step_emscripten, static_cast<void*>(window), 0, true);
}
#else
static void odClient_run(odWindow* window) {
	while (odWindow_get_open(window)) {
		odWindow_step(window);
	}
}
#endif

int main(int, char**) {
	odWindowSettings window_settings{odWindowSettings_get_defaults()};
	odWindow window;
	if (!odWindow_open(&window, &window_settings)) {
		return 1;
	}

	odClient_run(&window);

	return 0;
}
