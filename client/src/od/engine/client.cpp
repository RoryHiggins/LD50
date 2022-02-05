#include <od/engine/client.hpp>

#include <cstring>

#if OD_BUILD_EMSCRIPTEN
#include <emscripten.h>
#endif   // OD_BUILD_EMSCRIPTEN

#include <od/core/math.h>
#include <od/core/debug.h>
#include <od/core/bounds.h>
#include <od/platform/primitive.h>
#include <od/platform/image.hpp>
#include <od/platform/texture.hpp>
#include <od/platform/renderer.hpp>
#include <od/platform/window.hpp>

static void odClientFrame_start_next(odClientFrame* frame);

void odClientFrame_start_next(odClientFrame* frame) {
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
odClientFrame::odClientFrame()
: counter{0}, game_vertices{}, window_vertices{} {
}

const odClientSettings* odClientSettings_get_defaults() {
	static const odClientSettings settings{
		*odWindowSettings_get_defaults(),
		160,
		120
	};
	return &settings;
}
bool odClientSettings_check_valid(const odClientSettings* settings) {
	if (!OD_CHECK(settings != nullptr)
		|| !OD_CHECK(odWindowSettings_check_valid(&settings->window))
		|| !OD_CHECK(odInt32_fits_float(settings->game_height))
		|| !OD_CHECK(settings->game_width > 0)
		|| !OD_CHECK(odInt32_fits_float(settings->game_height))
		|| !OD_CHECK(settings->game_height > 0)) {
		return false;
	}

	return true;
}

bool odClient_init(odClient* engine, const odClientSettings* opt_settings) {
	if (!OD_CHECK(engine != nullptr)) {
		return false;
	}

	if (opt_settings != nullptr) {
		if (!OD_CHECK(odClientSettings_check_valid(opt_settings))) {
			return false;
		}
		engine->settings = *opt_settings;
	}

	if (!OD_CHECK(odWindow_init(&engine->window, &engine->settings.window))) {
		return false;
	}

	if (!OD_CHECK(odRenderer_init(&engine->renderer, &engine->window))) {
		return false;
	}

	if (!OD_CHECK(odTexture_init_blank(&engine->src_texture, &engine->window))) {
		return false;
	}

	if (!OD_CHECK(odRenderTexture_init(
		&engine->game_render_texture, &engine->window, engine->settings.game_width,
		engine->settings.game_height))) {
		return false;
	}

	// BEGIN throwaway texture loading test code - TODO remove
	odImage image{};
	if (!OD_CHECK(odImage_read_png_file(&image, "./examples/minimal/data/sprites.png"))) {
		return false;
	}
	if (!OD_CHECK(odTexture_init(
		&engine->src_texture,
		&engine->window,
		odImage_begin_const(&image),
		image.width,
		image.height))) {
		return false;
	}
	// END throwaway texture loading test code - TODO remove

	engine->is_initialized = true;

	return true;
}
void odClient_destroy(odClient* engine) {
	if (!OD_CHECK(engine != nullptr)) {
		return;
	}

	odEntityIndex_destroy(&engine->entity_index);

	engine->is_initialized = false;
	odRenderTexture_destroy(&engine->game_render_texture);
	odTexture_destroy(&engine->src_texture);
	odRenderer_destroy(&engine->renderer);
	odWindow_destroy(&engine->window);
}
bool odClient_set_settings(odClient* engine, const odClientSettings* settings) {
	if (!OD_CHECK(engine != nullptr)
		|| !OD_CHECK(odClientSettings_check_valid(settings))) {
		return false;
	}

	if (!engine->is_initialized) {
		engine->settings = *settings;
		return true;
	}

	if (!OD_CHECK(odWindow_set_settings(&engine->window, &settings->window))) {
		return false;
	}

	if (!OD_CHECK(odRenderTexture_init(&engine->game_render_texture, &engine->window, settings->game_width, settings->game_height))) {
		return false;
	}

	engine->settings = *settings;

	return true;
}
bool odClient_step(odClient* engine) {
	if (!OD_CHECK(engine != nullptr)) {
		return false;
	}

	if (!OD_CHECK(odWindow_check_valid(&engine->window))) {
		return false;
	}

	int32_t entity_vertices_count;
	const odVertex* entity_vertices = odEntityIndex_get_all_vertices(&engine->entity_index, &entity_vertices_count);
	if (entity_vertices_count > 0) {
		if (!OD_CHECK(entity_vertices)) {
			return false;
		}

		if (!OD_CHECK(engine->frame.game_vertices.extend(entity_vertices, entity_vertices_count))) {
			return false;
		}
	}

	// BEGIN throwaway rendering test code - TODO remove
	{
		odSpritePrimitive sprite{
			odBounds{0.0f, 0.0f, 288.0f, 128.0f},
			odBounds{0.0f, 0.0f, 144.0f, 64.0f},
			*odColor_get_white(),
			0.0f,
		};
		const int32_t vertices_count = OD_SPRITE_VERTEX_COUNT;
		odVertex vertices[vertices_count];
		odSpritePrimitive_get_vertices(&sprite, vertices);

		// OD_DISCARD(engine->frame.game_vertices.extend(vertices, vertices_count));
		OD_DISCARD(engine->frame.window_vertices.extend(vertices, vertices_count));
	}
	{
		const int32_t vertices_count = OD_TRIANGLE_VERTEX_COUNT;
		odVertex vertices_base[vertices_count] = {
			odVertex{odVector{0.0f,0.0f,0.0f,0.0f}, odColor{0xff,0x00,0x00,0xff}, 0.0f,0.0f},
			odVertex{odVector{0.0f,1.0f,0.0f,0.0f}, odColor{0xff,0xff,0x00,0xff}, 0.0f,0.0f},
			odVertex{odVector{1.0f,0.0f,0.0f,0.0f}, odColor{0xff,0xff,0x00,0xff}, 0.0f,0.0f},
		};

		const float scale_x = static_cast<float>(engine->settings.game_width / 2);
		const float scale_y = static_cast<float>(engine->settings.game_height / 2);

		const float translate_x = scale_x;
		const float translate_y = scale_y;

		odMatrix matrix{};
		odMatrix_init(&matrix, scale_x, scale_y, 1.0f, translate_x, translate_y, 0.0f);
		odMatrix_rotate_z_3d(&matrix, static_cast<float>(engine->frame.counter));
		odVertex vertices[vertices_count]{};

		for (int32_t i = 0; i < 12; i++) {
			memcpy(vertices, vertices_base, vertices_count * sizeof(odVertex));
			odMatrix_rotate_z_3d(&matrix, 30.0f);

			for (odVertex& vertex: vertices) {
				odVertex_transform_3d(&vertex, &matrix);
				vertex.color.b = static_cast<uint8_t>(3 * i);
				vertex.color.g = 192 - static_cast<uint8_t>(4 * i);
				vertex.pos.z = static_cast<float>(i);
			}
			OD_DISCARD(engine->frame.game_vertices.extend(vertices, vertices_count));
		}
	}
	// END throwaway rendering test code - TODO remove

	odClientSettings* engine_settings = &engine->settings;
	odMatrix projection{};
	odMatrix_init_ortho_2d(&projection, engine_settings->game_width, engine_settings->game_height);
	odRenderState draw_to_game{
		*odMatrix_get_identity(),
		projection,
		odBounds{
			0.0f,
			0.0f,
			static_cast<float>(engine_settings->game_width),
			static_cast<float>(engine_settings->game_height)
		},
		&engine->src_texture,
		&engine->game_render_texture
	};

	odWindowSettings* window_settings = &engine->window.settings;
	odMatrix window_projection{};
	odMatrix_init_ortho_2d(&window_projection, window_settings->window_width, window_settings->window_height);
	odRenderState draw_to_window{
		*odMatrix_get_identity(),
		window_projection,
		odBounds{
			0.0f,
			0.0f,
			static_cast<float>(window_settings->window_width),
			static_cast<float>(window_settings->window_height)
		},
		&engine->src_texture,
		/* opt_render_texture*/ nullptr
	};

	odRenderState copy_game_to_window{draw_to_window};
	copy_game_to_window.src_texture = odRenderTexture_get_texture(&engine->game_render_texture);

	// draw game
	odTrianglePrimitive_sort_vertices(
		reinterpret_cast<odTrianglePrimitive*>(engine->frame.game_vertices.begin()),
		engine->frame.game_vertices.get_count() / OD_TRIANGLE_VERTEX_COUNT);
	if (!OD_CHECK(odRenderer_clear(&engine->renderer, &draw_to_game, odColor_get_white()))) {
		return false;
	}
	if (!OD_CHECK(odRenderer_draw_vertices(
		&engine->renderer,
		&draw_to_game,
		engine->frame.game_vertices.begin(),
		engine->frame.game_vertices.get_count()))) {
		return false;
	}

	// draw window
	odTrianglePrimitive_sort_vertices(
		reinterpret_cast<odTrianglePrimitive*>(engine->frame.window_vertices.begin()),
		engine->frame.window_vertices.get_count() / OD_TRIANGLE_VERTEX_COUNT);
	if (!OD_CHECK(odRenderer_clear(&engine->renderer, &draw_to_window, odColor_get_white()))) {
		return false;
	}
	if (!OD_CHECK(odRenderer_draw_texture(&engine->renderer, &copy_game_to_window, nullptr, nullptr))) {
		return false;
	}
	if (!OD_CHECK(odRenderer_draw_vertices(
		&engine->renderer,
		&draw_to_window,
		engine->frame.window_vertices.begin(),
		engine->frame.window_vertices.get_count()))) {
		return false;
	}

	// wait for draw calls
	if (!OD_CHECK(odRenderer_flush(&engine->renderer))) {
		return false;
	}

	odClientFrame_start_next(&engine->frame);

	if (!odWindow_step(&engine->window)) {
		return false;
	}

	return true;
}
#if OD_BUILD_EMSCRIPTEN
void odClient_step_emscripten(void* engine_raw);
void odClient_step_emscripten(void* engine_raw) {
	if (!OD_CHECK(engine_raw != nullptr)) {
		return;
	}

	odClient* engine = reinterpret_cast<odClient*>(engine_raw);

	if (!engine->is_initialized) {
		if (!OD_CHECK(odClient_init(engine, nullptr))) {
			emscripten_cancel_main_loop();
			return;
		}

		engine->is_initialized = true;
	}

	if (!odClient_step(engine)) {
		emscripten_cancel_main_loop();
	}
}
OD_NO_DISCARD bool odClient_run(odClient* engine, const odClientSettings* opt_settings) {
	if (!OD_CHECK(engine != nullptr)) {
		return false;
	}

	if (opt_settings != nullptr) {
		if (!OD_CHECK(odClientSettings_check_valid(opt_settings))) {
			return false;
		}
		engine->settings = *opt_settings;
	}

	emscripten_set_main_loop_arg(&odClient_step_emscripten, reinterpret_cast<void*>(engine), 0, true);

	return true;
}
#else
OD_NO_DISCARD bool odClient_run(odClient* engine, const odClientSettings* opt_settings) {
	if (!OD_CHECK(engine != nullptr)
		|| !OD_CHECK((opt_settings == nullptr) || odClientSettings_check_valid(opt_settings))) {
		return false;
	}

	OD_INFO("Starting game loop");

	bool ok = true;
	int32_t logged_errors_before = odLog_get_logged_error_count();

	ok = ok && OD_CHECK(odClient_init(engine, opt_settings));

	while (ok && engine->window.is_open) {
		ok = ok && OD_CHECK(odClient_step(engine));
	}

	ok = ok && (odLog_get_logged_error_count() == logged_errors_before);

	if (ok) {
		OD_INFO("Game loop ended gracefully");
	} else {
		OD_INFO("Game loop ended with error logs");
	}

	return ok;
}
#endif
odClient::odClient()
	: settings{*odClientSettings_get_defaults()}, window{}, renderer{}, src_texture{},
	game_render_texture{}, is_initialized{false}, entity_index{}, frame{} {
}
odClient::odClient(odClient&& other) = default;
odClient& odClient::operator=(odClient&& other) = default;
odClient::~odClient() {
	odClient_destroy(this);
}
