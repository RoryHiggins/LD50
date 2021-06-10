// #include <od/core.h>
#include <od/platform/debug_gui/debug_gui.h>

#define GLEW_STATIC
#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GL/glew.h>
#include <GL/glu.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <od/platform/window.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wnamespaces"
#pragma GCC diagnostic ignored "-Wredundant-decls"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Winline"
#include <imgui.h>
#include <backends/imgui_impl_sdl.h>
#include <backends/imgui_impl_opengl2.h>
// #include <imgui.cpp>
// #include <imgui_demo.cpp>
// #include <imgui_draw.cpp>
// #include <imgui_tables.cpp>
// #include <imgui_widgets.cpp>
// #include <backends/imgui_impl_sdl.cpp>
// #include <backends/imgui_impl_opengl2.cpp>
#pragma GCC diagnostic pop

struct odDebugGui {
	bool initialized = false;
	SDL_Window* sdl_window = nullptr;
	SDL_GLContext sdl_gl_context = nullptr;
	ImGuiIO* io = nullptr;

	~odDebugGui() {
		if (initialized) {
			ImGui_ImplOpenGL2_Shutdown();
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

	gui.initialized = true;

	return &gui;
}
void odDebugGui_event(odDebugGui* /*gui*/, void* native_event) {
	ImGui_ImplSDL2_ProcessEvent(static_cast<SDL_Event*>(native_event));
}
void odDebugGui_draw(odDebugGui* gui) {
	ImGui_ImplOpenGL2_NewFrame();
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

	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}

