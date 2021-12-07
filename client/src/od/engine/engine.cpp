#include <od/engine/engine.hpp>

#if OD_BUILD_EMSCRIPTEN
#include <emscripten.h>
#endif   // OD_BUILD_EMSCRIPTEN

#include <od/core/debug.h>
#include <od/platform/window.hpp>

#if OD_BUILD_EMSCRIPTEN
void odEngine_step_emscripten(void* engine_raw);
#endif

bool odEngine_init(odEngine* engine) {
	odWindowSettings window_settings{*odWindowSettings_get_defaults()};
	return odWindow_init(&engine->window, &window_settings);
}
bool odEngine_step(odEngine* engine) {
	if (!engine->is_initialized) {
		if (!OD_CHECK(odEngine_init(engine))) {
			return false;
		}
		engine->is_initialized = true;
	}

	if (!OD_CHECK(odWindow_check_valid(&engine->window))) {
		return false;
	}

	if (!odWindow_step(&engine->window)) {
		return false;
	}

	return true;
}

#if OD_BUILD_EMSCRIPTEN
void odEngine_step_emscripten(void* engine_raw) {
	odEngine* engine = reinterpret_cast<odEngine*>(engine_raw);
	if (!odEngine_step(engine)) {
		emscripten_cancel_main_loop();
	}
}
OD_NO_DISCARD bool odEngine_run(odEngine* engine) {
	emscripten_set_main_loop_arg(&odEngine_step_emscripten, reinterpret_cast<void*>(engine), 0, true);

	return true;
}
#else
OD_NO_DISCARD bool odEngine_run(odEngine* engine) {
	OD_INFO("Running engine");

	int32_t logged_errors_before = odLog_get_logged_error_count();

	while (odEngine_step(engine)) {
	}

	if (odLog_get_logged_error_count() > logged_errors_before) {
		OD_INFO("Engine ended with errors");
		return false;
	}

	OD_INFO("Engine ended gracefully");
	return true;
}
#endif

odEngine::odEngine()
	: is_initialized{false}, window{}, tag_names{}, entity_index{}, lua_client{} {
}
odEngine::odEngine(odEngine&& other) = default;
odEngine& odEngine::operator=(odEngine&& other) = default;
odEngine::~odEngine() = default;
