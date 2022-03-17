#pragma once

#include <od/platform/module.h>

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odSDL_init_reentrant(void);
OD_API_C OD_PLATFORM_MODULE void
odSDL_destroy_reentrant(void);

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odSDLMixer_init_reentrant(void);
OD_API_C OD_PLATFORM_MODULE void
odSDLMixer_destroy_reentrant(void);
