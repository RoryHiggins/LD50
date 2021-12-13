#pragma once

#include <od/engine/module.h>

#define OD_TAGSET_ELEMENT_COUNT 3
#define OD_TAGSET_ELEMENT_SIZE 4
#define OD_TAGSET_ELEMENT_BIT_SIZE (8 * OD_TAGSET_ELEMENT_SIZE)
#define OD_TAGSET_BYTE_SIZE (OD_TAGSET_ELEMENT_COUNT * OD_TAGSET_ELEMENT_SIZE)
#define OD_TAGSET_BIT_SIZE (8 * OD_TAGSET_BYTE_SIZE)

#define OD_TAG_ID_COUNT (OD_TAGSET_BIT_SIZE)

typedef uint32_t odTagsetElement;

struct odTagset {
	odTagsetElement tagset[OD_TAGSET_ELEMENT_COUNT];
};

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD const char*
odTagset_get_debug_string(const struct odTagset* tagset);
OD_API_C OD_ENGINE_MODULE void
odTagset_set(struct odTagset* tagset, int32_t tag_id, bool enabled);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odTagset_get(const struct odTagset* tagset, int32_t required_tag_id);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD uint8_t
odTagset_get_max(void);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odTagset_intersects(const struct odTagset* tagset, const struct odTagset* required_tags);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odTagset_equals(const struct odTagset* a, const struct odTagset* b);
