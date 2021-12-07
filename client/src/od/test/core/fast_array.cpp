#include <od/core/fast_array.hpp>

#include <od/test/test.hpp>

OD_TEST(odTest_odFastArray_swap) {
	odFastArrayT<int32_t> array1;
	odFastArrayT<int32_t> array2;
	array1.swap(array2);
}
OD_TEST(odTest_odFastArray_set_capacity) {
	odFastArrayT<int32_t> array;
	int32_t test_sizes[] = {1, 4, 16, 64, (64 * 1024), (4 * 1024 * 1024)};
	for (int32_t test_size: test_sizes) {
		OD_ASSERT((array.set_capacity(test_size)));
		OD_ASSERT((array.capacity >= test_size));
		OD_ASSERT(array.count == 0);
	}
}
OD_TEST(odTest_odFastArray_set_capacity_zero) {
	odFastArrayT<int32_t> array;
	OD_ASSERT(array.set_capacity(0));
}
OD_TEST(odTest_odFastArray_set_count) {
	odFastArrayT<int32_t> array;
	OD_ASSERT(array.set_capacity(1));
	OD_ASSERT(array.set_count(1));
	OD_ASSERT(array.capacity >= 1);
	OD_ASSERT(array.count == 1);

	const int32_t final_count = 4;
	OD_ASSERT(array.set_count(final_count));
	OD_ASSERT(array.capacity >= final_count);
	OD_ASSERT(array.count == final_count);

	OD_ASSERT(array.set_count(1));
	OD_ASSERT(array.capacity >= 1);
	OD_ASSERT(array.count == 1);
}
OD_TEST(odTest_odFastArray_swap_pop) {
	odFastArrayT<int32_t> array;
	OD_ASSERT(array.set_count(2));
	*array[0] = 1;
	*array[1] = 2;
	OD_ASSERT(array.swap_pop(0));
	OD_ASSERT(array.count == 1);
	OD_ASSERT(*array[0] == 2);
}
OD_TEST(odTest_odFastArrayT_foreach) {
	odFastArrayT<int32_t> array;
	OD_ASSERT(array.set_count(2));
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
OD_TEST(odTest_odFastArrayT_extend) {
	odFastArrayT<int32_t> array;
	int32_t value = 2;
	OD_ASSERT(array.extend(&value, 1));
	OD_ASSERT(*array[0] == 2);
	OD_ASSERT(array.count == 1);
}

OD_TEST_SUITE(
	odTestSuite_odFastArray,
	odTest_odFastArray_swap,
	odTest_odFastArray_set_capacity,
	odTest_odFastArray_set_capacity_zero,
	odTest_odFastArray_set_count,
	odTest_odFastArray_swap_pop,
	odTest_odFastArrayT_foreach,
	odTest_odFastArrayT_extend,
)
