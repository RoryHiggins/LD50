#include <od/platform/render_texture.hpp>

#include <od/platform/window.hpp>
#include <od/test/test.hpp>

OD_TEST_FILTERED(odRenderTexture, init_destroy_with_window, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_get_open(&window));

	odRenderTexture render_texture;
	OD_ASSERT(odRenderTexture_init(&render_texture, window.render_context_native, 1, 1));

	// test double init
	OD_ASSERT(odRenderTexture_init(&render_texture, window.render_context_native, 1, 1));

	odRenderTexture_destroy(&render_texture);

	// test double destroy
	odRenderTexture_destroy(&render_texture);
}
OD_TEST_FILTERED(odRenderTexture, init_large_with_window, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));

	odRenderTexture render_texture;
	OD_ASSERT(odRenderTexture_init(&render_texture, window.render_context_native, 4096, 4096));
}
OD_TEST_FILTERED(odRenderTexture, destroy_after_window_destroy, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_get_open(&window));

	{
		odLogLevelScoped suppress_logs{OD_LOG_LEVEL_NONE};
		odRenderTexture render_texture_2;
		OD_ASSERT(odRenderTexture_init(&render_texture_2, window.render_context_native, 1, 1));

		odWindow_destroy(&window);
	
		odRenderTexture_destroy(&render_texture_2);
	}
}
OD_TEST_FILTERED(odRenderTexture, init_without_context_fails, OD_TEST_FILTER_SLOW) {
	odLogLevelScoped suppress_logs{OD_LOG_LEVEL_NONE};
	odRenderTexture render_texture;
	OD_ASSERT(!odRenderTexture_init(&render_texture, nullptr, 1, 1));
}
OD_TEST_FILTERED(odRenderTexture, destroy_without_window, OD_TEST_FILTER_SLOW) {
	odRenderTexture render_texture;
	odRenderTexture_destroy(&render_texture);
}
