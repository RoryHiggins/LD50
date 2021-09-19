#include <od/core/box.hpp>

#include <gtest/gtest.h>

#include <od/core/debug.hpp>
#include <od/core/type.hpp>
TEST(odBox, swap) {
	odBox ptr1{odType_get_char()};
	odBox ptr2{odType_get_char()};
	ASSERT_TRUE(odBox_allocate(&ptr1));
	ASSERT_TRUE(odBox_allocate(&ptr2));
	ASSERT_NE(odBox_get(&ptr1), nullptr);
	ASSERT_NE(odBox_get(&ptr2), nullptr);

	void* ptr1_old_ptr = odBox_get(&ptr1);
	void* ptr2_old_ptr = odBox_get(&ptr2);

	odBox_swap(&ptr1, &ptr2);
	ASSERT_EQ(odBox_get(&ptr1), ptr2_old_ptr);
	ASSERT_EQ(odBox_get(&ptr2), ptr1_old_ptr);
}
TEST(odBox, swap_unallocated) {
	odBox ptr1{odType_get_char()};
	odBox ptr2{odType_get_char()};
	ASSERT_TRUE(odBox_allocate(&ptr1));
	ASSERT_TRUE(odBox_get_type(&ptr1) == odType_get_char());
	ASSERT_NE(odBox_get(&ptr1), nullptr);
	ASSERT_TRUE(odBox_get_type(&ptr2) == odType_get_char());
	ASSERT_EQ(odBox_get(&ptr2), nullptr);

	odBox_swap(&ptr1, &ptr2);
	ASSERT_TRUE(odBox_get_type(&ptr1) == odType_get_char());
	ASSERT_EQ(odBox_get(&ptr1), nullptr);
	ASSERT_TRUE(odBox_get_type(&ptr2) == odType_get_char());
	ASSERT_NE(odBox_get(&ptr2), nullptr);
}
TEST(odBox, release) {
	odBox ptr{odType_get_char()};

	ASSERT_TRUE(odBox_allocate(&ptr));
	ASSERT_NE(odBox_get(&ptr), nullptr);

	odBox_release(&ptr);
	ASSERT_EQ(odBox_get(&ptr), nullptr);
}
TEST(odBox, set_type_deallocates) {
	odBox ptr{odType_get_char()};
	ASSERT_TRUE(odBox_allocate(&ptr));
	ASSERT_NE(odBox_get(&ptr), nullptr);

	odBox_set_type(&ptr, odType_get<int>());
	ASSERT_EQ(odBox_get(&ptr), nullptr);
}
TEST(odBox, allocate) {
	int32_t testSizes[] = {1, 4, 16, 64, (64 * 1024), (4 * 1024 * 1024)};
	const int32_t numTestSizes = (sizeof(testSizes) / sizeof(testSizes[0]));

	for (int32_t i = 0; i < numTestSizes; i++) {
		odBox ptr{odType_get_char()};
		ASSERT_TRUE(odBox_allocate(&ptr));
		ASSERT_NE(odBox_get(&ptr), nullptr);
	}
}
TEST(odBox, allocate_untyped_fails) {
	odBox ptr;
	{
		odLogLevelScoped suppress_logs{OD_LOG_LEVEL_NONE};
		ASSERT_FALSE(odBox_allocate(&ptr));
	}
}
TEST(odBox, get) {
	odBox ptr{odType_get_char()};
	ASSERT_EQ(odBox_get(&ptr), nullptr);

	ASSERT_TRUE(odBox_allocate(&ptr));
	ASSERT_NE(odBox_get(&ptr), nullptr);
}
