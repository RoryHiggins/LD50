#include <od/engine/atlas.hpp>

#include <od/core/bounds.h>
#include <od/core/color.h>
#include <od/platform/image.hpp>
#include <od/test/test.hpp>

OD_TEST(odTest_odAtlas_init_destroy) {
	odAtlas atlas;

	odAtlas_init(&atlas);
	odAtlas_destroy(&atlas);

	// double init
	odAtlas_init(&atlas);
	odAtlas_init(&atlas);

	// double destroy
	odAtlas_destroy(&atlas);
	odAtlas_destroy(&atlas);
}
OD_TEST(odTest_odAtlas_set_reset_get_region_bounds) {
	const int32_t width = 4;
	const int32_t height = 4;
	const odAtlasRegionId region_id = 3;
	const odColor pixels[width * height]{};

	odAtlas atlas;
	OD_ASSERT(odAtlas_set_region(&atlas, region_id, width, height, pixels, width));

	const odBounds* region_bounds = odAtlas_get_region_bounds(&atlas, region_id);
	OD_ASSERT(region_bounds != nullptr);
	OD_ASSERT(odBounds_get_width(region_bounds) == static_cast<float>(width));
	OD_ASSERT(odBounds_get_height(region_bounds) == static_cast<float>(height));
	OD_ASSERT(odAtlas_get_count(&atlas) == (region_id + 1));

	const odBounds empty_bounds{};
	OD_ASSERT(odAtlas_reset_region(&atlas, region_id));

	region_bounds = odAtlas_get_region_bounds(&atlas, region_id);
	OD_ASSERT(region_bounds != nullptr);
	OD_ASSERT(odBounds_equals(region_bounds, &empty_bounds));
	OD_ASSERT(odAtlas_get_count(&atlas) == (region_id + 1));
}
OD_TEST(odTest_odAtlas_set_reset_set_reused) {
	const int32_t width = 8;
	const int32_t height = 8;
	const odAtlasRegionId region_id = 3;
	const odColor pixels[width * height]{};

	odAtlas atlas;
	OD_ASSERT(odAtlas_set_region(&atlas, region_id, width, height, pixels, width));
	const odBounds* region_bounds = odAtlas_get_region_bounds(&atlas, region_id);
	OD_ASSERT(region_bounds != nullptr);
	OD_ASSERT(odBounds_get_width(region_bounds) == static_cast<float>(width));
	OD_ASSERT(odBounds_get_height(region_bounds) == static_cast<float>(height));
	OD_ASSERT(odAtlas_get_count(&atlas) == (region_id + 1));

	const odBounds old_region_bounds = *region_bounds;
	OD_ASSERT(odAtlas_reset_region(&atlas, region_id));
	OD_ASSERT(odAtlas_set_region(&atlas, region_id, width, height, pixels, width));
	region_bounds = odAtlas_get_region_bounds(&atlas, region_id);
	OD_ASSERT(region_bounds != nullptr);
	OD_ASSERT(odBounds_equals(region_bounds, &old_region_bounds));
	OD_ASSERT(odAtlas_get_count(&atlas) == (region_id + 1));
}
OD_TEST(odTest_odAtlas_set_reset_scaling_sizes) {
	const int32_t max_width_bits = 8;
	const int32_t max_height_bits = 8;
	const int32_t max_width = 1 << max_width_bits;
	const int32_t max_height = 1 << max_height_bits;
	const odColor pixels[max_width * max_height]{};

	odAtlas atlas;

	odAtlasRegionId region_id = 0;
	for (int32_t height_bits = 1; height_bits < max_height_bits; height_bits += 16) {
		for (int32_t width_bits = 1; width_bits < max_width_bits; width_bits += 16) {
			int32_t width = 1 << width_bits;
			int32_t height = 1 << height_bits;

			OD_ASSERT(odAtlas_set_region(&atlas, region_id, width, height, pixels, max_width));
			const odBounds* region_bounds = odAtlas_get_region_bounds(&atlas, region_id);
			OD_ASSERT(region_bounds != nullptr);
			OD_ASSERT(odBounds_get_width(region_bounds) == static_cast<float>(width));
			OD_ASSERT(odBounds_get_height(region_bounds) == static_cast<float>(height));
			OD_ASSERT(odAtlas_get_count(&atlas) == (region_id + 1));

			region_id++;
		}
	}

	const odBounds empty_bounds{};
	while (region_id > 0) {
		region_id--;

		OD_ASSERT(odAtlas_reset_region(&atlas, region_id));
		const odBounds* region_bounds = odAtlas_get_region_bounds(&atlas, region_id);
		OD_ASSERT(region_bounds != nullptr);
		OD_ASSERT(odBounds_equals(region_bounds, &empty_bounds));
	}
}
OD_TEST(odTest_odAtlas_set_reset_realistic) {
	const int32_t max_width = 1024;
	const float max_width_f = static_cast<float>(max_width);
	const int32_t max_height = 1024;
	const float max_height_f = static_cast<float>(max_height);
	const odBounds region_sizes[]{
		odBounds{0.0f, 0.0f, 16.0f, 16.0f},
		odBounds{0.0f, 0.0f, 1.0f, 1.0f},
		odBounds{0.0f, 0.0f, 1.0f, 2.0f},
		odBounds{0.0f, 0.0f, max_width_f, max_height_f},
		odBounds{0.0f, 0.0f, 2.0f, 1.0f},
		odBounds{0.0f, 0.0f, 16.0f, 16.0f},
		odBounds{0.0f, 0.0f, 8.0f, 8.0f},
		odBounds{0.0f, 0.0f, 8.0f, 8.0f},
		odBounds{0.0f, 0.0f, 8.0f, 8.0f},
		odBounds{0.0f, 0.0f, 8.0f, 8.0f},
		odBounds{0.0f, 0.0f, 8.0f, 8.0f},
		odBounds{0.0f, 0.0f, 8.0f, 8.0f},
		odBounds{0.0f, 0.0f, 64.0f, max_height_f / 2.0f},
		odBounds{0.0f, 0.0f, 8.0f, 8.0f},
		odBounds{0.0f, 0.0f, 8.0f, max_height_f},
		odBounds{0.0f, 0.0f, 8.0f, 8.0f},
		odBounds{0.0f, 0.0f, 8.0f, 16.0f},
		odBounds{0.0f, 0.0f, 8.0f, 16.0f},
		odBounds{0.0f, 0.0f, 8.0f, 8.0f},
		odBounds{0.0f, 0.0f, 8.0f, 8.0f},
		odBounds{0.0f, 0.0f, max_width_f / 2.0f, max_height_f / 2.0f},
		odBounds{0.0f, 0.0f, max_width_f / 2.0f, max_height_f / 2.0f},
		odBounds{0.0f, 0.0f, max_width_f / 2.0f, 1.0f},
		odBounds{0.0f, 0.0f, 1.0f, max_height_f / 2.0f},
		odBounds{0.0f, 0.0f, max_width_f - 123, 45.0f},
		odBounds{0.0f, 0.0f, 16.0f, 16.0f},
		odBounds{0.0f, 0.0f, 8.0f, 8.0f},
		odBounds{0.0f, 0.0f, 16.0f, 8.0f},
		odBounds{0.0f, 0.0f, 23.0f, max_height_f - 123.0f},
		odBounds{0.0f, 0.0f, 8.0f, 8.0f},
		odBounds{0.0f, 0.0f, 8.0f, 8.0f},
		odBounds{0.0f, 0.0f, 3.0f, max_height_f - 1.0f},
		odBounds{0.0f, 0.0f, 8.0f, 8.0f},
		odBounds{0.0f, 0.0f, max_width_f, 64.0f},
		odBounds{0.0f, 0.0f, 8.0f, 8.0f},
		odBounds{0.0f, 0.0f, 8.0f, 8.0f},
		odBounds{0.0f, 0.0f, max_width_f, max_height_f},
	};
	const int32_t region_sizes_count = sizeof(region_sizes) / sizeof(region_sizes[0]);

	int32_t sum_set_area = 0;

	odAtlas atlas;
	for (odAtlasRegionId region_id = 0; region_id < region_sizes_count; region_id++) {
		int32_t width = static_cast<int32_t>(region_sizes[region_id].x2);
		int32_t height = static_cast<int32_t>(region_sizes[region_id].y2);

		odImage image;
		OD_ASSERT(odImage_init(&image, width, height));
		const odColor* pixels = odImage_begin_const(&image);
		OD_ASSERT(pixels != nullptr);

		OD_ASSERT(odAtlas_set_region(&atlas, region_id, width, height, pixels, width));
		const odBounds* region_bounds = odAtlas_get_region_bounds(&atlas, region_id);
		OD_ASSERT(region_bounds != nullptr);
		OD_ASSERT(odBounds_get_width(region_bounds) == static_cast<float>(width));
		OD_ASSERT(odBounds_get_height(region_bounds) == static_cast<float>(height));
		OD_ASSERT(odAtlas_get_count(&atlas) == (region_id + 1));

		sum_set_area += width * height;
		OD_ASSERT((odAtlas_get_width(&atlas) * odAtlas_get_height(&atlas)) >= sum_set_area);
	}

	// simulate manual free at the end
	const odBounds empty_bounds{};
	for (odAtlasRegionId region_id = 0; region_id < region_sizes_count; region_id++) {
		OD_ASSERT(odAtlas_reset_region(&atlas, region_id));

		const odBounds* region_bounds = odAtlas_get_region_bounds(&atlas, region_id);
		OD_ASSERT(region_bounds != nullptr);
		OD_ASSERT(odBounds_equals(region_bounds, &empty_bounds));
		OD_ASSERT(odAtlas_get_count(&atlas) == region_sizes_count);
	}
}


OD_TEST_SUITE(
	odTestSuite_odAtlas,
	odTest_odAtlas_init_destroy,
	odTest_odAtlas_set_reset_get_region_bounds,
	odTest_odAtlas_set_reset_set_reused,
	odTest_odAtlas_set_reset_scaling_sizes,
	odTest_odAtlas_set_reset_realistic,
)
