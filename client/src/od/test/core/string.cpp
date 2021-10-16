#include <od/core/string.hpp>

#include <cstring>

#include <od/test/test.hpp>

OD_TEST(odString, copy) {
	odString str1;
	OD_ASSERT(odString_push(&str1, "yep", 3));

	odString str2;
	OD_ASSERT(odString_copy(&str2, &str1));

	const char* str2_data = odString_get_const(&str2, 0);
	OD_ASSERT(str2_data != nullptr);
	OD_ASSERT(strncmp(str2_data, "yep", 3) == 0);
}
OD_TEST(odString, ensure_null_terminated) {
	odString str;
	OD_ASSERT(!odString_get_null_terminated(&str));
	OD_ASSERT(odString_get_count(&str) == 0);
	OD_ASSERT(odString_get_capacity(&str) == 0);

	OD_ASSERT(odString_ensure_null_terminated(&str));
	OD_ASSERT(odString_get_null_terminated(&str));
	OD_ASSERT(odString_get_count(&str) == 1);

	const char* str_data = odString_get_const(&str, 0);
	OD_ASSERT(str_data != nullptr);
	OD_ASSERT(strncmp(str_data, "\0", 1) == 0);
}
OD_TEST(odString, push_formatted) {
	odString str;
	OD_ASSERT(odString_push_formatted(&str, "%s %d", "yep", 123));

	const char* str_data = odString_get_const(&str, 0);
	OD_ASSERT(str_data != nullptr);
	OD_ASSERT(strncmp(str_data, "yep 123", 7) == 0);
}
