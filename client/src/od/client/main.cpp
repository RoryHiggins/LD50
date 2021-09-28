#include <od/platform/window.hpp>

bool odClient_run(void);
bool odClient_init(void);
bool odClient_step(void);

static odWindow odClient_window;

bool odClient_init(void) {
	odWindowSettings window_settings{odWindowSettings_get_defaults()};
	return odWindow_open(&odClient_window, &window_settings);
}
bool odClient_step(void) {
	odWindow_step(&odClient_window);
	return odWindow_get_open(&odClient_window);
}

#if OD_BUILD_EMSCRIPTEN
#include <emscripten.h>

void odClient_step_emscripten(void);

void odClient_step_emscripten(void) {
	static bool is_initialized = false;
	if (!is_initialized) {
		if (!odClient_init()) {
			emscripten_cancel_main_loop();
		}
		is_initialized = true;
	}

	if (!odWindow_get_open(&odClient_window) || !odClient_step()) {
		emscripten_cancel_main_loop();
	}
}
bool odClient_run(void) {
	emscripten_set_main_loop(odClient_step_emscripten, 0, true);
	return true;
}
#else
bool odClient_run(void) {
	if (!odClient_init()) {
		return false;
	}

	while (odClient_step()) {
	}

	return true;
}
#endif

int main(int, char**) {
	return odClient_run() ? 0 : 1;
}
