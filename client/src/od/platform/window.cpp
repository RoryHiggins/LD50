#include <od/platform/window.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <od/core/debug.h>
#include <od/core/type.hpp>

struct odSDLInit {
	bool is_initialized;

	odSDLInit();
	~odSDLInit();
};

odSDLInit::odSDLInit() : is_initialized{false} {
}
odSDLInit::~odSDLInit() {
	OD_TRACE("is_initialized=%d", is_initialized);

	if (is_initialized) {
		OD_TRACE("performing SDL teardown");

		SDL_Quit();
	}

	is_initialized = false;
}

odWindowSettings odWindowSettings_get_defaults() {
	return odWindowSettings{
		/*caption*/ "",
		/*width*/ 640,
		/*height*/ 480,
		/*fps_limit*/ 60,
		/*is_fps_limit_enabled*/ true,
		/*is_vsync_enabled"*/ true,
		/*is_visible*/ true,
	};
}
odWindowSettings odWindowSettings_get_headless_defaults() {
	odWindowSettings headless_defaults{odWindowSettings_get_defaults()};
	headless_defaults.is_visible = false;
	headless_defaults.is_fps_limit_enabled = false;
	headless_defaults.is_vsync_enabled = false;
	return headless_defaults;
}

static bool odWindow_sdl_ensure_initialized() {
	static odSDLInit odSDL;

	OD_TRACE("is_initialized=%d", odSDL.is_initialized);

	if (odSDL.is_initialized) {
		return true;
	}

	const Uint32 sdl_init_flags = (SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_VIDEO);

	OD_TRACE("SDL_Init");
	if (SDL_Init(sdl_init_flags) != 0) {
		OD_ERROR("SDL_Init() failed with error=%s", SDL_GetError());
		return false;
	}

	odSDL.is_initialized = true;

	OD_TRACE("is_initialized=%d", odSDL.is_initialized);

	return true;
}
const odType* odWindow_get_type_constructor() {
	return odType_get<odWindow>();
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
	int32_t next_frame_ms_swap = window1->next_frame_ms;
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

	return odDebugString_format(
		"odWindow{this=%p, is_open=%s}",
		static_cast<const void*>(window),
		window->is_open ? "true" : "false");
}
bool odWindow_init(odWindow* window, odWindowSettings settings) {
	if (window == nullptr) {
		OD_ERROR("window=nullptr");
		return false;
	}

	OD_TRACE("window=%s", odWindow_get_debug_string(window));

	odWindow_destroy(window);

	window->settings = settings;

	if (!odWindow_sdl_ensure_initialized()) {
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

#if OD_BUILD_DEBUG_LOG
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

#if defined(OD_BUILD_OPENGL_FORWARD_COMPATIBLE) && OD_BUILD_OPENGL_FORWARD_COMPATIBLE
	// Use OpenGL 2.1 code with OpenGL 3.2 compatible context, for RenderDoc support
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif

	window->window_native = static_cast<void*>(SDL_CreateWindow(
		window->settings.caption,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		static_cast<int>(window->settings.width),
		static_cast<int>(window->settings.height),
		static_cast<SDL_WindowFlags>(
			(window->settings.is_visible ? SDL_WINDOW_SHOWN : SDL_WINDOW_HIDDEN)
			| SDL_WINDOW_OPENGL
			/*| SDL_WINDOW_RESIZABLE*/)));

	if (window->window_native == nullptr) {
		OD_ERROR("SDL_CreateWindow failed, error=%s", SDL_GetError());
		return false;
	}

	window->render_context_native = static_cast<void*>(
		SDL_GL_CreateContext(static_cast<SDL_Window*>(window->window_native))
	);
	if (window->render_context_native == nullptr) {
		OD_ERROR("SDL_GL_CreateContext failed, error=%s", SDL_GetError());
		return false;
	}

	if (SDL_GL_MakeCurrent(
		static_cast<SDL_Window*>(window->window_native),
		static_cast<SDL_GLContext*>(window->render_context_native))
		!= 0) {
		OD_ERROR("SDL_GL_MakeCurrent failed, error=%s", SDL_GetError());
		return false;
	}

	if (window->settings.is_vsync_enabled) {
		if (SDL_GL_SetSwapInterval(1) < 0) {
			OD_ERROR("SDL_GL_SetSwapInterval failed, error=%s; disabling vsync", SDL_GetError());
			// Lack of vsync support is not fatal; we have a frame timer as backup
			window->settings.is_vsync_enabled = false;
		}
	}

	if (!odRenderer_init(&window->renderer, window->render_context_native)) {
		return false;
	}

	window->is_open = true;

	return true;
}
void odWindow_destroy(odWindow* window) {
	if (window == nullptr) {
		OD_ERROR("window=nullptr");
		return;
	}

	OD_TRACE("window=%s", odWindow_get_debug_string(window));

	OD_TRACE("Destroying renderer");

	odRenderer_destroy(&window->renderer);

	if (window->render_context_native != nullptr) {
		OD_TRACE("Destroying render context");

		SDL_GL_DeleteContext(static_cast<SDL_GLContext*>(window->render_context_native));
		window->render_context_native = nullptr;
	}

	if (window->window_native != nullptr) {
		OD_TRACE("Destroying window");

		SDL_DestroyWindow(static_cast<SDL_Window*>(window->window_native));
		window->window_native = nullptr;
	}

	window->is_open = false;
}
bool odWindow_get_open(const odWindow* window) {
	if (window == nullptr) {
		OD_ERROR("window=nullptr");
		return false;
	}

	return window->is_open;
}
bool odWindow_set_visible(odWindow* window, bool is_visible) {
	if (window == nullptr) {
		OD_ERROR("window=nullptr");
		return false;
	}

	if (!window->is_open) {
		OD_ERROR("!window->is_open");
		return false;
	}

	if (window->window_native == nullptr) {
		OD_ERROR("window->window_native=nullptr");
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

	window->settings.is_visible = is_visible;

	return true;
}
void odWindow_step(odWindow* window) {
	if (window == nullptr) {
		OD_ERROR("window=nullptr");
		return;
	}

	if (!window->is_open) {
		OD_ERROR("!window->is_open");
		return;
	}

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

	if (!window->settings.is_fps_limit_enabled) {
		return;
	}

	if (window->settings.is_vsync_enabled) {
		return;
	}

	int32_t frame_duration_ms = 1000 / window->settings.fps_limit;
	int32_t time_ms = static_cast<int32_t>(SDL_GetTicks());
	int32_t wait_ms = window->next_frame_ms - time_ms;

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

	if (wait_ms >= 1) {
		SDL_Delay(static_cast<Uint32>(wait_ms));
	}
	window->next_frame_ms += frame_duration_ms;
}
const odWindowSettings* odWindow_get_settings(const odWindow* window) {
	if (window == nullptr) {
		OD_ERROR("window=nullptr");
		return nullptr;
	}

	return &window->settings;
}
void* odWindow_get_native_window(odWindow* window) {
	if (window == nullptr) {
		OD_ERROR("window=nullptr");
		return nullptr;
	}

	return window->window_native;
}

odWindow::odWindow()
	: window_native{nullptr}, is_open{false}, next_frame_ms{0}, settings{odWindowSettings_get_defaults()} {
}
odWindow::odWindow(odWindow&& other) : odWindow{} {
	odWindow_swap(this, &other);
}
odWindow& odWindow::operator=(odWindow&& other) {
	odWindow_swap(this, &other);
	return *this;
}
odWindow::~odWindow() {
	odWindow_destroy(this);
}
