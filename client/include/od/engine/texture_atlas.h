#pragma once

#include <od/engine/module.h>

#include <od/engine/atlas.h>

struct odWindow;
struct odTexture;

struct odTextureAtlas;

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odTextureAtlas_init(struct odTextureAtlas* atlas, struct odWindow* window);
OD_API_C OD_ENGINE_MODULE void
odTextureAtlas_destroy(struct odTextureAtlas* atlas);
OD_API_C OD_ENGINE_MODULE void
odTextureAtlas_swap(struct odTextureAtlas* atlas1, struct odTextureAtlas* atlas2);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odTextureAtlas_check_valid(const struct odTextureAtlas* atlas);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD const struct odTexture*
odTextureAtlas_get_texture_const(const struct odTextureAtlas* atlas);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD const struct odAtlas*
odTextureAtlas_get_atlas_const(const struct odTextureAtlas* atlas);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD int32_t
odTextureAtlas_get_width(const struct odTextureAtlas* atlas);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD int32_t
odTextureAtlas_get_height(const struct odTextureAtlas* atlas);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD int32_t
odTextureAtlas_get_count(const struct odTextureAtlas* atlas);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD const struct odBounds*
odTextureAtlas_get_region_bounds(const struct odTextureAtlas* atlas, odAtlasRegionId region_id);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odTextureAtlas_set_region(struct odTextureAtlas* atlas, odAtlasRegionId region_id,
						  int32_t width, int32_t height, const struct odColor* src, int32_t src_image_width);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odTextureAtlas_reset_region(struct odTextureAtlas* atlas, odAtlasRegionId region_id);
