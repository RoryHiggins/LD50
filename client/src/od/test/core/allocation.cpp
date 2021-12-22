#include <od/core/allocation.hpp>

#include <od/test/test.hpp>

OD_TEST(odTest_odAllocation_init_destroy) {
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
OD_TEST(odTest_odAllocation_init_destroy_zero) {
	odAllocation allocation;
	OD_ASSERT(odAllocation_init(&allocation, 0));
	OD_ASSERT(odAllocation_get(&allocation) == nullptr);

	odAllocation_destroy(&allocation);
}
OD_TEST(odTest_odAllocation_init_destroy_large) {
	odAllocation allocation;
	OD_ASSERT(odAllocation_init(&allocation, 1<<20));
	OD_ASSERT(odAllocation_get(&allocation) != nullptr);

	odAllocation_destroy(&allocation);
	OD_ASSERT(odAllocation_get(&allocation) == nullptr);
}

OD_TEST(odTest_odAllocation_swap) {
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
OD_TEST(odTest_odAllocation_swap_unallocated) {
	odAllocation allocation1;
	odAllocation allocation2;
	OD_ASSERT(odAllocation_init(&allocation1, 1));
	OD_ASSERT(odAllocation_get(&allocation1) != nullptr);
	OD_ASSERT(odAllocation_get(&allocation2) == nullptr);

	odAllocation_swap(&allocation1, &allocation2);
	OD_ASSERT(odAllocation_get(&allocation1) == nullptr);
	OD_ASSERT(odAllocation_get(&allocation2) != nullptr);
}
OD_TEST(odTest_odAllocation_get) {
	odAllocation allocation;
	OD_ASSERT(odAllocation_init(&allocation, 1));
	OD_ASSERT(odAllocation_get(&allocation) != nullptr);
}
OD_TEST(odTest_odAllocation_get_unallocated_fails) {
	odAllocation allocation;
	{
		odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
		OD_ASSERT(odAllocation_get(&allocation) == nullptr);
	}
}

OD_TEST_SUITE(
	odTestSuite_odAllocation,
	odTest_odAllocation_init_destroy,
	odTest_odAllocation_init_destroy_zero,
	odTest_odAllocation_init_destroy_large,
	odTest_odAllocation_swap,
	odTest_odAllocation_swap_unallocated,
	odTest_odAllocation_get,
	odTest_odAllocation_get_unallocated_fails
)
