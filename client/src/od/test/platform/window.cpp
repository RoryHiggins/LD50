#include <od/platform/window.hpp>

#include <od/test/test.hpp>

OD_TEST(odWindow, init_destroy) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_get_open(&window));

	// test double init
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_get_open(&window));

	odWindow_destroy(&window);

	// test double destroy
	odWindow_destroy(&window);
}

OD_TEST(odWindow, step) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));

	for (int32_t i = 0; i < 10; i++) {
		OD_ASSERT(odWindow_step(&window));
		OD_ASSERT(odWindow_get_open(&window));
	}
}

OD_TEST(odWindow, set_visible) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_get_open(&window));

	OD_ASSERT(odWindow_set_visible(&window, false));
	OD_ASSERT(odWindow_set_visible(&window, true));
	OD_ASSERT(odWindow_set_visible(&window, false));
}

OD_TEST(odWindow, set_visible_not_open_fails) {
	odWindow window;
	OD_ASSERT(!odWindow_get_open(&window));

	{
		odLogLevelScoped suppress_logs{OD_LOG_LEVEL_NONE};
		OD_ASSERT(!odWindow_set_visible(&window, false));
		OD_ASSERT(!odWindow_set_visible(&window, true));
	}

	OD_ASSERT(!odWindow_get_open(&window));
}

OD_TEST(odWindow, set_size) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_get_open(&window));

	OD_ASSERT(odWindow_set_size(&window, 1, 1));
	OD_ASSERT(odWindow_get_open(&window));

	odWindow_destroy(&window);
}

OD_TEST(odWindow, get_open) {
	odWindow window;
	OD_ASSERT(!odWindow_get_open(&window));

	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_get_open(&window));

	odWindow_destroy(&window);
	OD_ASSERT(!odWindow_get_open(&window));
}
