#include <od/platform/window.hpp>

#include <cstring>
#include <SDL2/SDL.h>

#if !OD_BUILD_EMSCRIPTEN
#include <GL/glew.h>
#endif  // !OD_BUILD_EMSCRIPTEN

#include <od/core/debug.h>
#include <od/core/math.h>
#include <od/core/type.hpp>

OD_NO_DISCARD static bool
odSDL_init_reentrant();
static void
odSDL_destroy_reentrant();

static void odWindow_try_set_vsync_enabled(odWindow* window, bool is_vsync_enabled);
OD_NO_DISCARD static bool
odWindow_set_caption(odWindow* window, const char* caption);

static int32_t odSDL_init_counter = 0;

bool odSDL_init_reentrant() {
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

const char* odWindowSettings_get_debug_string(const odWindowSettings* settings) {
	if (settings == nullptr) {
		return "null";
	}

	return odDebugString_format(
		"{\"caption\": %s, \"width\": %d, \"height\": %d, \"fps_limit\": %d, "
		"\"is_fps_limit_enabled\": %d, \"is_vsync_enabled\": %d, \"is_visible\": %d}",
		settings->caption,
		settings->width,
		settings->height,
		settings->fps_limit,
		static_cast<int>(settings->is_fps_limit_enabled),
		static_cast<int>(settings->is_vsync_enabled),
		static_cast<int>(settings->is_visible)
	);
}
const odWindowSettings* odWindowSettings_get_defaults() {
	static const odWindowSettings settings{
		/*caption*/ "",
		/*width*/ 640,
		/*height*/ 480,
		/*fps_limit*/ 60,
		/*is_fps_limit_enabled*/ true,
		/*is_vsync_enabled"*/ true,
		/*is_visible*/ true,
	};
	return &settings;
}
const odWindowSettings* odWindowSettings_get_headless_defaults() {
	static const odWindowSettings settings{
		/*caption*/ "",
		/*width*/ 640,
		/*height*/ 480,
		/*fps_limit*/ 60,
		/*is_fps_limit_enabled*/ false,
		/*is_vsync_enabled"*/ false,
		/*is_visible*/ false,
	};
	return &settings;
}
bool odWindowSettings_check_valid(const odWindowSettings* settings) {
	if (!OD_CHECK(settings != nullptr)
		|| !OD_CHECK(odInt32_fits_float(settings->width))
		|| !OD_CHECK(settings->width > 0)
		|| !OD_CHECK(odInt32_fits_float(settings->height))
		|| !OD_CHECK(settings->height > 0)
		|| !OD_CHECK(settings->fps_limit > 0)
		|| !OD_CHECK(settings->fps_limit <= 120)) {
		return false;
	}

	return true;
}

const odType* odWindow_get_type_constructor() {
	return odType_get<odWindow>();
}
void odWindow_swap(odWindow* window1, odWindow* window2) {
	if (!OD_CHECK(window1 != nullptr)
		|| !OD_CHECK(window2 != nullptr)) {
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
		return "null";
	}

	return odDebugString_format(
		"{\"is_open\"=%d, \"settings\": %s}",
		window->is_open,
		odWindowSettings_get_debug_string(&window->settings));
}
bool odWindow_check_valid(const odWindow* window) {
	if (!OD_CHECK(window != nullptr)
		|| !OD_CHECK((!window->is_open) || (window->window_native != nullptr))
		|| !OD_CHECK((!window->is_open) || (window->render_context_native != nullptr))) {
		return false;
	}

	return true;
}
static bool odWindow_set_context_impl(void* window_native, void* render_context_native) {
	if (!OD_CHECK(window_native != nullptr)
		|| !OD_CHECK(render_context_native != nullptr)) {
		return false;
	}

	if (!OD_CHECK(SDL_GL_MakeCurrent(
		static_cast<SDL_Window*>(window_native),
		static_cast<SDL_GLContext>(render_context_native)) == 0)) {
		OD_ERROR("SDL_GL_MakeCurrent failed, error=%s", SDL_GetError());
		return false;
	}

	return true;
}
#if !OD_BUILD_EMSCRIPTEN
static void odRenderer_gl_message_callback(
	GLenum /*source*/,
	GLenum /*type*/,
	GLuint /*id*/,
	GLenum severity,
	GLsizei /*length*/,
	const GLchar* message,
	const void* /*userParam*/) {
	if (severity == GL_DEBUG_SEVERITY_HIGH) {
		OD_ERROR("%s", message);
		return;
	}

	if ((severity == GL_DEBUG_SEVERITY_MEDIUM) || severity == GL_DEBUG_SEVERITY_LOW) {
		OD_WARN("%s", message);
		return;
	}

	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
		OD_DEBUG("%s", message);
		return;
	}

	OD_MAYBE_UNUSED(message);
}
OD_NO_DISCARD static bool odWindow_gl_init() {
	static bool is_initialized = false;

	OD_DEBUG("is_initialized=%d", is_initialized);

	if (is_initialized) {
		return true;
	}


	glewExperimental = true;
	GLenum glewResult = glewInit();
	if (!OD_CHECK(glewResult == GLEW_OK)) {
		OD_ERROR("glewInit() failed with error: %s", glewGetErrorString(glewResult));
		return false;
	}

	if (OD_BUILD_DEBUG && GLEW_ARB_debug_output) {
		OD_DEBUG("GLEW_ARB_debug_output=true, registering gl debug log callback");
		glDebugMessageCallbackARB(odRenderer_gl_message_callback, nullptr);
	}

	is_initialized = true;

	OD_TRACE("is_initialized=%d", is_initialized);

	return true;
}
#else  // !OD_BUILD_EMSCRIPTEN
OD_NO_DISCARD static bool odWindow_gl_init() {
	return true;
}
#endif  // #else  // !OD_BUILD_EMSCRIPTEN
bool odWindow_init(odWindow* window, const odWindowSettings* opt_settings) {
	OD_DEBUG("window=%s", odWindow_get_debug_string(window));

	if (!OD_CHECK(window != nullptr)) {
		return false;
	}

	odWindow_destroy(window);

	window->settings = *odWindowSettings_get_defaults();
	if (opt_settings != nullptr) {
		if (!OD_CHECK(odWindowSettings_check_valid(opt_settings))) {
			return false;
		}
		window->settings = *opt_settings;
	}

	window->is_sdl_init = odSDL_init_reentrant();
	if (!OD_CHECK(window->is_sdl_init)) {
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
		static_cast<int>(window->settings.width),
		static_cast<int>(window->settings.height),
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

	if (!OD_CHECK(odWindow_set_context_impl(window->window_native, window->render_context_native))) {
		return false;
	}

	if (!OD_CHECK(odWindow_gl_init())) {
		return false;
	}
	window->is_open = true;

	odWindow_try_set_vsync_enabled(window, window->settings.is_vsync_enabled);

	OD_DEBUG("Window opened");

	return true;
}
void odWindow_destroy(odWindow* window) {
	OD_DEBUG("window=%s", odWindow_get_debug_string(window));

	if (!OD_CHECK(window != nullptr)) {
		return;
	}

	for (odWindowResource* resource: window->resources) {
		resource->window = nullptr;
	}
	OD_DISCARD(OD_CHECK(window->resources.set_count(0)));

	window->next_frame_ms = 0;
	window->is_open = false;

	if (window->is_sdl_init) {
		odSDL_destroy_reentrant();
	}
	window->is_sdl_init = false;

	window->render_context_native = nullptr;
	window->window_native = nullptr;

	if ((window->window_native != nullptr)
		&& (window->render_context_native != nullptr)
		&& OD_CHECK(odWindow_prepare_render_context(window))) {
		OD_TRACE("Destroying opengl render context");
		SDL_GL_DeleteContext(static_cast<SDL_GLContext*>(window->render_context_native));
	}

	if (window->window_native != nullptr) {
		OD_TRACE("Destroying window");
		SDL_DestroyWindow(static_cast<SDL_Window*>(window->window_native));
	}
}
void* odWindow_prepare_render_context(odWindow* window) {
	if (!OD_CHECK(odWindow_check_valid(window))
		|| !OD_CHECK(window->window_native != nullptr)
		|| !OD_CHECK(window->render_context_native != nullptr)) {
		return nullptr;
	}

	if (!OD_CHECK(SDL_GL_MakeCurrent(static_cast<SDL_Window*>(window->window_native), static_cast<SDL_GLContext*>(window->render_context_native)) == 0)) {
		return nullptr;
	}

	return window->render_context_native;
}
OD_NO_DISCARD static bool odWindow_handle_event(odWindow* window, const SDL_Event *event) {
	if (!OD_CHECK(!window->is_open || odWindow_check_valid(window))) {
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
	if (!OD_CHECK(!window->is_open || odWindow_check_valid(window))) {
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
	if (!window->is_open
		|| !OD_CHECK(odWindow_check_valid(window))) {
		return false;
	}

	SDL_GL_SwapWindow(static_cast<SDL_Window*>(window->window_native));

	if (!OD_CHECK(odWindow_wait_step(window))) {
		return false;
	}

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (!OD_CHECK(odWindow_handle_event(window, &event))) {
			return false;
		}

		if (!window->is_open) {
			return false;
		}
	}

	if (!OD_CHECK(odWindow_prepare_render_context(window))) {
		return false;
	}

	return true;
}
bool odWindow_set_visible(odWindow* window, bool is_visible) {
	OD_DEBUG("window=%s, is_visible=%d", odWindow_get_debug_string(window), is_visible);

	if (!OD_CHECK(!window->is_open || odWindow_check_valid(window))) {
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

	if (!OD_CHECK(!window->is_open || odWindow_check_valid(window))) {
		return false;
	}

	if ((width == window->settings.width) && (height == window->settings.height)) {
		return true;
	}

	window->settings.width = width;
	window->settings.height = height;

	return true;
}
static void odWindow_try_set_vsync_enabled(odWindow* window, bool is_vsync_enabled) {
	OD_DEBUG("window=%s, is_vsync_enabled=%d", odWindow_get_debug_string(window), int(is_vsync_enabled));

	if (!OD_CHECK(!window->is_open || odWindow_check_valid(window))) {
		return;
	}

	if (is_vsync_enabled && (SDL_GL_GetSwapInterval() != static_cast<int>(is_vsync_enabled))) {
		if (!OD_CHECK(SDL_GL_SetSwapInterval(static_cast<int>(is_vsync_enabled)) >= 0)) {
			OD_WARN("SDL_GL_SetSwapInterval toggle failed, message=\"%s\"\n", SDL_GetError());
			return;
		}
	}

	window->settings.is_vsync_enabled = window->settings.is_vsync_enabled;
}
static bool odWindow_set_caption(odWindow* window, const char* caption) {
	OD_DEBUG("window=%s, caption=%s", odWindow_get_debug_string(window), caption);

	if (!OD_CHECK(!window->is_open || odWindow_check_valid(window))
		|| !OD_CHECK(caption != nullptr)) {
		return false;
	}

	SDL_Window* sdl_window = static_cast<SDL_Window*>(window->window_native);

	if (strcmp(window->settings.caption, caption) != 0) {
		SDL_SetWindowTitle(sdl_window, caption);
	}

	window->settings.caption = window->settings.caption;

	return true;
}
bool odWindow_set_settings(struct odWindow* window, const odWindowSettings* settings) {
	if (!OD_CHECK(window != nullptr)
		|| !OD_CHECK(odWindowSettings_check_valid(settings))) {
		return false;
	}

	if (!window->is_open) {
		window->settings = *settings;
		return true;
	}

	if (!OD_CHECK(odWindow_check_valid(window))) {
		return false;
	}

	if (!OD_CHECK(odWindow_set_caption(window, settings->caption))) {
		return false;
	}

	if (!OD_CHECK(odWindow_set_size(window, settings->width, settings->height))) {
		return false;
	}

	if (!OD_CHECK(odWindow_set_visible(window, settings->is_visible))) {
		return false;
	}

	// failing to set/unset vsync is not an error
	odWindow_try_set_vsync_enabled(window, settings->is_vsync_enabled);

	window->settings = *settings;

	return true;
}
const odWindowSettings* odWindow_get_settings(const odWindow* window) {
	if (!OD_CHECK(window != nullptr)) {
		return nullptr;
	}

	return &window->settings;
}
odWindow::odWindow()
	: settings{*odWindowSettings_get_defaults()}, window_native{nullptr}, render_context_native{nullptr},
	is_sdl_init{false}, is_open{false}, next_frame_ms{0} {
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

OD_NO_DISCARD bool odWindowResource_init(odWindowResource* resource, odWindow* opt_window) {
	if (!OD_CHECK(resource != nullptr)
		|| !OD_CHECK(opt_window == nullptr || (opt_window->window_native != nullptr))) {
		return false;
	}

	odWindowResource_destroy(resource);

	if (opt_window == nullptr) {
		return true;
	}
	
	if (!OD_CHECK(opt_window->resources.push(resource))) {
		return false;
	}

	resource->window = opt_window;

	return true;
}
void odWindowResource_destroy(odWindowResource* resource) {
	if (!OD_CHECK(resource != nullptr)) {
		return;
	}

	if (resource->window == nullptr) {
		return;
	}
	odWindow* window = resource->window;

	resource->window = nullptr;

	odWindowResource** resources = window->resources.begin();
	int32_t resources_count = window->resources.get_count();
	for (int32_t i = 0; i < resources_count; i++) {
		if (resources[i] == resource) {
			OD_DISCARD(OD_CHECK(window->resources.swap_pop(i)));
			break;
		}
	}
}
odWindowResource::odWindowResource()
: window{nullptr} {
}
odWindowResource::odWindowResource(odWindowResource&& other)
: odWindowResource{} {
	OD_DISCARD(OD_CHECK(odWindowResource_init(this, other.window)));
	odWindowResource_destroy(&other);
}
odWindowResource& odWindowResource::operator=(odWindowResource&& other) {
	OD_DISCARD(OD_CHECK(odWindowResource_init(this, other.window)));
	odWindowResource_destroy(&other);
	return *this;
}
odWindowResource::~odWindowResource() {
	odWindowResource_destroy(this);
}

bool odWindowScope_bind(odWindowScope* scope, odWindow* window) {
	if (!OD_CHECK(scope != nullptr)
		|| !OD_CHECK(odWindow_check_valid(window))) {
		return false;
	}

	scope->old_render_context_native = static_cast<void*>(SDL_GL_GetCurrentContext());
	if (!OD_CHECK(odWindow_set_context_impl(window->window_native, window->render_context_native))) {
		return false;
	}

	scope->window_native = window->window_native;

	return true;
}
bool odWindowScope_try_bind(odWindowScope* scope, odWindow* window) {
	if (!OD_CHECK(scope != nullptr)
		|| (window == nullptr)
		|| (window->window_native == nullptr)
		|| (window->render_context_native == nullptr)) {
		return false;
	}
	return odWindowScope_bind(scope, window);
}
odWindowScope::odWindowScope()
: window_native{nullptr}, old_render_context_native{nullptr} {
}
odWindowScope::~odWindowScope() {
	if ((window_native != nullptr) && (old_render_context_native != nullptr)) {
		OD_DISCARD(odWindow_set_context_impl(window_native, old_render_context_native));
	}

	window_native = nullptr;
	old_render_context_native = nullptr;
}
