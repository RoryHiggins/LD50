#include <od/platform/texture.hpp>

#include <cstring>

#include <od/core/type.hpp>
#include <od/core/array.hpp>
#include <od/core/primitive.h>
#include <od/platform/window.hpp>
#include <od/test/test.hpp>

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
	odTexture_get_size(&texture, &reported_width, &reported_height);
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
