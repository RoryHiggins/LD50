#pragma once

#include <od/platform/debug_gui/api.h>

struct odWindow;
struct odDebugGui;

OD_ENGINE_DEBUG_GUI_API_C odDebugGui* odDebugGui_get(void* native_window, void* native_render_context);
OD_ENGINE_DEBUG_GUI_API_C void odDebugGui_event(odDebugGui* gui, void* native_event);
OD_ENGINE_DEBUG_GUI_API_C void odDebugGui_draw(odDebugGui* gui);
