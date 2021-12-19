#pragma once

#include <od/platform/module.h>

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1

#if OD_BUILD_EMSCRIPTEN
#include <GLES2/gl2platform.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else  // !OD_BUILD_EMSCRIPTEN
#include <GL/glew.h>
#endif  // #else  // !OD_BUILD_EMSCRIPTEN

struct odLogContext;

OD_NO_DISCARD bool
odGl_check_ok(struct odLogContext log_context);
OD_NO_DISCARD bool
odGl_check_shader_ok(struct odLogContext log_context, GLuint shader);
OD_NO_DISCARD bool
odGl_check_program_ok(struct odLogContext log_context, GLuint program);
