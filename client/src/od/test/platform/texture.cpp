#include <od/platform/texture.hpp>

#include <od/platform/window.hpp>
#include <od/test/test.hpp>

OD_TEST_FILTERED(odTexture, init_destroy_with_window, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_get_open(&window));

	odTexture texture;

	OD_ASSERT(odTexture_init_blank(&texture, window.render_context_native));

	// test double init
	OD_ASSERT(odTexture_init_blank(&texture, window.render_context_native));

	odTexture_destroy(&texture);

	// test double destroy
	odTexture_destroy(&texture);
}
OD_TEST_FILTERED(odTexture, destroy_after_window_destroy, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_get_open(&window));

	{
		odLogLevelScoped suppress_logs{OD_LOG_LEVEL_NONE};
		odTexture texture_2;
		OD_ASSERT(odTexture_init_blank(&texture_2, window.render_context_native));

		odWindow_destroy(&window);
	
		odTexture_destroy(&texture_2);
	}
}
OD_TEST_FILTERED(odTexture, init_without_context_fails, OD_TEST_FILTER_SLOW) {
	odLogLevelScoped suppress_logs{OD_LOG_LEVEL_NONE};
	odTexture texture;
	OD_ASSERT(!odTexture_init_blank(&texture, nullptr));
}
OD_TEST_FILTERED(odTexture, destroy_without_window, OD_TEST_FILTER_SLOW) {
	odTexture texture;
	odTexture_destroy(&texture);
}
