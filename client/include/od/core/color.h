#pragma once

#include <od/core/module.h>

struct odColor {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

static const struct odColor odColor_white = {0xFF, 0xFF, 0xFF, 0xFF};
static const struct odColor odColor_black = {0x00, 0x00, 0x00, 0xFF};
static const struct odColor odColor_red = {0xFF, 0x00, 0x00, 0xFF};
static const struct odColor odColor_green = {0x00, 0xFF, 0x00, 0xFF};
static const struct odColor odColor_blue = {0x00, 0x00, 0xFF, 0xFF};
static const struct odColor odColor_transparent = {0x00, 0x00, 0x00, 0x00};
