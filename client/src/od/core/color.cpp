#include <od/core/color.h>

#include <cinttypes>

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
