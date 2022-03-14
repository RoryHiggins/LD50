#include <od/platform/ascii_font.h>

#include <cctype>
#include <cstdio>

#include <od/core/debug.h>
#include <od/core/bounds.h>
#include <od/core/color.h>
#include <od/core/vector.h>
#include <od/core/vertex.h>
#include <od/platform/primitive.h>

#define OD_ASCII_CHAR_VERTEX_COUNT OD_SPRITE_VERTEX_COUNT

bool odAsciiTextPrimitive_check_valid(const odAsciiTextPrimitive* text) {
	if (!OD_DEBUG_CHECK(text != nullptr)
		|| !OD_DEBUG_CHECK(text->str != nullptr)
		|| !OD_DEBUG_CHECK(text->str_count >= 0)
		|| !OD_DEBUG_CHECK(odBounds_check_valid(&text->max_bounds))
		|| !OD_DEBUG_CHECK(odBounds_has_area(&text->max_bounds))) {
		return false;
	}

	return true;
}
int32_t odAsciiTextPrimitive_get_max_vertices_count(const odAsciiTextPrimitive* text) {
	if (!OD_DEBUG_CHECK(odAsciiTextPrimitive_check_valid(text))) {
		return 0;
	}

	return OD_ASCII_CHAR_VERTEX_COUNT * text->str_count;
}

bool odAsciiFont_check_valid(const odAsciiFont* font) {
	if (!OD_CHECK(font != nullptr)
		|| !OD_CHECK(odBounds_check_valid(&font->texture_bounds))
		|| !OD_CHECK(odBounds_has_area(&font->texture_bounds))
		|| !OD_CHECK(font->char_width > 0)
		|| !OD_CHECK(font->char_height > 0)
		|| !OD_CHECK((static_cast<int32_t>(odBounds_get_width(&font->texture_bounds)) % font->char_width) == 0)
		|| !OD_CHECK(font->char_first >= 32)  // min printable character
		|| !OD_CHECK(font->char_last <= 126)  // min printable character
		|| !OD_CHECK(font->char_first <= font->char_last)) {
		return false;
	}

	return true;
}
bool odAsciiFont_text_get_vertices(const odAsciiFont* font, const odAsciiTextPrimitive* text,
								   int32_t* opt_out_vertices_count, odBounds* opt_out_bounds,
								   odVertex* opt_out_vertices) {
	if (!OD_DEBUG_CHECK(odAsciiFont_check_valid(font))
		|| !OD_DEBUG_CHECK(odAsciiTextPrimitive_check_valid(text))) {
		return 0;
	}

	odSpritePrimitive sprite_cur{
		odBounds{
			text->max_bounds.x1,
			text->max_bounds.y1,
			text->max_bounds.x1 + static_cast<float>(font->char_width),
			text->max_bounds.y1 + static_cast<float>(font->char_height),
		},
		odBounds{
			font->texture_bounds.x1,
			font->texture_bounds.y1,
			static_cast<float>(font->char_width),
			static_cast<float>(font->char_height)
		},
		text->color,
		text->depth
	};

	odBounds out_bounds{
		text->max_bounds.x1,
		text->max_bounds.y1,
		text->max_bounds.x1,
		text->max_bounds.y1,
	};

	int32_t columns_per_row = static_cast<int32_t>(odBounds_get_width(&font->texture_bounds)) / font->char_width;

	int32_t vertices_count = 0;
	for (int32_t i = 0; i < text->str_count; i++) {
		if ((sprite_cur.bounds.x2 > text->max_bounds.x2) || (sprite_cur.bounds.y2 > text->max_bounds.y2)) {
			OD_WARN("text->max_bounds too small to fit text, bounds=%s", odBounds_get_debug_string(&text->max_bounds));
			break;
		}

		char char_cur = text->str[i];
		if ((char_cur == '\0') || (char_cur == EOF)) {
			break;
		}
		if (char_cur == '\r') {
			continue;
		}
		if (char_cur == '\n') {
			sprite_cur.bounds.x1 = text->max_bounds.x1;
			sprite_cur.bounds.x2 = text->max_bounds.x1 + static_cast<float>(font->char_width);

			sprite_cur.bounds.y1 += static_cast<float>(font->char_height);
			sprite_cur.bounds.y2 += static_cast<float>(font->char_height);
			continue;
		}
		if (char_cur == '\t') {
			char_cur = ' ';
		}
		if (!OD_CHECK(char_cur >= font->char_first) || !OD_CHECK(char_cur <= font->char_last)) {
			char_cur = ' ';
		}

		int32_t char_index = static_cast<int32_t>(char_cur - font->char_first);
		float u = font->texture_bounds.x1 + static_cast<float>(font->char_width * (char_index % columns_per_row));
		float v = font->texture_bounds.y1 + static_cast<float>(font->char_height * (char_index / columns_per_row));

		sprite_cur.texture_bounds = odBounds{
			u, v, u + static_cast<float>(font->char_width), v + static_cast<float>(font->char_height)
		};
	
		if (opt_out_vertices != nullptr) {
			odSpritePrimitive_get_vertices(&sprite_cur, opt_out_vertices + vertices_count);
		}
		vertices_count += OD_ASCII_CHAR_VERTEX_COUNT;

		out_bounds.x2 = sprite_cur.bounds.x2 > out_bounds.x2 ? sprite_cur.bounds.x2 : out_bounds.x2;
		out_bounds.y2 = sprite_cur.bounds.y2 > out_bounds.y2 ? sprite_cur.bounds.y2 : out_bounds.y2;

		if (sprite_cur.bounds.x2 >= text->max_bounds.x2) {
			sprite_cur.bounds.x1 = text->max_bounds.x1;
			sprite_cur.bounds.x2 = text->max_bounds.x1 + static_cast<float>(font->char_width);

			sprite_cur.bounds.y1 += static_cast<float>(font->char_height);
			sprite_cur.bounds.y2 += static_cast<float>(font->char_height);
		} else {
			sprite_cur.bounds.x1 += static_cast<float>(font->char_width);
			sprite_cur.bounds.x2 += static_cast<float>(font->char_width);
		}
	}

	if (opt_out_bounds != nullptr) {
		*opt_out_bounds = out_bounds;
	}

	if (opt_out_vertices_count != nullptr) {
		*opt_out_vertices_count = vertices_count;
	}
	
	return true;
}
