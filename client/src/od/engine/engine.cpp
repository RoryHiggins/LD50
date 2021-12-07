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
	if (!OD_CHECK(engine != nullptr)) {
		return false;
	}

	odWindowSettings window_settings{*odWindowSettings_get_defaults()};
	return odWindow_init(&engine->window, &window_settings);
}
bool odEngine_step(odEngine* engine) {
	if (!OD_CHECK(engine != nullptr)) {
		return false;
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
	if (!OD_CHECK(engine_raw != nullptr)) {
		return false;
	}

	if (!engine->is_initialized) {
		if (!OD_CHECK(odEngine_init(engine))) {
			return false;
		}
		engine->is_initialized = true;
	}

	odEngine* engine = reinterpret_cast<odEngine*>(engine_raw);
	if (!odEngine_step(engine)) {
		emscripten_cancel_main_loop();
	}
}
OD_NO_DISCARD bool odEngine_run(odEngine* engine) {
	if (!OD_CHECK(engine != nullptr)) {
		return false;
	}

	emscripten_set_main_loop_arg(&odEngine_step_emscripten, reinterpret_cast<void*>(engine), 0, true);

	return true;
}
#else
OD_NO_DISCARD bool odEngine_run(odEngine* engine) {
	if (!OD_CHECK(engine != nullptr)) {
		return false;
	}

	OD_INFO("Running engine");

	int32_t logged_errors_before = odLog_get_logged_error_count();

	if (!OD_CHECK(odEngine_init(engine))) {
		return false;
	}

	while (engine->window.is_open) {
		if (!OD_CHECK(odEngine_step(engine))) {
			return false;
		}
	}

	if (odLog_get_logged_error_count() > logged_errors_before) {
		OD_INFO("Engine ended with error logs");
		return false;
	}

	OD_INFO("Engine ended gracefully");
	return true;
}
#endif

odEngine::odEngine()
	: is_initialized{false}, window{}, tag_names{}, entity_index{}, lua{} {
}
odEngine::odEngine(odEngine&& other) = default;
odEngine& odEngine::operator=(odEngine&& other) = default;
odEngine::~odEngine() = default;
