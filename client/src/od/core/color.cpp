#include <od/core/color.h>

#include <cinttypes>

#include <od/core/debug.h>

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
const struct odColorRGBA32* odColorRGBA32_get_white() {
	static const odColorRGBA32 color{0xFF, 0xFF, 0xFF, 0xFF};
	return &color;
}
const struct odColorRGBA32* odColorRGBA32_get_black() {
	static const odColorRGBA32 color{0x00, 0x00, 0x00, 0xFF};
	return &color;
}
const struct odColorRGBA32* odColorRGBA32_get_red() {
	static const odColorRGBA32 color{0xFF, 0x00, 0x00, 0xFF};
	return &color;
}
const struct odColorRGBA32* odColorRGBA32_get_green() {
	static const odColorRGBA32 color{0x00, 0xFF, 0x00, 0xFF};
	return &color;
}
const struct odColorRGBA32* odColorRGBA32_get_blue() {
	static const odColorRGBA32 color{0x00, 0x00, 0xFF, 0xFF};
	return &color;
}
const struct odColorRGBA32* odColorRGBA32_get_transparent() {
	static const odColorRGBA32 color{0x00, 0x00, 0x00, 0x00};
	return &color;
}
