#include <od/core.h>
#include <od/core/vector.hpp>

#include <string.h>

#include <gtest/gtest.h>

#include <od/core/debug.hpp>

TEST(odVector, swap) {
	odVector vector1{odType_get_char()};
	odVector vector2{odType_get_char()};
	ASSERT_TRUE(odVector_set_count(&vector1, 1));
	ASSERT_TRUE(odVector_set_count(&vector2, 2));
	ASSERT_TRUE(odVector_get(&vector1, 0) != nullptr);
	ASSERT_TRUE(odVector_get(&vector2, 0) != nullptr);

	void* ptr1_old_ptr = odVector_get(&vector1, 0);
	void* ptr2_old_ptr = odVector_get(&vector2, 0);

	odVector_swap(&vector1, &vector2);
	ASSERT_TRUE(odVector_get(&vector1, 0) == ptr2_old_ptr);
	ASSERT_TRUE(odVector_get_count(&vector1) == 2);
	ASSERT_TRUE(odVector_get(&vector2, 0) == ptr1_old_ptr);
	ASSERT_TRUE(odVector_get_count(&vector2) == 1);
}
TEST(odVector, release) {
	odVector vector{odType_get_char()};

	ASSERT_TRUE(odVector_set_count(&vector, 1));
	ASSERT_TRUE(odVector_get(&vector, 0) != nullptr);
	ASSERT_TRUE(odVector_get_count(&vector) == 1);
	ASSERT_TRUE(odVector_get_capacity(&vector) >= 1);

	odVector_release(&vector);
	ASSERT_TRUE(odVector_get_count(&vector) == 0);
	ASSERT_TRUE(odVector_get_capacity(&vector) == 0);
}
TEST(odVector, set_capacity) {
	uint32_t testSizes[] = {1, 4, 16, 64, (64 * 1024), (4 * 1024 * 1024)};
	const uint32_t numTestSizes = (sizeof(testSizes) / sizeof(testSizes[0]));

	for (uint32_t i = 0; i < numTestSizes; i++) {
		odVector vector{odType_get_char()};
		ASSERT_TRUE(odVector_set_capacity(&vector, testSizes[i]));
		ASSERT_TRUE(odVector_get_capacity(&vector) == testSizes[i]);
		ASSERT_TRUE(odVector_get_count(&vector) == 0);
	}
}
TEST(odVector, set_capacity_zero) {
	odVector vector{odType_get_char()};
	ASSERT_TRUE(odVector_set_capacity(&vector, 0));
}
TEST(odVector, ensure_capacity) {
	odVector vector{odType_get_char()};

	const uint32_t start_capacity = 2;
	ASSERT_TRUE(odVector_set_capacity(&vector, start_capacity));
	ASSERT_TRUE(odVector_get_capacity(&vector) == start_capacity);
	ASSERT_TRUE(odVector_get_count(&vector) == 0);

	ASSERT_TRUE(odVector_ensure_capacity(&vector, start_capacity - 1));
	ASSERT_TRUE(odVector_get_capacity(&vector) == start_capacity);
	ASSERT_TRUE(odVector_get_count(&vector) == 0);
	
	ASSERT_TRUE(odVector_ensure_capacity(&vector, (start_capacity + 1)));
	ASSERT_TRUE(odVector_get_capacity(&vector) >= (start_capacity + 1));
	ASSERT_TRUE(odVector_get_count(&vector) == 0);
}
TEST(odVector, set_count) {
	odVector vector{odType_get_char()};
	ASSERT_TRUE(odVector_set_count(&vector, 1));
	ASSERT_TRUE(odVector_get_capacity(&vector) >= 1);
	ASSERT_TRUE(odVector_get_count(&vector) == 1);
}
TEST(odVector, set_count_expand) {
	odVector vector{odType_get_char()};
	ASSERT_TRUE(odVector_set_capacity(&vector, 1));
	ASSERT_TRUE(odVector_set_count(&vector, 1));
	ASSERT_TRUE(odVector_get_capacity(&vector) == 1);
	ASSERT_TRUE(odVector_get_count(&vector) == 1);
	
	char* vector_elems = static_cast<char*>(odVector_get(&vector, 0));
	ASSERT_TRUE(vector_elems != nullptr);

	vector_elems[0] = '!';

	const uint32_t final_count = 4;
	ASSERT_TRUE(odVector_set_count(&vector, final_count));
	ASSERT_TRUE(odVector_get_capacity(&vector) >= final_count);
	ASSERT_TRUE(odVector_get_count(&vector) == final_count);
	ASSERT_TRUE(static_cast<char*>(odVector_get(&vector, 0)) != vector_elems);
	
	vector_elems = static_cast<char*>(odVector_get(&vector, 0));
	ASSERT_TRUE(vector_elems != nullptr);
	ASSERT_TRUE(vector_elems[0] == '!');
	for (uint32_t i = 1; i < final_count; i++) {
		vector_elems[i] = '\0';
	}
}
TEST(odVector, set_count_truncate) {
	odVector vector{odType_get_char()};
	const uint32_t start_count = 4;
	ASSERT_TRUE(odVector_set_count(&vector, start_count));
	
	char* vector_elems = static_cast<char*>(odVector_get(&vector, 0));
	ASSERT_TRUE(vector_elems != nullptr);
	for (uint32_t i = 0; i < start_count; i++) {
		vector_elems[i] = '!';
	}

	ASSERT_TRUE(odVector_set_count(&vector, 1));
	ASSERT_TRUE(odVector_set_count(&vector, 4));
	ASSERT_TRUE(static_cast<char*>(odVector_get(&vector, 0)) == vector_elems);
	ASSERT_TRUE(vector_elems[0] == '!');
	for (uint32_t i = 1; i < start_count; i++) {
		ASSERT_TRUE(vector_elems[i] == '\0');
	}
}
TEST(odVector, pop) {
	odVector vector{odType_get_char()};
	ASSERT_TRUE(odVector_set_count(&vector, 1));
	ASSERT_TRUE(odVector_get_count(&vector) == 1);

	ASSERT_TRUE(odVector_pop(&vector, 1));
	ASSERT_TRUE(odVector_get_count(&vector) == 0);
}
TEST(odVector, swap_pop) {
	odVector vector{odType_get_char()};
	ASSERT_TRUE(odVector_set_count(&vector, 4));

	char* vector_elems = static_cast<char*>(odVector_get(&vector, 0));
	ASSERT_TRUE(vector_elems != nullptr);
	strncpy(vector_elems, "123", 4);

	ASSERT_TRUE(odVector_swap_pop(&vector, 1));
	ASSERT_TRUE(odVector_set_count(&vector, 4));
	vector_elems = static_cast<char*>(odVector_get(&vector, 0));
	ASSERT_TRUE(vector_elems != nullptr);
	ASSERT_TRUE(strncmp(vector_elems, "13\0\0", 4));
}
TEST(odVector, get_out_of_bounds_fails) {
	odVector vector{odType_get_char()};
	ASSERT_TRUE(odVector_set_count(&vector, 2));
	ASSERT_TRUE(odVector_get(&vector, 0) != nullptr);
	{
		odLogLevelScoped suppressLogs{OD_LOG_LEVEL_NONE};
		ASSERT_TRUE(odVector_get(&vector, 2) == nullptr);
	}
}
