#include <od/core.h>
#include <od/platform/window.hpp>

#define GLEW_STATIC
#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GL/glew.h>
#include <GL/glu.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

struct odSDLInit {
	bool is_initialized;

	odSDLInit();
	~odSDLInit();
};

static bool odSDLInit_ensure_initialized() {
	static odSDLInit sdl;

	OD_TRACE("is_initialized=%d", sdl.is_initialized);

	if (sdl.is_initialized) {
		return true;
	}

	const Uint32 sdl_init_flags = (
		SDL_INIT_EVENTS
		| SDL_INIT_TIMER
		| SDL_INIT_VIDEO
		// | SDL_INIT_JOYSTICK
		// | SDL_INIT_HAPTIC
		// | SDL_INIT_GAMECONTROLLER
	);

	OD_TRACE("SDL_Init");
	if (SDL_Init(sdl_init_flags) != 0) {
		OD_ERROR("SDL_Init() failed with error=%s", SDL_GetError());
		return false;
	}

	OD_TRACE("is_initialized=%d", sdl.is_initialized);

	return true;
}

odSDLInit::odSDLInit()
: is_initialized{false} {
}
odSDLInit::~odSDLInit() {
	if (is_initialized) {
		SDL_Quit();
	}

	is_initialized = false;
}

odWindowSettings odWindowSettings_get_defaults() {
	return odWindowSettings{
		/*caption*/ "",
		/*width*/ 640,
		/*height*/ 480,
		/*fps_limit*/ 30,
		/*is_fps_limited*/ true,
		/*is_visible*/ true,
	};
}

void odWindow_construct(odWindow* window) {
	if (window == nullptr) {
		OD_ERROR("window=nullptr");
		return;
	}

	window->window_native = nullptr;
	window->is_open = false;
	window->next_frame_ms = 0;
	window->settings = odWindowSettings_get_defaults();
}
void odWindow_destruct(odWindow* window) {
	if (window == nullptr) {
		OD_ERROR("window=nullptr");
		return;
	}

	odWindow_close(window);
}
void odWindow_swap(odWindow* window1, odWindow* window2) {
	if (window1 == nullptr) {
		OD_ERROR("window1=nullptr");
		return;
	}

	if (window2 == nullptr) {
		OD_ERROR("window2=nullptr");
		return;
	}

	void* window_native_swap = window1->window_native;
	bool is_open_swap = window1->is_open;
	uint32_t next_frame_ms_swap = window1->next_frame_ms;
	odWindowSettings settings_swap = window1->settings;

	window1->window_native = window2->window_native;
	window1->is_open = window2->is_open;
	window1->next_frame_ms = window2->next_frame_ms;
	window1->settings = window2->settings;

	window2->window_native = window_native_swap;
	window2->is_open = is_open_swap;
	window2->next_frame_ms = next_frame_ms_swap;
	window2->settings = settings_swap;
}
const char* odWindow_get_debug_string(const odWindow* window) {
	if (window == nullptr) {
		return "odWindow{this=nullptr}";
	}

	return odDebugString_create_formatted(
		"odWindow{this=%p, is_open=%s}",
		static_cast<const void*>(window),
		window->is_open ? "true" : "false"
	);
}
bool odWindow_open(odWindow* window, const odWindowSettings* settings) {
	if (window == nullptr) {
		OD_ERROR("window=nullptr");
		return false;
	}

	if (settings == nullptr) {
		OD_ERROR("settings=nullptr");
		return false;
	}

	OD_TRACE("window=%s", odWindow_get_debug_string(window));

	if (!odSDLInit_ensure_initialized()) {
		return false;
	}

	if (window->is_open) {
		odWindow_close(window);
	}

	window->window_native = static_cast<void*>(SDL_CreateWindow(
		settings->caption,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		static_cast<int>(settings->width),
		static_cast<int>(settings->height),
		settings->is_visible ? SDL_WINDOW_SHOWN : SDL_WINDOW_HIDDEN
	));

	if (window->window_native == nullptr) {
		OD_ERROR("Window creation failed");
		return false;
	}

	window->settings = *settings;
	window->is_open = true;

	return true;
}
void odWindow_close(odWindow* window) {
	OD_TRACE("Closing window");

	if (window->window_native != nullptr) {
		SDL_DestroyWindow(static_cast<SDL_Window*>(window->window_native));
	}

	window->is_open = false;
	window->window_native = nullptr;
}
bool odWindow_get_open(const odWindow* window) {
	return window->is_open;
}
bool odWindow_set_visible(odWindow* window, bool is_visible) {
	if (window->window_native == nullptr) {
		OD_ERROR("window->window_native == nullptr");
		return false;
	}

	if (window->settings.is_visible == is_visible) {
		return true;
	}

	if (is_visible) {
		SDL_ShowWindow(static_cast<SDL_Window*>(window->window_native));
	} else {
		SDL_HideWindow(static_cast<SDL_Window*>(window->window_native));
	}
	return true;
}
void odWindow_step(odWindow* window) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT: {
				OD_DEBUG("Quit event received");
				window->is_open = false;
				break;
			}
			case SDL_KEYUP: {
				switch (event.key.keysym.sym) {
					case SDLK_ESCAPE: {
						OD_DEBUG("Escape key released");
						window->is_open = false;
						break;
					}
					default: {
						break;
					}
				}
				break;
			}
		}
	}

	if (!window->settings.is_fps_limited) {
		return;
	}

	uint32_t frame_duration_ms = 1000 / window->settings.fps_limit;
	uint32_t time_ms = static_cast<uint32_t>(SDL_GetTicks());
	uint32_t wait_ms = window->next_frame_ms - time_ms;

	// guard against wait_ms underflowing
	if (time_ms > window->next_frame_ms) {
		wait_ms = 0;
		window->next_frame_ms = time_ms;
	}

	// guard against waiting more than the full frame duration
	if (wait_ms > frame_duration_ms) {
		wait_ms = frame_duration_ms;
		window->next_frame_ms = time_ms;
	}

	if (wait_ms > 0) {
		SDL_Delay(static_cast<Uint32>(wait_ms));
	}
	window->next_frame_ms += frame_duration_ms;
}

odWindow::odWindow() {
	odWindow_construct(this);
}
odWindow::odWindow(odWindow&& other)
: odWindow{} {
	odWindow_swap(this, &other);
}
odWindow& odWindow::operator=(odWindow&& other) {
	odWindow_swap(this, &other);
	return *this;
}
odWindow::~odWindow() {
	odWindow_destruct(this);
}
