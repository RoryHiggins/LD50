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
#include <od/core/color.h>
#include <od/core/vertex.h>
#include <od/core/type.hpp>

#define OD_RENDERER_MESSAGE_BUFFER_SIZE 4096

/*https://www.khronos.org/registry/OpenGL/specs/gl/glspec21.pdf*/
/*https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.1.20.pdf*/
static const char odRenderer_vertex_shader[] = R"(
	#version 120

	// there is no model space: all inputs are required to be in world space
	uniform mat4 view;
	uniform mat4 projection;

	attribute vec3 src_pos;
	attribute vec4 src_col;
	attribute vec2 src_uv;

	varying vec4 col;
	varying vec2 uv;

	void main() {
		gl_Position = projection * view * vec4(src_pos, 1);
		col = src_col;
		uv = src_uv;
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

static GLchar odRenderer_message_buffer[OD_RENDERER_MESSAGE_BUFFER_SIZE] = {};

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
		return;
	}

	if ((severity == GL_DEBUG_SEVERITY_MEDIUM) || severity == GL_DEBUG_SEVERITY_LOW) {
		OD_WARN("%s", message);
		return;
	}

	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
		OD_DEBUG("%s", message);
		return;
	}
}

static bool odRenderer_glew_init() {
	static bool is_initialized = false;

	OD_DEBUG("is_initialized=%d", is_initialized);

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

	if (OD_BUILD_DEBUG && GLEW_ARB_debug_output) {
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
static bool odGl_getOk(odLogContext logger) {
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
static bool odGl_getShaderOk(odLogContext logger, GLuint shader) {
	GLint compile_status = GL_FALSE;

	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);

	if (compile_status == GL_FALSE) {
#if OD_BUILD_LOG
		GLsizei msg_max_size = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &msg_max_size);
		if (msg_max_size > OD_RENDERER_MESSAGE_BUFFER_SIZE) {
			msg_max_size = OD_RENDERER_MESSAGE_BUFFER_SIZE;
		}

		memset(static_cast<void*>(odRenderer_message_buffer), 0, static_cast<size_t>(msg_max_size));
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
static bool odGl_getProgramOk(odLogContext logger, GLuint program) {
	GLint link_status = GL_FALSE;

	glGetProgramiv(program, GL_LINK_STATUS, &link_status);

	if (link_status == GL_FALSE) {
#if OD_BUILD_LOG
		GLsizei msg_max_size = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &msg_max_size);
		if (msg_max_size > OD_RENDERER_MESSAGE_BUFFER_SIZE) {
			msg_max_size = OD_RENDERER_MESSAGE_BUFFER_SIZE;
		}

		memset(static_cast<void*>(odRenderer_message_buffer), 0, static_cast<size_t>(msg_max_size));
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
		OD_ERROR("render_context_native doesn't match what is set");
		return false;
	}

	OD_DEBUG("renderer=%s", odRenderer_get_debug_string(renderer));

	odRenderer_destroy(renderer);

	renderer->render_context_native = render_context_native;

	odRenderer_glew_init();

	OD_TRACE("creating vertex array, renderer=%s", odRenderer_get_debug_string(renderer));

	glGenBuffers(1, &renderer->vbo);

	glGenVertexArrays(1, &renderer->vao);

	glBindVertexArray(renderer->vao);
	glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);

	OD_TRACE("creating shader program, renderer=%s", odRenderer_get_debug_string(renderer));
	
	static const GLchar* vertex_shader_ptr = static_cast<const GLchar*>(odRenderer_vertex_shader);
	static const GLint vertex_shader_size = sizeof(odRenderer_vertex_shader);
	renderer->vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(renderer->vertex_shader, 1, &vertex_shader_ptr, &vertex_shader_size);
	glCompileShader(renderer->vertex_shader);

	if (!odGl_getOk(OD_LOGGER()) || !odGl_getShaderOk(OD_LOGGER(), renderer->vertex_shader)) {
		OD_ERROR("OpenGL error when creating vertex shader, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	static const GLchar* fragment_shader_ptr = static_cast<const GLchar*>(odRenderer_fragment_shader);
	static const GLint fragment_shader_size = sizeof(odRenderer_fragment_shader);
	renderer->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(renderer->fragment_shader, 1, &fragment_shader_ptr, &fragment_shader_size);
	glCompileShader(renderer->fragment_shader);

	if (!odGl_getOk(OD_LOGGER()) || !odGl_getShaderOk(OD_LOGGER(), renderer->fragment_shader)) {
		OD_ERROR("OpenGL error when creating fragment shader, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	renderer->program = glCreateProgram();
	glAttachShader(renderer->program, renderer->vertex_shader);
	glAttachShader(renderer->program, renderer->fragment_shader);

	glLinkProgram(renderer->program);
	glUseProgram(renderer->program);

	if (!odGl_getOk(OD_LOGGER()) || !odGl_getProgramOk(OD_LOGGER(), renderer->program)) {
		OD_ERROR("OpenGL error when creating program, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	OD_TRACE("configuring shader attributes, renderer=%s", odRenderer_get_debug_string(renderer));

	glBindAttribLocation(renderer->program, 0, "src_pos");
	glBindAttribLocation(renderer->program, 1, "src_col");
	glBindAttribLocation(renderer->program, 2, "src_uv");

	if (!odGl_getOk(OD_LOGGER())) {
		OD_ERROR("OpenGL error when configuring shader attributes, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	GLuint src_pos_attrib = static_cast<GLuint>(glGetAttribLocation(renderer->program, "src_pos"));
	GLuint src_col_attrib = static_cast<GLuint>(glGetAttribLocation(renderer->program, "src_col"));
	GLuint src_uv_attrib = static_cast<GLuint>(glGetAttribLocation(renderer->program, "src_uv"));
	OD_TRACE("renderer=%s, src_pos_attrib=%u, src_col_attrib=%u, src_uv_attrib=%u",
		odRenderer_get_debug_string(renderer), src_pos_attrib, src_col_attrib, src_uv_attrib);

	if (!odGl_getOk(OD_LOGGER())) {
		OD_ERROR("OpenGL error when configuring shader attributes, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	if (!odGl_getOk(OD_LOGGER())) {
		OD_ERROR("OpenGL error when configuring shader attributes, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	glEnableVertexAttribArray(src_pos_attrib);
	glEnableVertexAttribArray(src_col_attrib);
	glEnableVertexAttribArray(src_uv_attrib);

	const GLvoid* offset = static_cast<const GLvoid*>(nullptr);
	glVertexAttribPointer(src_pos_attrib, 3, GL_FLOAT, GL_FALSE, sizeof(odVertex), offset);
	offset = static_cast<const GLvoid*>(static_cast<const GLchar*>(offset) + (3 * sizeof(GLfloat)));

	glVertexAttribPointer(src_col_attrib, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(odVertex), offset);
	offset = static_cast<const GLvoid*>(static_cast<const GLchar*>(offset) + (4 * sizeof(GLubyte)));

	glVertexAttribPointer(src_uv_attrib, 2, GL_FLOAT, GL_FALSE, sizeof(odVertex), reinterpret_cast<const GLvoid*>(8 * sizeof(GLfloat)));
	offset = static_cast<const GLvoid*>(static_cast<const GLchar*>(offset) + (2 * sizeof(GLfloat)));

	if (!odGl_getOk(OD_LOGGER())) {
		OD_ERROR("OpenGL error when configuring shader attributes, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	OD_TRACE("creating texture, renderer=%s", odRenderer_get_debug_string(renderer));

	glGenTextures(1, &renderer->src_texture);
	glBindTexture(GL_TEXTURE_2D, renderer->src_texture);
	const odColor default_texture{0xFF, 0xFF, 0xFF,0xFF};
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		1,
		1,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		&default_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	if (!odGl_getOk(OD_LOGGER())) {
		OD_ERROR("OpenGL error when creating texture, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	if (!odGl_getOk(OD_LOGGER())) {
		OD_ERROR("OpenGL error when vertex array, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	OD_TRACE("configuring opengl context, renderer=%s", odRenderer_get_debug_string(renderer));

	if (odLogLevel_get_max() >= OD_LOG_LEVEL_DEBUG) {
		// technically requires opengl 4.1 but we asked for 3.2; it's okay for this call to fail.
		glEnable(GL_DEBUG_OUTPUT);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_DEPTH_TEST);

	glDisable(GL_CULL_FACE);

	glBindVertexArray(0);

	if (!odGl_getOk(OD_LOGGER())) {
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

	OD_DEBUG("renderer=%s", odRenderer_get_debug_string(renderer));

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
}
// static bool odRenderer_update_src_texture(odRenderer* renderer, ...)
bool odRenderer_draw(odRenderer* renderer, const odVertex* vertices, int32_t vertices_count, odRenderViewport viewport /*, odRenderTarget *target*/) {
	if (renderer == nullptr) {
		OD_ERROR("renderer=nullptr");
		return false;
	}

	if (vertices == nullptr) {
		OD_ERROR("vertices == nullptr");
		return false;
	}

	if (vertices_count < 0) {
		OD_ERROR("vertices_count < 0");
		return false;
	}

	if (SDL_GL_GetCurrentContext() != renderer->render_context_native) {
		OD_ERROR("rendererer context doesn't match current context");
		return false;
	}

	glUseProgram(renderer->program);
	glBindVertexArray(renderer->vao);

	GLuint view_uniform = static_cast<GLuint>(glGetUniformLocation(renderer->program, "view"));
	GLuint projection_uniform = static_cast<GLuint>(glGetUniformLocation(renderer->program, "projection"));

	const GLfloat translate_x = -1.0f;
	const GLfloat translate_y = 1.0f;
	GLfloat scale_x = 1.0f / static_cast<GLfloat>(viewport.w);
	GLfloat scale_y = -1.0f / static_cast<GLfloat>(viewport.h);
	// depths +/- 2^20; near the precise int limit for float32
	const GLfloat scale_z = 1.0f / static_cast<GLfloat>(1 << 20);

	GLfloat view[16] = {
		scale_x, 0, 0, 0,
		0, scale_y, 0, 0,
		0, 0, scale_z, 0,
		translate_x, translate_y, 0, 1
	};
	const GLfloat projection[16] = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	glUniformMatrix4fv(view_uniform, 1, false, view);
	glUniformMatrix4fv(projection_uniform, 1, false, projection);

	glViewport(viewport.x, -viewport.y, viewport.w, viewport.h);

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_DEPTH_BUFFER_BIT);

	glBufferData(
		GL_ARRAY_BUFFER,
		static_cast<GLsizeiptr>(static_cast<size_t>(vertices_count) * sizeof(odVertex)),
		vertices,
		GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices_count));

	glBindVertexArray(0);
	glUseProgram(0);

	if (!odGl_getOk(OD_LOGGER())) {
		OD_ERROR("OpenGL error when drawing, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	return true;
}
odRenderer::odRenderer()
	: render_context_native{nullptr}, vbo{0}, vao{0}, vertex_shader{0}, fragment_shader{0}, program{0}, src_texture{0} {
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
