#include <od/platform/texture.hpp>
#include <od/platform/renderer.hpp>

#include <cstring>

#include <od/core/debug.hpp>
#include <od/core/array.hpp>
#include <od/core/bounds.h>
#include <od/core/color.h>
#include <od/core/vector.h>
#include <od/core/vertex.h>
#include <od/platform/primitive.h>
#include <od/platform/window.hpp>
#include <od/test/test.hpp>

#define OD_RENDER_TEST_VERTEX_COUNT OD_TRIANGLE_VERTEX_COUNT

const odVertex odTest_odRenderer_test_vertices[OD_RENDER_TEST_VERTEX_COUNT] = {
	odVertex{odVector{0.0f,0.0f,0.0f,1.0f}, odColor{0x00,0xff,0x00,0xff}, 0.0f,0.0f},
	odVertex{odVector{0.0f,1.0f,0.0f,1.0f}, odColor{0x00,0xff,0x00,0xff}, 0.0f,0.0f},
	odVertex{odVector{1.0f,0.0f,0.0f,1.0f}, odColor{0x00,0xff,0x00,0xff}, 0.0f,0.0f},
};

const odBounds odTest_odRenderer_bounds{0.0f, 0.0f, 640.0f, 480.0f};

static odRenderState odTest_odRenderer_create_state(odTexture* texture) {
	return odRenderState{*odMatrix_get_identity(), *odMatrix_get_identity(), odTest_odRenderer_bounds, texture, nullptr};
}


OD_TEST_FILTERED(odTest_odRenderer_init_destroy, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_check_valid(&window));

	odRenderer renderer;
	OD_ASSERT(odRenderer_init(&renderer, &window));
	OD_ASSERT(odRenderer_check_valid(&renderer));

	// test double init
	OD_ASSERT(odRenderer_init(&renderer, &window));
	OD_ASSERT(odRenderer_check_valid(&renderer));

	odRenderer_destroy(&renderer);

	// test double destroy
	odRenderer_destroy(&renderer);

	// test reuse
	OD_ASSERT(odRenderer_init(&renderer, &window));
	OD_ASSERT(odRenderer_check_valid(&renderer));
	odRenderer_destroy(&renderer);
}
OD_TEST_FILTERED(odTest_odRenderer_destroy_after_window_destroy_fails, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_check_valid(&window));

	{
		odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
		odRenderer renderer;
		OD_ASSERT(odRenderer_init(&renderer, &window));
		OD_ASSERT(odRenderer_check_valid(&renderer));

		odWindow_destroy(&window);
		odRenderer_destroy(&renderer);
	}
}
OD_TEST_FILTERED(odTest_odRenderer_flush, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	odRenderer renderer;
	OD_ASSERT(odRenderer_init(&renderer, &window));

	OD_ASSERT(odRenderer_flush(&renderer));
}
OD_TEST_FILTERED(odTest_odRenderer_clear, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	odRenderer renderer;
	OD_ASSERT(odRenderer_init(&renderer, &window));
	odTexture texture;
	OD_ASSERT(odTexture_init_blank(&texture, &window));
	odRenderState state = odTest_odRenderer_create_state(&texture);

	OD_ASSERT(odRenderer_clear(&renderer, &state, odColor_get_white()));
	OD_ASSERT(odRenderer_flush(&renderer));

	OD_ASSERT(odRenderer_clear(&renderer, &state, odColor_get_white()));
	OD_ASSERT(odRenderer_flush(&renderer));
}
OD_TEST_FILTERED(odTest_odRenderer_draw_vertices, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	odRenderer renderer;
	OD_ASSERT(odRenderer_init(&renderer, &window));
	odTexture texture;
	OD_ASSERT(odTexture_init_blank(&texture, &window));
	odRenderTexture render_texture;
	OD_ASSERT(odRenderTexture_init(&render_texture, &window, 640, 480));
	odRenderState state = odTest_odRenderer_create_state(&texture);

	OD_ASSERT(odRenderer_draw_vertices(&renderer, &state, odTest_odRenderer_test_vertices, OD_RENDER_TEST_VERTEX_COUNT));
	OD_ASSERT(odRenderer_flush(&renderer));

	state.opt_render_texture = &render_texture;
	OD_ASSERT(odRenderer_draw_vertices(&renderer, &state, odTest_odRenderer_test_vertices, OD_RENDER_TEST_VERTEX_COUNT));
	OD_ASSERT(odRenderer_flush(&renderer));
}
OD_TEST_FILTERED(odTest_odRenderer_draw_texture, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	odRenderer renderer;
	OD_ASSERT(odRenderer_init(&renderer, &window));
	odTexture texture;
	OD_ASSERT(odTexture_init_blank(&texture, &window));
	odRenderTexture render_texture;
	OD_ASSERT(odRenderTexture_init(&render_texture, &window, 640, 480));
	odRenderState state = odTest_odRenderer_create_state(&texture);

	OD_ASSERT(odRenderer_draw_texture(&renderer, &state, nullptr, nullptr));
	OD_ASSERT(odRenderer_flush(&renderer));

	state.opt_render_texture = &render_texture;
	OD_ASSERT(odRenderer_draw_texture(&renderer, &state, nullptr, nullptr));
	OD_ASSERT(odRenderer_flush(&renderer));

	state.src_texture = odRenderTexture_get_texture(&render_texture);
	state.opt_render_texture = nullptr;
	OD_ASSERT(odRenderer_draw_texture(&renderer, &state, nullptr, nullptr));
	OD_ASSERT(odRenderer_flush(&renderer));
}
OD_TEST_FILTERED(odTest_odRenderer_init_multiple_renderers, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	odRenderer renderer;
	OD_ASSERT(odRenderer_init(&renderer, &window));
	odTexture texture;
	OD_ASSERT(odTexture_init_blank(&texture, &window));
	odRenderTexture render_texture;
	OD_ASSERT(odRenderTexture_init(&render_texture, &window, 640, 480));
	odRenderState state = odTest_odRenderer_create_state(&texture);

	odRenderer renderer2;
	OD_ASSERT(odRenderer_init(&renderer2, &window));
	OD_ASSERT(odRenderer_check_valid(&renderer2));
	OD_ASSERT(odRenderer_check_valid(&renderer2));

	OD_ASSERT(odRenderer_draw_vertices(&renderer, &state, odTest_odRenderer_test_vertices, OD_RENDER_TEST_VERTEX_COUNT));
	OD_ASSERT(odRenderer_flush(&renderer));

	OD_ASSERT(odRenderer_draw_vertices(&renderer2, &state, odTest_odRenderer_test_vertices, OD_RENDER_TEST_VERTEX_COUNT));
	OD_ASSERT(odRenderer_flush(&renderer2));

	odRenderer_destroy(&renderer2);
	OD_ASSERT(odRenderer_check_valid(&renderer));
	OD_ASSERT(odRenderer_draw_vertices(&renderer, &state, odTest_odRenderer_test_vertices, OD_RENDER_TEST_VERTEX_COUNT));
	OD_ASSERT(odRenderer_flush(&renderer));
}
OD_TEST(odTest_odRenderer_init_without_context_fails) {
	odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
	odRenderer renderer;
	OD_ASSERT(!odRenderer_init(&renderer, nullptr));
}
OD_TEST(odTest_odRenderer_destroy_invalid) {
	odRenderer renderer;
	odRenderer_destroy(&renderer);
}

OD_TEST_SUITE(
	odTestSuite_odRenderer,
	odTest_odRenderer_init_destroy,
	odTest_odRenderer_destroy_after_window_destroy_fails,
	odTest_odRenderer_flush,
	odTest_odRenderer_clear,
	odTest_odRenderer_draw_vertices,
	odTest_odRenderer_draw_texture,
	odTest_odRenderer_init_multiple_renderers,
	odTest_odRenderer_init_without_context_fails,
	odTest_odRenderer_destroy_invalid,
)
