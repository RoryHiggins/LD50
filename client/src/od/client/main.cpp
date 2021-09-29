#include <od/platform/window.hpp>

int main(int, char**) {
	odWindow window;
	odWindowSettings window_settings{odWindowSettings_get_defaults()};
	if (!odWindow_open(&window, &window_settings)) {
		return 1;
	}

	while (odWindow_get_open(&window)) {
		odWindow_step(&window);
	}

	return 0;
}
