#include <od/platform/rendering.hpp>

#include <cstring>

#include <od/core/primitive.h>
#include <od/core/container.hpp>
#include <od/platform/window.hpp>
#include <od/test/test.hpp>

const int32_t odRender_test_vertices_count = 3;
	const odVertex odRender_test_vertices[odRender_test_vertices_count] = {
		{{0.0f,0.0f,0.0f,1.0f}, {0x00,0xff,0x00,0xff}, 0.0f,0.0f},
		{{0.0f,1.0f,0.0f,1.0f}, {0x00,0xff,0x00,0xff}, 0.0f,0.0f},
		{{1.0f,0.0f,0.0f,1.0f}, {0x00,0xff,0x00,0xff}, 0.0f,0.0f},
	};


OD_TEST_FILTERED(odTexture, init_destroy, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_get_valid(&window));

	odTexture texture;
	OD_ASSERT(!odTexture_get_valid(&texture));
	OD_ASSERT(odTexture_init_blank(&texture, window.render_context_native));
	OD_ASSERT(odTexture_get_valid(&texture));

	// test double init
	OD_ASSERT(odTexture_init_blank(&texture, window.render_context_native));
	OD_ASSERT(odTexture_get_valid(&texture));

	odTexture_destroy(&texture);
	OD_ASSERT(!odTexture_get_valid(&texture));

	// test double destroy
	odTexture_destroy(&texture);
	OD_ASSERT(!odTexture_get_valid(&texture));

	// test reuse
	OD_ASSERT(odTexture_init_blank(&texture, window.render_context_native));
	OD_ASSERT(odTexture_get_valid(&texture));
	odTexture_destroy(&texture);
	OD_ASSERT(!odTexture_get_valid(&texture));
}
OD_TEST_FILTERED(odTexture, init_large, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_get_valid(&window));

	const int32_t width = 256;
	const int32_t height = 256;
	odArray texture_pixels{odType_get<odColor>()};
	OD_ASSERT(odArray_set_count(&texture_pixels, width * height));
	memset(odArray_get(&texture_pixels, 0), 0, width * height);

	odTexture texture;
	OD_ASSERT(!odTexture_get_valid(&texture));
	OD_ASSERT(odTexture_init(&texture, window.render_context_native, static_cast<odColor*>(odArray_get(&texture_pixels, 0)), width, height));
	OD_ASSERT(odTexture_get_valid(&texture));
}
OD_TEST_FILTERED(odTexture, destroy_after_window_destroy_fails, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_get_valid(&window));

	{
		odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
		odTexture texture;
		OD_ASSERT(odTexture_init_blank(&texture, window.render_context_native));
		OD_ASSERT(odTexture_get_valid(&texture));

		odWindow_destroy(&window);
		odTexture_destroy(&texture);
		OD_ASSERT(!odTexture_get_valid(&texture));
	}
}
OD_TEST_FILTERED(odTexture, get_size, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_get_valid(&window));

	odTexture texture;
	OD_ASSERT(!odTexture_get_valid(&texture));

	const int32_t width = 4;
	const int32_t height = 4;
	odColor texture_pixels[width * height]{};
	OD_ASSERT(odTexture_init(&texture, window.render_context_native, texture_pixels, width, height));
	OD_ASSERT(odTexture_get_valid(&texture));

	// test double init
	int32_t reported_width = -1;
	int32_t reported_height = -1;
	OD_ASSERT(odTexture_get_size(&texture, &reported_width, &reported_height));
	OD_ASSERT(reported_width == width);
	OD_ASSERT(reported_height == height);
}
OD_TEST(odTexture, init_without_context_fails) {
	odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
	odTexture texture;
	OD_ASSERT(!odTexture_init_blank(&texture, nullptr));
	OD_ASSERT(!odTexture_get_valid(&texture));
}
OD_TEST(odTexture, destroy_invalid) {
	odTexture texture;
	OD_ASSERT(!odTexture_get_valid(&texture));
	odTexture_destroy(&texture);
	OD_ASSERT(!odTexture_get_valid(&texture));
}


OD_TEST_FILTERED(odRenderTexture, init_destroy, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_get_valid(&window));

	odRenderTexture render_texture;
	OD_ASSERT(!odRenderTexture_get_valid(&render_texture));
	OD_ASSERT(odRenderTexture_init(&render_texture, window.render_context_native, 1, 1));
	OD_ASSERT(odRenderTexture_get_valid(&render_texture));

	// test double init
	OD_ASSERT(odRenderTexture_init(&render_texture, window.render_context_native, 1, 1));
	OD_ASSERT(odRenderTexture_get_valid(&render_texture));

	odRenderTexture_destroy(&render_texture);
	OD_ASSERT(!odRenderTexture_get_valid(&render_texture));

	// test double destroy
	odRenderTexture_destroy(&render_texture);
	OD_ASSERT(!odRenderTexture_get_valid(&render_texture));

	// test reuse
	OD_ASSERT(odRenderTexture_init(&render_texture, window.render_context_native, 1, 1));
	OD_ASSERT(odRenderTexture_get_valid(&render_texture));
	odRenderTexture_destroy(&render_texture);
	OD_ASSERT(!odRenderTexture_get_valid(&render_texture));
}
OD_TEST_FILTERED(odRenderTexture, init_large, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_get_valid(&window));

	odRenderTexture render_texture;
	OD_ASSERT(odRenderTexture_init(&render_texture, window.render_context_native, 4096, 4096));
}
OD_TEST_FILTERED(odRenderTexture, destroy_after_window_destroy_fails, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_get_valid(&window));

	{
		odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
		odRenderTexture render_texture;
		OD_ASSERT(odRenderTexture_init(&render_texture, window.render_context_native, 1, 1));
		OD_ASSERT(odRenderTexture_get_valid(&render_texture));

		odWindow_destroy(&window);
		odRenderTexture_destroy(&render_texture);
		OD_ASSERT(!odRenderTexture_get_valid(&render_texture));
	}
}
OD_TEST(odRenderTexture, init_without_context_fails) {
	odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
	odRenderTexture render_texture;
	OD_ASSERT(!odRenderTexture_init(&render_texture, nullptr, 1, 1));
	OD_ASSERT(!odRenderTexture_get_valid(&render_texture));
}
OD_TEST(odRenderTexture, destroy_invalid) {
	odRenderTexture render_texture;
	OD_ASSERT(!odRenderTexture_get_valid(&render_texture));
	odRenderTexture_destroy(&render_texture);
	OD_ASSERT(!odRenderTexture_get_valid(&render_texture));
}


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

	odRenderState state{odTransform_identity, odTransform_identity, odBounds{0.0f, 0.0f, 640.0f, 480.0f}, &window.texture, nullptr};

	OD_ASSERT(odRenderer_clear(&window.renderer, &state, odColor_white));
	OD_ASSERT(odRenderer_flush(&window.renderer));

	OD_ASSERT(odRenderer_clear(&window.renderer, &state, odColor_white));
	OD_ASSERT(odRenderer_flush(&window.renderer));
}
OD_TEST_FILTERED(odRenderer, draw_vertices, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));

	odRenderState state{odTransform_identity, odTransform_identity, odBounds{0.0f, 0.0f, 640.0f, 480.0f}, &window.texture, nullptr};
	OD_ASSERT(odRenderer_draw_vertices(&window.renderer, &state, odRender_test_vertices, odRender_test_vertices_count));
	OD_ASSERT(odRenderer_flush(&window.renderer));

	state.opt_render_texture = &window.game_render_texture;
	OD_ASSERT(odRenderer_draw_vertices(&window.renderer, &state, odRender_test_vertices, odRender_test_vertices_count));
	OD_ASSERT(odRenderer_flush(&window.renderer));
}
OD_TEST_FILTERED(odRenderer, draw_texture, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));

	odRenderState state{odTransform_identity, odTransform_identity, odBounds{0.0f, 0.0f, 640.0f, 480.0f}, &window.texture, nullptr};
	OD_ASSERT(odRenderer_draw_texture(&window.renderer, &state, nullptr));
	OD_ASSERT(odRenderer_flush(&window.renderer));

	state.opt_render_texture = &window.game_render_texture;
	OD_ASSERT(odRenderer_draw_texture(&window.renderer, &state, nullptr));
	OD_ASSERT(odRenderer_flush(&window.renderer));

	state.src_texture = odRenderTexture_get_texture(&window.game_render_texture);
	OD_ASSERT(odRenderer_draw_texture(&window.renderer, &state, nullptr));
	OD_ASSERT(odRenderer_flush(&window.renderer));
}
OD_TEST_FILTERED(odRenderer, init_multiple_renderers, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));

	odRenderState state{odTransform_identity, odTransform_identity, odBounds{0.0f, 0.0f, 640.0f, 480.0f}, &window.texture, nullptr};

	odRenderer renderer;
	OD_ASSERT(odRenderer_init(&renderer, window.render_context_native));
	OD_ASSERT(odRenderer_get_valid(&renderer));
	OD_ASSERT(odRenderer_get_valid(&window.renderer));

	OD_ASSERT(odRenderer_draw_vertices(&window.renderer, &state, odRender_test_vertices, odRender_test_vertices_count));
	OD_ASSERT(odRenderer_flush(&window.renderer));

	OD_ASSERT(odRenderer_draw_vertices(&renderer, &state, odRender_test_vertices, odRender_test_vertices_count));
	OD_ASSERT(odRenderer_flush(&renderer));

	odRenderer_destroy(&window.renderer);
	OD_ASSERT(!odRenderer_get_valid(&window.renderer));
	OD_ASSERT(odRenderer_get_valid(&renderer));
	OD_ASSERT(odRenderer_draw_vertices(&renderer, &state, odRender_test_vertices, odRender_test_vertices_count));
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
