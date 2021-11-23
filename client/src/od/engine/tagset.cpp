#pragma once

#include <od/engine/tagset.h>

#include <od/core/debug.h>

bool odTagset_get(const odTagset *tagset, int32_t required_tag_index) {
	if (!OD_DEBUG_CHECK(tagset != nullptr)
		|| !OD_DEBUG_CHECK((required_tag_index > 0) && (required_tag_index <= OD_TAG_INDEX_MAX))) {
		return false;
	}

	uint8_t byte = static_cast<uint8_t>(required_tag_index >> 3);
	uint8_t bit = static_cast<uint8_t>(required_tag_index & 7);
	uint8_t bit_mask = static_cast<uint8_t>(1 << bit);

	return (tagset->tagset[byte] & bit_mask) > 0;
}
bool odTagset_intersects(const odTagset* tagset, const odTagset* required_tags) {
	if (!OD_DEBUG_CHECK(tagset != nullptr)
		|| !OD_DEBUG_CHECK(required_tags != nullptr)) {
		return false;
	}

	bool has_tags = true;
	for (int32_t i = 0; i < OD_TAGSET_BYTE_SIZE; i++) {
		has_tags = has_tags && ((tagset->tagset[i] & required_tags->tagset[i]) == required_tags->tagset[i]);
	}
	return has_tags;
}
void odTagset_set(odTagset *tagset, int32_t tag_index, bool enabled) {
	if (!OD_DEBUG_CHECK(tagset != nullptr)
		|| !OD_DEBUG_CHECK((tag_index > 0) && (tag_index <= OD_TAG_INDEX_MAX))) {
		return;
	}

	uint8_t byte = static_cast<uint8_t>(tag_index >> 3);
	uint8_t bit = static_cast<uint8_t>(tag_index & 7);
	uint8_t bit_mask = static_cast<uint8_t>(1 << bit);
	uint8_t enabled_bit_mask = static_cast<uint8_t>(enabled << bit);
	
	tagset->tagset[byte] = (tagset->tagset[byte] ^ bit_mask) | enabled_bit_mask;
}
