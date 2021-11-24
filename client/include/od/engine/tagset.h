#pragma once

#include <od/engine/module.h>

#define OD_TAGSET_BYTE_SIZE 12

#define OD_TAG_ID_MAX (8 * OD_TAGSET_BYTE_SIZE)

struct odTagset {
	uint8_t tagset[OD_TAGSET_BYTE_SIZE];
};

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odTagset_get(const struct odTagset* tagset, int32_t required_tag_id);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odTagset_intersects(const struct odTagset* tagset, const struct odTagset* required_tags);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odTagset_equals(const struct odTagset* a, const struct odTagset* b);
OD_API_C OD_ENGINE_MODULE void
odTagset_set(struct odTagset* tagset, int32_t tag_id, bool enabled);
