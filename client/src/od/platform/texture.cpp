#include <od/platform/texture.hpp>

#include <cstring>

#include <od/core/debug.h>
#include <od/core/type.hpp>
#include <od/core/array.hpp>
#include <od/core/color.h>
#include <od/platform/window.hpp>
#include <od/platform/gl.h>

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

