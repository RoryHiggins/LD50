#include <od/core/type.hpp>

#include <od/test/test.hpp>

OD_TEST(odType, index) {
	 const int32_t array_size = 4;
	 int32_t array[array_size];
	 OD_ASSERT(
	 	static_cast<int32_t*>(odType_index(odType_get<int32_t>(), static_cast<void*>(array), array_size))
	 	== (array + array_size));
}
