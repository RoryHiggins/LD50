#include <od/platform/window.hpp>

#include <SDL2/SDL.h>

#include <od/core/debug.h>
#include <od/core/primitive.h>
#include <od/core/container.hpp>
#include <od/platform/rendering.h>

static int32_t odSDL_init_counter = 0;

OD_NO_DISCARD static bool odSDL_init_reentrant() {
	OD_DEBUG("odSDL_init_counter=%d", odSDL_init_counter);

	if (odSDL_init_counter == 0) {
		const Uint32 flags = (SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_VIDEO);
		int init_result = SDL_Init(flags);
		if (!OD_CHECK(init_result == 0)) {
			OD_ERROR("SDL_Init failed, init_result=%d", init_result);
			return false;
		}
	}

	odSDL_init_counter++;
	return true;
}
static void odSDL_destroy_reentrant() {
	OD_DEBUG("odSDL_init_counter=%d", odSDL_init_counter);

	if (odSDL_init_counter <= 0) {
		OD_WARN("odSDL_destroy_reentrant with no matching odSDL_init_reentrant");
		return;
	}

	odSDL_init_counter--;

	if (odSDL_init_counter == 0) {
		SDL_Quit();
	}
}

odWindowSettings odWindowSettings_get_defaults() {
	return odWindowSettings{
		/*caption*/ "",
		/*window_width*/ 640,
		/*window_height*/ 480,
		/*game_width*/ 160,
		/*game_height*/ 120,
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
	if (!OD_DEBUG_CHECK(window1 != nullptr)
		|| !OD_DEBUG_CHECK(window2 != nullptr)) {
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
	OD_DEBUG("window=%s", odWindow_get_debug_string(window));

	if (!OD_DEBUG_CHECK(window != nullptr)) {
		return false;
	}

	odWindow_destroy(window);

	window->settings = settings;

	window->is_sdl_init = odSDL_init_reentrant();
	if (!window->is_sdl_init) {
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

#if OD_BUILD_EMSCRIPTEN
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else  // !OD_BUILD_EMSCRIPTEN
	// Natively use OpenGL 2.0 code with OpenGL 3.2 compatible context, for RenderDoc support
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	if (odLogLevel_get_max() >= OD_LOG_LEVEL_ERROR) {
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	}
#endif  // !OD_BUILD_EMSCRIPTEN

	window->window_native = static_cast<void*>(SDL_CreateWindow(
		window->settings.caption,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		static_cast<int>(window->settings.window_width),
		static_cast<int>(window->settings.window_height),
		static_cast<SDL_WindowFlags>(
			(window->settings.is_visible ? SDL_WINDOW_SHOWN : SDL_WINDOW_HIDDEN)
			| SDL_WINDOW_OPENGL
			| SDL_WINDOW_RESIZABLE)));

	if (!OD_CHECK(window->window_native != nullptr)) {
		OD_ERROR("SDL_CreateWindow failed, error=%s", SDL_GetError());
		return false;
	}

	window->render_context_native = static_cast<void*>(
		SDL_GL_CreateContext(static_cast<SDL_Window*>(window->window_native))
	);
	if (!OD_CHECK(window->render_context_native != nullptr)) {
		OD_ERROR("SDL_GL_CreateContext failed, error=%s", SDL_GetError());
		return false;
	}

	if (!OD_CHECK(SDL_GL_MakeCurrent(
		static_cast<SDL_Window*>(window->window_native),
		static_cast<SDL_GLContext*>(window->render_context_native)) == 0)) {
		OD_ERROR("SDL_GL_MakeCurrent failed, error=%s", SDL_GetError());
		return false;
	}

	if (window->settings.is_vsync_enabled) {
		if (SDL_GL_SetSwapInterval(1) < 0) {
			OD_ERROR("SDL_GL_SetSwapInterval failed, error=%s; disabling vsync and continuing", SDL_GetError());
			// Lack of vsync support is not fatal; we have a frame timer as backup
			window->settings.is_vsync_enabled = false;
		}
	}

	if (!OD_CHECK(odRenderer_init(&window->renderer, window->render_context_native))) {
		return false;
	}

	if (!OD_CHECK(odTexture_init_blank(&window->texture, window->render_context_native))) {
		return false;
	}

	if (!OD_CHECK(odRenderTexture_init(&window->game_render_texture, window->render_context_native, window->settings.game_width, window->settings.game_height))) {
		return false;
	}

	window->is_open = true;

	OD_DEBUG("Window opened");

	return true;
}
void odWindow_destroy(odWindow* window) {
	OD_DEBUG("window=%s", odWindow_get_debug_string(window));

	if (!OD_DEBUG_CHECK(window != nullptr)) {
		return;
	}

	if (!OD_CHECK(!odWindow_get_valid(window) || odWindow_prepare_render_context(window))) {
		return;
	}

	OD_TRACE("Destroying render texture");

	odRenderTexture_destroy(&window->game_render_texture);

	OD_TRACE("Destroying texture");

	odTexture_destroy(&window->texture);

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
void* odWindow_prepare_render_context(odWindow* window) {
	if (!OD_DEBUG_CHECK(odWindow_get_valid(window))) {
		return nullptr;
	}

	if (!OD_CHECK(SDL_GL_MakeCurrent(static_cast<SDL_Window*>(window->window_native), static_cast<SDL_GLContext*>(window->render_context_native)) == 0)) {
		return nullptr;
	}

	return window->render_context_native;
}
bool odWindow_get_valid(const odWindow* window) {
	if (!OD_DEBUG_CHECK(window != nullptr)) {
		return false;
	}

	if (!window->is_open) {
		return false;
	}

	if (!OD_DEBUG_CHECK(window->window_native != nullptr)
		|| !OD_DEBUG_CHECK(window->render_context_native != nullptr)) {
		return false;
	}

	return true;
}
bool odWindow_set_visible(odWindow* window, bool is_visible) {
	OD_DEBUG("window=%s, is_visible=%d", odWindow_get_debug_string(window), is_visible);

	if (!OD_DEBUG_CHECK(odWindow_get_valid(window))) {
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
bool odWindow_set_size(odWindow* window, int32_t width, int32_t height) {
	OD_DEBUG("window=%s, width=%d, height=%d", odWindow_get_debug_string(window), width, height);

	if (!OD_DEBUG_CHECK(odWindow_get_valid(window))) {
		return false;
	}

	if ((width == window->settings.window_width) && (height == window->settings.window_height)) {
		return true;
	}

	window->settings.window_width = width;
	window->settings.window_height = height;

	return true;
}
OD_NO_DISCARD static bool odWindow_handle_event(odWindow* window, const SDL_Event *event) {
	if (!OD_DEBUG_CHECK(odWindow_get_valid(window))) {
		return false;
	}

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
OD_NO_DISCARD static bool odWindow_wait_step(odWindow* window) {
	if (!OD_DEBUG_CHECK(odWindow_get_valid(window))) {
		return false;
	}

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
	if (!OD_DEBUG_CHECK(odWindow_get_valid(window))) {
		return false;
	}

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (!OD_CHECK(odWindow_handle_event(window, &event))) {
			return false;
		}

		if (!odWindow_get_valid(window)) {
			return true;
		}
	}

	if (!OD_CHECK(odWindow_prepare_render_context(window))) {
		return false;
	}

	odTransform view_transform{};
	odTransform_init_view_transform(&view_transform, window->settings.game_width, window->settings.game_height);

	odRenderState view_state{
		view_transform,
		odTransform_identity,
		odBounds{0.0f, 0.0f, static_cast<float>(window->settings.game_width), static_cast<float>(window->settings.game_height)},
		&window->texture,
		&window->game_render_texture
	};

	odRenderState window_state{
		view_transform,
		odTransform_identity,
		odBounds{0.0f, 0.0f, static_cast<float>(window->settings.window_width), static_cast<float>(window->settings.window_height)},
		odRenderTexture_get_texture(&window->game_render_texture),
		/* opt_render_texture*/ nullptr
	};

	if (!OD_CHECK(odRenderer_clear(&window->renderer, &view_state, odColor_white))) {
		return false;
	}

	if (!OD_CHECK(odRenderer_clear(&window->renderer, &window_state, odColor_white))) {
		return false;
	}

	// BEGIN throwaway rendering test code - TODO remove
	{
		const int32_t test_offset = 0;
		const int32_t test_width = window->settings.game_width;
		const int32_t test_height = window->settings.game_height;
		const int32_t test_vertices_count = 3;
		const odVertex test_vertices[test_vertices_count] = {
			{{float(test_offset),float(test_offset),0.0f,1.0f},
			 {0xff,0x00,0x00,0xff},
			 0.0f,0.0f},
			{{float(test_offset),float(test_offset + test_height),0.0f,1.0f},
			 {0xff,0x00,0x00,0xff},
			 0.0f,0.0f},
			{{float(test_offset + test_width),float(test_offset),0.0f,1.0f},
			 {0xff,0x00,0x00,0xff},
			 0.0f,0.0f}
		};
		if (!OD_CHECK(odRenderer_draw_vertices(&window->renderer, &view_state, test_vertices, test_vertices_count))) {
			return false;
		}
	}
	// END throwaway rendering test code - TODO remove

	if (!OD_CHECK(odRenderer_draw_texture(&window->renderer, &window_state, nullptr))) {
		return false;
	}

	if (!OD_CHECK(odRenderer_flush(&window->renderer))) {
		return false;
	}

	SDL_GL_SwapWindow(static_cast<SDL_Window*>(window->window_native));

	if (!OD_CHECK(odWindow_wait_step(window))) {
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
	: settings{odWindowSettings_get_defaults()}, window_native{nullptr}, render_context_native{nullptr},
	is_sdl_init{false}, is_open{false}, next_frame_ms{0}, texture{}, game_render_texture{} {
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
