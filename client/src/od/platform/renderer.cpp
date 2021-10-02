#include <od/platform/renderer.hpp>

#include <cstring>

#define GLEW_STATIC
#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GL/glew.h>
#include <GL/glu.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <od/core/debug.h>
#include <od/core/type.hpp>

/*https://www.khronos.org/registry/OpenGL/specs/gl/glspec21.pdf*/
/*https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.1.20.pdf*/
static const char odRenderer_vertex_shader[] = R"(
	#version 120

	// Transform from world (+/- windowSize) to normalized device coords (-1.0 to 1.0)
	uniform vec3 scale_xyz;
	// Convert to normalized texture coords (0.0 to 1.0)
	uniform vec2 scale_uv;

	attribute vec4 src_pos;
	attribute vec4 src_col;
	attribute vec2 src_uv;

	varying vec4 col;
	varying vec2 uv;

	void main() {
		gl_Position = vec4(src_pos.xyz * scale_xyz, 1);
		col = src_col;
		uv = src_uv * scale_uv;
	}
)";
static const char odRenderer_fragment_shader[] = R"(
	#version 120

	uniform sampler2D src_texture;

	varying vec4 col;
	varying vec2 uv;

	void main() {
		gl_FragColor = texture2D(src_texture, uv).rgba * col;
	}
)";

#define OD_RENDERER_MESSAGE_BUFFER_SIZE 4096

static GLchar odRenderer_message_buffer[OD_RENDERER_MESSAGE_BUFFER_SIZE];

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
	} else if ((severity == GL_DEBUG_SEVERITY_MEDIUM) || severity == GL_DEBUG_SEVERITY_LOW) {
		OD_WARN("%s", message);
	} else if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
		OD_DEBUG("%s", message);
	}
}
static bool odRenderer_glew_ensure_initialized() {
	static bool is_initialized = false;

	OD_TRACE("is_initialized=%d", is_initialized);

	if (is_initialized) {
		return true;
	}

	OD_TRACE("glewInit");
	glewExperimental = true;
	GLenum glewResult = glewInit();
	if (glewResult != GLEW_OK) {
		OD_ERROR("glewInit() failed with error: %s", glewGetErrorString(glewResult));
		return false;
	}

	if (GLEW_ARB_debug_output) {
		OD_DEBUG("GLEW_ARB_debug_output=true, registering gl debug log callback");
		glDebugMessageCallbackARB(odRenderer_gl_message_callback, nullptr);
	}

	is_initialized = true;

	OD_TRACE("is_initialized=%d", is_initialized);

	return true;
}

const struct odType* odRenderer_get_type_constructor() {
	return odType_get<odRenderer>();
}
void odRenderer_swap(odRenderer* renderer1, odRenderer* renderer2) {
	if (renderer1 == nullptr) {
		OD_ERROR("renderer1=nullptr");
		return;
	}

	if (renderer2 == nullptr) {
		OD_ERROR("renderer2=nullptr");
		return;
	}

	odRenderer renderer_swap;
	memcpy(static_cast<void*>(&renderer_swap), static_cast<void*>(renderer1), sizeof(odRenderer));
	memcpy(static_cast<void*>(renderer1), static_cast<void*>(renderer2), sizeof(odRenderer));
	memcpy(static_cast<void*>(renderer2), static_cast<void*>(&renderer_swap), sizeof(odRenderer));
}
const char* odRenderer_get_debug_string(const odRenderer* renderer) {
	if (renderer == nullptr) {
		return "odRenderer{this=nullptr}";
	}

	return odDebugString_format(
		("odRenderer{this=%p, vertex_shader=%u, fragment_shader=%u, "
		 "program=%u, vbo=%u, vao=%u, src_texture=%u}"),
		static_cast<const void*>(renderer),
		renderer->vertex_shader,
		renderer->fragment_shader,
		renderer->program,
		renderer->vbo,
		renderer->vao,
		renderer->src_texture
	);
}
static bool jeGl_getOk(odLogContext logger) {
	bool ok = true;
	GLenum gl_error = GL_NO_ERROR;

	for (gl_error = glGetError(); gl_error != GL_NO_ERROR; gl_error = glGetError()) {
#if OD_BUILD_LOG
		odLog_log(
			logger,
			OD_LOG_LEVEL_ERROR,
			"OpenGL error, gl_error=%u, message=%s",
			gl_error,
			gluErrorString(gl_error));
#endif
		ok = false;
	}

	(void)logger;

	return ok;
}
static bool jeGl_getShaderOk(odLogContext logger, GLuint shader) {
	GLint compile_status = GL_FALSE;

	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);

	if (compile_status == GL_FALSE) {
#if OD_BUILD_LOG
		GLsizei msg_max_size = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &msg_max_size);
		if (msg_max_size > OD_RENDERER_MESSAGE_BUFFER_SIZE) {
			msg_max_size = OD_RENDERER_MESSAGE_BUFFER_SIZE;
		}

		glGetShaderInfoLog(shader, msg_max_size, nullptr, odRenderer_message_buffer);

		odLog_log(
			logger,
			OD_LOG_LEVEL_ERROR,
			"OpenGL shader compilation failed, error=\n%s",
			static_cast<const char*>(odRenderer_message_buffer));
#endif
		return false;
	}

	(void)logger;

	return true;
}
static bool jeGl_getProgramOk(odLogContext logger, GLuint program) {
	GLint link_status = GL_FALSE;

	glGetProgramiv(program, GL_LINK_STATUS, &link_status);

	if (link_status == GL_FALSE) {
#if OD_BUILD_LOG
		GLsizei msg_max_size = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &msg_max_size);
		if (msg_max_size > OD_RENDERER_MESSAGE_BUFFER_SIZE) {
			msg_max_size = OD_RENDERER_MESSAGE_BUFFER_SIZE;
		}

		glGetProgramInfoLog(program, msg_max_size, nullptr, odRenderer_message_buffer);

		odLog_log(
			logger,
			OD_LOG_LEVEL_ERROR,
			"OpenGL program linking failed, error=\n%s",
			static_cast<const char*>(odRenderer_message_buffer));
#endif
		return false;
	}

	(void)logger;

	return true;
}
bool odRenderer_init(odRenderer* renderer, void* render_context_native) {
	if (renderer == nullptr) {
		OD_ERROR("renderer=nullptr");
		return false;
	}

	if (render_context_native == nullptr) {
		OD_ERROR("render_context_native=nullptr");
		return false;
	}

	if (SDL_GL_GetCurrentContext() != render_context_native) {
		OD_ERROR("render_context_native not set");
		return false;
	}

	OD_TRACE("renderer=%s", odRenderer_get_debug_string(renderer));

	odRenderer_destroy(renderer);

	renderer->render_context_native = render_context_native;

	odRenderer_glew_ensure_initialized();

	OD_TRACE("creating shader program, renderer=%s", odRenderer_get_debug_string(renderer));
	
	static const GLchar* vertex_shader_ptr = static_cast<const GLchar*>(odRenderer_vertex_shader);
	static const GLint vertex_shader_size = sizeof(odRenderer_vertex_shader);
	renderer->vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(renderer->vertex_shader, 1, &vertex_shader_ptr, &vertex_shader_size);
	glCompileShader(renderer->vertex_shader);

	if (!jeGl_getOk(OD_LOGGER()) || !jeGl_getShaderOk(OD_LOGGER(), renderer->vertex_shader)) {
		OD_ERROR("OpenGL error when creating vertex shader, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	static const GLchar* fragment_shader_ptr = static_cast<const GLchar*>(odRenderer_fragment_shader);
	static const GLint fragment_shader_size = sizeof(odRenderer_fragment_shader);
	renderer->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(renderer->fragment_shader, 1, &fragment_shader_ptr, &fragment_shader_size);
	glCompileShader(renderer->fragment_shader);

	if (!jeGl_getOk(OD_LOGGER()) || !jeGl_getShaderOk(OD_LOGGER(), renderer->fragment_shader)) {
		OD_ERROR("OpenGL error when creating fragment shader, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	renderer->program = glCreateProgram();
	glAttachShader(renderer->program, renderer->vertex_shader);
	glAttachShader(renderer->program, renderer->fragment_shader);

	glBindAttribLocation(renderer->program, 0, "src_pos");
	glBindAttribLocation(renderer->program, 1, "src_col");
	glBindAttribLocation(renderer->program, 2, "src_uv");

	glLinkProgram(renderer->program);
	glUseProgram(renderer->program);

	if (!jeGl_getOk(OD_LOGGER()) || !jeGl_getProgramOk(OD_LOGGER(), renderer->program)) {
		OD_ERROR("OpenGL error when creating program, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	OD_TRACE("creating texture, renderer=%s", odRenderer_get_debug_string(renderer));

	glGenTextures(1, &renderer->src_texture);
	glBindTexture(GL_TEXTURE_2D, renderer->src_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	if (!jeGl_getOk(OD_LOGGER())) {
		OD_ERROR("OpenGL error when creating texture, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	OD_TRACE("creating vertex array, renderer=%s", odRenderer_get_debug_string(renderer));

	glGenBuffers(1, &renderer->vbo);

	glGenVertexArrays(1, &renderer->vao);

	glBindVertexArray(renderer->vao);
	glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);

	if (!jeGl_getOk(OD_LOGGER())) {
		OD_ERROR("OpenGL error when vertex array, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	OD_TRACE("configuring opengl context, renderer=%s", odRenderer_get_debug_string(renderer));

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);

	glDisable(GL_CULL_FACE);

	if (!jeGl_getOk(OD_LOGGER())) {
		OD_ERROR("OpenGL error when configuring opengl context, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	return true;
}
void odRenderer_destroy(odRenderer* renderer) {
	if (renderer == nullptr) {
		OD_ERROR("renderer=nullptr");
		return;
	}

	if (SDL_GL_GetCurrentContext() != renderer->render_context_native) {
		OD_TRACE("render_context_native not set, renderer=%s", odRenderer_get_debug_string(renderer));
		return;
	}

	OD_TRACE("renderer=%s", odRenderer_get_debug_string(renderer));

	if (renderer->vao != 0) {
		OD_TRACE("deleting vao=%u", renderer->vao);

		glBindVertexArray(renderer->vao);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteVertexArrays(1, &renderer->vao);
		renderer->vao = 0;

		glBindVertexArray(0);
	}

	if (renderer->vbo != 0) {
		OD_TRACE("deleting vbo=%u", renderer->vbo);

		glDeleteBuffers(1, &renderer->vbo);
		renderer->vbo = 0;
	}

	if (renderer->src_texture != 0) {
		OD_TRACE("deleting src_texture=%u", renderer->src_texture);

		glDeleteTextures(1, &renderer->src_texture);
		renderer->src_texture = 0;
	}

	if (renderer->program != 0) {
		OD_TRACE("deleting program=%u", renderer->program);

		if (renderer->fragment_shader != 0) {
			glDetachShader(renderer->program, renderer->fragment_shader);
		}

		if (renderer->vertex_shader != 0) {
			glDetachShader(renderer->program, renderer->vertex_shader);
		}
		glDeleteProgram(renderer->program);
		renderer->program = 0;
	}

	if (renderer->fragment_shader != 0) {
		OD_TRACE("deleting fragment_shader=%u", renderer->fragment_shader);

		glDeleteShader(renderer->fragment_shader);
		renderer->fragment_shader = 0;
	}

	if (renderer->vertex_shader != 0) {
		OD_TRACE("deleting vertex_shader=%u", renderer->vertex_shader);

		glDeleteShader(renderer->vertex_shader);
		renderer->vertex_shader = 0;
	}
}
// static bool odRenderer_set_src_texture(odRenderer* renderer, ...)
// static bool odRenderer_draw(odRenderer* renderer, odRenderTarget...) {
// 	glViewport(0, 0, width, height);
// }
odRenderer::odRenderer()
	: render_context_native{nullptr}, vertex_shader{0}, fragment_shader{0}, program{0}, vbo{0}, vao{0}, src_texture{0} {
}
odRenderer::odRenderer(odRenderer&& other) {
	odRenderer_swap(this, &other);
}
odRenderer& odRenderer::operator=(odRenderer&& other) {
	odRenderer_swap(this, &other);
	return *this;
}
odRenderer::~odRenderer() {
	odRenderer_destroy(this);
}
