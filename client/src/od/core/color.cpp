#include <od/core/color.h>

#include <cinttypes>
#include <cstring>

#include <od/core/debug.h>

const char* odColor_get_debug_string(const odColor* color) {
	if (color == nullptr) {
		return "null";
	}

	uint32_t color_u32 = (
		uint32_t(color->r)
		+ (uint32_t(color->g) << 8)
		+ (uint32_t(color->b) << 16)
		+ (uint32_t(color->a) << 24));

	return odDebugString_format("\"0x%08x\"", color_u32);
}
bool odColor_get_equals(const odColor* color1, const odColor* color2) {
	if (!OD_DEBUG_CHECK(color1 != nullptr)
		|| !OD_DEBUG_CHECK(color2 != nullptr)) {
		return false;
	}

	if ((color1->r != color2->r)
		|| (color1->g != color2->g)
		|| (color1->b != color2->b)
		|| (color1->a != color2->a)) {
		return false;
	}

	return true;
}
void odColor_blit(int32_t width, int32_t height, const odColor* src, int32_t src_image_width,
				  odColor* dest, int32_t dest_image_width) {
	if (!OD_DEBUG_CHECK(width >= 0)
		|| !OD_DEBUG_CHECK(height >= 0)
		|| !OD_DEBUG_CHECK(width <= src_image_width)
		|| !OD_DEBUG_CHECK((src != nullptr) || (width == 0) || (height == 0))
		|| !OD_DEBUG_CHECK((src_image_width >= width) || (height == 0))
		|| !OD_DEBUG_CHECK((dest != nullptr) || (width == 0) || (height == 0))
		|| !OD_DEBUG_CHECK(dest_image_width >= 0)
		|| !OD_DEBUG_CHECK((dest_image_width >= width) || (height == 0))) {
		return;
	}

	const odColor* src_end = src + (height * src_image_width);

	size_t row_size = sizeof(odColor) * static_cast<size_t>(width);
	while (src < src_end) {
		memcpy(dest, src, row_size);
		src += src_image_width;
		dest += dest_image_width;
	}
}
const odColor* odColor_get_white() {
	static const odColor color{0xFF, 0xFF, 0xFF, 0xFF};
	return &color;
}
const odColor* odColor_get_black() {
	static const odColor color{0x00, 0x00, 0x00, 0xFF};
	return &color;
}
const odColor* odColor_get_red() {
	static const odColor color{0xFF, 0x00, 0x00, 0xFF};
	return &color;
}
const odColor* odColor_get_green() {
	static const odColor color{0x00, 0xFF, 0x00, 0xFF};
	return &color;
}
const odColor* odColor_get_blue() {
	static const odColor color{0x00, 0x00, 0xFF, 0xFF};
	return &color;
}
const odColor* odColor_get_transparent() {
	static const odColor color{0x00, 0x00, 0x00, 0x00};
	return &color;
}
