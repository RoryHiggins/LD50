#include <od/platform/renderer.hpp>

#include <od/core/primitive.h>
#include <od/platform/window.hpp>
#include <od/platform/texture.hpp>
#include <od/platform/render_texture.hpp>
#include <od/platform/render_state.h>
#include <od/test/test.hpp>

const int32_t odRender_test_vertices_count = 3;
	const odVertex odRender_test_vertices[odRender_test_vertices_count] = {
		{0,0,0, 0x00,0xff,0x00,0xff,  0,0},
		{0,1,0, 0x00,0xff,0x00,0xff,  0,0},
		{1,0,0, 0x00,0xff,0x00,0xff,  0,0},
	};

OD_TEST_FILTERED(odRenderer, init_destroy, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_get_valid(&window));

	odRenderer renderer;
	OD_ASSERT(!odRenderer_get_valid(&renderer));
	OD_ASSERT(odRenderer_init(&renderer, window.render_context_native));
	OD_ASSERT(odRenderer_get_valid(&renderer));

	// test double init
	OD_ASSERT(odRenderer_init(&renderer, window.render_context_native));
	OD_ASSERT(odRenderer_get_valid(&renderer));

	odRenderer_destroy(&renderer);
	OD_ASSERT(!odRenderer_get_valid(&renderer));

	// test double destroy
	odRenderer_destroy(&renderer);
	OD_ASSERT(!odRenderer_get_valid(&renderer));

	// test reuse
	OD_ASSERT(odRenderer_init(&renderer, window.render_context_native));
	OD_ASSERT(odRenderer_get_valid(&renderer));
	odRenderer_destroy(&renderer);
	OD_ASSERT(!odRenderer_get_valid(&renderer));
}
OD_TEST_FILTERED(odRenderer, destroy_after_window_destroy_fails, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_get_valid(&window));

	{
		odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
		odRenderer renderer;
		OD_ASSERT(odRenderer_init(&renderer, window.render_context_native));
		OD_ASSERT(odRenderer_get_valid(&renderer));

		odWindow_destroy(&window);
		odRenderer_destroy(&renderer);
		OD_ASSERT(!odRenderer_get_valid(&renderer));
	}
}
OD_TEST_FILTERED(odRenderer, flush, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odRenderer_flush(&window.renderer));
}
OD_TEST_FILTERED(odRenderer, clear, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odRenderer_clear(&window.renderer, odColor_white, nullptr));
	OD_ASSERT(odRenderer_flush(&window.renderer));

	OD_ASSERT(odRenderer_clear(&window.renderer, odColor_white, &window.game_render_texture));
	OD_ASSERT(odRenderer_flush(&window.renderer));
}
OD_TEST_FILTERED(odRenderer, draw_vertices, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));

	odRenderState state{odTransform_identity, odTransform_identity, odBounds{0, 0, 640, 480}, &window.texture, nullptr};
	OD_ASSERT(odRenderer_draw_vertices(&window.renderer, state, odRender_test_vertices, odRender_test_vertices_count));
	OD_ASSERT(odRenderer_flush(&window.renderer));

	state.render_texture = &window.game_render_texture;
	OD_ASSERT(odRenderer_draw_vertices(&window.renderer, state, odRender_test_vertices, odRender_test_vertices_count));
	OD_ASSERT(odRenderer_flush(&window.renderer));
}
OD_TEST_FILTERED(odRenderer, draw_texture, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));

	odRenderState state{odTransform_identity, odTransform_identity, odBounds{0, 0, 640, 480}, &window.texture, nullptr};
	OD_ASSERT(odRenderer_draw_texture(&window.renderer, state, nullptr));
	OD_ASSERT(odRenderer_flush(&window.renderer));

	state.render_texture = &window.game_render_texture;
	OD_ASSERT(odRenderer_draw_texture(&window.renderer, state, nullptr));
	OD_ASSERT(odRenderer_flush(&window.renderer));

	state.src_texture = odRenderTexture_get_texture(&window.game_render_texture);
	OD_ASSERT(odRenderer_draw_texture(&window.renderer, state, nullptr));
	OD_ASSERT(odRenderer_flush(&window.renderer));
}
OD_TEST_FILTERED(odRenderer, init_multiple_renderers, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));

	odRenderState state{odTransform_identity, odTransform_identity, odBounds{0, 0, 640, 480}, &window.texture, nullptr};

	odRenderer renderer;
	OD_ASSERT(odRenderer_init(&renderer, window.render_context_native));
	OD_ASSERT(odRenderer_get_valid(&renderer));
	OD_ASSERT(odRenderer_get_valid(&window.renderer));

	OD_ASSERT(odRenderer_draw_vertices(&window.renderer, state, odRender_test_vertices, odRender_test_vertices_count));
	OD_ASSERT(odRenderer_flush(&window.renderer));

	OD_ASSERT(odRenderer_draw_vertices(&renderer, state, odRender_test_vertices, odRender_test_vertices_count));
	OD_ASSERT(odRenderer_flush(&renderer));

	odRenderer_destroy(&window.renderer);
	OD_ASSERT(!odRenderer_get_valid(&window.renderer));
	OD_ASSERT(odRenderer_get_valid(&renderer));
	OD_ASSERT(odRenderer_draw_vertices(&renderer, state, odRender_test_vertices, odRender_test_vertices_count));
	OD_ASSERT(odRenderer_flush(&renderer));
}
OD_TEST(odRenderer, init_without_context_fails) {
	odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
	odRenderer renderer;
	OD_ASSERT(!odRenderer_init(&renderer, nullptr));
	OD_ASSERT(!odRenderer_get_valid(&renderer));
}
OD_TEST(odRenderer, destroy_invalid) {
	odRenderer renderer;
	OD_ASSERT(!odRenderer_get_valid(&renderer));

	odRenderer_destroy(&renderer);
	OD_ASSERT(!odRenderer_get_valid(&renderer));
}
