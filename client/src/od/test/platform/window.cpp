#include <od/platform/window.hpp>

#include <cstring>

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

	// be extremely sparing with the number of tests that make a window visible
	OD_ASSERT(odWindow_set_visible(&window, false));
	OD_ASSERT(odWindow_set_visible(&window, true));
	OD_ASSERT(odWindow_set_visible(&window, false));
}
OD_TEST_FILTERED(odTest_odWindow_set_size, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_check_valid(&window));

	OD_ASSERT(odWindow_set_size(&window, 1, 1));
	OD_ASSERT(odWindow_check_valid(&window));

	odWindow_destroy(&window);
}
OD_TEST_FILTERED(odTest_odWindow_set_settings_headless, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_check_valid(&window));

	odWindowSettings settings = *odWindowSettings_get_headless_defaults();
	settings.caption = "no";
	settings.width = 44;
	settings.is_vsync_enabled = false;

	OD_ASSERT(odWindow_set_settings(&window, &settings));
	OD_ASSERT(odWindow_check_valid(&window));
	OD_ASSERT(memcmp(&settings, &window.settings, sizeof(odWindowSettings)) == 0);

	odWindow_destroy(&window);
}

OD_TEST_FILTERED(odTest_odWindow_set_settings_visible, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_check_valid(&window));

	// be extremely sparing with the number of tests that make a window visible
	OD_ASSERT(odWindow_set_visible(&window, true));

	odWindowSettings settings = *odWindowSettings_get_headless_defaults();
	settings.caption = "no";
	settings.width = 44;
	settings.is_vsync_enabled = true;
	settings.is_visible = false;

	OD_ASSERT(odWindow_set_settings(&window, &settings));
	OD_ASSERT(odWindow_check_valid(&window));
	OD_ASSERT(memcmp(&settings, &window.settings, sizeof(odWindowSettings)) == 0);

	odWindow_destroy(&window);
}
OD_TEST_FILTERED(odTest_odWindow_get_open, OD_TEST_FILTER_SLOW) {
	odWindow window;

	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_check_valid(&window));

	odWindow_destroy(&window);
}
OD_TEST_FILTERED(odTest_odWindow_get_mouse_state, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_check_valid(&window));

	odWindowMouseState state;
	odWindow_get_mouse_state(&window, &state);
}
OD_TEST_FILTERED(odTest_odWindow_get_key_state, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_check_valid(&window));

	OD_DISCARD(odWindow_get_key_state(&window, "Up"));
	OD_DISCARD(odWindow_get_key_state(&window, "Down"));
	OD_DISCARD(odWindow_get_key_state(&window, "Left"));
	OD_DISCARD(odWindow_get_key_state(&window, "Right"));
}
OD_TEST_FILTERED(odTest_odWindow_get_key_state_invalid_name_fails, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_check_valid(&window));

	{
		odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
		OD_ASSERT(!odWindow_get_key_state(&window, "Top"));
	}
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
	odTest_odWindow_set_size,
	odTest_odWindow_set_settings_headless,
	odTest_odWindow_set_settings_visible,
	odTest_odWindow_get_open,
	odTest_odWindow_get_mouse_state,
	odTest_odWindow_get_key_state,
	odTest_odWindow_get_key_state_invalid_name_fails,
	odTest_odWindow_init_multiple_windows,
	odTest_odWindow_destroy_invalid,
)
