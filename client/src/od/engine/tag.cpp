#pragma once

#include <od/engine/tag.h>

#include <od/core/debug.h>

bool odTagset_has_tag(const odTagset *tags, odTagIndex required_tag_index) {
	if (!OD_DEBUG_CHECK(tags != nullptr) && !OD_DEBUG_CHECK(required_tag_index <= OD_TAGSET_MAX)) {
		return false;
	}

	uint8_t byte = required_tag_index >> 3;
	uint8_t bit = required_tag_index & 7;
	uint8_t bit_mask = static_cast<uint8_t>(1 << bit);

	return (tags->tags[byte] & bit_mask) > 0;
}
bool odTagset_has_tags(const odTagset* tags, const odTagset* required_tags) {
	if (!OD_DEBUG_CHECK(tags != nullptr) && !OD_DEBUG_CHECK(required_tags != nullptr)) {
		return false;
	}

	bool has_tags = true;
	for (int32_t i = 0; i < OD_TAGSET_BYTE_SIZE; i++) {
		has_tags = has_tags && ((tags->tags[i] & required_tags->tags[i]) == required_tags->tags[i]);
	}
	return has_tags;
}
void odTagset_set_tag(odTagset *tags, odTagIndex tag_index, bool enabled) {
	if (!OD_DEBUG_CHECK(tags != nullptr) && !OD_DEBUG_CHECK(tag_index <= OD_TAGSET_MAX)) {
		return;
	}

	uint8_t byte = tag_index >> 3;
	uint8_t bit = tag_index & 7;
	uint8_t bit_mask = static_cast<uint8_t>(1 << bit);
	uint8_t enabled_bit_mask = static_cast<uint8_t>(enabled << bit);
	
	tags->tags[byte] = (tags->tags[byte] ^ bit_mask) | enabled_bit_mask;
}
