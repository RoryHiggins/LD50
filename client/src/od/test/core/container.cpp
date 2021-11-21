#include <od/core/container.hpp>

#include <cstring>

#include <od/test/test.hpp>

template struct odArrayT<int32_t>;
template struct odRangeT<int32_t>;
template OD_NO_DISCARD const odType* odType_get<int32_t>();

OD_TEST(odType, index) {
	 const int32_t array_size = 4;
	 int32_t array[array_size];
	 OD_ASSERT(
	 	static_cast<int32_t*>(odType_index(odType_get<int32_t>(), static_cast<void*>(array), array_size))
	 	== (array + array_size));
}

OD_TEST(odAllocation, swap) {
	odAllocation allocation1;
	odAllocation allocation2;
	OD_ASSERT(odAllocation_init(&allocation1, 1));
	OD_ASSERT(odAllocation_init(&allocation2, 2));
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
	OD_ASSERT(odAllocation_init(&allocation1, 1));
	OD_ASSERT(odAllocation_get(&allocation1) != nullptr);
	OD_ASSERT(odAllocation_get(&allocation2) == nullptr);

	odAllocation_swap(&allocation1, &allocation2);
	OD_ASSERT(odAllocation_get(&allocation1) == nullptr);
	OD_ASSERT(odAllocation_get(&allocation2) != nullptr);
}
OD_TEST(odAllocation, init_destroy) {
	odAllocation allocation;

	OD_ASSERT(odAllocation_init(&allocation, 1));
	OD_ASSERT(odAllocation_get(&allocation) != nullptr);

	// test multiple init
	OD_ASSERT(odAllocation_init(&allocation, 1));
	OD_ASSERT(odAllocation_get(&allocation) != nullptr);

	odAllocation_destroy(&allocation);
	OD_ASSERT(odAllocation_get(&allocation) == nullptr);

	// test multiple destroy
	odAllocation_destroy(&allocation);
	OD_ASSERT(odAllocation_get(&allocation) == nullptr);

	// test varying sizes
	int32_t test_sizes[] = {1, 4, 16, 64, (64 * 1024), (4 * 1024 * 1024)};
	for (int32_t test_size: test_sizes) {
		OD_ASSERT(odAllocation_init(&allocation, test_size));
		OD_ASSERT(odAllocation_get(&allocation) != nullptr);
	}
}
OD_TEST(odAllocation, allocate_zero) {
	odAllocation allocation;
	OD_ASSERT(odAllocation_init(&allocation, 0));
	OD_ASSERT(odAllocation_get(&allocation) == nullptr);
}
OD_TEST(odAllocation, get) {
	odAllocation allocation;
	OD_ASSERT(odAllocation_init(&allocation, 1));
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
OD_TEST(odArray, init_destroy) {
	odArray array{odType_get_char()};

	OD_ASSERT(odArray_set_count(&array, 1));
	OD_ASSERT(odArray_get(&array, 0) != nullptr);
	OD_ASSERT(odArray_get_count(&array) == 1);
	OD_ASSERT(odArray_get_capacity(&array) >= 1);

	odArray_destroy(&array);
	OD_ASSERT(odArray_get_count(&array) == 0);
	OD_ASSERT(odArray_get_capacity(&array) == 0);
}
OD_TEST(odArray, set_capacity) {
	odArray array{odType_get_char()};
	int32_t test_sizes[] = {1, 4, 16, 64, (64 * 1024), (4 * 1024 * 1024)};
	for (int32_t test_size: test_sizes) {
		OD_ASSERT(odArray_set_capacity(&array, test_size));
		OD_ASSERT(odArray_get_capacity(&array) == test_size);
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
OD_TEST(odArray, pop) {
	odArray array{odType_get_char()};
	OD_ASSERT(odArray_set_count(&array, 1));
	OD_ASSERT(odArray_get_count(&array) == 1);

	OD_ASSERT(odArray_pop(&array, 1));
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

OD_TEST(odString, swap) {
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
OD_TEST(odString, init_destroy) {
	odString string;

	OD_ASSERT(odString_set_count(&string, 1));
	OD_ASSERT(odString_get(&string, 0) != nullptr);
	OD_ASSERT(odString_get_count(&string) == 1);
	OD_ASSERT(odString_get_capacity(&string) >= 1);

	odString_destroy(&string);
	OD_ASSERT(odString_get_count(&string) == 0);
	OD_ASSERT(odString_get_capacity(&string) == 0);
}
OD_TEST(odString, set_capacity) {
	odString string;
	int32_t test_sizes[] = {1, 4, 16, 64, (64 * 1024), (4 * 1024 * 1024)};
	for (int32_t test_size: test_sizes) {
		OD_ASSERT(odString_set_capacity(&string, test_size));
		OD_ASSERT(odString_get_capacity(&string) == test_size);
		OD_ASSERT(odString_get_count(&string) == 0);
	}
}
OD_TEST(odString, set_capacity_zero) {
	odString string;
	OD_ASSERT(odString_set_capacity(&string, 0));
}
OD_TEST(odString, ensure_capacity) {
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
OD_TEST(odString, set_count) {
	odString string;
	OD_ASSERT(odString_set_count(&string, 1));
	OD_ASSERT(odString_get_capacity(&string) >= 1);
	OD_ASSERT(odString_get_count(&string) == 1);
}
OD_TEST(odString, set_count_expand) {
	odString string;
	OD_ASSERT(odString_set_capacity(&string, 1));
	OD_ASSERT(odString_set_count(&string, 1));
	OD_ASSERT(odString_get_capacity(&string) == 1);
	OD_ASSERT(odString_get_count(&string) == 1);

	char* array_ptr = static_cast<char*>(odString_get(&string, 0));
	OD_ASSERT(array_ptr != nullptr);

	array_ptr[0] = '!';

	const int32_t final_count = 4;
	OD_ASSERT(odString_set_count(&string, final_count));
	OD_ASSERT(odString_get_capacity(&string) >= final_count);
	OD_ASSERT(odString_get_count(&string) == final_count);
	OD_ASSERT(static_cast<char*>(odString_get(&string, 0)) != array_ptr);

	array_ptr = static_cast<char*>(odString_get(&string, 0));
	OD_ASSERT(array_ptr != nullptr);
	OD_ASSERT(array_ptr[0] == '!');
	for (int32_t i = 1; i < final_count; i++) {
		array_ptr[i] = '\0';
	}
}
OD_TEST(odString, set_count_truncate) {
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
OD_TEST(odString, get_out_of_bounds_fails) {
	odString string;
	OD_ASSERT(odString_set_count(&string, 2));
	OD_ASSERT(odString_get(&string, 0) != nullptr);
	{
		odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
		OD_ASSERT(odString_get(&string, 2) == nullptr);
	}
}
OD_TEST(odString, compare) {
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
	OD_ASSERT(odString_compare(&string1, &string2) == -1);  // {"hello ", "hello loser "}

	OD_ASSERT(odString_set_count(&string1, 0));
	OD_ASSERT(odString_set_count(&string2, 0));
	OD_ASSERT(odString_push(&string1, hello, static_cast<int32_t>(strlen(hello))));
	OD_ASSERT(odString_push(&string2, loser, static_cast<int32_t>(strlen(loser))));
	OD_ASSERT(odString_compare(&string1, &string2) == -1);  // {"hello ", "loser "}
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

OD_TEST(odRangeT, foreach) {
	const int32_t xs_count = 4;
	int32_t xs[xs_count] = {};
	odRangeT<int32_t> range{xs, xs_count};
	OD_ASSERT(range.begin() == xs);
	OD_ASSERT(range.end() == (xs + xs_count));

	for (int32_t x: range) {
		OD_ASSERT(x == 0);
	}
	for (int32_t& x: xs) {
		x = 2;
	}
	for (int32_t x: range) {
		OD_ASSERT(x == 2);
	}
}
OD_TEST(odRangeT, foreach_empty) {
	odRangeT<int32_t> empty_range;
	OD_ASSERT(empty_range.count == 0);
	OD_ASSERT(empty_range.begin() == empty_range.end());

	for (int32_t x: empty_range) {
		OD_MAYBE_UNUSED(x);
		OD_ASSERT(false); // there must be no elements
	}
}

OD_TEST(odArrayT, get) {
	odArrayT<int32_t> array;
	OD_ASSERT(odArray_set_count(&array, 2));
	OD_ASSERT(array[0] != nullptr);

	OD_ASSERT(*array[1] == 0);
	*array[1] = 2;
	OD_ASSERT(*array[1] == 2);
}
OD_TEST(odArrayT, foreach) {
	odArrayT<int32_t> array;
	OD_ASSERT(odArray_set_count(&array, 2));
	for (int32_t elem: array) {
		OD_ASSERT(elem == 0);
	}

	for (int32_t &elem: array) {
		elem = 2;
	}
	for (int32_t elem: array) {
		OD_ASSERT(elem == 2);
	}
}
OD_TEST(odArrayT, push) {
	odArrayT<int32_t> array;
	array.push(2);
	OD_ASSERT(*array[0] == 2);
	OD_ASSERT(odArray_get_count(&array) == 1);
}
OD_TEST(odArrayT, get_out_of_bounds_fails) {
	odArrayT<int32_t> array;
	OD_ASSERT(odArray_set_count(&array, 2));
	{
		odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
		OD_ASSERT(array[2] == nullptr);
	}
	OD_ASSERT(array[0] != nullptr);

	OD_ASSERT(odArray_set_count(&array, 0));
	{
		odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
		OD_ASSERT(array[0] == nullptr);
	}
}
