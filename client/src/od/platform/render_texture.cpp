#include <od/platform/render_texture.hpp>

#include <od/core/debug.h>
#include <od/core/primitive.h>
#include <od/core/array.hpp>
#include <od/core/type.hpp>
#include <od/platform/gl.h>
#include <od/platform/texture.hpp>

const struct odType* odRenderTexture_get_type_constructor() {
	return odType_get<odRenderTexture>();
}
void odRenderTexture_swap(odRenderTexture* render_texture1, odRenderTexture* render_texture2) {
	if (!OD_DEBUG_CHECK(render_texture1 != nullptr)
		|| !OD_DEBUG_CHECK(render_texture2 != nullptr)) {
		return;
	}

	odTexture render_texture_swap;
	memcpy(static_cast<void*>(&render_texture_swap), static_cast<void*>(render_texture1), sizeof(odTexture));
	memcpy(static_cast<void*>(render_texture1), static_cast<void*>(render_texture2), sizeof(odTexture));
	memcpy(static_cast<void*>(render_texture2), static_cast<void*>(&render_texture_swap), sizeof(odTexture));
}
bool odRenderTexture_get_valid(const odRenderTexture* render_texture) {
	if ((render_texture == nullptr)
		|| (!odTexture_get_valid(&render_texture->texture))
		|| (render_texture->fbo == 0)) {
		return false;
	}

	return true;
}
const char* odRenderTexture_get_debug_string(const odRenderTexture* render_texture) {
	if (render_texture == nullptr) {
		return "odTexture{this=nullptr}";
	}

	return odDebugString_format(
		"odTexture{this=%p, texture=%s, fbo=%u}",
		static_cast<const void*>(render_texture),
		odTexture_get_debug_string(&render_texture->texture),
		render_texture->fbo
	);
}
bool odRenderTexture_init(odRenderTexture* render_texture, void* render_context_native, int32_t width, int32_t height) {
	if (!OD_DEBUG_CHECK(render_texture != nullptr)
		|| !OD_DEBUG_CHECK(width >= 0)
		|| !OD_DEBUG_CHECK(height >= 0)
		|| !OD_CHECK(render_context_native != nullptr)
		|| !OD_CHECK(SDL_GL_GetCurrentContext() == render_context_native)) {
		return false;
	}

	odRenderTexture_destroy(render_texture);

	// allocate
	int32_t texture_pixels_count = width * height;
	odArray texture_pixels_buffer{odType_get<odColor>()};
	if (!OD_CHECK(odArray_set_count(&texture_pixels_buffer, texture_pixels_count))) {
		return false;
	}

	if (!OD_CHECK(odTexture_init(&render_texture->texture, render_context_native, nullptr, width, height))) {
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
	if (!OD_DEBUG_CHECK(render_texture != nullptr)) {
		return;
	}

	if (render_texture->fbo != 0) {
		glDeleteFramebuffers(1, &render_texture->fbo);

		render_texture->fbo = 0;
	}

	odTexture_destroy(&render_texture->texture);
}
struct odTexture* odRenderTexture_get_texture(odRenderTexture* render_texture) {
	if (!OD_DEBUG_CHECK(render_texture != nullptr)
		|| !OD_DEBUG_CHECK(render_texture->fbo != 0)) {
		return nullptr;
	}

	return &render_texture->texture;
}
const struct odTexture* odRenderTexture_get_texture_const(const odRenderTexture* render_texture) {
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
