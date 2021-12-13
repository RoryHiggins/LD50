#include <od/platform/rendering.hpp>

#include <cstring>

#include <od/platform/window.hpp>

#include <od/core/debug.h>
#include <od/core/color.h>
#include <od/core/vector.h>
#include <od/core/type.hpp>
#include <od/core/array.hpp>
#include <od/platform/vertex.h>
#include <od/platform/primitive.h>
#include <od/platform/window.hpp>

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1

#if OD_BUILD_EMSCRIPTEN
#include <GLES2/gl2platform.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else  // !OD_BUILD_EMSCRIPTEN

#include <GL/glew.h>
#endif  // #else  // !OD_BUILD_EMSCRIPTEN

#define OD_RENDERER_MESSAGE_BUFFER_SIZE 4096

struct odRendererScope {
	explicit odRendererScope(odRenderer* renderer);
	~odRendererScope();
};


/*https://www.khronos.org/registry/OpenGL/specs/gl/glspec21.pdf*/
/*https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.1.20.pdf*/
static const char odRenderer_vertex_shader[] = R"(
	// there is no model space: all inputs are required to be in entity_index space
	uniform mat4 view;
	uniform mat4 projection;
	uniform vec2 uv_scale;

	attribute vec4 src_pos;
	attribute vec4 src_col;
	attribute vec2 src_uv;

	varying vec4 col;
	varying vec2 uv;

	void main() {
		gl_Position = projection * view * vec4(src_pos.xyz, 1);
		col = src_col;
		uv = src_uv * uv_scale;
	}
)";
#if OD_BUILD_EMSCRIPTEN
static const char odRenderer_fragment_shader[] = R"(
	precision mediump float;
	uniform sampler2D src_texture;

	varying vec4 col;
	varying vec2 uv;

	void main() {
		gl_FragColor = texture2D(src_texture, uv).rgba * col;
	}
)";
#else   // !OD_BUILD_EMSCRIPTEN
static const char odRenderer_fragment_shader[] = R"(
	uniform sampler2D src_texture;

	varying vec4 col;
	varying vec2 uv;

	void main() {
		gl_FragColor = texture2D(src_texture, uv).rgba * col;
	}
)";
#endif

static GLchar odRenderer_message_buffer[OD_RENDERER_MESSAGE_BUFFER_SIZE] = {};

OD_NO_DISCARD static bool odGl_check_ok(odLogContext log_context) {
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
OD_NO_DISCARD static bool odGl_check_shader_ok(odLogContext log_context, GLuint shader) {
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
OD_NO_DISCARD static bool odGl_check_program_ok(odLogContext log_context, GLuint program) {
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

bool odRenderState_check_valid(const odRenderState* state) {
	if (!OD_CHECK(state != nullptr)
		|| !OD_CHECK(odMatrix4_check_valid(&state->view))
		|| !OD_CHECK(odMatrix4_check_valid(&state->projection))
		|| !OD_CHECK(odBounds_check_valid(&state->viewport))
		|| !OD_CHECK(odTexture_check_valid(state->src_texture))
		|| !OD_CHECK((state->opt_render_texture == nullptr) || odRenderTexture_check_valid(state->opt_render_texture))) {
		return false;
	}

	return true;
}

const odType* odTexture_get_type_constructor() {
	return odType_get<odTexture>();
}
void odTexture_swap(odTexture* texture1, odTexture* texture2) {
	if (!OD_DEBUG_CHECK(texture1 != nullptr)
		|| !OD_DEBUG_CHECK(texture2 != nullptr)) {
		return;
	}

	odTexture texture_swap;
	memcpy(static_cast<void*>(&texture_swap), static_cast<void*>(texture1), sizeof(odTexture));
	memcpy(static_cast<void*>(texture1), static_cast<void*>(texture2), sizeof(odTexture));
	memcpy(static_cast<void*>(texture2), static_cast<void*>(&texture_swap), sizeof(odTexture));
}
bool odTexture_check_valid(const odTexture* texture) {
	if (!OD_CHECK(texture != nullptr)
		|| !OD_CHECK(odWindow_check_valid(texture->window))
		|| !OD_CHECK(texture->texture > 0)
		|| !OD_CHECK(texture->width > 0)
		|| !OD_CHECK(texture->height > 0)) {
		return false;
	}

	return true;
}
const char* odTexture_get_debug_string(const odTexture* texture) {
	if (texture == nullptr) {
		return "null";
	}

	return odDebugString_format(
		"{\"texture\": %u}",
		texture->texture
	);
}
bool odTexture_init(odTexture* texture, odWindow* window,
					const odColor* opt_pixels, int32_t width, int32_t height) {
	if (!OD_CHECK(texture != nullptr)
		|| !OD_CHECK(odWindow_check_valid(window))
		|| !OD_CHECK(width >= 0)
		|| !OD_CHECK(height >= 0)) {
		return false;
	}

	odTexture_destroy(texture);

	texture->window = window;

	odWindowScope window_scope;
	if (!OD_CHECK(odWindowScope_bind(&window_scope, texture->window))) {
		return false;
	}

	glGenTextures(1, &texture->texture);
	glBindTexture(GL_TEXTURE_2D, texture->texture);
	glTexImage2D(
		GL_TEXTURE_2D,
		/*level*/ 0,
		/*internalformat*/ GL_RGBA,
		static_cast<GLsizei>(width),
		static_cast<GLsizei>(height),
		/*border*/ 0,
		/*format*/ GL_RGBA,
		GL_UNSIGNED_BYTE,
		opt_pixels
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	texture->width = width;
	texture->height = height;

	if (!odGl_check_ok(OD_LOG_GET_CONTEXT())) {
		OD_ERROR("OpenGL error creating texture, texture=%s", odTexture_get_debug_string(texture));
		return false;
	}

	return true;
}
bool odTexture_init_blank(odTexture* texture, odWindow* window) {
	const odColor default_texture = {0xFF, 0xFF, 0xFF,0xFF};
	return odTexture_init(texture, window, &default_texture, 1, 1);
}
void odTexture_destroy(odTexture* texture) {
	if (!OD_CHECK(texture != nullptr)) {
		return;
	}

	odWindowScope window_scope;
	if (odWindowScope_try_bind(&window_scope, texture->window)) {
		if (texture->texture != 0) {
			glDeleteTextures(1, &texture->texture);
		}
	}

	texture->height = 0;
	texture->width = 0;
	texture->texture = 0;

	texture->window = nullptr;
}
bool odTexture_get_size(const odTexture* texture, int32_t* out_opt_width, int32_t* out_opt_height) {
	int32_t unused;
	out_opt_width = (out_opt_width != nullptr) ? out_opt_width : &unused;
	out_opt_height = (out_opt_height != nullptr) ? out_opt_height : &unused;

	*out_opt_width = 0;
	*out_opt_height = 0;

	if (!OD_DEBUG_CHECK(odTexture_check_valid(texture))) {
		return false;
	}

	*out_opt_width = static_cast<int32_t>(texture->width);
	*out_opt_height = static_cast<int32_t>(texture->height);

	return true;
}

odTexture::odTexture()
	: window{nullptr}, texture{0} {
}
odTexture::odTexture(odTexture&& other) {
	odTexture_swap(this, &other);
}
odTexture& odTexture::operator=(odTexture&& other) {
	odTexture_swap(this, &other);
	return *this;
}
odTexture::~odTexture() {
	odTexture_destroy(this);
}


const odType* odRenderTexture_get_type_constructor() {
	return odType_get<odRenderTexture>();
}
void odRenderTexture_swap(odRenderTexture* render_texture1, odRenderTexture* render_texture2) {
	if (!OD_CHECK(render_texture1 != nullptr)
		|| !OD_CHECK(render_texture2 != nullptr)) {
		return;
	}

	odTexture render_texture_swap;
	memcpy(static_cast<void*>(&render_texture_swap), static_cast<void*>(render_texture1), sizeof(odTexture));
	memcpy(static_cast<void*>(render_texture1), static_cast<void*>(render_texture2), sizeof(odTexture));
	memcpy(static_cast<void*>(render_texture2), static_cast<void*>(&render_texture_swap), sizeof(odTexture));
}
bool odRenderTexture_check_valid(const odRenderTexture* render_texture) {
	if (!OD_CHECK(render_texture != nullptr)
		|| !OD_CHECK(odTexture_check_valid(&render_texture->texture))
		|| !OD_CHECK(render_texture->fbo > 0)) {
		return false;
	}

	return true;
}
const char* odRenderTexture_get_debug_string(const odRenderTexture* render_texture) {
	if (render_texture == nullptr) {
		return "null";
	}

	return odDebugString_format(
		"{\"texture\": %s, \"fbo\": %u}",
		odTexture_get_debug_string(&render_texture->texture),
		render_texture->fbo
	);
}
bool odRenderTexture_init(odRenderTexture* render_texture, odWindow* window, int32_t width, int32_t height) {
	if (!OD_CHECK(render_texture != nullptr)
		|| !OD_CHECK(odWindow_check_valid(window))
		|| !OD_CHECK(width >= 0)
		|| !OD_CHECK(height >= 0)) {
		return false;
	}

	odRenderTexture_destroy(render_texture);

	// allocate
	int32_t texture_pixels_count = width * height;
	odArray texture_pixels_buffer{odType_get<odColor>()};
	if (!OD_CHECK(odArray_set_count(&texture_pixels_buffer, texture_pixels_count))) {
		return false;
	}

	if (!OD_CHECK(odTexture_init(&render_texture->texture, window, nullptr, width, height))) {
		return false;
	}

	odWindowScope window_scope;
	if (!OD_CHECK(odWindowScope_bind(&window_scope, render_texture->texture.window))) {
		return false;
	}

	glGenFramebuffers(1, &render_texture->fbo);

	glBindTexture(GL_TEXTURE_2D, render_texture->texture.texture);
	glBindFramebuffer(GL_FRAMEBUFFER, render_texture->fbo);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, render_texture->texture.texture, 0);  

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}
void odRenderTexture_destroy(odRenderTexture* render_texture) {
	if (!OD_CHECK(render_texture != nullptr)) {
		return;
	}

	odWindowScope window_scope;
	if (odWindowScope_try_bind(&window_scope, render_texture->texture.window)) {
		if (render_texture->fbo != 0) {
			glDeleteFramebuffers(1, &render_texture->fbo);
		}
	}

	render_texture->fbo = 0;

	odTexture_destroy(&render_texture->texture);
}
odTexture* odRenderTexture_get_texture(odRenderTexture* render_texture) {
	if (!OD_DEBUG_CHECK(render_texture != nullptr)
		|| !OD_DEBUG_CHECK(render_texture->fbo != 0)) {
		return nullptr;
	}

	return &render_texture->texture;
}
const odTexture* odRenderTexture_get_texture_const(const odRenderTexture* render_texture) {
	return odRenderTexture_get_texture(const_cast<odRenderTexture*>(render_texture));
}

odRenderTexture::odRenderTexture()
	: texture{}, fbo{0} {
}
odRenderTexture::odRenderTexture(odRenderTexture&& other) {
	odRenderTexture_swap(this, &other);
}
odRenderTexture& odRenderTexture::operator=(odRenderTexture&& other) {
	odRenderTexture_swap(this, &other);
	return *this;
}
odRenderTexture::~odRenderTexture() {
	odRenderTexture_destroy(this);
}


const odType* odRenderer_get_type_constructor() {
	return odType_get<odRenderer>();
}
void odRenderer_swap(odRenderer* renderer1, odRenderer* renderer2) {
	if (!OD_CHECK(renderer1 != nullptr)
		|| !OD_CHECK(renderer2 != nullptr)) {
		return;
	}

	odRenderer renderer_swap;
	memcpy(static_cast<void*>(&renderer_swap), static_cast<void*>(renderer1), sizeof(odRenderer));
	memcpy(static_cast<void*>(renderer1), static_cast<void*>(renderer2), sizeof(odRenderer));
	memcpy(static_cast<void*>(renderer2), static_cast<void*>(&renderer_swap), sizeof(odRenderer));
}
bool odRenderer_check_valid(const odRenderer* renderer) {
	if (!OD_CHECK(renderer != nullptr)
		|| !OD_CHECK(odWindow_check_valid(renderer->window))
		|| !OD_CHECK(renderer->vbo > 0)
#if !OD_BUILD_EMSCRIPTEN
		|| !OD_CHECK(renderer->vao > 0)
#endif  // !OD_BUILD_EMSCRIPTEN
		|| !OD_CHECK(renderer->vertex_shader > 0)
		|| !OD_CHECK(renderer->fragment_shader > 0)
		|| !OD_CHECK(renderer->program > 0)) {
		return false;
	}

	return true;
}
const char* odRenderer_get_debug_string(const odRenderer* renderer) {
	if (renderer == nullptr) {
		return "null";
	}

	return odDebugString_format(
		("{\"vertex_shader\": %u, \"fragment_shader\": %u, \"program\": %u, \"vbo\": %u, \"vao\": %u}"),
		renderer->vertex_shader,
		renderer->fragment_shader,
		renderer->program,
		renderer->vbo,
		renderer->vao
	);
}
bool odRenderer_init(odRenderer* renderer, odWindow* window) {
	OD_DEBUG("renderer=%s", odRenderer_get_debug_string(renderer));

	if (!OD_CHECK(renderer != nullptr)
		|| !OD_CHECK(odWindow_check_valid(window))) {
		return false;
	}

	odRenderer_destroy(renderer);

	renderer->window = window;

	odWindowScope window_scope;
	if (!OD_CHECK(odWindowScope_bind(&window_scope, renderer->window))) {
		return false;
	}

	OD_TRACE("configuring opengl context, renderer=%s", odRenderer_get_debug_string(renderer));

#if !OD_BUILD_EMSCRIPTEN
	if (OD_BUILD_DEBUG) {
		// issue: requires opengl 4.1 but we ask for a 3.2 context
		glEnable(GL_DEBUG_OUTPUT);
	}
#endif  // !OD_BUILD_EMSCRIPTEN

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// unnecessary as we will sort geometry
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);

	if (!odGl_check_ok(OD_LOG_GET_CONTEXT())) {
		OD_ERROR("OpenGL error when configuring opengl context, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	OD_TRACE("creating vertex buffer, renderer=%s", odRenderer_get_debug_string(renderer));
	glGenBuffers(1, &renderer->vbo);

	if (!odGl_check_ok(OD_LOG_GET_CONTEXT())) {
		OD_ERROR("OpenGL error when creating vertex buffer, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

#if !OD_BUILD_EMSCRIPTEN
	OD_TRACE("creating vertex array, renderer=%s", odRenderer_get_debug_string(renderer));
	glGenVertexArrays(1, &renderer->vao);
#endif  // !OD_BUILD_EMSCRIPTEN

	if (!odGl_check_ok(OD_LOG_GET_CONTEXT())) {
		OD_ERROR("OpenGL error when creating vertex array, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	OD_TRACE("creating shader program, renderer=%s", odRenderer_get_debug_string(renderer));
	
	static const GLchar* vertex_shader_ptr = static_cast<const GLchar*>(odRenderer_vertex_shader);
	static const GLint vertex_shader_size = sizeof(odRenderer_vertex_shader);
	renderer->vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(renderer->vertex_shader, 1, &vertex_shader_ptr, &vertex_shader_size);
	glCompileShader(renderer->vertex_shader);

	if (!odGl_check_ok(OD_LOG_GET_CONTEXT()) || !odGl_check_shader_ok(OD_LOG_GET_CONTEXT(), renderer->vertex_shader)) {
		OD_ERROR("OpenGL error when creating vertex shader, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	static const GLchar* fragment_shader_ptr = static_cast<const GLchar*>(odRenderer_fragment_shader);
	static const GLint fragment_shader_size = sizeof(odRenderer_fragment_shader);
	renderer->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(renderer->fragment_shader, 1, &fragment_shader_ptr, &fragment_shader_size);
	glCompileShader(renderer->fragment_shader);

	if (!odGl_check_ok(OD_LOG_GET_CONTEXT()) || !odGl_check_shader_ok(OD_LOG_GET_CONTEXT(), renderer->fragment_shader)) {
		OD_ERROR("OpenGL error when creating fragment shader, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	renderer->program = glCreateProgram();
	glAttachShader(renderer->program, renderer->vertex_shader);
	glAttachShader(renderer->program, renderer->fragment_shader);

	glLinkProgram(renderer->program);

	if (!odGl_check_ok(OD_LOG_GET_CONTEXT()) || !odGl_check_program_ok(OD_LOG_GET_CONTEXT(), renderer->program)) {
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

	if (!odGl_check_ok(OD_LOG_GET_CONTEXT())) {
		OD_ERROR("OpenGL error when binding shader attributes, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	OD_TRACE(
		"renderer=%s, src_pos_attrib=%u, src_col_attrib=%u, src_uv_attrib=%u",
		odRenderer_get_debug_string(renderer), src_pos_attrib, src_col_attrib, src_uv_attrib);

	{
		odRendererScope renderer_scope{renderer};

		glEnableVertexAttribArray(src_pos_attrib);
		glEnableVertexAttribArray(src_col_attrib);
		glEnableVertexAttribArray(src_uv_attrib);

		const GLvoid* offset = static_cast<const GLvoid*>(nullptr);
		glVertexAttribPointer(src_pos_attrib, 4, GL_FLOAT, GL_FALSE, sizeof(odVertex), offset);
		offset = static_cast<const GLvoid*>(static_cast<const GLchar*>(offset) + (sizeof(odVector4)));

		glVertexAttribPointer(src_col_attrib, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(odVertex), offset);
		offset = static_cast<const GLvoid*>(static_cast<const GLchar*>(offset) + (sizeof(odColor)));

		glVertexAttribPointer(src_uv_attrib, 2, GL_FLOAT, GL_FALSE, sizeof(odVertex), offset);
		offset = static_cast<const GLvoid*>(static_cast<const GLchar*>(offset) + (2 * sizeof(GLfloat)));
	}

	if (!odGl_check_ok(OD_LOG_GET_CONTEXT())) {
		OD_ERROR("OpenGL error when configuring shader attributes, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	return true;
}
void odRenderer_destroy(odRenderer* renderer) {
	OD_DEBUG("renderer=%s", odRenderer_get_debug_string(renderer));

	if (!OD_CHECK(renderer != nullptr)) {
		return;
	}

	odWindowScope window_scope;
	if (odWindowScope_try_bind(&window_scope, renderer->window)) {
		if (renderer->fragment_shader != 0) {
			glDetachShader(renderer->program, renderer->fragment_shader);
			glDeleteShader(renderer->fragment_shader);
		}

		if (renderer->vertex_shader != 0) {
			glDetachShader(renderer->program, renderer->vertex_shader);
			glDeleteShader(renderer->vertex_shader);
		}

		if (renderer->program != 0) {
			glDeleteProgram(renderer->program);
		}

		if (renderer->vao != 0) {
#if !OD_BUILD_EMSCRIPTEN
			glDeleteVertexArrays(1, &renderer->vao);
#endif  // !OD_BUILD_EMSCRIPTEN
		}

		if (renderer->vbo != 0) {
			glDeleteBuffers(1, &renderer->vbo);
		}
	}

	renderer->program = 0;
	renderer->fragment_shader = 0;
	renderer->vertex_shader = 0;
	renderer->vao = 0;
	renderer->vbo = 0;

	renderer->window = nullptr;
}
bool odRenderer_flush(odRenderer* renderer) {
	if (!OD_CHECK(odRenderer_check_valid(renderer))) {
		return false;
	}

	odWindowScope window_scope;
	if (!OD_CHECK(odWindowScope_bind(&window_scope, renderer->window))) {
		return false;
	}

	glFlush();

	if (!odGl_check_ok(OD_LOG_GET_CONTEXT())) {
		return false;
	}

	return true;
}
bool odRenderer_clear(odRenderer* renderer, odRenderState* state, const odColor* color) {
	if (!OD_CHECK(odRenderer_check_valid(renderer))
		|| !OD_CHECK(odRenderState_check_valid(state))
		|| !OD_CHECK(color != nullptr)) {
		return false;
	}

	odWindowScope window_scope;
	if (!OD_CHECK(odWindowScope_bind(&window_scope, renderer->window))) {
		return false;
	}

	odRendererScope renderer_scope{renderer};
	glBindFramebuffer(GL_FRAMEBUFFER, (state->opt_render_texture != nullptr) ? state->opt_render_texture->fbo : 0);

	glClearColor(
		static_cast<GLfloat>(color->r) / 255.0f,
		static_cast<GLfloat>(color->g) / 255.0f,
		static_cast<GLfloat>(color->b) / 255.0f,
		static_cast<GLfloat>(color->a) / 255.0f
	);
	glClear(GL_COLOR_BUFFER_BIT);


	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_DEPTH_BUFFER_BIT);

	if (!odGl_check_ok(OD_LOG_GET_CONTEXT())) {
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}
bool odRenderer_draw_vertices(odRenderer* renderer, odRenderState *state,
							  const odVertex* vertices, int32_t vertices_count) {
	if (!OD_CHECK(odRenderer_check_valid(renderer))
		|| !OD_CHECK(odRenderState_check_valid(state))
		|| !OD_CHECK((vertices_count == 0) || (vertices != nullptr))
		|| !OD_CHECK(vertices_count >= 0)) {
		return false;
	}

	if (vertices_count == 0) {
		return true;
	}

	if (OD_BUILD_DEBUG) {
		for (int32_t i = 0; i < vertices_count; i++) {
			if (!OD_DEBUG_CHECK(odVertex_check_valid(&vertices[i]))) {
				return false;
			}
		}
	}

	int32_t texture_width = 0;
	int32_t texture_height = 0;
	if (!OD_CHECK(odTexture_get_size(state->src_texture, &texture_width, &texture_height))) {
		return false;
	}
	GLfloat texture_scale_x = 1.0f / static_cast<GLfloat>(texture_width ? texture_width : 1);
	GLfloat texture_scale_y = 1.0f / static_cast<GLfloat>(texture_height ? texture_height : 1);

	odWindowScope window_scope;
	if (!OD_CHECK(odWindowScope_bind(&window_scope, renderer->window))) {
		return false;
	}

	odRendererScope renderer_scope{renderer};
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, state->src_texture->texture);
	glBindFramebuffer(GL_FRAMEBUFFER, (state->opt_render_texture != nullptr) ? state->opt_render_texture->fbo : 0);

	GLuint view_uniform = static_cast<GLuint>(glGetUniformLocation(renderer->program, "view"));
	GLuint projection_uniform = static_cast<GLuint>(glGetUniformLocation(renderer->program, "projection"));
	GLuint uv_scale_uniform = static_cast<GLuint>(glGetUniformLocation(renderer->program, "uv_scale"));

	glUniformMatrix4fv(view_uniform, 1, false, state->view.matrix);
	glUniformMatrix4fv(projection_uniform, 1, false, state->projection.matrix);
	glUniform2f(uv_scale_uniform, texture_scale_x, texture_scale_y);

	glViewport(
		static_cast<GLint>(state->viewport.x1),
		static_cast<GLint>(state->viewport.y1),
		static_cast<GLint>(odBounds_get_width(&state->viewport)),
		static_cast<GLint>(odBounds_get_height(&state->viewport)));

	glBufferData(
		GL_ARRAY_BUFFER,
		static_cast<GLsizeiptr>(static_cast<size_t>(vertices_count) * sizeof(odVertex)),
		vertices,
		GL_DYNAMIC_DRAW);

	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices_count));

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (!odGl_check_ok(OD_LOG_GET_CONTEXT())) {
		OD_ERROR("OpenGL error when drawing, renderer=%s", odRenderer_get_debug_string(renderer));
		return false;
	}

	return true;
}
bool odRenderer_draw_texture(odRenderer* renderer, odRenderState* state,
							 const odBounds* opt_src_bounds, const struct odMatrix4* opt_transform) {
	if (!OD_CHECK(odRenderer_check_valid(renderer))
		|| !OD_CHECK(renderer != nullptr)
		|| !OD_CHECK(odRenderState_check_valid(state))
		|| !OD_CHECK((opt_src_bounds == nullptr) || odBounds_check_valid(opt_src_bounds))
		|| !OD_CHECK((opt_transform == nullptr) || odMatrix4_check_valid(opt_transform))) {
		return false;
	}

	int32_t src_width = 0;
	int32_t src_height = 0;
	if (!OD_CHECK(odTexture_get_size(state->src_texture, &src_width, &src_height))) {
		return false;
	}

	odBounds src_bounds = {0.0f, 0.0f, static_cast<float>(src_width), static_cast<float>(src_height)};
	if (opt_src_bounds != nullptr) {
		src_bounds = *opt_src_bounds;
	}

	odMatrix4 transform{};
	odMatrix4_init(
		&transform,
		odBounds_get_width(&state->viewport),
		odBounds_get_height(&state->viewport),
		1.0f,
		0.0f,
		0.0f,
		0.0f
	);

	if (opt_transform != nullptr) {
		transform = *opt_transform;
	}

	odRectPrimitive rect{
		{0.0f, 0.0f, 1.0f, 1.0f},
		{src_bounds.x1, src_bounds.y1, src_bounds.x2, src_bounds.y2},
		*odColor_get_white(),
		0.0f,
	};
	odVertex vertices[OD_RECT_PRIMITIVE_VERTEX_COUNT]{};
	odRectPrimitive_get_vertices(&rect, vertices);

	for (int32_t i = 0; i < OD_RECT_PRIMITIVE_VERTEX_COUNT; i++) {
		odVertex_transform(vertices + i, &transform);
	}

	return odRenderer_draw_vertices(renderer, state, vertices, OD_RECT_PRIMITIVE_VERTEX_COUNT);
}

static void odRenderer_unbind() {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
#if !OD_BUILD_EMSCRIPTEN
	glBindVertexArray(0);
#endif  // !OD_BUILD_EMSCRIPTEN
	glUseProgram(0);
}

static void odRenderer_bind(odRenderer* renderer) {
	odWindowScope window_scope;
	if (!OD_CHECK(odWindowScope_bind(&window_scope, renderer->window))) {
		return;
	}

	odRenderer_unbind();

	if (!OD_CHECK(odRenderer_check_valid(renderer))) {
		return;
	}

	glUseProgram(renderer->program);
#if !OD_BUILD_EMSCRIPTEN
	glBindVertexArray(renderer->vao);
#endif  // !OD_BUILD_EMSCRIPTEN
	glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
}

odRenderer::odRenderer()
	: window{nullptr}, vbo{0}, vao{0}, vertex_shader{0}, fragment_shader{0}, program{0} {
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
