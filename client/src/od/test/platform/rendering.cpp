#include <od/platform/rendering.hpp>

#include <cstring>

#include <od/core/type.hpp>
#include <od/core/array.hpp>
#include <od/platform/window.hpp>
#include <od/platform/vertex.h>
#include <od/test/test.hpp>

const int32_t odRender_test_vertices_count = 3;
	const odVertex odRender_test_vertices[odRender_test_vertices_count] = {
		{{0.0f,0.0f,0.0f,1.0f}, {0x00,0xff,0x00,0xff}, 0.0f,0.0f},
		{{0.0f,1.0f,0.0f,1.0f}, {0x00,0xff,0x00,0xff}, 0.0f,0.0f},
		{{1.0f,0.0f,0.0f,1.0f}, {0x00,0xff,0x00,0xff}, 0.0f,0.0f},
	};


OD_TEST_FILTERED(odTest_odTexture_init_destroy, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_check_valid(&window));

	odTexture texture;
	OD_ASSERT(odTexture_init_blank(&texture, &window));
	OD_ASSERT(odTexture_check_valid(&texture));

	// test double init
	OD_ASSERT(odTexture_init_blank(&texture, &window));
	OD_ASSERT(odTexture_check_valid(&texture));

	odTexture_destroy(&texture);

	// test double destroy
	odTexture_destroy(&texture);

	// test reuse
	OD_ASSERT(odTexture_init_blank(&texture, &window));
	OD_ASSERT(odTexture_check_valid(&texture));
	odTexture_destroy(&texture);
}
OD_TEST_FILTERED(odTest_odTexture_init_large, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_check_valid(&window));

	const int32_t width = 256;
	const int32_t height = 256;
	odArray texture_pixels{odType_get<odColor>()};
	OD_ASSERT(odArray_set_count(&texture_pixels, width * height));
	memset(odArray_get(&texture_pixels, 0), 0, width * height);

	odTexture texture;
	OD_ASSERT(odTexture_init(&texture, &window, static_cast<odColor*>(odArray_get(&texture_pixels, 0)), width, height));
	OD_ASSERT(odTexture_check_valid(&texture));
}
OD_TEST_FILTERED(odTest_odTexture_destroy_after_window_destroy_fails, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_check_valid(&window));

	{
		odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
		odTexture texture;
		OD_ASSERT(odTexture_init_blank(&texture, &window));
		OD_ASSERT(odTexture_check_valid(&texture));

		odWindow_destroy(&window);
		odTexture_destroy(&texture);
	}
}
OD_TEST_FILTERED(odTest_odTexture_get_size, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_check_valid(&window));

	odTexture texture;

	const int32_t width = 4;
	const int32_t height = 4;
	odColor texture_pixels[width * height]{};
	OD_ASSERT(odTexture_init(&texture, &window, texture_pixels, width, height));
	OD_ASSERT(odTexture_check_valid(&texture));

	// test double init
	int32_t reported_width = -1;
	int32_t reported_height = -1;
	OD_ASSERT(odTexture_get_size(&texture, &reported_width, &reported_height));
	OD_ASSERT(reported_width == width);
	OD_ASSERT(reported_height == height);
}
OD_TEST(odTest_odTexture_init_without_context_fails) {
	odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
	odTexture texture;
	OD_ASSERT(!odTexture_init_blank(&texture, nullptr));
}
OD_TEST(odTest_odTexture_destroy_invalid) {
	odTexture texture;
	odTexture_destroy(&texture);
}

OD_TEST_FILTERED(odTest_odRenderTexture_init_destroy, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_check_valid(&window));

	odRenderTexture render_texture;
	OD_ASSERT(odRenderTexture_init(&render_texture, &window, 1, 1));
	OD_ASSERT(odRenderTexture_check_valid(&render_texture));

	// // test double init
	// OD_ASSERT(odRenderTexture_init(&render_texture, &window, 1, 1));
	// OD_ASSERT(odRenderTexture_check_valid(&render_texture));

	// odRenderTexture_destroy(&render_texture);

	// // test double destroy
	// odRenderTexture_destroy(&render_texture);

	// // test reuse
	// OD_ASSERT(odRenderTexture_init(&render_texture, &window, 1, 1));
	// OD_ASSERT(odRenderTexture_check_valid(&render_texture));
	// odRenderTexture_destroy(&render_texture);
}
OD_TEST_FILTERED(odTest_odRenderTexture_init_large, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_check_valid(&window));

	odRenderTexture render_texture;
	OD_ASSERT(odRenderTexture_init(&render_texture, &window, 4096, 4096));
}
OD_TEST_FILTERED(odTest_odRenderTexture_destroy_after_window_destroy_fails, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_check_valid(&window));

	{
		odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
		odRenderTexture render_texture;
		OD_ASSERT(odRenderTexture_init(&render_texture, &window, 1, 1));
		OD_ASSERT(odRenderTexture_check_valid(&render_texture));

		odWindow_destroy(&window);
		odRenderTexture_destroy(&render_texture);
	}
}
OD_TEST(odTest_odRenderTexture_init_without_context_fails) {
	odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
	odRenderTexture render_texture;
	OD_ASSERT(!odRenderTexture_init(&render_texture, nullptr, 1, 1));
}
OD_TEST(odTest_odRenderTexture_destroy_invalid) {
	odRenderTexture render_texture;
	odRenderTexture_destroy(&render_texture);
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

	odRenderState state{*odMatrix4_get_identity(), *odMatrix4_get_identity(), odBounds{0.0f, 0.0f, 640.0f, 480.0f}, &texture, nullptr};

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
	odRenderState state{*odMatrix4_get_identity(), *odMatrix4_get_identity(), odBounds{0.0f, 0.0f, 640.0f, 480.0f}, &texture, nullptr};

	OD_ASSERT(odRenderer_draw_vertices(&renderer, &state, odRender_test_vertices, odRender_test_vertices_count));
	OD_ASSERT(odRenderer_flush(&renderer));

	state.opt_render_texture = &render_texture;
	OD_ASSERT(odRenderer_draw_vertices(&renderer, &state, odRender_test_vertices, odRender_test_vertices_count));
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
	odRenderState state{*odMatrix4_get_identity(), *odMatrix4_get_identity(), odBounds{0.0f, 0.0f, 640.0f, 480.0f}, &texture, nullptr};

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
	odRenderState state{*odMatrix4_get_identity(), *odMatrix4_get_identity(), odBounds{0.0f, 0.0f, 640.0f, 480.0f}, &texture, nullptr};

	odRenderer renderer2;
	OD_ASSERT(odRenderer_init(&renderer2, &window));
	OD_ASSERT(odRenderer_check_valid(&renderer2));
	OD_ASSERT(odRenderer_check_valid(&renderer2));

	OD_ASSERT(odRenderer_draw_vertices(&renderer, &state, odRender_test_vertices, odRender_test_vertices_count));
	OD_ASSERT(odRenderer_flush(&renderer));

	OD_ASSERT(odRenderer_draw_vertices(&renderer2, &state, odRender_test_vertices, odRender_test_vertices_count));
	OD_ASSERT(odRenderer_flush(&renderer2));

	odRenderer_destroy(&renderer2);
	OD_ASSERT(odRenderer_check_valid(&renderer));
	OD_ASSERT(odRenderer_draw_vertices(&renderer, &state, odRender_test_vertices, odRender_test_vertices_count));
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
	odTestSuite_odRendering,
	odTest_odTexture_init_destroy,
	odTest_odTexture_init_large,
	odTest_odTexture_destroy_after_window_destroy_fails,
	odTest_odTexture_get_size,
	odTest_odTexture_init_without_context_fails,
	odTest_odTexture_destroy_invalid,
	odTest_odRenderTexture_init_destroy,
	odTest_odRenderTexture_init_large,
	odTest_odRenderTexture_destroy_after_window_destroy_fails,
	odTest_odRenderTexture_init_without_context_fails,
	odTest_odRenderTexture_destroy_invalid,
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
