#include <od/test.hpp>
#include <od/platform/window.hpp>

static odWindowSettings odWindowSettings_get_test_defaults() {
	odWindowSettings test_defaults = odWindowSettings_get_defaults();
	test_defaults.is_visible = false;
	test_defaults.is_fps_limited = false;
	return test_defaults;
}

TEST(odWindow, open) {
	odWindowSettings window_settings{odWindowSettings_get_test_defaults()};
	odWindow window;
	ASSERT_TRUE(odWindow_open(&window, &window_settings));
	ASSERT_TRUE(odWindow_get_open(&window));

	for (uint32_t i = 0; i < 10; i++) {
		odWindow_step(&window);
		ASSERT_TRUE(odWindow_get_open(&window));
	}

	odWindow_close(&window);
	ASSERT_FALSE(odWindow_get_open(&window));
}
