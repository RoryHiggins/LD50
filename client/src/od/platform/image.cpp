#include <od/platform/image.hpp>

#include <cstring>

#include <png.h>

#include <od/core/debug.h>
#include <od/core/color.h>
#include <od/core/allocation.hpp>
#include <od/platform/file.hpp>

bool odImage_check_valid(const odImage* image) {
	if (!OD_CHECK(image != nullptr)
		|| !OD_CHECK(odAllocation_check_valid(&image->allocation))
		|| !OD_CHECK(image->width >= 0)
		|| !OD_CHECK(image->height >= 0)) {
		return false;
	}
	
	return true;
}
const char* odImage_get_debug_string(const odImage* image) {
	if (image == nullptr) {
		return "null";
	}

	return odDebugString_format("{\"width\": %d, \"height\": %d}", image->width, image->height);
}
bool odImage_init(odImage* image, int32_t width, int32_t height) {
	if (!OD_CHECK(image != nullptr)
		|| !OD_CHECK(width >= 0)
		|| !OD_CHECK(height >= 0)) {
		return false;
	}

	odImage_destroy(image);

	if ((width == 0) || (height == 0)) {
		return true;
	}

	int32_t size = static_cast<int32_t>(sizeof(odColorRGBA32)) * width * height;
	if (size == 0) {
		return true;
	}

	if (!OD_CHECK(odAllocation_init(&image->allocation, size))) {
		return false;
	}

	void* image_data = odAllocation_get(&image->allocation);
	if (!OD_CHECK(image_data != nullptr)) {
		return false;
	}

	image->width = width;
	image->height = height;

	return true;
}
void odImage_destroy(odImage* image) {
	if (!OD_CHECK(image != nullptr)) {
		return;
	}

	image->height = 0;
	image->width = 0;
	odAllocation_destroy(&image->allocation);
}
bool odImage_copy(odImage* image, const odImage* src_image) {
	if (!OD_CHECK(image != nullptr)
		|| !OD_CHECK(src_image != nullptr)) {
		return false;
	}

	odImage_destroy(image);

	int32_t size = static_cast<int32_t>(sizeof(odColorRGBA32)) * src_image->width * src_image->height;
	if (size == 0) {
		return true;
	}

	if (!OD_CHECK(odAllocation_init(&image->allocation, size))) {
		return false;
	}

	void* image_data = odAllocation_get(&image->allocation);
	if (!OD_CHECK(image_data != nullptr)) {
		return false;
	}

	const void* src_image_data = odAllocation_get_const(&src_image->allocation);
	if (OD_CHECK(src_image_data != nullptr)) {
		return false;
	}

	memcpy(image_data, src_image_data, static_cast<size_t>(size));
	image->width = src_image->width;
	image->height = src_image->height;

	return true;
}
void odImage_swap(odImage* image1, odImage* image2) {
	if (!OD_CHECK(image1 != nullptr)
		|| !OD_CHECK(image2 != nullptr)) {
		return;
	}

	int32_t swap_width = image1->width;
	int32_t swap_height = image1->height;

	image1->width = image2->width;
	image1->height = image2->height;

	image2->width = swap_width;
	image2->height = swap_height;

	odAllocation_swap(&image1->allocation, &image2->allocation);
}
bool odImage_resize(odImage* image, int32_t new_width, int32_t new_height) {
	if (!OD_CHECK(image != nullptr)
		|| !OD_CHECK(new_width >= 0)
		|| !OD_CHECK(new_height >= 0)) {
		return false;
	}

	odImage new_image;

	if (!OD_CHECK(odImage_init(&new_image, new_width, new_height))) {
		return false;
	}

	int32_t copy_width = image->width < new_width ? image->width : new_width;
	int32_t copy_height = image->height < new_height ? image->height : new_height;
	odColorRGBA32* dest_pos = odImage_get(&new_image);
	const odColorRGBA32* src_pos = odImage_get(image);
	const odColorRGBA32* src_end_pos = src_pos + (copy_height * image->width);
	if (!OD_DEBUG_CHECK(copy_width >= 0)
		|| !OD_DEBUG_CHECK(copy_height >= 0)
		|| !OD_CHECK((dest_pos != nullptr) || (copy_width == 0) || (copy_height == 0))
		|| !OD_CHECK((src_pos != nullptr) || (copy_width == 0) || (copy_height == 0))
		|| !OD_DEBUG_CHECK(src_end_pos >= src_pos)) {
		return false;
	}

	size_t row_size = sizeof(odColorRGBA32) * static_cast<size_t>(copy_width);
	while (src_pos < src_end_pos) {
		memcpy(dest_pos, src_pos, row_size);
		src_pos += image->width;
		dest_pos += new_width;
	}

	odImage_swap(image, &new_image);

	return true;
}
void odImage_get_size(const odImage* image, int32_t* out_opt_width, int32_t* out_opt_height) {
	int32_t unused;
	out_opt_width = (out_opt_width != nullptr) ? out_opt_width : &unused;
	out_opt_height = (out_opt_height != nullptr) ? out_opt_height : &unused;

	*out_opt_width = 0;
	*out_opt_height = 0;

	if (!OD_CHECK(odImage_check_valid(image))) {
		return;
	}

	*out_opt_width = image->width;
	*out_opt_height = image->height;
}
bool odImage_read_png(odImage* image, const void* src_png, int32_t src_png_size) {
	if (!OD_CHECK(image != nullptr)
		|| !OD_CHECK(src_png != nullptr)
		|| !OD_CHECK(src_png_size > 0)) {
		return false;
	}

	png_image png;
	memset(static_cast<void*>(&png), 0, sizeof(png));
	png.version = PNG_IMAGE_VERSION;

	if (!OD_CHECK(png_image_begin_read_from_memory(&png, src_png, static_cast<size_t>(src_png_size)) != 0)) {
		return false;
	}

	png.format = PNG_FORMAT_RGBA;

	image->width = static_cast<int32_t>(png.width);
	image->height = static_cast<int32_t>(png.height);
	if (!OD_CHECK(PNG_IMAGE_SIZE(png) == static_cast<unsigned>(image->width * image->height * static_cast<int32_t>(sizeof(odColorRGBA32))))) {
		return false;
	}

	if (!OD_CHECK(odImage_init(image, image->width, image->height))) {
		return false;
	}

	void* image_data = odAllocation_get(&image->allocation);
	if (!OD_CHECK(image_data != nullptr)) {
		return false;
	}

	if (!OD_CHECK(png_image_finish_read(&png, /*background*/ nullptr, image_data, /*row_stride*/ 0, /*colormap*/ nullptr) != 0)) {
		return false;
	}

	return true;
}
bool odImage_read_png_file(odImage* image, const char* file_path) {
	if (!OD_CHECK(image != nullptr)
		|| !OD_CHECK(file_path != nullptr)
		|| !OD_CHECK(odFilePath_get_exists(file_path))) {
		return false;
	}

	odImage_destroy(image);

	struct odAllocation allocation{};
	int32_t src_png_size = 0;
	if (!OD_CHECK(odFilePath_read_all(file_path, "rb", &allocation, &src_png_size))) {
		return false;
	}

	void* src_png = odAllocation_get(&allocation);
	if (!OD_CHECK(src_png != nullptr)
		|| !OD_CHECK(src_png_size > 0)) {
		return false;
	}

	if (!OD_CHECK(odImage_read_png(image, src_png, src_png_size))) {
		return false;
	}

	return true;
}
odColorRGBA32* odImage_get(odImage* image) {
	if (!OD_CHECK(odImage_check_valid(image))) {
		return nullptr;
	}

	return static_cast<odColorRGBA32*>(odAllocation_get(&image->allocation));
}
const odColorRGBA32* odImage_get_const(const odImage* image) {
	return odImage_get(const_cast<odImage*>(image));
}
odImage::odImage() : allocation{}, width{0}, height{0} {
}
odImage::odImage(odImage const& other) : odImage{} {
	if (!OD_CHECK(odImage_copy(this, &other))) {
		return;
	}
}
odImage::odImage(odImage&& other) : odImage{} {
	odImage_swap(this, &other);
}
odImage& odImage::operator=(const odImage& other) {
	if (!OD_CHECK(odImage_copy(this, &other))) {
		return *this;
	}
	return *this;
}
odImage& odImage::operator=(odImage&& other) {
	odImage_swap(this, &other);
	return *this;
}
odImage::~odImage() {
	odImage_destroy(this);
}
