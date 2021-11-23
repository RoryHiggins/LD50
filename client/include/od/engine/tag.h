#pragma once

#include <od/engine/module.h>

#define OD_TAGSET_BYTE_SIZE 12
#define OD_TAGSET_MAX (8 * OD_TAGSET_BYTE_SIZE)

typedef uint8_t odTagIndex;

struct odTagset {
	uint8_t tags[OD_TAGSET_BYTE_SIZE];
};
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odTagset_has_tag(const struct odTagset* tags, odTagIndex required_tag_index);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odTagset_has_tags(const struct odTagset* tags, const struct odTagset* required_tags);
OD_API_C OD_ENGINE_MODULE void
odTagset_set_tag(struct odTagset* tags, odTagIndex tag_index, bool enabled);
