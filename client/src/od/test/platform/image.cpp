#include <od/platform/image.hpp>

#include <od/core/color.h>
#include <od/test/test.hpp>

OD_TEST(odTest_odImage_init_destroy) {
	int32_t allocated_width = -1;
	int32_t allocated_height = -1;

	odImage image;
	OD_ASSERT(odImage_begin(&image) == nullptr);
	odImage_begin_size(&image, &allocated_width, &allocated_height);
	OD_ASSERT(allocated_width == 0);
	OD_ASSERT(allocated_height == 0);

	const int32_t width = 4;
	const int32_t height = 4;
	OD_ASSERT(odImage_init(&image, width, height));
	OD_ASSERT(odImage_begin(&image) != nullptr);

	odImage_begin_size(&image, &allocated_width, &allocated_height);
	OD_ASSERT(allocated_width == width);
	OD_ASSERT(allocated_height == height);

	odImage_destroy(&image);
	OD_ASSERT(odImage_begin(&image) == nullptr);
	odImage_begin_size(&image, &allocated_width, &allocated_height);
	OD_ASSERT(allocated_width == 0);
	OD_ASSERT(allocated_height == 0);

	// double init
	OD_ASSERT(odImage_init(&image, width, height));
	OD_ASSERT(odImage_begin(&image) != nullptr);

	// double destroy
	odImage_destroy(&image);
	odImage_destroy(&image);
}
OD_TEST(odTest_odImage_resize) {
	int32_t start_width = 4;
	int32_t start_height = 4;

	int32_t allocated_width = -1;
	int32_t allocated_height = -1;

	odImage image;
	OD_ASSERT(odImage_init(&image, start_width, start_height));
	odImage_begin_size(&image, &allocated_width, &allocated_height);
	OD_ASSERT(allocated_width == start_width);
	OD_ASSERT(allocated_height == start_height);

	odColor* pixels = odImage_begin(&image);
	OD_ASSERT(pixels != nullptr);

	for (int32_t y = 0; y < start_height; y++) {
		for (int32_t x = 0; x < start_width; x++) {
			int32_t i = x + (y * start_width);
			pixels[i] = {0, 0, 0, static_cast<uint8_t>(i % 255)};
		}
	}

	{
		int32_t new_width = start_width * 2;
		int32_t new_height = start_height * 2;
		OD_ASSERT(odImage_resize(&image, new_width, new_height));
		odImage_begin_size(&image, &allocated_width, &allocated_height);
		pixels = odImage_begin(&image);
		OD_ASSERT(allocated_width == new_width);
		OD_ASSERT(allocated_height == new_height);
		OD_ASSERT(pixels != nullptr);

		for (int32_t y = 0; y < start_height; y++) {
			for (int32_t x = 0; x < start_width; x++) {
				int32_t src_i = x + (y * start_width);
				int32_t dest_i = x + (y * new_width);
				odColor expected_color{0, 0, 0, static_cast<uint8_t>(src_i % 255)};
				OD_ASSERT(odColor_get_equals(&pixels[dest_i], &expected_color));
			}
		}
	}

	{
		int32_t new_width = start_width / 2;
		int32_t new_height = start_height / 2;
		OD_ASSERT(odImage_resize(&image, new_width, new_height));
		odImage_begin_size(&image, &allocated_width, &allocated_height);
		pixels = odImage_begin(&image);
		OD_ASSERT(allocated_width == new_width);
		OD_ASSERT(allocated_height == new_height);
		OD_ASSERT(pixels != nullptr);

		for (int32_t y = 0; y < new_height; y++) {
			for (int32_t x = 0; x < new_width; x++) {
				int32_t src_i = x + (y * start_width);
				int32_t dest_i = x + (y * new_width);
				odColor expected_color{0, 0, 0, static_cast<uint8_t>(src_i % 255)};
				OD_ASSERT(odColor_get_equals(&pixels[dest_i], &expected_color));
			}
		}
	}

	{
		int32_t new_width = 0;
		int32_t new_height = 0;
		OD_ASSERT(odImage_resize(&image, new_width, new_height));
		odImage_begin_size(&image, &allocated_width, &allocated_height);
		OD_ASSERT(allocated_width == new_width);
		OD_ASSERT(allocated_height == new_height);
	}
}
OD_TEST(odTest_odImage_resize_empty) {
	int32_t allocated_width = -1;
	int32_t allocated_height = -1;
	odImage image;
	OD_ASSERT(odImage_resize(&image, 0, 0));
	odImage_begin_size(&image, &allocated_width, &allocated_height);
	OD_ASSERT(allocated_width == 0);
	OD_ASSERT(allocated_height == 0);
}
OD_TEST(odTest_odImage_read_png) {
	// 69-byte 1x1 png to verify
	const uint8_t png[] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48,
						   0x44, 0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x08, 0x02, 0x00, 0x00,
						   0x00, 0x90, 0x77, 0x53, 0xde, 0x00, 0x00, 0x00, 0x0c, 0x49, 0x44, 0x41, 0x54, 0x08,
						   0xd7, 0x63, 0xd8, 0x7a, 0xe1, 0x02, 0x00, 0x04, 0x93, 0x02, 0x56, 0xea, 0x5d, 0xb7,
						   0xd6, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82};
	const int32_t png_size = sizeof(png);

	odImage image;
	OD_ASSERT(odImage_read_png(&image, static_cast<const void*>(png), png_size));
	OD_ASSERT(odImage_begin(&image) != nullptr);

	int32_t allocated_width = 0;
	int32_t allocated_height = 0;
	odImage_begin_size(&image, &allocated_width, &allocated_height);
	OD_ASSERT(allocated_width == 1);
	OD_ASSERT(allocated_height == 1);
}
OD_TEST(odTest_odImage_read_invalid_png_fails) {
	const uint8_t invalid_png[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A};
	const int32_t invalid_png_size = sizeof(invalid_png);

	odImage image;
	{
		odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
		OD_ASSERT(!odImage_read_png(&image, static_cast<const void*>(invalid_png), invalid_png_size));
	}
}

OD_TEST_SUITE(
	odTestSuite_odImage,
	odTest_odImage_init_destroy,
	odTest_odImage_resize,
	odTest_odImage_read_png,
	odTest_odImage_read_invalid_png_fails,
)
