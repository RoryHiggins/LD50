#pragma once

#include <od/core/api.h>

#define OD_RENDERER_NONE 0
#define OD_RENDERER_OPENGL3 1
#define OD_RENDERER_OPENGLES2 2
#define OD_RENDERER_FIRST 1
#define OD_RENDERER_LAST 2
#define OD_RENDERER_DEFAULT 1

#if (OD_BUILD_RENDERER == OD_RENDERER_OPENGL3) || (OD_BUILD_RENDERER == OD_RENDERER_OPENGLES2)
    #define OD_RENDERER_USING_OPENGL 1
#else
    #define OD_RENDERER_USING_OPENGL 0
#endif
