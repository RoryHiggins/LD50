#include <od/platform/window.hpp>

#include <gtest/gtest.h>

#include <od/core/debug.hpp>

TEST(odWindow, init_destroy) {
	odWindow window;
	ASSERT_TRUE(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	ASSERT_TRUE(odWindow_get_open(&window));

	// test double init
	ASSERT_TRUE(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	ASSERT_TRUE(odWindow_get_open(&window));

	odWindow_destroy(&window);

	// test double destroy
	odWindow_destroy(&window);
}

TEST(odWindow, step) {
	odWindow window;
	ASSERT_TRUE(odWindow_init(&window, odWindowSettings_get_headless_defaults()));

	for (int32_t i = 0; i < 10; i++) {
		ASSERT_TRUE(odWindow_step(&window));
		ASSERT_TRUE(odWindow_get_open(&window));
	}
}

TEST(odWindow, set_visible) {
	odWindow window;
	ASSERT_TRUE(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	ASSERT_TRUE(odWindow_get_open(&window));

	ASSERT_TRUE(odWindow_set_visible(&window, false));
	ASSERT_TRUE(odWindow_set_visible(&window, true));
	ASSERT_TRUE(odWindow_set_visible(&window, false));
}

TEST(odWindow, set_visible_not_open_fails) {
	odWindow window;
	ASSERT_FALSE(odWindow_get_open(&window));

	{
		odLogLevelScoped suppress_logs{OD_LOG_LEVEL_NONE};
		ASSERT_FALSE(odWindow_set_visible(&window, false));
		ASSERT_FALSE(odWindow_set_visible(&window, true));
	}

	ASSERT_FALSE(odWindow_get_open(&window));
}

TEST(odWindow, set_size) {
	odWindow window;
	ASSERT_TRUE(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	ASSERT_TRUE(odWindow_get_open(&window));

	ASSERT_TRUE(odWindow_set_size(&window, 1, 1));
	ASSERT_TRUE(odWindow_get_open(&window));

	odWindow_destroy(&window);
}

TEST(odWindow, get_open) {
	odWindow window;
	ASSERT_FALSE(odWindow_get_open(&window));

	ASSERT_TRUE(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	ASSERT_TRUE(odWindow_get_open(&window));

	odWindow_destroy(&window);
	ASSERT_FALSE(odWindow_get_open(&window));
}
