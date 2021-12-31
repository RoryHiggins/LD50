#include <od/core/color.h>

#include <cinttypes>
#include <cstring>

#include <od/core/debug.h>
#include <od/core/bounds.h>

const char* odColorRGBA32_get_debug_string(const odColorRGBA32* color) {
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
bool odColorRGBA32_equals(const odColorRGBA32* color1, const odColorRGBA32* color2) {
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
void odColorRGBA32_blit(int32_t width, int32_t height, const odColorRGBA32* src, int32_t src_row_stride,
						odColorRGBA32* dest, int32_t dest_row_stride) {
	if (!OD_DEBUG_CHECK(width >= 0)
		|| !OD_DEBUG_CHECK(height >= 0)
		|| !OD_DEBUG_CHECK(width <= src_row_stride)
		|| !OD_DEBUG_CHECK(height <= dest_row_stride)
		|| !OD_DEBUG_CHECK((src != nullptr) || (width == 0) || (height == 0))
		|| !OD_DEBUG_CHECK(src_row_stride >= 0)
		|| !OD_DEBUG_CHECK((dest != nullptr) || (width == 0) || (height == 0))
		|| !OD_DEBUG_CHECK(dest_row_stride >= 0)
		|| !OD_DEBUG_CHECK((dest_row_stride > 0) || (width == 0) || (height == 0))) {
		return;
	}

	const odColorRGBA32* src_end = src + (height * src_row_stride);

	size_t row_size = sizeof(odColorRGBA32) * static_cast<size_t>(width);
	while (src < src_end) {
		memcpy(dest, src, row_size);
		src += src_row_stride;
		dest += dest_row_stride;
	}
}
const odColorRGBA32* odColorRGBA32_get_white() {
	static const odColorRGBA32 color{0xFF, 0xFF, 0xFF, 0xFF};
	return &color;
}
const odColorRGBA32* odColorRGBA32_get_black() {
	static const odColorRGBA32 color{0x00, 0x00, 0x00, 0xFF};
	return &color;
}
const odColorRGBA32* odColorRGBA32_get_red() {
	static const odColorRGBA32 color{0xFF, 0x00, 0x00, 0xFF};
	return &color;
}
const odColorRGBA32* odColorRGBA32_get_green() {
	static const odColorRGBA32 color{0x00, 0xFF, 0x00, 0xFF};
	return &color;
}
const odColorRGBA32* odColorRGBA32_get_blue() {
	static const odColorRGBA32 color{0x00, 0x00, 0xFF, 0xFF};
	return &color;
}
const odColorRGBA32* odColorRGBA32_get_transparent() {
	static const odColorRGBA32 color{0x00, 0x00, 0x00, 0x00};
	return &color;
}
