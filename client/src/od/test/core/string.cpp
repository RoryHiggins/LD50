#include <od/core/string.hpp>

#include <cstring>

#include <od/test/test.hpp>

OD_TEST(odTest_odString_swap) {
	odString string1;
	odString string2;
	OD_ASSERT(odString_set_count(&string1, 1));
	OD_ASSERT(odString_set_count(&string2, 2));
	OD_ASSERT(odString_get(&string1, 0) != nullptr);
	OD_ASSERT(odString_get(&string2, 0) != nullptr);

	void* ptr1_old_ptr = odString_get(&string1, 0);
	void* ptr2_old_ptr = odString_get(&string2, 0);

	odString_swap(&string1, &string2);
	OD_ASSERT(odString_get(&string1, 0) == ptr2_old_ptr);
	OD_ASSERT(odString_get_count(&string1) == 2);
	OD_ASSERT(odString_get(&string2, 0) == ptr1_old_ptr);
	OD_ASSERT(odString_get_count(&string2) == 1);
}
OD_TEST(odTest_odString_init_destroy) {
	odString string;

	OD_ASSERT(odString_set_count(&string, 1));
	OD_ASSERT(odString_get(&string, 0) != nullptr);
	OD_ASSERT(odString_get_count(&string) == 1);
	OD_ASSERT(odString_get_capacity(&string) >= 1);

	odString_destroy(&string);
	OD_ASSERT(odString_get_count(&string) == 0);
	OD_ASSERT(odString_get_capacity(&string) == 0);
}
OD_TEST(odTest_odString_set_capacity) {
	odString string;
	int32_t test_sizes[] = {1, 4, 16, 64, (64 * 1024), (4 * 1024 * 1024)};
	for (int32_t test_size: test_sizes) {
		OD_ASSERT(odString_set_capacity(&string, test_size));
		OD_ASSERT(odString_get_capacity(&string) == test_size);
		OD_ASSERT(odString_get_count(&string) == 0);
	}
}
OD_TEST(odTest_odString_set_capacity_zero) {
	odString string;
	OD_ASSERT(odString_set_capacity(&string, 0));
}
OD_TEST(odTest_odString_ensure_capacity) {
	odString string;

	const int32_t start_capacity = 2;
	OD_ASSERT(odString_set_capacity(&string, start_capacity));
	OD_ASSERT(odString_get_capacity(&string) == start_capacity);
	OD_ASSERT(odString_get_count(&string) == 0);

	OD_ASSERT(odString_ensure_capacity(&string, start_capacity - 1));
	OD_ASSERT(odString_get_capacity(&string) == start_capacity);
	OD_ASSERT(odString_get_count(&string) == 0);

	OD_ASSERT(odString_ensure_capacity(&string, (start_capacity + 1)));
	OD_ASSERT(odString_get_capacity(&string) >= (start_capacity + 1));
	OD_ASSERT(odString_get_count(&string) == 0);
}
OD_TEST(odTest_odString_set_count) {
	odString string;
	OD_ASSERT(odString_set_count(&string, 1));
	OD_ASSERT(odString_get_capacity(&string) >= 1);
	OD_ASSERT(odString_get_count(&string) == 1);
}
OD_TEST(odTest_odString_set_count_expand) {
	odString string;
	OD_ASSERT(odString_set_capacity(&string, 1));
	OD_ASSERT(odString_get_capacity(&string) == 1);

	OD_ASSERT(odString_set_count(&string, 1));
	OD_ASSERT(odString_get_count(&string) == 1);
	OD_ASSERT(odString_get_capacity(&string) >= 1);

	char* array_ptr = static_cast<char*>(odString_get(&string, 0));
	OD_ASSERT(array_ptr != nullptr);

	array_ptr[0] = '!';

	int32_t final_count = odString_get_capacity(&string) + 1;
	OD_ASSERT(odString_set_count(&string, final_count));
	OD_ASSERT(odString_get_capacity(&string) >= final_count);
	OD_ASSERT(odString_get_count(&string) == final_count);

	array_ptr = static_cast<char*>(odString_get(&string, 0));
	OD_ASSERT(array_ptr != nullptr);
	OD_ASSERT(array_ptr[0] == '!');
}
OD_TEST(odTest_odString_set_count_truncate) {
	odString string;
	const int32_t start_count = 4;
	OD_ASSERT(odString_set_count(&string, start_count));

	char* array_ptr = static_cast<char*>(odString_get(&string, 0));
	OD_ASSERT(array_ptr != nullptr);
	for (int32_t i = 0; i < start_count; i++) {
		array_ptr[i] = '!';
	}

	OD_ASSERT(odString_set_count(&string, 1));
	OD_ASSERT(odString_set_count(&string, 4));
	OD_ASSERT(static_cast<char*>(odString_get(&string, 0)) == array_ptr);
	OD_ASSERT(array_ptr[0] == '!');
	for (int32_t i = 1; i < start_count; i++) {
		OD_ASSERT(array_ptr[i] == '\0');
	}
}
OD_TEST(odTest_odString_debug_get_out_of_bounds_fails) {
	if (OD_BUILD_DEBUG) {
		odString string;
		OD_ASSERT(odString_set_count(&string, 2));
		OD_ASSERT(odString_get(&string, 0) != nullptr);
		{
			odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
			OD_ASSERT(odString_get(&string, 2) == nullptr);
		}
	}
}
OD_TEST(odTest_odString_compare) {
	odString string1;
	odString string2;

	OD_ASSERT(odString_compare(&string1, &string2) == 0);  // {"", ""}

	const char* hello = "hello ";
	const char* loser = "loser ";

	OD_ASSERT(odString_push(&string1, hello, static_cast<int32_t>(strlen(hello))));
	OD_ASSERT(odString_compare(&string1, &string2) == 1);  // {"hello", ""}

	OD_ASSERT(odString_push(&string2, hello, static_cast<int32_t>(strlen(hello))));
	OD_ASSERT(odString_compare(&string1, &string2) == 0);  // {"hello", "hello"}

	OD_ASSERT(odString_push(&string2, loser, static_cast<int32_t>(strlen(loser))));
	OD_ASSERT(odString_compare(&string1, &string2) < 0);  // {"hello ", "hello loser "}

	OD_ASSERT(odString_set_count(&string1, 0));
	OD_ASSERT(odString_set_count(&string2, 0));
	OD_ASSERT(odString_push(&string1, hello, static_cast<int32_t>(strlen(hello))));
	OD_ASSERT(odString_push(&string2, loser, static_cast<int32_t>(strlen(loser))));
	OD_ASSERT(odString_compare(&string1, &string2) < 0);  // {"hello ", "loser "}
}
OD_TEST(odTest_odString_copy) {
	odString str1;
	OD_ASSERT(odString_push(&str1, "yep", 3));

	odString str2;
	OD_ASSERT(odString_copy(&str2, &str1));

	const char* str2_data = odString_get_const(&str2, 0);
	OD_ASSERT(str2_data != nullptr);
	OD_ASSERT(strncmp(str2_data, "yep", 3) == 0);
}
OD_TEST(odTest_odString_push_formatted) {
	odString str;
	OD_ASSERT(odString_push_formatted(&str, "%s %d", "yep", 123));

	const char* str_data = odString_get_const(&str, 0);
	OD_ASSERT(str_data != nullptr);
	OD_ASSERT(strncmp(str_data, "yep 123", 7) == 0);
}

OD_TEST_SUITE(
	odTestSuite_odString,
	odTest_odString_swap,
	odTest_odString_init_destroy,
	odTest_odString_set_capacity,
	odTest_odString_set_capacity_zero,
	odTest_odString_ensure_capacity,
	odTest_odString_set_count,
	odTest_odString_set_count_expand,
	odTest_odString_set_count_truncate,
	odTest_odString_debug_get_out_of_bounds_fails,
	odTest_odString_compare,
	odTest_odString_copy,
	odTest_odString_push_formatted,
)
