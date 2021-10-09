#include <od/platform/window.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <od/core/debug.h>
#include <od/core/vertex.h>
#include <od/core/type.hpp>

static int32_t odSDL_init_counter = 0;

static bool odSDL_init_reentrant() {
	OD_TRACE("odSDL_init_counter=%d", odSDL_init_counter);

	if (odSDL_init_counter == 0) {
		OD_TRACE("SDL_Init");

		const Uint32 flags = (SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_VIDEO);
		int init_result = SDL_Init(flags);
		if (init_result != 0) {
			OD_ERROR("SDL_Init failed, init_result=%d", init_result);
			return false;
		}
	}

	odSDL_init_counter++;
	return true;
}
static void odSDL_destroy_reentrant() {
	if (odSDL_init_counter <= 0) {
		OD_WARN("odSDL_destroy_reentrant with no matching odSDL_init_reentrant");
		return;
	}

	odSDL_init_counter--;

	if (odSDL_init_counter == 0) {
		OD_TRACE("SDL_Quit");
		SDL_Quit();
	}
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
		"odWindow{this=%p, is_open=%d}",
		static_cast<const void*>(window),
		window->is_open);
}
bool odWindow_init(odWindow* window, odWindowSettings settings) {
	if (window == nullptr) {
		OD_ERROR("window=nullptr");
		return false;
	}

	OD_DEBUG("window=%s", odWindow_get_debug_string(window));

	odWindow_destroy(window);

	window->settings = settings;

	window->is_sdl_init = odSDL_init_reentrant();
	if (!window->is_sdl_init) {
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

#if OD_BUILD_DEBUG_LOG
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

#define OD_BUILD_OPENGL_FORWARD_COMPATIBLE 1
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
			| SDL_WINDOW_RESIZABLE)));

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

	OD_DEBUG("Window opened");

	return true;
}
void odWindow_destroy(odWindow* window) {
	OD_TRACE("window=%s", odWindow_get_debug_string(window));

	OD_DEBUG("Window closed");

	if (window == nullptr) {
		OD_ERROR("window=nullptr");
		return;
	}

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

	if (window->is_sdl_init) {
		odSDL_destroy_reentrant();
		window->is_sdl_init = false;
	}

	window->is_open = false;
}
bool odWindow_get_open(const odWindow* window) {
	if (window == nullptr) {
		OD_ERROR("window=nullptr");
		return false;
	}

	if (!window->is_open) {
		return false;
	}

	if (window->window_native == nullptr) {
		OD_ERROR("window->window_native=nullptr");
		return false;
	}

	return true;
}
bool odWindow_set_visible(odWindow* window, bool is_visible) {
	if (!odWindow_get_open(window)) {
		OD_ERROR("!odWindow_get_open(), window=%s", odWindow_get_debug_string(window));
		return false;
	}

	OD_TRACE("window=%s, is_visible=%d", odWindow_get_debug_string(window), is_visible);

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
bool odWindow_set_size(odWindow* window, int32_t width, int32_t height) {
	if (!odWindow_get_open(window)) {
		OD_ERROR("!odWindow_get_open(), window=%s", odWindow_get_debug_string(window));
		return false;
	}

	OD_TRACE("window=%s, width=%d, height=%d", odWindow_get_debug_string(window), width, height);

	if ((width == window->settings.width) && (height == window->settings.height)) {
		return true;
	}

	window->settings.width = width;
	window->settings.height = height;

	return true;
}
static bool odWindow_handle_event(odWindow* window, const SDL_Event *event) {
	switch (event->type) {
		case SDL_QUIT: {
			OD_DEBUG("Quit event received");
			odWindow_destroy(window);
			break;
		}
		case SDL_WINDOWEVENT: {
			switch (event->window.event) {
				case SDL_WINDOWEVENT_RESIZED:
				case SDL_WINDOWEVENT_SIZE_CHANGED: {
					int new_width = 0;
					int new_height = 0;
					SDL_GetWindowSize(static_cast<SDL_Window*>(window->window_native), &new_width, &new_height);

					OD_DEBUG("window resized, new_width=%d, new_height=%d", new_width, new_height);

					if (!odWindow_set_size(window, new_width, new_height)) {
						OD_ERROR("odWindow_set_size failed");
						odWindow_destroy(window);
					}
					break;
				}
			}
			break;
		}
		case SDL_KEYUP: {
			OD_DEBUG("SDL_KEYUP, key=%s", SDL_GetKeyName(event->key.keysym.sym));

			switch (event->key.keysym.sym) {
				case SDLK_ESCAPE: {
					OD_DEBUG("Escape key released");
					odWindow_destroy(window);
					break;
				}
				default: {
					break;
				}
			}
			break;
		}
		case SDL_KEYDOWN: {
			if (event->key.repeat == 0) {
				OD_DEBUG("SDL_KEYDOWN, key=%s", SDL_GetKeyName(event->key.keysym.sym));
			} else {
				OD_TRACE("SDL_KEYDOWN, key=%s, repeat=%d", SDL_GetKeyName(event->key.keysym.sym), static_cast<int32_t>(event->key.repeat));
			}
			break;
		}
		case SDL_CONTROLLERDEVICEADDED: {
			OD_DEBUG("SDL_CONTROLLERDEVICEADDED, index=%d", event->cdevice.which);
			break;
		}
		case SDL_CONTROLLERDEVICEREMOVED: {
			OD_DEBUG("SDL_CONTROLLERDEVICEREMOVED, index=%d", event->cdevice.which);
			break;
		}
		case SDL_CONTROLLERDEVICEREMAPPED: {
			OD_DEBUG("SDL_CONTROLLERDEVICEREMAPPED, index=%d", event->cdevice.which);
			break;
		}
		case SDL_CONTROLLERBUTTONDOWN: {
			OD_DEBUG(
				"SDL_CONTROLLERBUTTONDOWN, button=%s",
				SDL_GameControllerGetStringForButton(static_cast<SDL_GameControllerButton>(event->cbutton.button)));
			break;
		}
		case SDL_CONTROLLERBUTTONUP: {
			OD_DEBUG(
				"SDL_CONTROLLERBUTTONUP, button=%s",
				SDL_GameControllerGetStringForButton(static_cast<SDL_GameControllerButton>(event->cbutton.button)));
			break;
		}
		case SDL_CONTROLLERAXISMOTION: {
			OD_TRACE(
				"SDL_CONTROLLERAXISMOTION, axis=%s, value=%d",
				SDL_GameControllerGetStringForAxis(static_cast<SDL_GameControllerAxis>(event->caxis.axis)),
				static_cast<int>(event->caxis.value));
			break;
		}
		default: {
			OD_TRACE("unhandled event, event->type=%u", event->type);
			break;
		}
	}
	return true;
}
static bool odWindow_wait_step(odWindow* window) {
	if (!window->settings.is_fps_limit_enabled) {
		return true;
	}

	if (window->settings.is_vsync_enabled) {
		return true;
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
	return true;
}
bool odWindow_step(odWindow* window) {
	if (!odWindow_get_open(window)) {
		OD_ERROR("!window->is_open");
		return false;
	}

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (!odWindow_handle_event(window, &event)) {
			OD_ERROR("failed handling SDL event");
			return false;
		}

		if (!odWindow_get_open(window)) {
			return true;
		}
	}

	glViewport(0, 0, window->settings.width, window->settings.height);
	const odVertex test_vertices[] = {
		{-0.5f,-0.5f,0.0f,  0,0xff,0,0xff,  0.0f,0.0f},
		{-0.5f, 0.5f,0.0f,  0,0xff,0,0xff,  0.0f,0.0f},
		{ 0.5f,-0.5f,0.0f,  0,0xff,0,0xff,  0.0f,0.0f},
	};
	const int32_t test_vertices_count = 3;
	if (!odRenderer_draw(&window->renderer, test_vertices, test_vertices_count)) {
		OD_ERROR("failed drawing");
		return false;
	}
	SDL_GL_SwapWindow(static_cast<SDL_Window*>(window->window_native));

	if (!odWindow_wait_step(window)) {
		OD_ERROR("failed waiting for next step");
		return false;
	}

	return true;
}
const odWindowSettings* odWindow_get_settings(const odWindow* window) {
	if (window == nullptr) {
		OD_ERROR("window=nullptr");
		return nullptr;
	}

	return &window->settings;
}

odWindow::odWindow()
	: window_native{nullptr}, render_context_native{nullptr}, is_sdl_init{false},
	  is_open{false}, next_frame_ms{0}, settings{odWindowSettings_get_defaults()} {
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
	SDL_Quit();
}
