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

	if (!OD_CHECK(odWindowResource_init(texture, window))) {
		return false;
	}

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
	const odColor default_texture = *odColor_get_white();
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

	odWindowResource_destroy(texture);
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
	: odWindowResource{}, texture{0} {
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
