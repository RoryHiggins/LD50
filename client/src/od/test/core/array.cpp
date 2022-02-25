#include <od/core/array.hpp>

#include <cstring>

#include <od/test/test.hpp>

struct odArrayTestingContainer;

struct odArrayTestingContainer {
	const odArrayTestingContainer* original_self;
	int32_t move_assign_count;
	int32_t destruct_count;

	odArrayTestingContainer();
	odArrayTestingContainer(odArrayTestingContainer&&);
	odArrayTestingContainer& operator=(odArrayTestingContainer&& other);
	~odArrayTestingContainer();

	odArrayTestingContainer(const odArrayTestingContainer&) = delete;
	odArrayTestingContainer& operator=(const odArrayTestingContainer&) = delete;
};

odArrayTestingContainer::odArrayTestingContainer()
	: original_self{this}, move_assign_count{0}, destruct_count{0} {
}
odArrayTestingContainer::odArrayTestingContainer(odArrayTestingContainer&& other) {
	original_self = other.original_self;
	move_assign_count = other.move_assign_count + 1;
	destruct_count = other.destruct_count;
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

OD_TEST(odTest_odTrivialArray_init_destroy) {
	odTrivialArrayT<int32_t> array;
	OD_ASSERT(odTrivialArray_check_valid(&array));

	// double init
	odTrivialArray_init(&array);
	OD_ASSERT(odTrivialArray_check_valid(&array));

	OD_ASSERT(array.set_count(1));
	OD_ASSERT(odTrivialArray_check_valid(&array));
	OD_ASSERT(array.begin() != nullptr);
	OD_ASSERT(array.get_count() == 1);
	OD_ASSERT(array.get_capacity() >= 1);

	odTrivialArray_destroy(&array);
	OD_ASSERT(array.get_count() == 0);
	OD_ASSERT(array.get_capacity() == 0);

	// double destroy
	odTrivialArray_destroy(&array);

	// reinit
	odTrivialArray_init(&array);
	OD_ASSERT(odTrivialArray_check_valid(&array));

	OD_ASSERT(array.set_count(1));
	OD_ASSERT(odTrivialArray_check_valid(&array));
	OD_ASSERT(array.begin() != nullptr);
	OD_ASSERT(array.get_count() == 1);
	OD_ASSERT(array.get_capacity() >= 1);
}
OD_TEST(odTest_odTrivialArray_swap) {
	odTrivialArrayT<int32_t> array1;
	odTrivialArrayT<int32_t> array2;
	OD_ASSERT(array1.set_count(1));
	OD_ASSERT(array2.set_count(2));
	OD_ASSERT(array1.get(0) != nullptr);
	OD_ASSERT(array2.get(0) != nullptr);

	void* ptr1_old_ptr = array1.get(0);
	void* ptr2_old_ptr = array2.get(0);

	odTrivialArray_swap(&array1, &array2);

	OD_ASSERT(array1.get_count() == 2);
	OD_ASSERT(array2.get_count() == 1);

	// ensure allocations are preseved by the swap
	OD_ASSERT(array1.get(0) == ptr2_old_ptr);
	OD_ASSERT(array2.get(0) == ptr1_old_ptr);
}
OD_TEST(odTest_odTrivialArray_compare) {
	odTrivialArrayT<int32_t> array1;
	odTrivialArrayT<int32_t> array2;

	OD_ASSERT(array1.compare(array2) == 0);

	OD_ASSERT(array1.set_count(1));
	OD_ASSERT(array1.compare(array2) > 0);

	OD_ASSERT(array2.set_count(1));
	OD_ASSERT(array1.compare(array2) == 0);

	array2[0] = 1;
	OD_ASSERT(array1.compare(array2) < 0);
}
OD_TEST(odTest_odTrivialArray_get_set_capacity) {
	odTrivialArrayT<int32_t> array;
	int32_t test_sizes[] = {1, 4, 16, 64, (64 * 1024), (4 * 1024 * 1024)};
	for (int32_t test_size: test_sizes) {
		OD_ASSERT(array.set_capacity(test_size));
		OD_ASSERT(array.get_capacity() == test_size);
		OD_ASSERT(array.get_count() == 0);
	}
}
OD_TEST(odTest_odTrivialArray_set_capacity_zero) {
	odTrivialArrayT<int32_t> array;
	OD_ASSERT(array.set_capacity(0));

	OD_ASSERT(array.set_capacity(1));
	OD_ASSERT(array.set_capacity(0));
}
OD_TEST(odTest_odTrivialArray_ensure_capacity) {
	odTrivialArrayT<int32_t> array;

	const int32_t start_capacity = 2;
	OD_ASSERT(array.set_capacity(start_capacity));
	OD_ASSERT(array.get_capacity() == start_capacity);
	OD_ASSERT(array.get_count() == 0);

	OD_ASSERT(array.ensure_capacity(start_capacity - 1));
	OD_ASSERT(array.get_capacity() == start_capacity);
	OD_ASSERT(array.get_count() == 0);

	OD_ASSERT(array.ensure_capacity((start_capacity + 1)));
	OD_ASSERT(array.get_capacity() >= (start_capacity + 1));
	OD_ASSERT(array.get_count() == 0);
}
OD_TEST(odTest_odTrivialArray_get_set_count) {
	odTrivialArrayT<int32_t> array;
	OD_ASSERT(array.set_count(1));
	OD_ASSERT(array.get_capacity() >= 1);
	OD_ASSERT(array.get_count() == 1);
}
OD_TEST(odTest_odTrivialArray_set_count_expand) {
	odTrivialArrayT<char> array;
	OD_ASSERT(array.set_capacity(1));
	OD_ASSERT(array.set_count(1));
	OD_ASSERT(array.get_capacity() == 1);
	OD_ASSERT(array.get_count() == 1);

	array[0] = '!';

	const int32_t final_count = 4;
	OD_ASSERT(array.set_count(final_count));
	OD_ASSERT(array.get_capacity() >= final_count);
	OD_ASSERT(array.get_count() == final_count);

	OD_ASSERT(array[0] == '!');
	for (int32_t i = 1; i < final_count; i++) {
		array[i] = '\0';
	}
}
OD_TEST(odTest_odTrivialArray_set_count_truncate) {
	odTrivialArrayT<char> array;
	const int32_t start_count = 4;
	OD_ASSERT(array.set_count(start_count));

	char* array_ptr = static_cast<char*>(array.begin());
	OD_ASSERT(array_ptr != nullptr);
	for (int32_t i = 0; i < start_count; i++) {
		array_ptr[i] = '!';
	}

	OD_ASSERT(array.set_count(1));
	OD_ASSERT(array.set_count(4));
	OD_ASSERT(static_cast<char*>(array.begin()) == array_ptr);
	OD_ASSERT(array_ptr[0] == '!');
	for (int32_t i = 1; i < start_count; i++) {
		OD_ASSERT(array_ptr[i] == '\0');
	}
}
OD_TEST(odTest_odTrivialArray_extend_push) {
	odTrivialArrayT<char> array;
	OD_ASSERT(array.extend("hello", 5));
	OD_ASSERT(array.get_count() == 5);
	OD_ASSERT(strncmp(array.begin(), "hello", 5) == 0);

	OD_ASSERT(array.push('!'));
	OD_ASSERT(array.get_count() == 6);
	OD_ASSERT(strncmp(array.begin(), "hello!", 6) == 0);
}
OD_TEST(odTest_odTrivialArray_pop) {
	odTrivialArrayT<char> array;
	OD_ASSERT(array.set_count(1));
	OD_ASSERT(array.get_count() == 1);

	OD_ASSERT(array.pop(1));
	OD_ASSERT(array.get_count() == 0);
}
OD_TEST(odTest_odTrivialArray_swap_pop) {
	odTrivialArrayT<char> array;
	OD_ASSERT(array.set_count(4));

	char* array_ptr = static_cast<char*>(array.begin());
	OD_ASSERT(array_ptr != nullptr);
	strncpy(array_ptr, "123", 4);

	OD_ASSERT(array.swap_pop(1));
	OD_ASSERT(array.get_count() == 3);
	OD_ASSERT(array.set_count(4));
	array_ptr = static_cast<char*>(array.begin());
	OD_ASSERT(array_ptr != nullptr);
	OD_ASSERT(strncmp(array_ptr, "13\0\0", 4));
}
OD_TEST(odTest_odTrivialArray_assign) {
	odTrivialArrayT<char> array;
	OD_ASSERT(array.extend("hello", 5));
	OD_ASSERT(strncmp(array.begin(), "hello", 5) == 0);

	OD_ASSERT(array.assign("goodbye", 7));
	OD_ASSERT(strncmp(array.begin(), "goodbye", 7) == 0);
}
OD_TEST(odTest_odTrivialArray_get) {
	odTrivialArrayT<int32_t> array;
	OD_ASSERT(array.set_count(2));
	OD_ASSERT(array.get_count() == 2);
	OD_ASSERT(array.get(0) != nullptr);

	OD_ASSERT(*array.get(1) == 0);
	*array.get(1) = 2;
	OD_ASSERT(*array.get(1) == 2);

	for (int32_t i = 0; i < array.get_count(); i++) {
		OD_ASSERT(array[i] == *array.get(i));
		OD_ASSERT(&array[i] == array.get(i));
	}
}
OD_TEST(odTest_odTrivialArray_begin_end) {
	odTrivialArrayT<int32_t> array;
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

OD_TEST(odTest_odArray_init_destroy) {
	odArrayT<odArrayTestingContainer> array;
	OD_ASSERT(odArray_check_valid(&array));

	// double init
	odArray_init(&array, odType_get<odArrayTestingContainer>());
	OD_ASSERT(odArray_check_valid(&array));

	OD_ASSERT(array.set_count(1));
	OD_ASSERT(odArray_check_valid(&array));
	OD_ASSERT(array.begin() != nullptr);
	OD_ASSERT(array.get_count() == 1);
	OD_ASSERT(array.get_capacity() >= 1);

	odArray_destroy(&array);
	OD_ASSERT(array.get_count() == 0);
	OD_ASSERT(array.get_capacity() == 0);

	// double destroy
	odArray_destroy(&array);

	// reinit
	odArray_init(&array, odType_get<odArrayTestingContainer>());
	OD_ASSERT(odArray_check_valid(&array));

	OD_ASSERT(array.set_count(1));
	OD_ASSERT(odArray_check_valid(&array));
	OD_ASSERT(array.begin() != nullptr);
	OD_ASSERT(array.get_count() == 1);
	OD_ASSERT(array.get_capacity() >= 1);
}
OD_TEST(odTest_odArray_swap) {
	odArrayT<odArrayTestingContainer> array1;
	odArrayT<odArrayTestingContainer> array2;
	OD_ASSERT(array1.set_count(1));
	OD_ASSERT(array2.set_count(2));
	OD_ASSERT(array1.get(0) != nullptr);
	OD_ASSERT(array2.get(0) != nullptr);

	void* ptr1_old_ptr = array1.get(0);
	void* ptr2_old_ptr = array2.get(0);

	odArray_swap(&array1, &array2);

	OD_ASSERT(array1.get_count() == 2);
	OD_ASSERT(array2.get_count() == 1);

	// ensure allocations are preseved by the swap
	OD_ASSERT(array1.get(0) == ptr2_old_ptr);
	OD_ASSERT(array2.get(0) == ptr1_old_ptr);
}
OD_TEST(odTest_odArray_get_set_capacity) {
	odArrayT<odArrayTestingContainer> array;
	int32_t test_sizes[] = {1, 4, 16, 64, (64 * 1024), (4 * 1024 * 1024)};
	for (int32_t test_size: test_sizes) {
		OD_ASSERT(array.set_capacity(test_size));
		OD_ASSERT(array.get_capacity() == test_size);
		OD_ASSERT(array.get_count() == 0);
	}
}
OD_TEST(odTest_odArray_set_capacity_zero) {
	odArrayT<odArrayTestingContainer> array;
	OD_ASSERT(array.set_capacity(0));

	OD_ASSERT(array.set_capacity(1));
	OD_ASSERT(array.set_capacity(0));
}
OD_TEST(odTest_odArray_ensure_capacity) {
	odArrayT<odArrayTestingContainer> array;

	const int32_t start_capacity = 2;
	OD_ASSERT(array.set_capacity(start_capacity));
	OD_ASSERT(array.get_capacity() == start_capacity);
	OD_ASSERT(array.get_count() == 0);

	OD_ASSERT(array.ensure_capacity(start_capacity - 1));
	OD_ASSERT(array.get_capacity() == start_capacity);
	OD_ASSERT(array.get_count() == 0);

	OD_ASSERT(array.ensure_capacity((start_capacity + 1)));
	OD_ASSERT(array.get_capacity() >= (start_capacity + 1));
	OD_ASSERT(array.get_count() == 0);
}
OD_TEST(odTest_odArray_get_set_count) {
	odArrayT<odArrayTestingContainer> array;
	OD_ASSERT(array.set_count(1));
	OD_ASSERT(array.get_capacity() >= 1);
	OD_ASSERT(array.get_count() == 1);
}
OD_TEST(odTest_odArray_set_count_expand) {
	odArrayT<char> array;
	OD_ASSERT(array.set_capacity(1));
	OD_ASSERT(array.set_count(1));
	OD_ASSERT(array.get_capacity() == 1);
	OD_ASSERT(array.get_count() == 1);

	array[0] = '!';

	const int32_t final_count = 4;
	OD_ASSERT(array.set_count(final_count));
	OD_ASSERT(array.get_capacity() >= final_count);
	OD_ASSERT(array.get_count() == final_count);

	OD_ASSERT(array[0] == '!');
	for (int32_t i = 1; i < final_count; i++) {
		array[i] = '\0';
	}
}
OD_TEST(odTest_odArray_set_count_truncate) {
	odArrayT<char> array;
	const int32_t start_count = 4;
	OD_ASSERT(array.set_count(start_count));

	char* array_ptr = static_cast<char*>(array.begin());
	OD_ASSERT(array_ptr != nullptr);
	for (int32_t i = 0; i < start_count; i++) {
		array_ptr[i] = '!';
	}

	OD_ASSERT(array.set_count(1));
	OD_ASSERT(array.set_count(4));
	OD_ASSERT(static_cast<char*>(array.begin()) == array_ptr);
	OD_ASSERT(array_ptr[0] == '!');
	for (int32_t i = 1; i < start_count; i++) {
		OD_ASSERT(array_ptr[i] == '\0');
	}
}
OD_TEST(odTest_odArray_push) {
	odArrayT<char> array;
	OD_ASSERT(array.push('!'));
	OD_ASSERT(array.get_count() == 1);
	OD_ASSERT(strncmp(array.begin(), "!", 1) == 0);
}
OD_TEST(odTest_odArray_pop) {
	odArrayT<char> array;
	OD_ASSERT(array.set_count(1));
	OD_ASSERT(array.get_count() == 1);

	OD_ASSERT(array.pop(1));
	OD_ASSERT(array.get_count() == 0);
}
OD_TEST(odTest_odArray_swap_pop) {
	odArrayT<char> array;
	OD_ASSERT(array.set_count(4));

	char* array_ptr = static_cast<char*>(array.begin());
	OD_ASSERT(array_ptr != nullptr);
	strncpy(array_ptr, "123", 4);

	OD_ASSERT(array.swap_pop(1));
	OD_ASSERT(array.get_count() == 3);
	OD_ASSERT(array.set_count(4));
	array_ptr = static_cast<char*>(array.begin());
	OD_ASSERT(array_ptr != nullptr);
	OD_ASSERT(strncmp(array_ptr, "13\0\0", 4));
}
OD_TEST(odTest_odArray_get) {
	odArrayT<odArrayTestingContainer> array;
	OD_ASSERT(array.set_count(2));
	OD_ASSERT(array.get_count() == 2);

	for (int32_t i = 0; i < array.get_count(); i++) {
		OD_ASSERT(array.get(i) != nullptr);
		OD_ASSERT(&array[i] == array.get(i));
	}
}
OD_TEST(odTest_odArray_begin_end) {
	odArrayT<odArrayTestingContainer> array;
	OD_ASSERT(array.set_count(2));
	for (const odArrayTestingContainer& elem: array) {
		OD_ASSERT(&elem == elem.original_self);
	}

	OD_ASSERT(array.set_count(32));
	for (odArrayTestingContainer& elem: array) {
		elem.original_self = &elem;
	}
	for (odArrayTestingContainer& elem: array) {
		OD_ASSERT(&elem == elem.original_self);
	}
}


OD_TEST_SUITE(
	odTestSuite_odArray,
	odTest_odTrivialArray_init_destroy,
	odTest_odTrivialArray_swap,
	odTest_odTrivialArray_compare,
	odTest_odTrivialArray_get_set_capacity,
	odTest_odTrivialArray_set_capacity_zero,
	odTest_odTrivialArray_ensure_capacity,
	odTest_odTrivialArray_get_set_count,
	odTest_odTrivialArray_set_count_expand,
	odTest_odTrivialArray_set_count_truncate,
	odTest_odTrivialArray_extend_push,
	odTest_odTrivialArray_pop,
	odTest_odTrivialArray_swap_pop,
	odTest_odTrivialArray_assign,
	odTest_odTrivialArray_get,
	odTest_odTrivialArray_begin_end,

	odTest_odArray_init_destroy,
	odTest_odArray_swap,
	odTest_odArray_get_set_capacity,
	odTest_odArray_set_capacity_zero,
	odTest_odArray_ensure_capacity,
	odTest_odArray_get_set_count,
	odTest_odArray_set_count_expand,
	odTest_odArray_set_count_truncate,
	odTest_odArray_push,
	odTest_odArray_pop,
	odTest_odArray_swap_pop,
	odTest_odArray_get,
	odTest_odArray_begin_end,
)
