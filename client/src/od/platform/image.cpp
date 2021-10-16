#include <od/platform/image.hpp>

#include <cstring>

#include <png.h>

#include <od/core/color.h>
#include <od/core/debug.h>
#include <od/core/type.hpp>

const odType* odImage_get_type_constructor() {
	return odType_get<odImage>();
}
bool odImage_copy(odImage* image, const odImage* src_image) {
	if (!OD_DEBUG_CHECK(image != nullptr)
		|| !OD_DEBUG_CHECK(src_image != nullptr)) {
		return false;
	}

	odImage_release(image);

	int32_t size = static_cast<int32_t>(sizeof(odColor)) * src_image->width * src_image->height;
	if (size == 0) {
		return true;
	}

	if (!OD_CHECK(odAllocation_allocate(&image->allocation, size))) {
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
	if (!OD_DEBUG_CHECK(image1 != nullptr)
		|| !OD_DEBUG_CHECK(image2 != nullptr)) {
		return;
	}

	int32_t swap_width = image1->width;
	int32_t swap_height = image1->height;

	image1->width = image2->width;
	image1->height = image2->height;

	image2->width = swap_width;
	image2->height = swap_height;
}
bool odImage_get_valid(const odImage* image) {
	if ((image == nullptr)
		|| (!odAllocation_get_valid(&image->allocation))
		|| (image->width < 0)
		|| (image->height < 0)) {
		return false;
	}
	
	return true;
}
const char* odImage_get_debug_string(const odImage* image) {
	if (image == nullptr) {
		return "odImage{this=nullptr}";
	}

	return odDebugString_format(
		"odImage{this=%p, allocation=%s, width=%d, height=%d}",
		static_cast<const void*>(image),
		odAllocation_get_debug_string(&image->allocation),
		image->width,
		image->height);
}
bool odImage_allocate(odImage* image, int32_t width, int32_t height) {
	if (!OD_DEBUG_CHECK(image != nullptr)
		|| !OD_DEBUG_CHECK(width >= 0)
		|| !OD_DEBUG_CHECK(height >= 0)) {
		return false;
	}

	odImage_release(image);

	int32_t size = static_cast<int32_t>(sizeof(odColor)) * width * height;
	if (size == 0) {
		return true;
	}

	if (!OD_CHECK(odAllocation_allocate(&image->allocation, size))) {
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
void odImage_release(odImage* image) {
	if (!OD_DEBUG_CHECK(image != nullptr)) {
		return;
	}

	image->height = 0;
	image->width = 0;
	odAllocation_release(&image->allocation);
}
void odImage_get_size(const odImage* image, int32_t* out_opt_width, int32_t* out_opt_height) {
	int32_t unused;
	out_opt_width = (out_opt_width != nullptr) ? out_opt_width : &unused;
	out_opt_height = (out_opt_height != nullptr) ? out_opt_height : &unused;

	*out_opt_width = 0;
	*out_opt_height = 0;

	if (!OD_DEBUG_CHECK(odImage_get_valid(image))) {
		return;
	}

	*out_opt_width = image->width;
	*out_opt_height = image->height;
}
bool odImage_read_png(odImage* image, const void* src_png, int32_t src_png_size) {
	if (!OD_DEBUG_CHECK(image != nullptr)
		|| !OD_DEBUG_CHECK(src_png != nullptr)
		|| !OD_DEBUG_CHECK(src_png_size > 0)) {
		return false;
	}

	png_image png;
	memset(static_cast<void*>(&png), 0, sizeof(png));
	png.version = PNG_IMAGE_VERSION;

	if (!OD_CHECK(png_image_begin_read_from_memory(&png, src_png, static_cast<size_t>(src_png_size)) != 0)) {
		return false;
	}

	png.format = PNG_FORMAT_RGBA;

	int32_t width = static_cast<int32_t>(png.width);
	int32_t height = static_cast<int32_t>(png.height);
	if (!OD_DEBUG_CHECK(PNG_IMAGE_SIZE(png) == static_cast<unsigned>(width * height * static_cast<int32_t>(sizeof(odColor))))) {
		return false;
	}

	if (!OD_CHECK(odImage_allocate(image, width, height))) {
		return false;
	}

	void* image_data = odAllocation_get(&image->allocation);
	if (!OD_DEBUG_CHECK(image_data != nullptr)) {
		return false;
	}

	if (!OD_CHECK(png_image_finish_read(&png, /*background*/ nullptr, image_data, /*row_stride*/ 0, /*colormap*/ nullptr) != 0)) {
		return false;
	}

	image->width = width;
	image->height = height;

	return true;
}
odColor* odImage_get(odImage* image) {
	if (!OD_DEBUG_CHECK(odImage_get_valid(image))) {
		return nullptr;
	}

	return static_cast<odColor*>(odAllocation_get(&image->allocation));
}
const odColor* odImage_get_const(const odImage* image) {
	return odImage_get(const_cast<odImage*>(image));
}

odImage::odImage() : allocation{}, width{0}, height{0} {
}
odImage::odImage(odImage const& other) : odImage{} {
	OD_ASSERT(odImage_copy(this, &other));
}
odImage::odImage(odImage&& other) : odImage{} {
	odImage_swap(this, &other);
}
odImage& odImage::operator=(const odImage& other) {
	OD_ASSERT(odImage_copy(this, &other));
	return *this;
}
odImage& odImage::operator=(odImage&& other) {
	odImage_swap(this, &other);
	return *this;
}
odImage::~odImage() {
	odImage_release(this);
}
