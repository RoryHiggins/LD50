#include <od/core/allocation.hpp>

#include <gtest/gtest.h>

#include <od/core/debug.hpp>

TEST(odAllocation, swap) {
	odAllocation allocation1;
	odAllocation allocation2;
	ASSERT_TRUE(odAllocation_allocate(&allocation1, 1));
	ASSERT_TRUE(odAllocation_allocate(&allocation2, 2));
	ASSERT_NE(odAllocation_get(&allocation1), nullptr);
	ASSERT_NE(odAllocation_get(&allocation2), nullptr);

	void* allocation1_old_ptr = odAllocation_get(&allocation1);
	void* allocation2_old_ptr = odAllocation_get(&allocation2);

	odAllocation_swap(&allocation1, &allocation2);
	ASSERT_EQ(odAllocation_get(&allocation1), allocation2_old_ptr);
	ASSERT_EQ(odAllocation_get(&allocation2), allocation1_old_ptr);
}
TEST(odAllocation, swap_unallocated) {
	odAllocation allocation1;
	odAllocation allocation2;
	ASSERT_TRUE(odAllocation_allocate(&allocation1, 1));
	ASSERT_NE(odAllocation_get(&allocation1), nullptr);
	ASSERT_EQ(odAllocation_get(&allocation2), nullptr);

	odAllocation_swap(&allocation1, &allocation2);
	ASSERT_EQ(odAllocation_get(&allocation1), nullptr);
	ASSERT_NE(odAllocation_get(&allocation2), nullptr);
}
TEST(odAllocation, release) {
	odAllocation allocation;

	ASSERT_TRUE(odAllocation_allocate(&allocation, 1));
	ASSERT_NE(odAllocation_get(&allocation), nullptr);

	odAllocation_release(&allocation);
	ASSERT_EQ(odAllocation_get(&allocation), nullptr);
}
TEST(odAllocation, allocate) {
	uint32_t testSizes[] = {1, 4, 16, 64, (64 * 1024), (4 * 1024 * 1024)};
	const uint32_t numTestSizes = (sizeof(testSizes) / sizeof(testSizes[0]));

	for (uint32_t i = 0; i < numTestSizes; i++) {
		odAllocation allocation;
		ASSERT_TRUE(odAllocation_allocate(&allocation, testSizes[i]));
		ASSERT_NE(odAllocation_get(&allocation), nullptr);
	}
}
TEST(odAllocation, allocate_zero) {
	odAllocation allocation;
	ASSERT_TRUE(odAllocation_allocate(&allocation, 0));
	ASSERT_EQ(odAllocation_get(&allocation), nullptr);
}
TEST(odAllocation, get) {
	odAllocation allocation;
	ASSERT_TRUE(odAllocation_allocate(&allocation, 1));
	ASSERT_NE(odAllocation_get(&allocation), nullptr);
}
TEST(odAllocation, get_unallocated_fails) {
	odAllocation allocation;
	{
		odLogLevelScoped suppress_logs{OD_LOG_LEVEL_NONE};
		ASSERT_EQ(odAllocation_get(&allocation), nullptr);
	}
}
