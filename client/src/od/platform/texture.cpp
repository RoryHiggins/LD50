#include <od/platform/texture.hpp>

#include <od/core/debug.h>
#include <od/core/primitive.h>
#include <od/core/type.hpp>
#include <od/platform/image.h>
#include <od/platform/gl.h>

const struct odType* odTexture_get_type_constructor() {
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
bool odTexture_get_valid(const odTexture* texture) {
	if ((texture == nullptr)
		|| (texture->render_context_native == nullptr)
		|| (texture->texture == 0)) {
		return false;
	}

	return true;
}
const char* odTexture_get_debug_string(const odTexture* texture) {
	if (texture == nullptr) {
		return "odTexture{this=nullptr}";
	}

	return odDebugString_format(
		"odTexture{this=%p, render_context_native=%p, texture=%u}",
		static_cast<const void*>(texture),
		static_cast<const void*>(texture->render_context_native),
		texture->texture
	);
}
bool odTexture_init(odTexture* texture, void* render_context_native, const odColor* opt_pixels, int32_t width, int32_t height) {
	if (!OD_DEBUG_CHECK(texture != nullptr)
		|| !OD_DEBUG_CHECK(width >= 0)
		|| !OD_DEBUG_CHECK(height >= 0)
		|| !OD_CHECK(render_context_native != nullptr)
		|| !OD_CHECK(SDL_GL_GetCurrentContext() == render_context_native)) {
		return false;
	}

	odTexture_destroy(texture);

	texture->render_context_native = render_context_native;

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

	glBindTexture(GL_TEXTURE_2D, 0);

	if (!odGl_check_ok(OD_LOGGER())) {
		OD_ERROR("OpenGL error creating texture, texture=%s", odTexture_get_debug_string(texture));
		return false;
	}

	return true;
}
bool odTexture_init_blank(struct odTexture* texture, void* render_context_native) {
	const odColor default_texture = {0xFF, 0xFF, 0xFF,0xFF};
	return odTexture_init(texture, render_context_native, &default_texture, 1, 1);
}
void odTexture_destroy(odTexture* texture) {
	if (!OD_DEBUG_CHECK(texture != nullptr)) {
		return;
	}

	bool context_current = (
		(texture->render_context_native != nullptr) && OD_CHECK(SDL_GL_GetCurrentContext() == texture->render_context_native));

	if (context_current && (texture->texture != 0)) {
		glDeleteTextures(1, &texture->texture);
	}
	texture->texture = 0;

	texture->render_context_native = nullptr;
}
void odTexture_get_size(const odTexture* texture, int32_t* out_opt_width, int32_t* out_opt_height) {
	int32_t unused;
	out_opt_width = (out_opt_width != nullptr) ? out_opt_width : &unused;
	out_opt_height = (out_opt_height != nullptr) ? out_opt_height : &unused;

	*out_opt_width = 0;
	*out_opt_height = 0;

	if (!OD_DEBUG_CHECK(texture != nullptr)
		|| !OD_DEBUG_CHECK(texture->texture != 0)
		|| !OD_CHECK(SDL_GL_GetCurrentContext() == texture->render_context_native)) {
		return;
	}

	int width = 0;
	int height = 0;
	glBindTexture(GL_TEXTURE_2D, texture->texture);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, /*mipLevel*/ 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, /*mipLevel*/ 0, GL_TEXTURE_HEIGHT, &height);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (!odGl_check_ok(OD_LOGGER())) {
		OD_ERROR("OpenGL error creating texture, texture=%s", odTexture_get_debug_string(texture));
		return;
	}

	*out_opt_width = static_cast<int32_t>(width);
	*out_opt_height = static_cast<int32_t>(height);
}

odTexture::odTexture()
	: render_context_native{nullptr}, texture{0} {
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
