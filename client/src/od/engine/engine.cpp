#include <od/engine/engine.hpp>

#if defined(__cplusplus)
extern "C" {
#endif
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#if defined(__cplusplus)
}  // extern "C"
#endif

#if OD_BUILD_EMSCRIPTEN
#include <emscripten.h>
#endif   // OD_BUILD_EMSCRIPTEN

#include <od/core/math.h>
#include <od/core/debug.h>
#include <od/platform/primitive.h>
#include <od/platform/image.hpp>
#include <od/platform/rendering.hpp>
#include <od/platform/window.hpp>

static void odEngineFrame_start_next(odEngineFrame* frame);

void odEngineFrame_start_next(odEngineFrame* frame) {
	if (!OD_DEBUG_CHECK(frame != nullptr)) {
		return;
	}

	if (!OD_DEBUG_CHECK(frame->game_vertices.set_count(0))) {
		return;
	}

	if (!OD_DEBUG_CHECK(frame->window_vertices.set_count(0))) {
		return;
	}

	frame->counter++;
}
odEngineFrame::odEngineFrame()
: counter{0}, game_vertices{}, window_vertices{} {
}

#if OD_BUILD_EMSCRIPTEN
void odEngine_step_emscripten(void* engine_raw);
#endif

bool odEngine_init(odEngine* engine) {
	if (!OD_CHECK(engine != nullptr)) {
		return false;
	}

	odWindowSettings window_settings{*odWindowSettings_get_defaults()};
	if (!OD_CHECK(odWindow_init(&engine->window, &window_settings))) {
		return false;
	}

	// BEGIN throwaway texture loading test code - TODO remove
	odImage image{};
	if (!OD_CHECK(odImage_read_png_file(&image, "./modules/example_minimal/data/sprites.png"))) {
		return false;
	}
	if (!OD_CHECK(odTexture_init(
		&engine->window.texture,
		engine->window.render_context_native,
		odImage_get_const(&image),
		image.width,
		image.height))) {
		return false;
	}
	// END throwaway texture loading test code - TODO remove

	return true;
}
bool odEngine_step(odEngine* engine) {
	if (!OD_CHECK(engine != nullptr)) {
		return false;
	}

	if (!OD_CHECK(odWindow_check_valid(&engine->window))) {
		return false;
	}

	// BEGIN throwaway rendering test code - TODO remove
	const odWindowSettings* window_settings = odWindow_get_settings(&engine->window);
	const int32_t scale_x = window_settings->game_width / 2;
	const int32_t scale_y = window_settings->game_height / 2;
	{
		// float u = 80.0f + (8.0f * float((engine->frame.counter >> 3) % 4));
		// float v = 40.0f;
		odPrimitiveRect rect{
			odBounds{0.0f, 0.0f, 288.0f, 128.0f},
			odBounds{0.0f, 0.0f, 144.0f, 64.0f},
			*odColor_get_white(),
			0.0f,
		};
		const int32_t vertices_count = OD_PRIMITIVE_RECT_VERTEX_COUNT;
		odVertex vertices[vertices_count];
		odPrimitiveRect_get_vertices(&rect, vertices);

		// OD_DISCARD_RESULT(engine->frame.game_vertices.extend(vertices, vertices_count));
		OD_DISCARD_RESULT(engine->frame.window_vertices.extend(vertices, vertices_count));
	}
	{
		const int32_t vertices_count = 3;
		odVertex vertices[vertices_count] = {
			{{0.0f,0.0f,0.0f,1.0f},
			 {0xff,0x00,0x00,0xff},
			 0.0f,0.0f},
			{{0.0f,1.0f,0.0f,1.0f},
			 {0x00,0xff,0x00,0xff},
			 0.0f,0.0f},
			{{1.0f,0.0f,0.0f,1.0f},
			 {0x00,0x00,0xff,0xff},
			 0.0f,0.0f},
		};

		int32_t counter_staggered = (engine->frame.counter >> 3) << 3;

		odMatrix4 matrix{};
		odMatrix4_init(&matrix, float(scale_x), float(scale_y), 1.0f, float(scale_x), float(scale_y), 0.0f);
		odMatrix4_rotate_clockwise_2d(&matrix, float(counter_staggered) * (OD_MATH_PI / 64.0f));
		for (odVertex& vertex: vertices) {
			odVertex_transform(&vertex, &matrix);
		}

		OD_DISCARD_RESULT(engine->frame.game_vertices.extend(vertices, vertices_count));
	}
	// END throwaway rendering test code - TODO remove

	odWindowFrame window_frame{
		engine->frame.game_vertices.begin(),
		engine->frame.game_vertices.count,
		engine->frame.window_vertices.begin(),
		engine->frame.window_vertices.count
	};

	if (!odWindow_step(&engine->window, &window_frame)) {
		return false;
	}

	odEngineFrame_start_next(&engine->frame);

	return true;
}

#if OD_BUILD_EMSCRIPTEN
void odEngine_step_emscripten(void* engine_raw) {
	if (!OD_CHECK(engine_raw != nullptr)) {
		return;
	}

	odEngine* engine = reinterpret_cast<odEngine*>(engine_raw);

	if (!engine->is_initialized) {
		if (!OD_CHECK(odEngine_init(engine))) {
			emscripten_cancel_main_loop();
			return;
		}

		engine->is_initialized = true;
	}

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
	: is_initialized{false}, window{}, tag_names{}, entity_index{}, frame{} {
}
odEngine::odEngine(odEngine&& other) = default;
odEngine& odEngine::operator=(odEngine&& other) = default;
odEngine::~odEngine() = default;
