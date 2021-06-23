#include <od/platform/image.hpp>

#include <gtest/gtest.h>

#include <od/core/debug.hpp>

TEST(odImage, allocate_release) {
	uint32_t allocated_width = -1u;
	uint32_t allocated_height = -1u;

	odImage image;
	ASSERT_EQ(odImage_get(&image), nullptr);
	odImage_get_size(&image, &allocated_width, &allocated_height);
	ASSERT_EQ(allocated_width, 0);
	ASSERT_EQ(allocated_height, 0);

	const uint32_t width = 4;
	const uint32_t height = 4;
	ASSERT_TRUE(odImage_allocate(&image, width, height));
	ASSERT_NE(odImage_get(&image), nullptr);

	odImage_get_size(&image, &allocated_width, &allocated_height);
	ASSERT_EQ(allocated_width, width);
	ASSERT_EQ(allocated_height, height);

	odImage_release(&image);
	ASSERT_EQ(odImage_get(&image), nullptr);
	odImage_get_size(&image, &allocated_width, &allocated_height);
	ASSERT_EQ(allocated_width, 0);
	ASSERT_EQ(allocated_height, 0);
}

TEST(odImage, read_png) {
	// 69-byte 1x1 png to verify
	const uint8_t png[] = {
		0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
		0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
		0x08, 0x02, 0x00, 0x00, 0x00, 0x90, 0x77, 0x53, 0xde, 0x00, 0x00, 0x00,
		0x0c, 0x49, 0x44, 0x41, 0x54, 0x08, 0xd7, 0x63, 0xd8, 0x7a, 0xe1, 0x02,
		0x00, 0x04, 0x93, 0x02, 0x56, 0xea, 0x5d, 0xb7, 0xd6, 0x00, 0x00, 0x00,
		0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
	};
	const uint32_t png_size = sizeof(png);

	odImage image;
	ASSERT_TRUE(odImage_read_png(&image, static_cast<const void*>(png), png_size));
	ASSERT_NE(odImage_get(&image), nullptr);

	uint32_t allocated_width = -1u;
	uint32_t allocated_height = -1u;
	odImage_get_size(&image, &allocated_width, &allocated_height);
	ASSERT_EQ(allocated_width, 1);
	ASSERT_EQ(allocated_height, 1);
}

TEST(odImage, read_invalid_png_fails) {
	const uint8_t invalid_png[] = {0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A};
	const uint32_t invalid_png_size = sizeof(invalid_png);

	odImage image;
	{
		odLogLevelScoped suppress_logs{OD_LOG_LEVEL_NONE};
		ASSERT_FALSE(odImage_read_png(&image, static_cast<const void*>(invalid_png), invalid_png_size));
	}
}
