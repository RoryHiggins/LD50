#include <od/core/containers.hpp>

#include <cstring>

#include <od/test/test.hpp>

OD_TEST(odAllocation, swap) {
	odAllocation allocation1;
	odAllocation allocation2;
	OD_ASSERT(odAllocation_allocate(&allocation1, 1));
	OD_ASSERT(odAllocation_allocate(&allocation2, 2));
	OD_ASSERT(odAllocation_get(&allocation1) != nullptr);
	OD_ASSERT(odAllocation_get(&allocation2) != nullptr);

	void* allocation1_old_ptr = odAllocation_get(&allocation1);
	void* allocation2_old_ptr = odAllocation_get(&allocation2);

	odAllocation_swap(&allocation1, &allocation2);
	OD_ASSERT(odAllocation_get(&allocation1) == allocation2_old_ptr);
	OD_ASSERT(odAllocation_get(&allocation2) == allocation1_old_ptr);
}
OD_TEST(odAllocation, swap_unallocated) {
	odAllocation allocation1;
	odAllocation allocation2;
	OD_ASSERT(odAllocation_allocate(&allocation1, 1));
	OD_ASSERT(odAllocation_get(&allocation1) != nullptr);
	OD_ASSERT(odAllocation_get(&allocation2) == nullptr);

	odAllocation_swap(&allocation1, &allocation2);
	OD_ASSERT(odAllocation_get(&allocation1) == nullptr);
	OD_ASSERT(odAllocation_get(&allocation2) != nullptr);
}
OD_TEST(odAllocation, release) {
	odAllocation allocation;

	OD_ASSERT(odAllocation_allocate(&allocation, 1));
	OD_ASSERT(odAllocation_get(&allocation) != nullptr);

	odAllocation_release(&allocation);
	OD_ASSERT(odAllocation_get(&allocation) == nullptr);
}
OD_TEST(odAllocation, allocate) {
	int32_t testSizes[] = {1, 4, 16, 64, (64 * 1024), (4 * 1024 * 1024)};
	const int32_t numTestSizes = (sizeof(testSizes) / sizeof(testSizes[0]));

	for (int32_t i = 0; i < numTestSizes; i++) {
		odAllocation allocation;
		OD_ASSERT(odAllocation_allocate(&allocation, testSizes[i]));
		OD_ASSERT(odAllocation_get(&allocation) != nullptr);
	}
}
OD_TEST(odAllocation, allocate_zero) {
	odAllocation allocation;
	OD_ASSERT(odAllocation_allocate(&allocation, 0));
	OD_ASSERT(odAllocation_get(&allocation) == nullptr);
}
OD_TEST(odAllocation, get) {
	odAllocation allocation;
	OD_ASSERT(odAllocation_allocate(&allocation, 1));
	OD_ASSERT(odAllocation_get(&allocation) != nullptr);
}
OD_TEST(odAllocation, get_unallocated_fails) {
	odAllocation allocation;
	{
		odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
		OD_ASSERT(odAllocation_get(&allocation) == nullptr);
	}
}

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
		odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
		OD_ASSERT(odArray_get(&array, 2) == nullptr);
	}
}


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
