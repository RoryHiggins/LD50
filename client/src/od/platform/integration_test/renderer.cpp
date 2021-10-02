#include <od/platform/window.hpp>
#include <od/platform/renderer.hpp>

#include <gtest/gtest.h>

#include <od/core/debug.hpp>

TEST(odRenderer, init_destroy_with_window) {
	odWindow window;
	ASSERT_TRUE(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	ASSERT_TRUE(odWindow_get_open(&window));

	odRenderer& renderer = window.renderer;
	odRenderer_destroy(&renderer);

	ASSERT_TRUE(odRenderer_init(&renderer, window.render_context_native));

	// test double init
	ASSERT_TRUE(odRenderer_init(&renderer, window.render_context_native));

	odRenderer_destroy(&renderer);

	// test double destroy
	odRenderer_destroy(&renderer);
}

TEST(odRenderer, destroy_after_window_destroy) {
	odWindow window;
	ASSERT_TRUE(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	ASSERT_TRUE(odWindow_get_open(&window));

	odRenderer renderer_2;
	ASSERT_TRUE(odRenderer_init(&renderer_2, window.render_context_native));

	odWindow_destroy(&window);

	{
		odLogLevelScoped suppress_logs{OD_LOG_LEVEL_NONE};
		odRenderer_destroy(&renderer_2);
	}
}

TEST(odRenderer, init_invalid_context_fails) {
	odRenderer renderer;

	{
		odLogLevelScoped suppress_logs{OD_LOG_LEVEL_NONE};
		ASSERT_FALSE(odRenderer_init(&renderer, nullptr));
	}
}

TEST(odRenderer, destroy_without_window) {
	odRenderer renderer;
	odRenderer_destroy(&renderer);
}
