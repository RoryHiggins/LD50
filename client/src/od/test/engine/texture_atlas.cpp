#include <od/engine/texture_atlas.hpp>

#include <od/core/color.h>
#include <od/platform/window.hpp>
#include <od/test/test.hpp>

OD_TEST_FILTERED(odTest_odTextureAtlas_init_destroy, OD_TEST_FILTER_SLOW) {
	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_check_valid(&window));

	odTextureAtlas atlas;
	OD_ASSERT(odTextureAtlas_init(&atlas, &window));
	odTextureAtlas_destroy(&atlas);

	// double init
	OD_ASSERT(odTextureAtlas_init(&atlas, &window));
	OD_ASSERT(odTextureAtlas_init(&atlas, &window));

	// double destroy
	odTextureAtlas_destroy(&atlas);
	odTextureAtlas_destroy(&atlas);
}
OD_TEST_FILTERED(odTest_odTextureAtlas_set_reset_get_region_bounds, OD_TEST_FILTER_SLOW) {
	const int32_t width = 4;
	const int32_t height = 4;
	const odAtlasRegionId region_id = 3;
	const odColor pixels[width * height]{};

	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_check_valid(&window));

	odTextureAtlas atlas;
	OD_ASSERT(odTextureAtlas_init(&atlas, &window));

	OD_ASSERT(odTextureAtlas_init(&atlas, &window));
	OD_ASSERT(odTextureAtlas_set_region(&atlas, region_id, width, height, pixels, width));

	const odBounds* region_bounds = odTextureAtlas_get_region_bounds(&atlas, region_id);
	OD_ASSERT(region_bounds != nullptr);
	OD_ASSERT(odBounds_get_width(region_bounds) == static_cast<float>(width));
	OD_ASSERT(odBounds_get_height(region_bounds) == static_cast<float>(height));
	OD_ASSERT(odTextureAtlas_get_count(&atlas) == (region_id + 1));

	const odBounds empty_bounds{};
	OD_ASSERT(odTextureAtlas_reset_region(&atlas, region_id));

	region_bounds = odTextureAtlas_get_region_bounds(&atlas, region_id);
	OD_ASSERT(region_bounds != nullptr);
	OD_ASSERT(odBounds_get_equals(region_bounds, &empty_bounds));
	OD_ASSERT(odTextureAtlas_get_count(&atlas) == (region_id + 1));
}
OD_TEST_FILTERED(odTest_odTextureAtlas_set_reset_set_reused, OD_TEST_FILTER_SLOW) {
	const int32_t width = 8;
	const int32_t height = 8;
	const odAtlasRegionId region_id = 3;
	const odColor pixels[width * height]{};

	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_check_valid(&window));

	odTextureAtlas atlas;
	OD_ASSERT(odTextureAtlas_init(&atlas, &window));

	OD_ASSERT(odTextureAtlas_set_region(&atlas, region_id, width, height, pixels, width));
	const odBounds* region_bounds = odTextureAtlas_get_region_bounds(&atlas, region_id);
	OD_ASSERT(region_bounds != nullptr);
	OD_ASSERT(odBounds_get_width(region_bounds) == static_cast<float>(width));
	OD_ASSERT(odBounds_get_height(region_bounds) == static_cast<float>(height));
	OD_ASSERT(odTextureAtlas_get_count(&atlas) == (region_id + 1));

	const odBounds old_region_bounds = *region_bounds;
	OD_ASSERT(odTextureAtlas_reset_region(&atlas, region_id));
	OD_ASSERT(odTextureAtlas_set_region(&atlas, region_id, width, height, pixels, width));
	region_bounds = odTextureAtlas_get_region_bounds(&atlas, region_id);
	OD_ASSERT(region_bounds != nullptr);
	OD_ASSERT(odBounds_get_equals(region_bounds, &old_region_bounds));
	OD_ASSERT(odTextureAtlas_get_count(&atlas) == (region_id + 1));
}
OD_TEST_FILTERED(odTest_odTextureAtlas_set_reset_scaling_sizes, OD_TEST_FILTER_SLOW) {
	const int32_t max_width_bits = 8;
	const int32_t max_height_bits = 8;
	const int32_t max_width = 1 << max_width_bits;
	const int32_t max_height = 1 << max_height_bits;
	const odColor pixels[max_width * max_height]{};

	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_check_valid(&window));

	odTextureAtlas atlas;
	OD_ASSERT(odTextureAtlas_init(&atlas, &window));

	odAtlasRegionId region_id = 0;
	for (int32_t height_bits = 1; height_bits < max_height_bits; height_bits += 16) {
		for (int32_t width_bits = 1; width_bits < max_width_bits; width_bits += 16) {
			int32_t width = 1 << width_bits;
			int32_t height = 1 << height_bits;

			OD_ASSERT(odTextureAtlas_set_region(&atlas, region_id, width, height, pixels, max_width));
			const odBounds* region_bounds = odTextureAtlas_get_region_bounds(&atlas, region_id);
			OD_ASSERT(region_bounds != nullptr);
			OD_ASSERT(odBounds_get_width(region_bounds) == static_cast<float>(width));
			OD_ASSERT(odBounds_get_height(region_bounds) == static_cast<float>(height));
			OD_ASSERT(odTextureAtlas_get_count(&atlas) == (region_id + 1));

			region_id++;
		}
	}

	const odBounds empty_bounds{};
	while (region_id > 0) {
		region_id--;

		OD_ASSERT(odTextureAtlas_reset_region(&atlas, region_id));
		const odBounds* region_bounds = odTextureAtlas_get_region_bounds(&atlas, region_id);
		OD_ASSERT(region_bounds != nullptr);
		OD_ASSERT(odBounds_get_equals(region_bounds, &empty_bounds));
	}
}
OD_TEST_FILTERED(odTest_odTextureAtlas_set_reset_realistic, OD_TEST_FILTER_SLOW) {
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

	odWindow window;
	OD_ASSERT(odWindow_init(&window, odWindowSettings_get_headless_defaults()));
	OD_ASSERT(odWindow_check_valid(&window));

	odTextureAtlas atlas;
	OD_ASSERT(odTextureAtlas_init(&atlas, &window));

	for (odAtlasRegionId region_id = 0; region_id < region_sizes_count; region_id++) {
		int32_t width = static_cast<int32_t>(region_sizes[region_id].x2);
		int32_t height = static_cast<int32_t>(region_sizes[region_id].y2);

		odImage image;
		OD_ASSERT(odImage_init(&image, width, height));
		const odColor* pixels = odImage_begin_const(&image);
		OD_ASSERT(pixels != nullptr);

		OD_ASSERT(odTextureAtlas_set_region(&atlas, region_id, width, height, pixels, width));
		const odBounds* region_bounds = odTextureAtlas_get_region_bounds(&atlas, region_id);
		OD_ASSERT(region_bounds != nullptr);
		OD_ASSERT(odBounds_get_width(region_bounds) == static_cast<float>(width));
		OD_ASSERT(odBounds_get_height(region_bounds) == static_cast<float>(height));
		OD_ASSERT(odTextureAtlas_get_count(&atlas) == (region_id + 1));

		sum_set_area += width * height;
		OD_ASSERT((odTextureAtlas_get_width(&atlas) * odTextureAtlas_get_height(&atlas)) >= sum_set_area);
	}

	// simulate manual free at the end
	const odBounds empty_bounds{};
	for (odAtlasRegionId region_id = 0; region_id < region_sizes_count; region_id++) {
		OD_ASSERT(odTextureAtlas_reset_region(&atlas, region_id));

		const odBounds* region_bounds = odTextureAtlas_get_region_bounds(&atlas, region_id);
		OD_ASSERT(region_bounds != nullptr);
		OD_ASSERT(odBounds_get_equals(region_bounds, &empty_bounds));
		OD_ASSERT(odTextureAtlas_get_count(&atlas) == region_sizes_count);
	}
}

OD_TEST_SUITE(
	odTestSuite_odTextureAtlas,
	odTest_odTextureAtlas_init_destroy,
	odTest_odTextureAtlas_set_reset_get_region_bounds,
	odTest_odTextureAtlas_set_reset_set_reused,
	odTest_odTextureAtlas_set_reset_scaling_sizes,
	odTest_odTextureAtlas_set_reset_realistic,
)
