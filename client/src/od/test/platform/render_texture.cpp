#include <od/platform/render_texture.hpp>
#include <od/platform/renderer.hpp>

#include <od/core/array.hpp>
#include <od/core/color.h>
#include <od/platform/window.hpp>
#include <od/core/vertex.h>
#include <od/test/test.hpp>

OD_TEST_FILTERED(odTest_odRenderTexture_init_destroy, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_check_valid(&window));

	odRenderTexture render_texture;
	OD_ASSERT(odRenderTexture_init(&render_texture, &window, 1, 1));
	OD_ASSERT(odRenderTexture_check_valid(&render_texture));

	// test double init
	OD_ASSERT(odRenderTexture_init(&render_texture, &window, 1, 1));
	OD_ASSERT(odRenderTexture_check_valid(&render_texture));

	odRenderTexture_destroy(&render_texture);

	// test double destroy
	odRenderTexture_destroy(&render_texture);

	// test reuse
	OD_ASSERT(odRenderTexture_init(&render_texture, &window, 1, 1));
	OD_ASSERT(odRenderTexture_check_valid(&render_texture));
	odRenderTexture_destroy(&render_texture);
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

OD_TEST_SUITE(
	odTestSuite_odRenderTexture,
	odTest_odRenderTexture_init_destroy,
	odTest_odRenderTexture_init_large,
	odTest_odRenderTexture_destroy_after_window_destroy_fails,
	odTest_odRenderTexture_init_without_context_fails,
	odTest_odRenderTexture_destroy_invalid,
)
