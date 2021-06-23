#include <od/debug_gui/debug_gui.h>

#include <od/platform/renderer.h>

#if OD_BUILD_DEBUG_GUI && (OD_BUILD_RENDERER != OD_RENDERER_NONE)

#if OD_BUILD_RENDERER == OD_RENDERER_OPENGL3
	#define GLEW_STATIC
	#define GL_GLEXT_PROTOTYPES 1
	#define GL3_PROTOTYPES 1
	#include <GL/glew.h>
	#include <GL/glu.h>

	#include <SDL2/SDL_opengl.h>
#elif OD_BUILD_RENDERER == OD_RENDERER_OPENGLES2
	#include <SDL2/SDL_opengles2.h>
#else
	#error "Debug gui module only supports OpenGL3 and OpenGLES2; disable with -D OD_BUILD_DEBUG_GUI=0"
#endif

#include <SDL2/SDL.h>

#include <imgui.h>
#include <backends/imgui_impl_sdl.h>
#include <backends/imgui_impl_opengl3.h>

#include <imgui.cpp>
#include <imgui_demo.cpp>
#include <imgui_draw.cpp>
#include <imgui_tables.cpp>
#include <imgui_widgets.cpp>
#include <backends/imgui_impl_sdl.cpp>
#include <backends/imgui_impl_opengl3.cpp>

struct odDebugGui {
	bool initialized = false;
	SDL_Window* sdl_window = nullptr;
	SDL_GLContext sdl_gl_context = nullptr;
	ImGuiIO* io = nullptr;

	~odDebugGui() {
		if (initialized) {
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplSDL2_Shutdown();
			ImGui::DestroyContext();
		}
	}
};
static odDebugGui debug_gui;
odDebugGui* odDebugGui_get(void* native_window, void* native_render_context) {
	static odDebugGui gui;

	gui.sdl_window = static_cast<SDL_Window*>(native_window);
	gui.sdl_gl_context = static_cast<SDL_GLContext>(native_render_context);

	if (gui.initialized) {
		return &gui;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	gui.io = &ImGui::GetIO();

	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForOpenGL(gui.sdl_window, gui.sdl_gl_context);
	ImGui_ImplOpenGL3_Init("#version 100");

	gui.initialized = true;

	return &gui;
}
void odDebugGui_event(odDebugGui* /*gui*/, void* native_event) {
	ImGui_ImplSDL2_ProcessEvent(static_cast<SDL_Event*>(native_event));
}
void odDebugGui_draw(odDebugGui* gui) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(gui->sdl_window);
	ImGui::NewFrame();

	bool show_demo_window = true;
	ImGui::ShowDemoWindow(&show_demo_window);

	ImGui::Begin("Hello, world!");
	ImGui::End();

	ImGui::Render();
	glViewport(0, 0, 640, 480);
	glClearColor(0.45f, 0.55f, 0.60f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

#endif  // OD_BUILD_DEBUG_GUI && (OD_BUILD_RENDERER != OD_RENDERER_NONE)
