#pragma once

#include <od/debug_gui/module.h>

struct odDebugGui;

OD_API_C OD_ENGINE_DEBUG_GUI_MODULE odDebugGui* odDebugGui_get(void* native_window, void* native_render_context);
OD_API_C OD_ENGINE_DEBUG_GUI_MODULE void odDebugGui_event(odDebugGui* gui, void* native_event);
OD_API_C OD_ENGINE_DEBUG_GUI_MODULE void odDebugGui_draw(odDebugGui* gui);
