#include <od/platform/image.hpp>

#include <od/test/test.hpp>

OD_TEST(odImage, allocate_release) {
	int32_t allocated_width = -1;
	int32_t allocated_height = -1;

	odImage image;
	OD_ASSERT(odImage_get(&image) == nullptr);
	odImage_get_size(&image, &allocated_width, &allocated_height);
	OD_ASSERT(allocated_width == 0);
	OD_ASSERT(allocated_height == 0);

	const int32_t width = 4;
	const int32_t height = 4;
	OD_ASSERT(odImage_allocate(&image, width, height));
	OD_ASSERT(odImage_get(&image) != nullptr);

	odImage_get_size(&image, &allocated_width, &allocated_height);
	OD_ASSERT(allocated_width == width);
	OD_ASSERT(allocated_height == height);

	odImage_release(&image);
	OD_ASSERT(odImage_get(&image) == nullptr);
	odImage_get_size(&image, &allocated_width, &allocated_height);
	OD_ASSERT(allocated_width == 0);
	OD_ASSERT(allocated_height == 0);
}
OD_TEST(odImage, read_png) {
	// 69-byte 1x1 png to verify
	const uint8_t png[] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48,
						   0x44, 0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x08, 0x02, 0x00, 0x00,
						   0x00, 0x90, 0x77, 0x53, 0xde, 0x00, 0x00, 0x00, 0x0c, 0x49, 0x44, 0x41, 0x54, 0x08,
						   0xd7, 0x63, 0xd8, 0x7a, 0xe1, 0x02, 0x00, 0x04, 0x93, 0x02, 0x56, 0xea, 0x5d, 0xb7,
						   0xd6, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82};
	const int32_t png_size = sizeof(png);

	odImage image;
	OD_ASSERT(odImage_read_png(&image, static_cast<const void*>(png), png_size));
	OD_ASSERT(odImage_get(&image) != nullptr);

	int32_t allocated_width = 0;
	int32_t allocated_height = 0;
	odImage_get_size(&image, &allocated_width, &allocated_height);
	OD_ASSERT(allocated_width == 1);
	OD_ASSERT(allocated_height == 1);
}
OD_TEST(odImage, read_invalid_png_fails) {
	const uint8_t invalid_png[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A};
	const int32_t invalid_png_size = sizeof(invalid_png);

	odImage image;
	{
		odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
		OD_ASSERT(!odImage_read_png(&image, static_cast<const void*>(invalid_png), invalid_png_size));
	}
}
