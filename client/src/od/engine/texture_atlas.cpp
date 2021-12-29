#include <od/engine/texture_atlas.hpp>

#include <od/core/debug.h>
#include <od/core/bounds.h>

struct odAtlasRegion {
	odBounds2f bounds;
};

bool odAtlas_init(odAtlas* atlas) {
	if (!OD_CHECK(atlas != nullptr)) {
		return false;
	}

	odAtlas_destroy(atlas);

	if (!OD_CHECK(odImage_init(&atlas->image, 0, 0))) {
		return false;
	}
	if (!OD_CHECK(odTrivialArray_init(&atlas->regions))) {
		return false;
	}
	if (!OD_CHECK(odTrivialArray_init(&atlas->free_regions))) {
		return false;
	}

	return true;
}
void odAtlas_destroy(odAtlas* atlas) {
	if (!OD_CHECK(atlas != nullptr)) {
		return;
	}

	odImage_destroy(&atlas->image);
	odTrivialArray_destroy(&atlas->regions);
	odTrivialArray_destroy(&atlas->free_regions);
}
// int32_t odAtlas_get_count(const odAtlas* atlas);
// const odBounds2f* odAtlas_get(const odAtlas* atlas, odAtlasRegionId region_id);
// bool odAtlas_set(odAtlas* atlas, odAtlasRegionId region_id,
// 				 int32_t width, int32_t height, const odColorRGBA32* pixels);

template struct odTrivialArrayT<odAtlasRegion>;
