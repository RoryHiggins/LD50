#pragma once

#define GLEW_STATIC
#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GL/glew.h>
#include <GL/glu.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <od/platform/module.h>

struct odLogContext;

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD
bool odGl_check_ok(struct odLogContext logger);

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD
bool odGl_check_shader_ok(struct odLogContext logger, GLuint shader);

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD
bool odGl_check_program_ok(struct odLogContext logger, GLuint program);
