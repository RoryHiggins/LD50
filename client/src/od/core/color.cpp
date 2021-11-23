#include <od/core/color.h>

#include <cinttypes>

#include <od/core/debug.h>

const char* odColor_get_debug_string(const odColor* color) {
	if (color == nullptr) {
		return "odColor{this=nullptr}";
	}

	return odDebugString_format(
		"odColor{this=%p, r=%" PRIu8 ", g=%" PRIu8 ", b=%" PRIu8 ", a=%" PRIu8 "}",
		static_cast<const void*>(color),
		color->r,
		color->g,
		color->b,
		color->a);
}
const struct odColor* odColor_get_white() {
	static const odColor color{0xFF, 0xFF, 0xFF, 0xFF};
	return &color;
}
const struct odColor* odColor_get_black() {
	static const odColor color{0x00, 0x00, 0x00, 0xFF};
	return &color;
}
const struct odColor* odColor_get_red() {
	static const odColor color{0xFF, 0x00, 0x00, 0xFF};
	return &color;
}
const struct odColor* odColor_get_green() {
	static const odColor color{0x00, 0xFF, 0x00, 0xFF};
	return &color;
}
const struct odColor* odColor_get_blue() {
	static const odColor color{0x00, 0x00, 0xFF, 0xFF};
	return &color;
}
const struct odColor* odColor_get_transparent() {
	static const odColor color{0x00, 0x00, 0x00, 0x00};
	return &color;
}
