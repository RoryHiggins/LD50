#include <od/platform/texture.hpp>
#include <od/platform/renderer.hpp>

#include <od/core/array.hpp>
#include <od/core/color.h>
#include <od/platform/window.hpp>
#include <od/core/vertex.h>
#include <od/test/test.hpp>

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
	odTrivialArrayT<odColor> texture_pixels;
	OD_ASSERT(texture_pixels.set_count(width * height));

	odTexture texture;
	OD_ASSERT(odTexture_init(&texture, &window, texture_pixels.begin(), width, height));
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

OD_TEST_SUITE(
	odTestSuite_odTexture,
	odTest_odTexture_init_destroy,
	odTest_odTexture_init_large,
	odTest_odTexture_destroy_after_window_destroy_fails,
	odTest_odTexture_get_size,
	odTest_odTexture_init_without_context_fails,
	odTest_odTexture_destroy_invalid,
)
