#include <od/engine/texture_atlas.hpp>

#include <cstring>

#include <od/core/debug.h>
#include <od/core/color.h>
#include <od/platform/texture.hpp>
#include <od/engine/atlas.hpp>


bool odTextureAtlas_init(odTextureAtlas* atlas, odWindow* window) {
	if (!OD_CHECK(atlas != nullptr)
		|| !OD_CHECK(odWindow_check_valid(window))) {
		return false;
	}

	odTextureAtlas_destroy(atlas);

	odColor white = *odColor_get_white();
	if (!OD_CHECK(odTexture_init(&atlas->texture, window, &white, 1, 1))) {
		return false;
	}

	odAtlas_init(&atlas->atlas);

	return true;
}
void odTextureAtlas_destroy(odTextureAtlas* atlas) {
	if (!OD_CHECK(atlas != nullptr)) {
		return;
	}

	odTexture_destroy(&atlas->texture);
	odAtlas_destroy(&atlas->atlas);
}
void odTextureAtlas_swap(odTextureAtlas* atlas1, odTextureAtlas* atlas2) {
	odTexture_swap(&atlas1->texture, &atlas2->texture);
	odAtlas_swap(&atlas1->atlas, &atlas2->atlas);
}
bool odTextureAtlas_check_valid(const odTextureAtlas* atlas) {
	if (!OD_CHECK(atlas != nullptr)
		|| !OD_CHECK(odTexture_check_valid(&atlas->texture))
		|| !OD_CHECK(odAtlas_check_valid(&atlas->atlas))) {
		return false;
	}

	return true;
}
const odTexture* odTextureAtlas_get_texture_const(const odTextureAtlas* atlas) {
	if (!OD_CHECK(odTextureAtlas_check_valid(atlas))) {
		return nullptr;
	}

	return &atlas->texture;
}
const odAtlas* odTextureAtlas_get_atlas_const(const odTextureAtlas* atlas) {
	if (!OD_CHECK(odTextureAtlas_check_valid(atlas))) {
		return nullptr;
	}

	return &atlas->atlas;
}
int32_t odTextureAtlas_get_width(const odTextureAtlas* atlas) {
	if (!OD_CHECK(odTextureAtlas_check_valid(atlas))) {
		return 0;
	}

	return odAtlas_get_width(&atlas->atlas);
}
int32_t odTextureAtlas_get_height(const odTextureAtlas* atlas) {
	if (!OD_CHECK(odTextureAtlas_check_valid(atlas))) {
		return 0;
	}

	return odAtlas_get_height(&atlas->atlas);
}
int32_t odTextureAtlas_get_count(const odTextureAtlas* atlas) {
	if (!OD_CHECK(odTextureAtlas_check_valid(atlas))) {
		return 0;
	}

	return odAtlas_get_count(&atlas->atlas);
}
const odBounds* odTextureAtlas_get_region_bounds(const odTextureAtlas* atlas, odAtlasRegionId region_id) {
	if (!OD_CHECK(odTextureAtlas_check_valid(atlas))) {
		return nullptr;
	}

	return odAtlas_get_region_bounds(&atlas->atlas, region_id);
}
static bool odTextureAtlas_update_texture(odTextureAtlas* atlas) {
	if (!OD_CHECK(odTextureAtlas_check_valid(atlas))) {
		return false;
	}

	odWindow* window = atlas->texture.window;
	if (!OD_CHECK(window != nullptr)) {
		return false;
	}

	const odColor* pixels = odAtlas_begin_const(&atlas->atlas);
	if (!OD_CHECK(pixels != nullptr)) {
		return false;
	}

	int32_t width = odAtlas_get_width(&atlas->atlas);
	int32_t height = odAtlas_get_height(&atlas->atlas);

	if (!OD_CHECK(odTexture_init(&atlas->texture, window, pixels, width, height))) {
		return false;
	}

	return true;
}
bool odTextureAtlas_set_region(odTextureAtlas* atlas, odAtlasRegionId region_id,
							   int32_t width, int32_t height, const odColor* src, int32_t src_image_width) {
	if (!OD_CHECK(odTextureAtlas_check_valid(atlas))) {
		return false;
	}

	if (!OD_CHECK(odAtlas_set_region(&atlas->atlas, region_id, width, height, src, src_image_width))) {
		return false;
	}

	if (!OD_CHECK(odTextureAtlas_update_texture(atlas))) {
		return false;
	}

	return true;
}
bool odTextureAtlas_reset_region(odTextureAtlas* atlas, odAtlasRegionId region_id) {
	if (!OD_CHECK(odTextureAtlas_check_valid(atlas))) {
		return false;
	}

	if (!OD_CHECK(odAtlas_reset_region(&atlas->atlas, region_id))) {
		return false;
	}

	if (!OD_CHECK(odTextureAtlas_update_texture(atlas))) {
		return false;
	}

	return true;
}

odTextureAtlas::odTextureAtlas()
: texture{}, atlas{} {
}
odTextureAtlas::odTextureAtlas(odTextureAtlas&& other)
: odTextureAtlas{} {
	odTextureAtlas_swap(this, &other);
}
odTextureAtlas& odTextureAtlas::operator=(odTextureAtlas&& other) {
	odTextureAtlas_swap(this, &other);
	return *this;
}
odTextureAtlas::~odTextureAtlas() {
	odTextureAtlas_destroy(this);
}
