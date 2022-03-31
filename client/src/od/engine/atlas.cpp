#include <od/engine/atlas.hpp>

#include <cstring>

#include <od/core/debug.h>
#include <od/core/math.h>
#include <od/core/color.h>
#include <od/core/bounds.h>
#include <od/platform/image.hpp>

#define OD_ATLAS_REGION_ALLOCATE_MAX_FREE_REGIONS 2
#define OD_ATLAS_REGION_ID_INVALID -1

typedef int32_t odAtlasFreeRegionId;

static OD_NO_DISCARD bool
odAtlasRegion_check_valid(const odAtlasRegion* region);
static OD_NO_DISCARD bool
odAtlasRegion_has_space(const odAtlasRegion* region);
static OD_NO_DISCARD bool
odAtlasRegion_can_allocate(const odAtlasRegion* region, int32_t width, int32_t height);
static OD_NO_DISCARD bool
odAtlasRegion_allocate(
	const odAtlasRegion* region, odAtlasRegion* out_region, int32_t width, int32_t height,
	odAtlasRegion* out_free_regions, int32_t* out_free_regions_count);

static OD_NO_DISCARD bool
odAtlas_ensure_count(odAtlas* atlas, int32_t min_count);
static OD_NO_DISCARD odAtlasFreeRegionId
odAtlas_allocate_free_region(odAtlas* atlas, int32_t width, int32_t height);
static OD_NO_DISCARD odAtlasFreeRegionId
odAtlas_get_free_region(odAtlas* atlas, int32_t width, int32_t height);
static OD_NO_DISCARD bool
odAtlas_set_region_size(odAtlas* atlas, odAtlasRegionId region_id, int32_t width, int32_t height);

bool odAtlasRegion_check_valid(const odAtlasRegion* region) {
	if (!OD_CHECK(odBounds_check_valid(&region->bounds))) {
		return false;
	}

	return true;
}
bool odAtlasRegion_has_space(const odAtlasRegion* region) {
	if (!OD_CHECK(odBounds_check_valid(&region->bounds))) {
		return false;
	}

	if (!odBounds_has_area(&region->bounds)) {
		return false;
	}

	return true;
}
bool odAtlasRegion_can_allocate(const odAtlasRegion* region, int32_t width, int32_t height) {
	if (!OD_CHECK(odAtlasRegion_check_valid(region))
		|| !OD_CHECK(odInt32_fits_float(width))
		|| !OD_CHECK(width > 0)
		|| !OD_CHECK(odInt32_fits_float(height))
		|| !OD_CHECK(height > 0)) {
		return false;
	}

	if ((odBounds_get_width(&region->bounds) < static_cast<float>(width))
		|| (odBounds_get_height(&region->bounds) < static_cast<float>(height))) {
		return false;
	}

	return true;
}
bool odAtlasRegion_allocate(const odAtlasRegion* region, odAtlasRegion* out_region, int32_t width, int32_t height,
								   odAtlasRegion* out_free_regions, int32_t* out_free_regions_count) {
	if (!OD_CHECK(odAtlasRegion_check_valid(region))
		|| !OD_CHECK(odInt32_fits_float(width))
		|| !OD_CHECK(width > 0)
		|| !OD_CHECK(odInt32_fits_float(height))
		|| !OD_CHECK(height > 0)
		|| !OD_CHECK(out_region != nullptr)
		|| !OD_CHECK(!odAtlasRegion_has_space(out_region))
		|| !OD_CHECK(out_free_regions != nullptr)
		|| !OD_CHECK(out_free_regions_count != nullptr)
		|| !OD_CHECK(odAtlasRegion_can_allocate(region, width, height))) {
		return false;
	}

	/*
	we allocate the top-left corner, and split the remaining region into the
	remaining region to the right + the remaining region below.

	example alloc: 2x2 region from 4x4 region, A=allocated, B=right free, C=below free:
		AABB
		AABB
		CCCC
		CCCC
	*/

	float width_f = static_cast<float>(width);
	float height_f = static_cast<float>(height);

	*out_region = odAtlasRegion{odBounds{
		region->bounds.x1,
		region->bounds.y1,
		region->bounds.x1 + width_f,
		region->bounds.y1 + height_f,
	}};
	if (!OD_DEBUG_CHECK(odBounds_contains(&region->bounds, &out_region->bounds))) {
		return false;
	}

	odAtlasRegion right_free{odBounds{
		region->bounds.x1 + width_f,
		region->bounds.y1,
		region->bounds.x2,
		region->bounds.y1 + height_f,
	}};

	odAtlasRegion below_free{odBounds{
		region->bounds.x1,
		region->bounds.y1 + height_f,
		region->bounds.x2,
		region->bounds.y2,
	}};

	*out_free_regions_count = 0;
	if (odAtlasRegion_has_space(&right_free)) {
		if (!OD_DEBUG_CHECK(odBounds_contains(&region->bounds, &right_free.bounds))) {
			return false;
		}
		out_free_regions[*out_free_regions_count] = right_free;
		(*out_free_regions_count)++;
	}
	if (odAtlasRegion_has_space(&below_free)) {
		if (!OD_DEBUG_CHECK(odBounds_contains(&region->bounds, &below_free.bounds))) {
			return false;
		}
		out_free_regions[*out_free_regions_count] = below_free;
		(*out_free_regions_count)++;
	}

	return true;
}

bool odAtlas_ensure_count(odAtlas* atlas, int32_t min_count) {
	if (!OD_CHECK(atlas != nullptr)
		|| !OD_CHECK(min_count >= 0)) {
		return false;
	}

	if (!OD_CHECK(atlas->regions.ensure_count(min_count))) {
		return false;
	}

	return true;
}
odAtlasFreeRegionId odAtlas_allocate_free_region(odAtlas* atlas, int32_t width, int32_t height) {
	if (!OD_CHECK(atlas != nullptr)
		|| !OD_CHECK(odInt32_fits_float(width))
		|| !OD_CHECK(width > 0)
		|| !OD_CHECK(odInt32_fits_float(height))
		|| !OD_CHECK(height > 0)) {
		return OD_ATLAS_REGION_ID_INVALID;
	}

	int32_t max_width = width > atlas->image.width ? width : atlas->image.width;
	int32_t new_height = atlas->image.height + height;

	odAtlasRegion region{odBounds{
		0.0f,
		static_cast<float>(atlas->image.height),
		static_cast<float>(max_width),
		static_cast<float>(new_height)
	}};
	if (!OD_CHECK(odImage_resize(&atlas->image, max_width, new_height))) {
		return OD_ATLAS_REGION_ID_INVALID;
	}
	odAtlasFreeRegionId region_id = atlas->free_regions.get_count();
	if (!OD_CHECK(atlas->free_regions.push(region))) {
		return OD_ATLAS_REGION_ID_INVALID;
	}

	return region_id;
}
odAtlasFreeRegionId odAtlas_get_free_region(odAtlas* atlas, int32_t width, int32_t height) {
	if (!OD_CHECK(atlas != nullptr)
		|| !OD_CHECK(odInt32_fits_float(width))
		|| !OD_CHECK(width > 0)
		|| !OD_CHECK(odInt32_fits_float(height))
		|| !OD_CHECK(height > 0)) {
		return OD_ATLAS_REGION_ID_INVALID;
	}

	int32_t src_free_region_id = OD_ATLAS_REGION_ID_INVALID;

	int32_t free_regions_count = atlas->free_regions.get_count();
	for (int32_t i = 0; i < free_regions_count; i++) {
		const odAtlasRegion* free_region = atlas->free_regions.get(i);
		if (!OD_DEBUG_CHECK(free_region != nullptr)) {
			return OD_ATLAS_REGION_ID_INVALID;
		}

		if (odAtlasRegion_can_allocate(free_region, width, height)) {
			src_free_region_id = i;
			break;
		}
	}

	if (src_free_region_id == OD_ATLAS_REGION_ID_INVALID) {
		src_free_region_id = odAtlas_allocate_free_region(atlas, width, height);
	}

	return src_free_region_id;
}
bool odAtlas_set_region_size(odAtlas* atlas, odAtlasRegionId region_id, int32_t width, int32_t height) {
	if (!OD_CHECK(atlas != nullptr)
		|| !OD_CHECK(region_id >= 0)
		|| !OD_CHECK(odInt32_fits_float(width))
		|| !OD_CHECK(width > 0)
		|| !OD_CHECK(odInt32_fits_float(height))
		|| !OD_CHECK(height > 0)) {
		return false;
	}

	if (!OD_CHECK(odAtlas_reset_region(atlas, region_id))) {
		return false;
	}

	if ((width == 0) || (height == 0)) {
		return true;
	}

	odAtlasFreeRegionId src_free_region_id = odAtlas_get_free_region(atlas, width, height);
	if (!OD_CHECK(src_free_region_id != OD_ATLAS_REGION_ID_INVALID)) {
		return false;
	}

	const odAtlasRegion* src_free_region = atlas->free_regions.get(src_free_region_id);
	if (!OD_CHECK(src_free_region != nullptr)) {
		return false;
	}

	odAtlasRegion* dest_region = atlas->regions.get(region_id);
	if (!OD_CHECK(dest_region != nullptr)) {
		return false;
	}

	odAtlasRegion new_free_regions[OD_ATLAS_REGION_ALLOCATE_MAX_FREE_REGIONS]{};
	int32_t new_free_regions_count = 0;
	if (!OD_CHECK(odAtlasRegion_allocate(
		src_free_region,
		dest_region,
		width,
		height,
		new_free_regions,
		&new_free_regions_count))) {
		return false;
	}

	if (!OD_CHECK((new_free_regions_count >= 0)
		|| !OD_CHECK(new_free_regions_count <= OD_ATLAS_REGION_ALLOCATE_MAX_FREE_REGIONS))) {
		return false;
	}

	if (!OD_CHECK(atlas->free_regions.extend(new_free_regions, new_free_regions_count))) {
		return false;
	}

	if (!OD_CHECK(atlas->free_regions.swap_pop(src_free_region_id))) {
		return false;
	}

	return true;
}
void odAtlas_init(odAtlas* atlas) {
	if (!OD_CHECK(atlas != nullptr)) {
		return;
	}

	odAtlas_destroy(atlas);

	// start with a 1x1 px white texture
	if (!OD_CHECK(odImage_init(&atlas->image, 1, 1))) {
		return;
	}

	odColor* pixel = odImage_get(&atlas->image, 0, 0);
	if (!OD_CHECK(pixel != nullptr)) {
		return;
	}

	*pixel = *odColor_get_white();
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
bool odAtlas_check_valid(const odAtlas* atlas) {
	if (!OD_CHECK(atlas != nullptr)
		|| !OD_CHECK(odImage_check_valid(&atlas->image))
		|| !OD_CHECK(odTrivialArray_check_valid(&atlas->regions))
		|| !OD_CHECK(odTrivialArray_check_valid(&atlas->free_regions))) {
		return false;
	}

	return true;
}
const odColor* odAtlas_begin_const(const odAtlas* atlas) {
	if (!OD_CHECK(atlas != nullptr)) {
		return nullptr;
	}

	return odImage_begin_const(&atlas->image);
}
int32_t odAtlas_get_width(const odAtlas* atlas) {
	if (!OD_CHECK(atlas != nullptr)) {
		return 0;
	}

	return atlas->image.width;
}
int32_t odAtlas_get_height(const odAtlas* atlas) {
	if (!OD_CHECK(atlas != nullptr)) {
		return 0;
	}

	return atlas->image.height;
}
int32_t odAtlas_get_count(const odAtlas* atlas) {
	if (!OD_CHECK(atlas != nullptr)) {
		return 0;
	}

	return atlas->regions.get_count();
}
const odBounds* odAtlas_get_region_bounds(const odAtlas* atlas, odAtlasRegionId region_id) {
	if (!OD_CHECK(atlas != nullptr)
		|| !OD_CHECK(region_id >= 0)
		|| !OD_CHECK(region_id < atlas->regions.get_count())) {
		return nullptr;
	}

	const odAtlasRegion* region = atlas->regions.get(region_id);
	if (!OD_CHECK(region != nullptr)) {
		return nullptr;
	}

	return &region->bounds;
}
bool odAtlas_set_region(odAtlas* atlas, odAtlasRegionId region_id,
						int32_t width, int32_t height, const odColor* src, int32_t src_image_width) {
	if (!OD_CHECK(atlas != nullptr)
		|| !OD_CHECK(region_id >= 0)
		|| !OD_CHECK((src != nullptr) || (width == 0) || (height == 0))
		|| !OD_CHECK(odInt32_fits_float(width))
		|| !OD_CHECK(width > 0)
		|| !OD_CHECK(odInt32_fits_float(height))
		|| !OD_CHECK(height > 0)
		|| !OD_CHECK((src_image_width >= width) || (height == 0))) {
		return false;
	}

	if (!OD_CHECK(odAtlas_ensure_count(atlas, static_cast<int32_t>(region_id + 1)))) {
		return false;
	}

	if (!OD_CHECK(odAtlas_set_region_size(atlas, region_id, width, height))) {
		return false;
	}

	odAtlasRegion* dest_region = atlas->regions.get(region_id);
	if (!OD_CHECK(dest_region != nullptr)) {
		return false;
	}

	odColor* dest = odImage_get(
		&atlas->image,
		static_cast<int32_t>(dest_region->bounds.x1),
		static_cast<int32_t>(dest_region->bounds.y1));

	if (!OD_CHECK(dest != nullptr)) {
		return false;
	}

	odColor_blit(width, height, src, src_image_width, dest, atlas->image.width);

	return true;
}
bool odAtlas_reset_region(odAtlas* atlas, odAtlasRegionId region_id) {
	if (!OD_CHECK(atlas != nullptr)
		|| !OD_CHECK(region_id >= 0)) {
		return false;
	}

	if (!OD_CHECK(odAtlas_ensure_count(atlas, static_cast<int32_t>(region_id + 1)))) {
		return false;
	}

	odAtlasRegion* region = atlas->regions.get(region_id);
	if (!OD_CHECK(region != nullptr)) {
		return false;
	}

	if (odAtlasRegion_has_space(region)) {
		if (!OD_CHECK(atlas->free_regions.push(*region))) {
			return false;
		}
	}

	*region = {};

	return true;
}
odAtlas::odAtlas()
: image{}, regions{}, free_regions{} {
}
odAtlas::odAtlas(odAtlas&& other)
: odAtlas{} {
	odAtlas_swap(this, &other);
}
odAtlas& odAtlas::operator=(odAtlas&& other) {
	odAtlas_swap(this, &other);
	return *this;
}
odAtlas::~odAtlas() {
	odAtlas_destroy(this);
}
