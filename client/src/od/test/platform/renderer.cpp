#include <od/platform/renderer.hpp>

#include <od/platform/window.hpp>
#include <od/test/test.hpp>

OD_TEST_FILTERED(odRenderer, init_destroy_with_window, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_get_open(&window));

	odRenderer renderer;

	OD_ASSERT(odRenderer_init(&renderer, window.render_context_native));

	// test double init
	OD_ASSERT(odRenderer_init(&renderer, window.render_context_native));

	odRenderer_destroy(&renderer);

	// test double destroy
	odRenderer_destroy(&renderer);
}
OD_TEST_FILTERED(odRenderer, destroy_after_window_destroy, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_get_open(&window));

	{
		odLogLevelScoped suppress_logs{OD_LOG_LEVEL_NONE};
		odRenderer renderer_2;
		OD_ASSERT(odRenderer_init(&renderer_2, window.render_context_native));

		odWindow_destroy(&window);
	
		odRenderer_destroy(&renderer_2);
	}
}
OD_TEST_FILTERED(odRenderer, init_without_context_fails, OD_TEST_FILTER_SLOW) {
	odLogLevelScoped suppress_logs{OD_LOG_LEVEL_NONE};
	odRenderer renderer;
	OD_ASSERT(!odRenderer_init(&renderer, nullptr));
}
OD_TEST_FILTERED(odRenderer, destroy_without_window, OD_TEST_FILTER_SLOW) {
	odRenderer renderer;
	odRenderer_destroy(&renderer);
}
