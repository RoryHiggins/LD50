#include <od/core/allocation.hpp>

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
		odLogLevelScoped suppress_logs{OD_LOG_LEVEL_NONE};
		OD_ASSERT(odAllocation_get(&allocation) == nullptr);
	}
}
