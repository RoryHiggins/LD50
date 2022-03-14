#include <od/platform/ascii_font.h>

#include <cstring>

#include <od/core/array.hpp>
#include <od/core/bounds.h>
#include <od/core/color.h>
#include <od/core/vertex.h>
#include <od/test/test.hpp>

OD_TEST(odTest_odAsciiFont_text_get_vertices) {
	odAsciiFont font{
		odBounds{0.0f, 160.0f, 64.0f, 256.0f},
		8,
		8,
		' ',
		'~'
	};
	OD_ASSERT(odAsciiFont_check_valid(&font));

	const odColor color = *odColor_get_white();
	const odBounds max_bounds{16.0f, 16.0f, 64.0f, 64.0f};
	const char* str = "hello";
	float depth = 1.0f;
	int32_t str_count = static_cast<int32_t>(strlen(str));
	odAsciiTextPrimitive text{
		str,
		str_count,
		max_bounds,
		color,
		depth
	};
	OD_ASSERT(odAsciiTextPrimitive_check_valid(&text));

	int32_t vertices_count = 0;
	odBounds bounds{};
	OD_ASSERT(odAsciiFont_text_get_vertices(&font, &text, &vertices_count, &bounds, nullptr));
	OD_ASSERT(vertices_count == (6 * str_count));
	OD_ASSERT(odBounds_contains(&text.max_bounds, &bounds));

	odTrivialArrayT<odVertex> vertices{};
	OD_ASSERT(vertices.set_count(vertices_count));

	OD_ASSERT(odAsciiFont_text_get_vertices(&font, &text, nullptr, nullptr, vertices.begin()));
	for (odVertex vertex: vertices) {
		OD_ASSERT(odColor_get_equals(&vertex.color, &color));
		odBounds origin{vertex.pos.x, vertex.pos.y, vertex.pos.x + 1, vertex.pos.y + 1};
		OD_ASSERT(odBounds_contains(&max_bounds, &origin));
		OD_ASSERT(vertex.pos.z == depth);
	}
}

OD_TEST_SUITE(
	odTestSuite_odAsciiFont,
	odTest_odAsciiFont_text_get_vertices,
)
