#include <od/platform/renderer.hpp>

#include <cstring>

#include <od/core/type.hpp>
#include <od/core/debug.h>
#include <od/core/color.h>
#include <od/core/transform.h>
#include <od/core/vertex.h>
#include <od/core/bounds.h>
#include <od/platform/gl.h>
#include <od/platform/texture.hpp>
#include <od/platform/render_texture.hpp>
#include <od/platform/render_state.h>

#define OD_RENDERER_MESSAGE_BUFFER_SIZE 4096

struct odRendererScope {
	odRendererScope(struct odRenderer* renderer);
	~odRendererScope();
};

/*https://www.khronos.org/registry/OpenGL/specs/gl/glspec21.pdf*/
/*https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.1.20.pdf*/
static const char odRenderer_vertex_shader[] = R"(
	#version 120

	// there is no model space: all inputs are required to be in world space
	uniform mat4 view;
	uniform mat4 projection;
	uniform vec2 uv_scale;

	attribute vec3 src_pos;
	attribute vec4 src_col;
	attribute vec2 src_uv;

	varying vec4 col;
	varying vec2 uv;

	void main() {
		gl_Position = projection * view * vec4(src_pos, 1);
		col = src_col;
		uv = src_uv * uv_scale;
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

OD_NO_DISCARD static bool odRenderer_glew_init() {
	static bool is_initialized = false;

	OD_DEBUG("is_initialized=%d", is_initialized);

	if (is_initialized) {
		return true;
	}

	OD_TRACE("glewInit");
	glewExperimental = true;
	GLenum glewResult = glewInit();
	if (!OD_CHECK(glewResult == GLEW_OK)) {
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

const odType* odRenderer_get_type_constructor() {
	return odType_get<odRenderer>();
}
void odRenderer_swap(odRenderer* renderer1, odRenderer* renderer2) {
	if (!OD_DEBUG_CHECK(renderer1 != nullptr)
		|| !OD_DEBUG_CHECK(renderer2 != nullptr)) {
		return;
	}

	odRenderer renderer_swap;
	memcpy(static_cast<void*>(&renderer_swap), static_cast<void*>(renderer1), sizeof(odRenderer));
	memcpy(static_cast<void*>(renderer1), static_cast<void*>(renderer2), sizeof(odRenderer));
	memcpy(static_cast<void*>(renderer2), static_cast<void*>(&renderer_swap), sizeof(odRenderer));
}
bool odRenderer_get_valid(const odRenderer* renderer) {
	if ((renderer == nullptr)
		|| (renderer->render_context_native == nullptr)
		|| (renderer->vbo == 0)
		|| (renderer->vao == 0)
		|| (renderer->vertex_shader == 0)
		|| (renderer->fragment_shader == 0)
		|| (renderer->program == 0)) {
		return false;
	}

	return true;
}
const char* odRenderer_get_debug_string(const odRenderer* renderer) {
	if (renderer == nullptr) {
		return "odRenderer{this=nullptr}";
	}

	return odDebugString_format(
		("odRenderer{this=%p, render_context_native=%p, vertex_shader=%u, "
		 "fragment_shader=%u, program=%u, vbo=%u, vao=%u}"),
		static_cast<const void*>(renderer),
		static_cast<const void*>(renderer->render_context_native),
		renderer->vertex_shader,
		renderer->fragment_shader,
		renderer->program,
		renderer->vbo,
		renderer->vao
	);
}
bool odGl_check_ok(odLogContext logger) {
	bool ok = true;
	GLenum gl_error = GL_NO_ERROR;

	for (gl_error = glGetError(); gl_error != GL_NO_ERROR; gl_error = glGetError()) {
		if (OD_BUILD_LOG) {
			odLog_log(
				logger,
				OD_LOG_LEVEL_ERROR,
				"OpenGL error, gl_error=%u, message=%s",
				gl_error,
				gluErrorString(gl_error));
		}
		ok = false;
	}

	OD_MAYBE_UNUSED(logger);

	return ok;
}
bool odGl_check_shader_ok(odLogContext logger, GLuint shader) {
	if (!odGl_check_ok(logger)) {
		return false;
	}

	GLint compile_status = GL_FALSE;

	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);

	if (compile_status == GL_FALSE) {
		if (OD_BUILD_LOG) {
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
		}
		return false;
	}

	OD_MAYBE_UNUSED(logger);

	return true;
}
bool odGl_check_program_ok(odLogContext logger, GLuint program) {
	if (!odGl_check_ok(logger)) {
		return false;
	}

	GLint link_status = GL_FALSE;

	glGetProgramiv(program, GL_LINK_STATUS, &link_status);

	if (link_status == GL_FALSE) {
		if (OD_BUILD_LOG) {
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
		}
		return false;
	}

	OD_MAYBE_UNUSED(logger);

	return true;
}
bool odRenderer_init(odRenderer* renderer, void* render_context_native) {
	OD_DEBUG("renderer=%s", odRenderer_get_debug_string(renderer));

	if (!OD_DEBUG_CHECK(renderer != nullptr)
		|| !OD_CHECK(render_context_native != nullptr)
		|| !OD_CHECK(SDL_GL_GetCurrentContext() == render_context_native)) {
		return false;
	}

	odRenderer_destroy(renderer);

	renderer->render_context_native = render_context_native;

	if (!OD_CHECK(odRenderer_glew_init())) {
		return false;
	}

	OD_TRACE("configuring opengl context, renderer=%s", odRenderer_get_debug_string(renderer));

	if (OD_BUILD_DEBUG) {
		// issue: requires opengl 4.1 but we ask for a 3.2 context
		glEnable(GL_DEBUG_OUTPUT);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_DEPTH_TEST);

	glDisable(GL_CULL_FACE);

	if (!odGl_check_ok(OD_LOGGER())) {
		OD_ERROR("OpenGL error when configuring opengl context, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	OD_TRACE("creating vertex buffer, renderer=%s", odRenderer_get_debug_string(renderer));
	glGenBuffers(1, &renderer->vbo);

	if (!odGl_check_ok(OD_LOGGER())) {
		OD_ERROR("OpenGL error when creating vertex buffer, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	OD_TRACE("creating vertex array, renderer=%s", odRenderer_get_debug_string(renderer));
	glGenVertexArrays(1, &renderer->vao);

	if (!odGl_check_ok(OD_LOGGER())) {
		OD_ERROR("OpenGL error when creating vertex array, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	OD_TRACE("creating shader program, renderer=%s", odRenderer_get_debug_string(renderer));
	
	static const GLchar* vertex_shader_ptr = static_cast<const GLchar*>(odRenderer_vertex_shader);
	static const GLint vertex_shader_size = sizeof(odRenderer_vertex_shader);
	renderer->vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(renderer->vertex_shader, 1, &vertex_shader_ptr, &vertex_shader_size);
	glCompileShader(renderer->vertex_shader);

	if (!odGl_check_ok(OD_LOGGER()) || !odGl_check_shader_ok(OD_LOGGER(), renderer->vertex_shader)) {
		OD_ERROR("OpenGL error when creating vertex shader, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	static const GLchar* fragment_shader_ptr = static_cast<const GLchar*>(odRenderer_fragment_shader);
	static const GLint fragment_shader_size = sizeof(odRenderer_fragment_shader);
	renderer->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(renderer->fragment_shader, 1, &fragment_shader_ptr, &fragment_shader_size);
	glCompileShader(renderer->fragment_shader);

	if (!odGl_check_ok(OD_LOGGER()) || !odGl_check_shader_ok(OD_LOGGER(), renderer->fragment_shader)) {
		OD_ERROR("OpenGL error when creating fragment shader, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	renderer->program = glCreateProgram();
	glAttachShader(renderer->program, renderer->vertex_shader);
	glAttachShader(renderer->program, renderer->fragment_shader);

	glLinkProgram(renderer->program);

	if (!odGl_check_ok(OD_LOGGER()) || !odGl_check_program_ok(OD_LOGGER(), renderer->program)) {
		OD_ERROR("OpenGL error when creating program, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	OD_TRACE("configuring shader attributes, renderer=%s", odRenderer_get_debug_string(renderer));

	glBindAttribLocation(renderer->program, 0, "src_pos");
	glBindAttribLocation(renderer->program, 1, "src_col");
	glBindAttribLocation(renderer->program, 2, "src_uv");

	GLuint src_pos_attrib = static_cast<GLuint>(glGetAttribLocation(renderer->program, "src_pos"));
	GLuint src_col_attrib = static_cast<GLuint>(glGetAttribLocation(renderer->program, "src_col"));
	GLuint src_uv_attrib = static_cast<GLuint>(glGetAttribLocation(renderer->program, "src_uv"));

	OD_TRACE(
		"renderer=%s, src_pos_attrib=%u, src_col_attrib=%u, src_uv_attrib=%u",
		odRenderer_get_debug_string(renderer), src_pos_attrib, src_col_attrib, src_uv_attrib);

	{
		odRendererScope renderer_scope{renderer};

		glEnableVertexAttribArray(src_pos_attrib);
		glEnableVertexAttribArray(src_col_attrib);
		glEnableVertexAttribArray(src_uv_attrib);

		const GLvoid* offset = static_cast<const GLvoid*>(nullptr);
		glVertexAttribPointer(src_pos_attrib, 3, GL_INT, GL_FALSE, sizeof(odVertex), offset);
		offset = static_cast<const GLvoid*>(static_cast<const GLchar*>(offset) + (3 * sizeof(GLint)));

		glVertexAttribPointer(src_col_attrib, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(odVertex), offset);
		offset = static_cast<const GLvoid*>(static_cast<const GLchar*>(offset) + (4 * sizeof(GLubyte)));

		glVertexAttribPointer(src_uv_attrib, 2, GL_INT, GL_FALSE, sizeof(odVertex), offset);
		offset = static_cast<const GLvoid*>(static_cast<const GLchar*>(offset) + (2 * sizeof(GLint)));
	}

	if (!odGl_check_ok(OD_LOGGER())) {
		OD_ERROR("OpenGL error when configuring shader attributes, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	if (!odGl_check_ok(OD_LOGGER())) {
		OD_ERROR("OpenGL error when configuring opengl context, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	return true;
}
void odRenderer_destroy(odRenderer* renderer) {
	OD_DEBUG("renderer=%s", odRenderer_get_debug_string(renderer));

	if (!OD_DEBUG_CHECK(renderer != nullptr)) {
		return;
	}

	bool context_current = (
		(renderer->render_context_native != nullptr) && OD_CHECK(SDL_GL_GetCurrentContext() == renderer->render_context_native));

	if (context_current && (renderer->fragment_shader != 0)) {
		glDetachShader(renderer->program, renderer->fragment_shader);
		glDeleteShader(renderer->fragment_shader);
	}
	renderer->fragment_shader = 0;

	if (context_current && (renderer->vertex_shader != 0)) {
		glDetachShader(renderer->program, renderer->vertex_shader);
		glDeleteShader(renderer->vertex_shader);
	}
	renderer->vertex_shader = 0;

	if (context_current && (renderer->program != 0)) {
		glDeleteProgram(renderer->program);
	}
	renderer->program = 0;

	if (context_current && (renderer->vao != 0)) {
		glDeleteVertexArrays(1, &renderer->vao);
	}
	renderer->vao = 0;

	if (context_current && (renderer->vbo != 0)) {
		glDeleteBuffers(1, &renderer->vbo);
	}
	renderer->vbo = 0;

	renderer->render_context_native = nullptr;
}
bool odRenderer_flush(odRenderer* renderer) {
	if (!OD_DEBUG_CHECK(odRenderer_get_valid(renderer))
		|| !OD_CHECK(SDL_GL_GetCurrentContext() == renderer->render_context_native)) {
		return false;
	}

	glFlush();

	if (!odGl_check_ok(OD_LOGGER())) {
		return false;
	}

	return true;
}
bool odRenderer_clear(odRenderer* renderer, odColor color, odRenderTexture* opt_render_texture) {
	if (!OD_DEBUG_CHECK(odRenderer_get_valid(renderer))
		|| !OD_CHECK(SDL_GL_GetCurrentContext() == renderer->render_context_native)
		|| !OD_DEBUG_CHECK((opt_render_texture == nullptr) || odRenderTexture_get_valid(opt_render_texture))) {
		return false;
	}

	odRendererScope renderer_scope{renderer};
	glBindFramebuffer(GL_FRAMEBUFFER, (opt_render_texture != nullptr) ? opt_render_texture->fbo : 0);

	glClearColor(
		static_cast<GLfloat>(color.r) / 255.0f,
		static_cast<GLfloat>(color.g) / 255.0f,
		static_cast<GLfloat>(color.b) / 255.0f,
		static_cast<GLfloat>(color.a) / 255.0f
	);
	glClear(GL_COLOR_BUFFER_BIT);


	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_DEPTH_BUFFER_BIT);

	if (!odGl_check_ok(OD_LOGGER())) {
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}
bool odRenderer_draw_vertices(odRenderer* renderer, odRenderState state, const odVertex* vertices, int32_t vertices_count) {
	if (!OD_DEBUG_CHECK(odRenderer_get_valid(renderer))
		|| !OD_DEBUG_CHECK(vertices != nullptr)
		|| !OD_DEBUG_CHECK(vertices_count >= 0)
		|| !OD_DEBUG_CHECK((state.src_texture == nullptr) || odTexture_get_valid(state.src_texture))
		|| !OD_DEBUG_CHECK((state.render_texture == nullptr) || odRenderTexture_get_valid(state.render_texture))
		|| !OD_CHECK(SDL_GL_GetCurrentContext() == renderer->render_context_native)) {
		return false;
	}

	int32_t texture_width = 0;
	int32_t texture_height = 0;
	if (state.src_texture != nullptr) {
		odTexture_get_size(state.src_texture, &texture_width, &texture_height);
	}
	GLfloat texture_scale_x = 1.0f / static_cast<GLfloat>(texture_width ? texture_width : 1);
	GLfloat texture_scale_y = 1.0f / static_cast<GLfloat>(texture_height ? texture_height : 1);

	odRendererScope renderer_scope{renderer};
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, (state.src_texture != nullptr) ? state.src_texture->texture : 0);
	glBindFramebuffer(GL_FRAMEBUFFER, (state.render_texture != nullptr) ? state.render_texture->fbo : 0);

	GLuint view_uniform = static_cast<GLuint>(glGetUniformLocation(renderer->program, "view"));
	GLuint projection_uniform = static_cast<GLuint>(glGetUniformLocation(renderer->program, "projection"));
	GLuint uv_scale_uniform = static_cast<GLuint>(glGetUniformLocation(renderer->program, "uv_scale"));

	glUniformMatrix4fv(view_uniform, 1, false, state.view.matrix);
	glUniformMatrix4fv(projection_uniform, 1, false, state.projection.matrix);
	glUniform2f(uv_scale_uniform, texture_scale_x, texture_scale_y);

	glViewport(state.viewport.x, state.viewport.y, state.viewport.width, state.viewport.height);

	glBufferData(
		GL_ARRAY_BUFFER,
		static_cast<GLsizeiptr>(static_cast<size_t>(vertices_count) * sizeof(odVertex)),
		vertices,
		GL_DYNAMIC_DRAW);

	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices_count));

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (!odGl_check_ok(OD_LOGGER())) {
		OD_ERROR("OpenGL error when drawing, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	return true;
}
bool odRenderer_draw_texture(odRenderer* renderer, odRenderState state, const odBounds *opt_texture_bounds) {
	if (!OD_DEBUG_CHECK(odRenderer_get_valid(renderer))
		|| !OD_DEBUG_CHECK(renderer != nullptr)
		|| !OD_DEBUG_CHECK(state.src_texture != nullptr)
		|| !OD_DEBUG_CHECK((state.render_texture == nullptr) || odRenderTexture_get_valid(state.render_texture))
		|| !OD_CHECK(SDL_GL_GetCurrentContext() == renderer->render_context_native)) {
		return false;
	}

	odBounds bounds{};
	if (opt_texture_bounds != nullptr) {
		bounds = *opt_texture_bounds;
	} else {
		odTexture_get_size(state.src_texture, &bounds.width, &bounds.height);
	}

	/* Bounds triangle index positions (assumes front faces are counter-clockwise):
		 0   2,3
		1,4   5
	*/
	const int32_t vertices_count = 6;
	const odVertex vertices[vertices_count] = {
		{bounds.x, bounds.y, 0,
		 0xff, 0xff, 0xff, 0xff,
		 0, 0},
		{bounds.x, bounds.y + bounds.height, 0,
		 0xff, 0xff, 0xff, 0xff,
		 0, bounds.y + bounds.height},
		{bounds.x + bounds.width, bounds.y, 0,
		 0xff, 0xff, 0xff, 0xff,
		 bounds.x + bounds.width, 0},

		{bounds.x + bounds.width, bounds.y, 0,
		 0xff, 0xff, 0xff, 0xff,
		 bounds.x + bounds.width, 0},
		{bounds.x, bounds.y + bounds.height, 0,
		 0xff, 0xff, 0xff, 0xff,
		 0, bounds.y + bounds.height},
		{bounds.x + bounds.width, bounds.y + bounds.height, 0,
		 0xff, 0xff, 0xff, 0xff,
		 bounds.x + bounds.width, bounds.y + bounds.height},
	};

	return odRenderer_draw_vertices(renderer, state, vertices, vertices_count);
}

static void odRenderer_unbind() {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

static void odRenderer_bind(odRenderer* renderer) {
	odRenderer_unbind();

	if (!OD_DEBUG_CHECK(odRenderer_get_valid(renderer))
		|| !OD_CHECK(SDL_GL_GetCurrentContext() == renderer->render_context_native)) {
		return;
	}

	glUseProgram(renderer->program);
	glBindVertexArray(renderer->vao);
	glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
}

odRenderer::odRenderer()
	: render_context_native{nullptr}, vbo{0}, vao{0}, vertex_shader{0}, fragment_shader{0}, program{0} {
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

odRendererScope::odRendererScope(odRenderer* renderer) {
	odRenderer_bind(renderer);
}
odRendererScope::~odRendererScope() {
	odRenderer_unbind();
}
