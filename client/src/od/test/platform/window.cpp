#include <od/platform/window.hpp>

#include <od/test/test.hpp>

OD_TEST_FILTERED(odTest_odWindow_init_destroy, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_check_valid(&window));

	// test double init
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_check_valid(&window));

	odWindow_destroy(&window);

	// test double destroy
	odWindow_destroy(&window);
}
OD_TEST_FILTERED(odTest_odWindow_step, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));

	for (int32_t i = 0; i < 10; i++) {
		OD_ASSERT(odWindow_step(&window));
		OD_ASSERT(odWindow_check_valid(&window));
	}
}
OD_TEST_FILTERED(odTest_odWindow_set_visible, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_check_valid(&window));

	OD_ASSERT(odWindow_set_visible(&window, false));
	OD_ASSERT(odWindow_set_visible(&window, true));
	OD_ASSERT(odWindow_set_visible(&window, false));
}
OD_TEST_FILTERED(odTest_odWindow_set_visible_not_open_fails, OD_TEST_FILTER_SLOW) {
	odWindow window;

	{
		odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
		OD_ASSERT(!odWindow_set_visible(&window, false));
		OD_ASSERT(!odWindow_set_visible(&window, true));
	}

}
OD_TEST_FILTERED(odTest_odWindow_set_size, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_check_valid(&window));

	OD_ASSERT(odWindow_set_size(&window, 1, 1));
	OD_ASSERT(odWindow_check_valid(&window));

	odWindow_destroy(&window);
}
OD_TEST_FILTERED(odTest_odWindow_get_open, OD_TEST_FILTER_SLOW) {
	odWindow window;

	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_check_valid(&window));

	odWindow_destroy(&window);
}
OD_TEST_FILTERED(odTest_odWindow_init_multiple_windows, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_check_valid(&window));

	odWindow window2;
	OD_ASSERT(odWindow_init(&window2, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_check_valid(&window2));

	odWindow_destroy(&window);
	OD_ASSERT(odWindow_check_valid(&window2));

	odWindow_destroy(&window2);
}
OD_TEST(odTest_odWindow_destroy_invalid) {
	odWindow window;
	odWindow_destroy(&window);
}

OD_TEST_SUITE(
	odTestSuite_odWindow,
	odTest_odWindow_init_destroy,
	odTest_odWindow_step,
	odTest_odWindow_set_visible,
	odTest_odWindow_set_visible_not_open_fails,
	odTest_odWindow_set_size,
	odTest_odWindow_get_open,
	odTest_odWindow_init_multiple_windows,
	odTest_odWindow_destroy_invalid,
)
