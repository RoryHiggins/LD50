#include <od/engine/tagset.h>

#include <cstdio>

#include <od/core/debug.h>

static_assert(
	OD_TAGSET_ELEMENT_SIZE == sizeof(odTagsetElement),
	"tagset element size must match size of element type");

const char* odTagset_get_debug_string(const odTagset* tagset) {
	if (tagset == nullptr) {
		return "null";
	}

	const int32_t tagset_hex_size = (2 * OD_TAGSET_BYTE_SIZE);
	char tagset_hex[tagset_hex_size + 1] = "";

	const uint8_t* tagset_bytes = reinterpret_cast<const uint8_t*>(tagset->tagset);
	for (int32_t i = 0; i < OD_TAGSET_BYTE_SIZE; i++) {
		const int32_t byte_hex_str_size = 16;
		char byte_hex_str[byte_hex_str_size]{};
		snprintf(byte_hex_str, byte_hex_str_size, "%02X", static_cast<unsigned>(tagset_bytes[i]));

		sprintf(tagset_hex + (2 * i), "%2s", byte_hex_str);
	}

	return odDebugString_format("\"0x%*s\"", tagset_hex_size, tagset_hex);
}
void odTagset_set(odTagset *tagset, int32_t tag_id, bool enabled) {
	if (!OD_DEBUG_CHECK(tagset != nullptr)
		|| !OD_DEBUG_CHECK((tag_id >= 0) && (tag_id <= OD_TAG_ID_COUNT))) {
		return;
	}

	odTagsetElement element = static_cast<odTagsetElement>(tag_id / OD_TAGSET_ELEMENT_BIT_SIZE);
	odTagsetElement bit = static_cast<odTagsetElement>(tag_id & (OD_TAGSET_ELEMENT_BIT_SIZE - 1));
	odTagsetElement bit_mask = static_cast<odTagsetElement>(1 << bit);
	uint8_t enabled_bit_mask = static_cast<uint8_t>(enabled << bit);

	tagset->tagset[element] = (tagset->tagset[element] ^ bit_mask) | enabled_bit_mask;
}

bool odTagset_get(const odTagset *tagset, int32_t required_tag_id) {
	if (!OD_DEBUG_CHECK(tagset != nullptr)
		|| !OD_DEBUG_CHECK((required_tag_id >= 0) && (required_tag_id <= OD_TAG_ID_COUNT))) {
		return false;
	}

	odTagsetElement element = static_cast<odTagsetElement>(required_tag_id / OD_TAGSET_ELEMENT_BIT_SIZE);
	odTagsetElement bit = static_cast<odTagsetElement>(required_tag_id & (OD_TAGSET_ELEMENT_BIT_SIZE - 1));
	odTagsetElement bit_mask = static_cast<odTagsetElement>(1 << bit);

	return (tagset->tagset[element] & bit_mask) > 0;
}
bool odTagset_intersects(const odTagset* tagset, const odTagset* required_tags) {
	if (!OD_DEBUG_CHECK(tagset != nullptr)
		|| !OD_DEBUG_CHECK(required_tags != nullptr)) {
		return false;
	}

	bool intersects = true;
	for (int32_t i = 0; i < OD_TAGSET_ELEMENT_COUNT; i++) {
		intersects = intersects && ((tagset->tagset[i] & required_tags->tagset[i]) == required_tags->tagset[i]);
	}

	return intersects;
}
bool odTagset_get_equals(const odTagset* a, const odTagset* b) {
	if (!OD_DEBUG_CHECK(a != nullptr)
		|| !OD_DEBUG_CHECK(b != nullptr)) {
		return false;
	}

	bool get_equals = true;
	for (int32_t i = 0; i < OD_TAGSET_ELEMENT_COUNT; i++) {
		get_equals = get_equals && (a->tagset[i] == b->tagset[i]);
	}

	return get_equals;
}
