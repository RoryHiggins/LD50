#pragma once

#include <od/engine/tagset.h>

#include <cstdio>

#include <od/core/debug.h>

const char* odTagset_get_debug_string(const odTagset* tagset) {
	if (tagset == nullptr) {
		return "odEntity{this=nullptr}";
	}

	char tagset_hex[1 + (2 * OD_TAGSET_BYTE_SIZE)];
	for (int32_t i = 0; i < OD_TAGSET_BYTE_SIZE; i++) {
		snprintf(tagset_hex + (2 * i), 3, "%02x", static_cast<unsigned>(tagset->tagset[i]));
	}

	return odDebugString_format(
		"odEntity{this=%p, tagset=0x%*s}",
		static_cast<const void*>(tagset),
		2 * OD_TAGSET_BYTE_SIZE,
		tagset_hex);
}
bool odTagset_get(const odTagset *tagset, int32_t required_tag_id) {
	if (!OD_DEBUG_CHECK(tagset != nullptr)
		|| !OD_DEBUG_CHECK((required_tag_id >= 0) && (required_tag_id <= OD_TAG_ID_MAX))) {
		return false;
	}

	uint8_t byte = static_cast<uint8_t>(required_tag_id >> 3);
	uint8_t bit = static_cast<uint8_t>(required_tag_id & 7);
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
bool odTagset_equals(const odTagset* a, const odTagset* b) {
	if (!OD_DEBUG_CHECK(a != nullptr)
		|| !OD_DEBUG_CHECK(b != nullptr)) {
		return false;
	}

	for (int32_t i = 0; i < OD_TAGSET_BYTE_SIZE; i++) {
		if (a->tagset[i] != b->tagset[i]) {
			return false;
		}
	}

	return true;
}
void odTagset_set(odTagset *tagset, int32_t tag_id, bool enabled) {
	if (!OD_DEBUG_CHECK(tagset != nullptr)
		|| !OD_DEBUG_CHECK((tag_id >= 0) && (tag_id <= OD_TAG_ID_MAX))) {
		return;
	}

	uint8_t byte = static_cast<uint8_t>(tag_id >> 3);
	uint8_t bit = static_cast<uint8_t>(tag_id & 7);
	uint8_t bit_mask = static_cast<uint8_t>(1 << bit);
	uint8_t enabled_bit_mask = static_cast<uint8_t>(enabled << bit);
	
	tagset->tagset[byte] = (tagset->tagset[byte] ^ bit_mask) | enabled_bit_mask;
}
