#include <od/platform/gl.h>

#include <cstring>

#include <od/core/debug.h>

#define OD_RENDERER_MESSAGE_BUFFER_SIZE 4096

static GLchar odRenderer_message_buffer[OD_RENDERER_MESSAGE_BUFFER_SIZE] = {};

bool odGl_check_ok(odLogContext log_context) {
	bool ok = true;
	GLenum gl_error = GL_NO_ERROR;

	for (gl_error = glGetError(); gl_error != GL_NO_ERROR; gl_error = glGetError()) {
		if (OD_BUILD_LOGS) {
			const char* error_str = "";
#if !OD_BUILD_EMSCRIPTEN
			error_str = reinterpret_cast<const char*>(gluErrorString(gl_error));
#endif  // !OD_BUILD_EMSCRIPTEN
			odLog_log(&log_context, OD_LOG_LEVEL_ERROR, "gl_error=%u, message=%s", gl_error, error_str);
		}
		ok = false;
	}

	OD_MAYBE_UNUSED(log_context);

	return ok;
}
bool odGl_check_shader_ok(odLogContext log_context, GLuint shader) {
	if (!odGl_check_ok(log_context)) {
		return false;
	}

	GLint compile_status = GL_FALSE;

	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);

	if (compile_status == GL_FALSE) {
		if (OD_BUILD_LOGS) {
			GLsizei msg_max_size = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &msg_max_size);
			if (msg_max_size > OD_RENDERER_MESSAGE_BUFFER_SIZE) {
				msg_max_size = OD_RENDERER_MESSAGE_BUFFER_SIZE;
			}

			memset(static_cast<void*>(odRenderer_message_buffer), 0, static_cast<size_t>(msg_max_size));
			glGetShaderInfoLog(shader, msg_max_size, nullptr, odRenderer_message_buffer);

			odLog_log(
				&log_context,
				OD_LOG_LEVEL_ERROR,
				"OpenGL shader compilation failed, error=\n%s",
				static_cast<const char*>(odRenderer_message_buffer));
		}
		return false;
	}

	OD_MAYBE_UNUSED(log_context);

	return true;
}
bool odGl_check_program_ok(odLogContext log_context, GLuint program) {
	if (!odGl_check_ok(log_context)) {
		return false;
	}

	GLint link_status = GL_FALSE;

	glGetProgramiv(program, GL_LINK_STATUS, &link_status);

	if (link_status == GL_FALSE) {
		if (OD_BUILD_LOGS) {
			GLsizei msg_max_size = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &msg_max_size);
			if (msg_max_size > OD_RENDERER_MESSAGE_BUFFER_SIZE) {
				msg_max_size = OD_RENDERER_MESSAGE_BUFFER_SIZE;
			}

			memset(static_cast<void*>(odRenderer_message_buffer), 0, static_cast<size_t>(msg_max_size));
			glGetProgramInfoLog(program, msg_max_size, nullptr, odRenderer_message_buffer);

			odLog_log(
				&log_context,
				OD_LOG_LEVEL_ERROR,
				"OpenGL program linking failed, error=\n%s",
				static_cast<const char*>(odRenderer_message_buffer));
		}
		return false;
	}

	OD_MAYBE_UNUSED(log_context);

	return true;
}

