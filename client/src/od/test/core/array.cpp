#include <od/core/array.hpp>

#include <cstring>

#include <od/test/test.hpp>

struct odArrayTestingContainer;

struct odArrayTestingContainer {
	const odArrayTestingContainer* original_self;
	int32_t move_assign_count;
	int32_t destruct_count;

	odArrayTestingContainer();
	odArrayTestingContainer& operator=(odArrayTestingContainer&& other);
	~odArrayTestingContainer();

	odArrayTestingContainer(const odArrayTestingContainer&) = delete;
	odArrayTestingContainer(odArrayTestingContainer&&) = delete;
	odArrayTestingContainer& operator=(const odArrayTestingContainer&) = delete;
};

template struct odArrayT<odArrayTestingContainer>;

odArrayTestingContainer::odArrayTestingContainer()
	: original_self{this}, move_assign_count{0}, destruct_count{0} {
}
odArrayTestingContainer& odArrayTestingContainer::operator=(odArrayTestingContainer&& other) {
	original_self = other.original_self;
	move_assign_count = other.move_assign_count + 1;
	destruct_count = other.destruct_count;

	return *this;
}
odArrayTestingContainer::~odArrayTestingContainer() {
	destruct_count++;
}

OD_TEST(odTest_odArray_swap) {
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
OD_TEST(odTest_odArray_init_destroy) {
	odArray array{odType_get_char()};

	OD_ASSERT(odArray_set_count(&array, 1));
	OD_ASSERT(odArray_get(&array, 0) != nullptr);
	OD_ASSERT(odArray_get_count(&array) == 1);
	OD_ASSERT(odArray_get_capacity(&array) >= 1);

	odArray_destroy(&array);
	OD_ASSERT(odArray_get_count(&array) == 0);
	OD_ASSERT(odArray_get_capacity(&array) == 0);
}
OD_TEST(odTest_odArray_set_capacity) {
	odArray array{odType_get_char()};
	int32_t test_sizes[] = {1, 4, 16, 64, (64 * 1024), (4 * 1024 * 1024)};
	for (int32_t test_size: test_sizes) {
		OD_ASSERT(odArray_set_capacity(&array, test_size));
		OD_ASSERT(odArray_get_capacity(&array) == test_size);
		OD_ASSERT(odArray_get_count(&array) == 0);
	}
}
OD_TEST(odTest_odArray_set_capacity_zero) {
	odArray array{odType_get_char()};
	OD_ASSERT(odArray_set_capacity(&array, 0));
}
OD_TEST(odTest_odArray_ensure_capacity) {
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
OD_TEST(odTest_odArray_set_count) {
	odArray array{odType_get_char()};
	OD_ASSERT(odArray_set_count(&array, 1));
	OD_ASSERT(odArray_get_capacity(&array) >= 1);
	OD_ASSERT(odArray_get_count(&array) == 1);
}
OD_TEST(odTest_odArray_set_count_expand) {
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
OD_TEST(odTest_odArray_set_count_truncate) {
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
OD_TEST(odTest_odArray_pop) {
	odArray array{odType_get_char()};
	OD_ASSERT(odArray_set_count(&array, 1));
	OD_ASSERT(odArray_get_count(&array) == 1);

	OD_ASSERT(odArray_pop(&array, 1));
	OD_ASSERT(odArray_get_count(&array) == 0);
}
OD_TEST(odTest_odArray_swap_pop) {
	odArray array{odType_get_char()};
	OD_ASSERT(odArray_set_count(&array, 4));

	char* array_ptr = static_cast<char*>(odArray_get(&array, 0));
	OD_ASSERT(array_ptr != nullptr);
	strncpy(array_ptr, "123", 4);

	OD_ASSERT(odArray_swap_pop(&array, 1));
	OD_ASSERT(odArray_get_count(&array) == 3);
	OD_ASSERT(odArray_set_count(&array, 4));
	array_ptr = static_cast<char*>(odArray_get(&array, 0));
	OD_ASSERT(array_ptr != nullptr);
	OD_ASSERT(strncmp(array_ptr, "13\0\0", 4));
}
OD_TEST(odTest_odArray_debug_get_out_of_bounds_fails) {
	if (OD_BUILD_DEBUG) {
		odArray array{odType_get_char()};
		OD_ASSERT(odArray_set_count(&array, 2));
		OD_ASSERT(odArray_get(&array, 0) != nullptr);
		{
			odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
			OD_ASSERT(odArray_get(&array, 2) == nullptr);
		}
	}
}

OD_TEST(odTest_odArrayT_get) {
	odArrayT<int32_t> array;
	OD_ASSERT(odArray_set_count(&array, 2));
	OD_ASSERT(array[0] != nullptr);

	OD_ASSERT(*array[1] == 0);
	*array[1] = 2;
	OD_ASSERT(*array[1] == 2);
}
OD_TEST(odTest_odArrayT_foreach) {
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
OD_TEST(odTest_odArrayT_push) {
	odArrayT<int32_t> array;
	OD_ASSERT(array.push(2));
	OD_ASSERT(*array[0] == 2);
	OD_ASSERT(odArray_get_count(&array) == 1);
}
OD_TEST(odTest_odArrayT_debug_get_out_of_bounds_fails) {
	if (OD_BUILD_DEBUG) {
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
}
OD_TEST(odTest_odArrayT_push_pop_container) {
	odArrayT<odArrayTestingContainer> array;

	odArrayTestingContainer test_container{};

	OD_ASSERT(array.push(static_cast<odArrayTestingContainer&&>(test_container)));
	OD_ASSERT(odArray_get_count(&array) == 1);

	const odArrayTestingContainer* pushed_container = array[0];
	OD_ASSERT(pushed_container != nullptr);
	OD_ASSERT(pushed_container->original_self == test_container.original_self);
	OD_ASSERT(pushed_container->destruct_count == 0);
	OD_ASSERT(pushed_container->move_assign_count > 0);

	OD_ASSERT(odArray_swap_pop(&array, 0));
	OD_ASSERT(odArray_get_count(&array) == 0);

	OD_ASSERT(array.push(static_cast<odArrayTestingContainer&&>(test_container)));
	OD_ASSERT(array.push(static_cast<odArrayTestingContainer&&>(test_container)));
	OD_ASSERT(odArray_get_count(&array) == 2);
	OD_ASSERT(array[0]->original_self == test_container.original_self);
	OD_ASSERT(array[1]->original_self == test_container.original_self);

	OD_ASSERT(odArray_swap_pop(&array, 0));
	OD_ASSERT(odArray_get_count(&array) == 1);
	OD_ASSERT(array[0]->original_self == test_container.original_self);
}

OD_TEST_SUITE(
	odTestSuite_odArray,
	odTest_odArray_swap,
	odTest_odArray_init_destroy,
	odTest_odArray_set_capacity,
	odTest_odArray_set_capacity_zero,
	odTest_odArray_ensure_capacity,
	odTest_odArray_set_count,
	odTest_odArray_set_count_expand,
	odTest_odArray_set_count_truncate,
	odTest_odArray_pop,
	odTest_odArray_swap_pop,
	odTest_odArray_debug_get_out_of_bounds_fails,
	odTest_odArrayT_get,
	odTest_odArrayT_foreach,
	odTest_odArrayT_push,
	odTest_odArrayT_debug_get_out_of_bounds_fails,
	odTest_odArrayT_push_pop_container,
)
