#include <od/engine/texture_atlas.hpp>

#include <cstring>

#include <od/core/debug.h>

#define OD_ATLAS_WIDTH 4096

static bool odAtlasRegion_check_valid(const odAtlasRegion* region) {
	if (!OD_CHECK(region != nullptr)
		|| !OD_CHECK(odBounds2f_is_integral(&region->bounds))) {
		return false;
	}

	return true;
}
static bool odAtlasRegion_can_fit(const odAtlasRegion* region, int32_t width, int32_t height) {
	if (!OD_CHECK(odAtlasRegion_check_valid(region))
		|| !OD_CHECK(width >= 0)
		|| !OD_CHECK(height >= 0)) {
		return false;
	}

	if ((static_cast<int32_t>(odBounds2f_get_width(&region->bounds)) > width)
		|| (static_cast<int32_t>(odBounds2f_get_height(&region->bounds)) > height)) {
		return false;
	}

	return true;
}

static OD_NO_DISCARD bool odAtlas_ensure_count(odAtlas* atlas, int32_t min_count) {
	if (!OD_CHECK(atlas != nullptr)
		|| !OD_CHECK(min_count >= 0)) {
		return false;
	}

	if (!OD_CHECK(atlas->regions.ensure_count(min_count))) {
		return false;
	}

	return true;
}
void odAtlas_init(odAtlas* atlas) {
	if (!OD_CHECK(atlas != nullptr)) {
		return;
	}

	odAtlas_destroy(atlas);
}
void odAtlas_destroy(odAtlas* atlas) {
	if (!OD_CHECK(atlas != nullptr)) {
		return;
	}

	odTrivialArray_destroy(&atlas->regions);
	odTrivialArray_destroy(&atlas->free_regions);
	odImage_destroy(&atlas->image);
}
void odAtlas_swap(odAtlas* atlas1, odAtlas* atlas2) {
	if (!OD_CHECK(atlas1 != nullptr)
		|| !OD_CHECK(atlas2 != nullptr)) {
		return;
	}

	odImage_swap(&atlas1->image, &atlas2->image);
	odTrivialArray_swap(&atlas1->regions, &atlas2->regions);
	odTrivialArray_swap(&atlas1->free_regions, &atlas2->free_regions);
}
int32_t odAtlas_get_count(const odAtlas* atlas) {
	if (!OD_CHECK(atlas != nullptr)) {
		return 0;
	}

	return atlas->regions.get_count();
}
const odBounds2f* odAtlas_get(const odAtlas* atlas, odAtlasRegionId region_id) {
	if (!OD_CHECK(atlas != nullptr)
		|| !OD_CHECK((region_id >= 0) && (region_id < atlas->regions.get_count()))) {
		return nullptr;
	}

	const odAtlasRegion* region = atlas->regions.get(region_id);
	if (!OD_CHECK(region != nullptr)) {
		return nullptr;
	}

	return &region->bounds;
}
// bool odAtlas_set(odAtlas* atlas, odAtlasRegionId region_id,
// 				 int32_t width, int32_t height, const odColorRGBA32* pixels);

odAtlas::odAtlas()
: image{}, regions{}, free_regions{} {
}
odAtlas::odAtlas(odAtlas&& other) {
	odAtlas_swap(this, &other);
}
odAtlas& odAtlas::operator=(odAtlas&& other) {
	odAtlas_swap(this, &other);
	return *this;
}
odAtlas::~odAtlas() {
	odAtlas_destroy(this);
}

template struct odTrivialArrayT<odAtlasRegion>;
