#include <od/platform/file.hpp>

#include <cstdlib>
#include <cstring>
#include <time.h>

#include <od/core/allocation.hpp>
#include <od/core/string.hpp>
#include <od/test/test.hpp>

static odString odFile_test_create_random_name() {
	const char prefix[] = "./odFile_test_";
	const int32_t prefix_size = sizeof(prefix) - 1;
	const int32_t random_size = 16;
	const int32_t size = prefix_size + random_size;
	odString name;
	OD_ASSERT(name.set_count(size));

	char* name_ptr = name.begin();
	OD_ASSERT(name_ptr != nullptr);

	memcpy(name_ptr, prefix, prefix_size);

	// mitigate collision risk from parallel test runs
	static bool random_seed_set = false;
	if (!random_seed_set) {
		unsigned seed =
			(static_cast<unsigned>(time(nullptr)) + static_cast<unsigned>(reinterpret_cast<uintptr_t>(name_ptr)));
		srand(seed);
		random_seed_set = true;
	}

	for (int32_t i = prefix_size; i < size; i++) {
		name_ptr[i] = static_cast<char>(static_cast<int>('a') + (rand() % 20));
	}

	return name;
}
OD_TEST(odTest_odFile_open) {
	odString file_name = odFile_test_create_random_name();
	const char* file_name_str = file_name.begin();
	OD_ASSERT(file_name_str != nullptr);

	const int32_t write_modes_count = 6;
	const char write_modes[write_modes_count][8] = {"w", "wb", "w+", "wb+", "a", "a+"};
	for (int32_t mode = 0; mode < write_modes_count; mode++) {
		odFile file;
		OD_ASSERT(odFile_open(&file, write_modes[mode], file_name_str));
	}

	const int32_t read_modes_count = 4;
	const char read_modes[read_modes_count][8] = {"r", "rb", "r+", "rb+"};
	for (int32_t mode = 0; mode < read_modes_count; mode++) {
		odFile file;
		OD_ASSERT(odFile_open(&file, read_modes[mode], file_name_str));
	}

	OD_ASSERT(odFilePath_delete(file_name_str));
}
OD_TEST(odTest_odFile_write_read_delete) {
	odString file_name = odFile_test_create_random_name();
	const char* file_name_str = file_name.begin();

	const char test_str[] = "hello";
	const int32_t test_string_size = sizeof(test_str);

	odFile file;
	OD_ASSERT(odFile_open(&file, "w", file_name_str));

	OD_ASSERT(odFile_write(&file, test_str, test_string_size));

	odFile_close(&file);
	OD_ASSERT(odFile_open(&file, "r", file_name_str));

	int32_t read_size = 0;
	char read_buffer[test_string_size] = {};
	OD_ASSERT(odFile_read(&file, read_buffer, test_string_size, &read_size));
	OD_ASSERT(read_size == test_string_size);
	OD_ASSERT(strncmp(test_str, read_buffer, test_string_size) == 0);
	odFile_close(&file);

	OD_ASSERT(odFilePath_delete(file_name_str));
}
OD_TEST(odTest_odFilePath_write_read_delete) {
	odString file_name = odFile_test_create_random_name();
	const char* file_name_str = file_name.begin();

	const char test_str[] = "hello";
	const int32_t test_string_size = sizeof(test_str);

	OD_ASSERT(odFilePath_write_all(file_name_str, "w", test_str, test_string_size));

	odAllocation read_allocation;
	int32_t read_size = 0;

	OD_ASSERT(odFilePath_read_all(file_name_str, "r", &read_allocation, &read_size));
	OD_ASSERT(read_size == test_string_size);
	const char* read_allocation_str = static_cast<const char*>(odAllocation_get_const(&read_allocation));
	OD_ASSERT(read_allocation_str != nullptr);
	OD_ASSERT(strncmp(test_str, read_allocation_str, test_string_size) == 0);

	OD_ASSERT(odFilePath_delete(file_name_str));
}

OD_TEST_SUITE(
	odTestSuite_odFile,
	odTest_odFile_open,
	odTest_odFile_write_read_delete,
	odTest_odFilePath_write_read_delete,
)
