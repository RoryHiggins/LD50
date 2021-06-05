#include <od/core.h>
#include <od/core/string.hpp>

#include <string.h>

#include <gtest/gtest.h>

TEST(odString, ensure_null_terminated) {
	odString str;
	ASSERT_TRUE(odString_get_null_terminated(&str) == false);
	ASSERT_TRUE(odString_get_count(&str) == 0);
	ASSERT_TRUE(odString_get_capacity(&str) == 0);
	
	ASSERT_TRUE(odString_ensure_null_terminated(&str));
	ASSERT_TRUE(odString_get_null_terminated(&str));
	ASSERT_TRUE(odString_get_count(&str) == 1);
	
	const char* str_data = odString_get_const(&str, 0);
	ASSERT_TRUE(str_data != nullptr);
	ASSERT_TRUE(strncmp(str_data, "\0", 1) == 0);
}
TEST(odString, push_formatted) {
	odString str;
	ASSERT_TRUE(odString_push_formatted(&str, "%d%s", 1, "yep"));

	const char* str_data = odString_get_const(&str, 0);
	ASSERT_TRUE(str_data != nullptr);
	ASSERT_TRUE(strncmp(str_data, "1yep", 4) == 0);
}
