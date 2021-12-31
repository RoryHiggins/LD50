#pragma once

#include <od/engine/module.h>

struct odColorRGBA32;
struct odBounds2f;
struct odWindow;

struct odAtlas;

typedef int32_t odAtlasRegionId;

OD_API_C OD_ENGINE_MODULE void
odAtlas_init(struct odAtlas* atlas);
OD_API_C OD_ENGINE_MODULE void
odAtlas_destroy(struct odAtlas* atlas);
OD_API_C OD_ENGINE_MODULE void
odAtlas_swap(struct odAtlas* atlas1, struct odAtlas* atlas2);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD int32_t
odAtlas_get_count(const struct odAtlas* atlas);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD const struct odBounds2f*
odAtlas_get_region_bounds(const struct odAtlas* atlas, odAtlasRegionId region_id);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odAtlas_set_region(struct odAtlas* atlas, odAtlasRegionId region_id,
				   int32_t width, int32_t height, const odColorRGBA32* src, int32_t src_row_stride);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odAtlas_unset_region(odAtlas* atlas, odAtlasRegionId region_id);
