// TODO split out texture tests
#include <od/platform/texture.hpp>
#include <od/platform/renderer.hpp>

#include <cstring>

#include <od/core/type.hpp>
#include <od/core/array.hpp>
#include <od/core/vertex.h>
#include <od/platform/primitive.h>
#include <od/platform/window.hpp>
#include <od/test/test.hpp>

#define OD_RENDER_TEST_VERTEX_COUNT OD_TRIANGLE_VERTEX_COUNT
const odVertex odRender_test_vertices[OD_RENDER_TEST_VERTEX_COUNT] = {
	odVertex{odVector{0.0f,0.0f,0.0f,1.0f}, odColor{0x00,0xff,0x00,0xff}, 0.0f,0.0f},
	odVertex{odVector{0.0f,1.0f,0.0f,1.0f}, odColor{0x00,0xff,0x00,0xff}, 0.0f,0.0f},
	odVertex{odVector{1.0f,0.0f,0.0f,1.0f}, odColor{0x00,0xff,0x00,0xff}, 0.0f,0.0f},
};


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

	odRenderState state{*odMatrix_get_identity(), *odMatrix_get_identity(), odBounds{0, 0, 640, 480}, &texture, nullptr};

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
	odRenderState state{*odMatrix_get_identity(), *odMatrix_get_identity(), odBounds{0, 0, 640, 480}, &texture, nullptr};

	OD_ASSERT(odRenderer_draw_vertices(&renderer, &state, odRender_test_vertices, OD_RENDER_TEST_VERTEX_COUNT));
	OD_ASSERT(odRenderer_flush(&renderer));

	state.opt_render_texture = &render_texture;
	OD_ASSERT(odRenderer_draw_vertices(&renderer, &state, odRender_test_vertices, OD_RENDER_TEST_VERTEX_COUNT));
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
	odRenderState state{*odMatrix_get_identity(), *odMatrix_get_identity(), odBounds{0, 0, 640, 480}, &texture, nullptr};

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
	odRenderState state{*odMatrix_get_identity(), *odMatrix_get_identity(), odBounds{0, 0, 640, 480}, &texture, nullptr};

	odRenderer renderer2;
	OD_ASSERT(odRenderer_init(&renderer2, &window));
	OD_ASSERT(odRenderer_check_valid(&renderer2));
	OD_ASSERT(odRenderer_check_valid(&renderer2));

	OD_ASSERT(odRenderer_draw_vertices(&renderer, &state, odRender_test_vertices, OD_RENDER_TEST_VERTEX_COUNT));
	OD_ASSERT(odRenderer_flush(&renderer));

	OD_ASSERT(odRenderer_draw_vertices(&renderer2, &state, odRender_test_vertices, OD_RENDER_TEST_VERTEX_COUNT));
	OD_ASSERT(odRenderer_flush(&renderer2));

	odRenderer_destroy(&renderer2);
	OD_ASSERT(odRenderer_check_valid(&renderer));
	OD_ASSERT(odRenderer_draw_vertices(&renderer, &state, odRender_test_vertices, OD_RENDER_TEST_VERTEX_COUNT));
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
