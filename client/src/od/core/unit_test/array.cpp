#include <od/core/array.hpp>

#include <cstring>

#include <gtest/gtest.h>

#include <od/core/debug.hpp>
#include <od/core/type.hpp>

TEST(odArray, swap) {
	odArray array1{odType_get_char()};
	odArray array2{odType_get_char()};
	ASSERT_TRUE(odArray_set_count(&array1, 1));
	ASSERT_TRUE(odArray_set_count(&array2, 2));
	ASSERT_NE(odArray_get(&array1, 0), nullptr);
	ASSERT_NE(odArray_get(&array2, 0), nullptr);

	void* ptr1_old_ptr = odArray_get(&array1, 0);
	void* ptr2_old_ptr = odArray_get(&array2, 0);

	odArray_swap(&array1, &array2);
	ASSERT_EQ(odArray_get(&array1, 0), ptr2_old_ptr);
	ASSERT_EQ(odArray_get_count(&array1), 2);
	ASSERT_EQ(odArray_get(&array2, 0), ptr1_old_ptr);
	ASSERT_EQ(odArray_get_count(&array2), 1);
}
TEST(odArray, release) {
	odArray array{odType_get_char()};

	ASSERT_TRUE(odArray_set_count(&array, 1));
	ASSERT_NE(odArray_get(&array, 0), nullptr);
	ASSERT_EQ(odArray_get_count(&array), 1);
	ASSERT_GE(odArray_get_capacity(&array), 1);

	odArray_release(&array);
	ASSERT_EQ(odArray_get_count(&array), 0);
	ASSERT_EQ(odArray_get_capacity(&array), 0);
}
TEST(odArray, set_capacity) {
	int32_t testSizes[] = {1, 4, 16, 64, (64 * 1024), (4 * 1024 * 1024)};
	const int32_t numTestSizes = (sizeof(testSizes) / sizeof(testSizes[0]));

	for (int32_t i = 0; i < numTestSizes; i++) {
		odArray array{odType_get_char()};
		ASSERT_TRUE(odArray_set_capacity(&array, testSizes[i]));
		ASSERT_TRUE(odArray_get_capacity(&array) == testSizes[i]);
		ASSERT_EQ(odArray_get_count(&array), 0);
	}
}
TEST(odArray, set_capacity_zero) {
	odArray array{odType_get_char()};
	ASSERT_TRUE(odArray_set_capacity(&array, 0));
}
TEST(odArray, ensure_capacity) {
	odArray array{odType_get_char()};

	const int32_t start_capacity = 2;
	ASSERT_TRUE(odArray_set_capacity(&array, start_capacity));
	ASSERT_EQ(odArray_get_capacity(&array), start_capacity);
	ASSERT_EQ(odArray_get_count(&array), 0);

	ASSERT_TRUE(odArray_ensure_capacity(&array, start_capacity - 1));
	ASSERT_EQ(odArray_get_capacity(&array), start_capacity);
	ASSERT_EQ(odArray_get_count(&array), 0);

	ASSERT_TRUE(odArray_ensure_capacity(&array, (start_capacity + 1)));
	ASSERT_TRUE(odArray_get_capacity(&array) >= (start_capacity + 1));
	ASSERT_EQ(odArray_get_count(&array), 0);
}
TEST(odArray, set_count) {
	odArray array{odType_get_char()};
	ASSERT_TRUE(odArray_set_count(&array, 1));
	ASSERT_GE(odArray_get_capacity(&array), 1);
	ASSERT_EQ(odArray_get_count(&array), 1);
}
TEST(odArray, set_count_expand) {
	odArray array{odType_get_char()};
	ASSERT_TRUE(odArray_set_capacity(&array, 1));
	ASSERT_TRUE(odArray_set_count(&array, 1));
	ASSERT_EQ(odArray_get_capacity(&array), 1);
	ASSERT_EQ(odArray_get_count(&array), 1);

	char* array_ptr = static_cast<char*>(odArray_get(&array, 0));
	ASSERT_NE(array_ptr, nullptr);

	array_ptr[0] = '!';

	const int32_t final_count = 4;
	ASSERT_TRUE(odArray_set_count(&array, final_count));
	ASSERT_GE(odArray_get_capacity(&array), final_count);
	ASSERT_EQ(odArray_get_count(&array), final_count);
	ASSERT_NE(static_cast<char*>(odArray_get(&array, 0)), array_ptr);

	array_ptr = static_cast<char*>(odArray_get(&array, 0));
	ASSERT_NE(array_ptr, nullptr);
	ASSERT_TRUE(array_ptr[0] == '!');
	for (int32_t i = 1; i < final_count; i++) {
		array_ptr[i] = '\0';
	}
}
TEST(odArray, set_count_truncate) {
	odArray array{odType_get_char()};
	const int32_t start_count = 4;
	ASSERT_TRUE(odArray_set_count(&array, start_count));

	char* array_ptr = static_cast<char*>(odArray_get(&array, 0));
	ASSERT_NE(array_ptr, nullptr);
	for (int32_t i = 0; i < start_count; i++) {
		array_ptr[i] = '!';
	}

	ASSERT_TRUE(odArray_set_count(&array, 1));
	ASSERT_TRUE(odArray_set_count(&array, 4));
	ASSERT_EQ(static_cast<char*>(odArray_get(&array, 0)), array_ptr);
	ASSERT_TRUE(array_ptr[0] == '!');
	for (int32_t i = 1; i < start_count; i++) {
		ASSERT_TRUE(array_ptr[i] == '\0');
	}
}
TEST(odArray, shrink) {
	odArray array{odType_get_char()};
	ASSERT_TRUE(odArray_set_count(&array, 1));
	ASSERT_EQ(odArray_get_count(&array), 1);

	ASSERT_TRUE(odArray_shrink(&array, 1));
	ASSERT_EQ(odArray_get_count(&array), 0);
}
TEST(odArray, swap_pop) {
	odArray array{odType_get_char()};
	ASSERT_TRUE(odArray_set_count(&array, 4));

	char* array_ptr = static_cast<char*>(odArray_get(&array, 0));
	ASSERT_NE(array_ptr, nullptr);
	strncpy(array_ptr, "123", 4);

	ASSERT_TRUE(odArray_swap_pop(&array, 1));
	ASSERT_TRUE(odArray_set_count(&array, 4));
	array_ptr = static_cast<char*>(odArray_get(&array, 0));
	ASSERT_NE(array_ptr, nullptr);
	ASSERT_TRUE(strncmp(array_ptr, "13\0\0", 4));
}
TEST(odArray, get_out_of_bounds_fails) {
	odArray array{odType_get_char()};
	ASSERT_TRUE(odArray_set_count(&array, 2));
	ASSERT_NE(odArray_get(&array, 0), nullptr);
	{
		odLogLevelScoped suppress_logs{OD_LOG_LEVEL_NONE};
		ASSERT_EQ(odArray_get(&array, 2), nullptr);
	}
}
