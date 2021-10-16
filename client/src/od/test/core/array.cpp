#include <od/core/array.hpp>

#include <cstring>

#include <od/test/test.hpp>
#include <od/core/type.hpp>

OD_TEST(odArray, swap) {
	odArray array1{odType_get_char()};
	odArray array2{odType_get_char()};
	OD_ASSERT(odArray_set_count(&array1, 1));
	OD_ASSERT(odArray_set_count(&array2, 2));
	OD_ASSERT(odArray_get(&array1, 0) != nullptr);
	OD_ASSERT(odArray_get(&array2, 0) != nullptr);

	void* ptr1_old_ptr = odArray_get(&array1, 0);
	void* ptr2_old_ptr = odArray_get(&array2, 0);

	odArray_swap(&array1, &array2);
	OD_ASSERT(odArray_get(&array1, 0) == ptr2_old_ptr);
	OD_ASSERT(odArray_get_count(&array1) == 2);
	OD_ASSERT(odArray_get(&array2, 0) == ptr1_old_ptr);
	OD_ASSERT(odArray_get_count(&array2) == 1);
}
OD_TEST(odArray, release) {
	odArray array{odType_get_char()};

	OD_ASSERT(odArray_set_count(&array, 1));
	OD_ASSERT(odArray_get(&array, 0) != nullptr);
	OD_ASSERT(odArray_get_count(&array) == 1);
	OD_ASSERT(odArray_get_capacity(&array) >= 1);

	odArray_release(&array);
	OD_ASSERT(odArray_get_count(&array) == 0);
	OD_ASSERT(odArray_get_capacity(&array) == 0);
}
OD_TEST(odArray, set_capacity) {
	int32_t testSizes[] = {1, 4, 16, 64, (64 * 1024), (4 * 1024 * 1024)};
	const int32_t numTestSizes = (sizeof(testSizes) / sizeof(testSizes[0]));

	for (int32_t i = 0; i < numTestSizes; i++) {
		odArray array{odType_get_char()};
		OD_ASSERT(odArray_set_capacity(&array, testSizes[i]));
		OD_ASSERT(odArray_get_capacity(&array) == testSizes[i]);
		OD_ASSERT(odArray_get_count(&array) == 0);
	}
}
OD_TEST(odArray, set_capacity_zero) {
	odArray array{odType_get_char()};
	OD_ASSERT(odArray_set_capacity(&array, 0));
}
OD_TEST(odArray, ensure_capacity) {
	odArray array{odType_get_char()};

	const int32_t start_capacity = 2;
	OD_ASSERT(odArray_set_capacity(&array, start_capacity));
	OD_ASSERT(odArray_get_capacity(&array) == start_capacity);
	OD_ASSERT(odArray_get_count(&array) == 0);

	OD_ASSERT(odArray_ensure_capacity(&array, start_capacity - 1));
	OD_ASSERT(odArray_get_capacity(&array) == start_capacity);
	OD_ASSERT(odArray_get_count(&array) == 0);

	OD_ASSERT(odArray_ensure_capacity(&array, (start_capacity + 1)));
	OD_ASSERT(odArray_get_capacity(&array) >= (start_capacity + 1));
	OD_ASSERT(odArray_get_count(&array) == 0);
}
OD_TEST(odArray, set_count) {
	odArray array{odType_get_char()};
	OD_ASSERT(odArray_set_count(&array, 1));
	OD_ASSERT(odArray_get_capacity(&array) >= 1);
	OD_ASSERT(odArray_get_count(&array) == 1);
}
OD_TEST(odArray, set_count_expand) {
	odArray array{odType_get_char()};
	OD_ASSERT(odArray_set_capacity(&array, 1));
	OD_ASSERT(odArray_set_count(&array, 1));
	OD_ASSERT(odArray_get_capacity(&array) == 1);
	OD_ASSERT(odArray_get_count(&array) == 1);

	char* array_ptr = static_cast<char*>(odArray_get(&array, 0));
	OD_ASSERT(array_ptr != nullptr);

	array_ptr[0] = '!';

	const int32_t final_count = 4;
	OD_ASSERT(odArray_set_count(&array, final_count));
	OD_ASSERT(odArray_get_capacity(&array) >= final_count);
	OD_ASSERT(odArray_get_count(&array) == final_count);
	OD_ASSERT(static_cast<char*>(odArray_get(&array, 0)) != array_ptr);

	array_ptr = static_cast<char*>(odArray_get(&array, 0));
	OD_ASSERT(array_ptr != nullptr);
	OD_ASSERT(array_ptr[0] == '!');
	for (int32_t i = 1; i < final_count; i++) {
		array_ptr[i] = '\0';
	}
}
OD_TEST(odArray, set_count_truncate) {
	odArray array{odType_get_char()};
	const int32_t start_count = 4;
	OD_ASSERT(odArray_set_count(&array, start_count));

	char* array_ptr = static_cast<char*>(odArray_get(&array, 0));
	OD_ASSERT(array_ptr != nullptr);
	for (int32_t i = 0; i < start_count; i++) {
		array_ptr[i] = '!';
	}

	OD_ASSERT(odArray_set_count(&array, 1));
	OD_ASSERT(odArray_set_count(&array, 4));
	OD_ASSERT(static_cast<char*>(odArray_get(&array, 0)) == array_ptr);
	OD_ASSERT(array_ptr[0] == '!');
	for (int32_t i = 1; i < start_count; i++) {
		OD_ASSERT(array_ptr[i] == '\0');
	}
}
OD_TEST(odArray, shrink) {
	odArray array{odType_get_char()};
	OD_ASSERT(odArray_set_count(&array, 1));
	OD_ASSERT(odArray_get_count(&array) == 1);

	OD_ASSERT(odArray_shrink(&array, 1));
	OD_ASSERT(odArray_get_count(&array) == 0);
}
OD_TEST(odArray, swap_pop) {
	odArray array{odType_get_char()};
	OD_ASSERT(odArray_set_count(&array, 4));

	char* array_ptr = static_cast<char*>(odArray_get(&array, 0));
	OD_ASSERT(array_ptr != nullptr);
	strncpy(array_ptr, "123", 4);

	OD_ASSERT(odArray_swap_pop(&array, 1));
	OD_ASSERT(odArray_set_count(&array, 4));
	array_ptr = static_cast<char*>(odArray_get(&array, 0));
	OD_ASSERT(array_ptr != nullptr);
	OD_ASSERT(strncmp(array_ptr, "13\0\0", 4));
}
OD_TEST(odArray, get_out_of_bounds_fails) {
	odArray array{odType_get_char()};
	OD_ASSERT(odArray_set_count(&array, 2));
	OD_ASSERT(odArray_get(&array, 0) != nullptr);
	{
		odLogLevelScoped suppress_logs{OD_LOG_LEVEL_NONE};
		OD_ASSERT(odArray_get(&array, 2) == nullptr);
	}
}
