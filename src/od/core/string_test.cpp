#include <od/core.h>
#include <od/core/string.hpp>

#include <string.h>

#include <gtest/gtest.h>

TEST(odString, copy) {
	odString str1;
	ASSERT_TRUE(odString_push(&str1, "yep", 3));

	odString str2;
	ASSERT_TRUE(odString_copy(&str2, &str1));

	const char* str2_data = odString_get_const(&str2, 0);
	ASSERT_NE(str2_data, nullptr);
	ASSERT_EQ(strncmp(str2_data, "yep", 3), 0);
}
TEST(odString, ensure_null_terminated) {
	odString str;
	ASSERT_FALSE(odString_get_null_terminated(&str));
	ASSERT_EQ(odString_get_count(&str), 0);
	ASSERT_EQ(odString_get_capacity(&str), 0);
	
	ASSERT_TRUE(odString_ensure_null_terminated(&str));
	ASSERT_TRUE(odString_get_null_terminated(&str));
	ASSERT_EQ(odString_get_count(&str), 1);
	
	const char* str_data = odString_get_const(&str, 0);
	ASSERT_NE(str_data, nullptr);
	ASSERT_EQ(strncmp(str_data, "\0", 1), 0);
}
TEST(odString, push_formatted) {
	odString str;
	ASSERT_TRUE(odString_push_formatted(&str, "%d%s", 1, "yep"));

	const char* str_data = odString_get_const(&str, 0);
	ASSERT_NE(str_data, nullptr);
	ASSERT_EQ(strncmp(str_data, "1yep", 4), 0);
}
