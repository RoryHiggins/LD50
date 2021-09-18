#include <od/platform/file.hpp>

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <gtest/gtest.h>

#include <od/core/debug.hpp>
#include <od/core/string.hpp>

static odString odFile_test_create_random_name() {
	const char prefix[] = "./odFile_test_";
	const uint32_t prefix_size = sizeof(prefix) - 1;
	const uint32_t random_size = 16;
	const uint32_t size = prefix_size + random_size;
	odString name;
	OD_ASSERT(odString_set_count(&name, size));

	char* name_ptr = odString_get(&name, 0);
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

	for (uint32_t i = prefix_size; i < size; i++) {
		name_ptr[i] = 'a' + static_cast<char>(rand() % 20);
	}

	OD_ASSERT(odString_ensure_null_terminated(&name));

	return name;
}

TEST(odFile, open) {
	odString file_name = odFile_test_create_random_name();
	const char* file_name_str = odString_get_const(&file_name, 0);
	ASSERT_NE(file_name_str, nullptr);

	const uint32_t write_modes_count = 6;
	const char write_modes[write_modes_count][8] = {"w", "wb", "w+", "wb+", "a", "a+"};
	for (uint32_t mode = 0; mode < write_modes_count; mode++) {
		odFile file;
		ASSERT_TRUE(odFile_open(&file, write_modes[mode], file_name_str));
	}

	const uint32_t read_modes_count = 4;
	const char read_modes[read_modes_count][8] = {"r", "rb", "r+", "rb+"};
	for (uint32_t mode = 0; mode < read_modes_count; mode++) {
		odFile file;
		ASSERT_TRUE(odFile_open(&file, read_modes[mode], file_name_str));
	}

	ASSERT_TRUE(odFilePath_delete(file_name_str));
}

TEST(odFile, write_read_delete) {
	odString file_name = odFile_test_create_random_name();
	const char* file_name_str = odString_get_const(&file_name, 0);

	const char test_str[] = "hello";
	const uint32_t test_string_size = sizeof(test_str);

	odFile file;
	ASSERT_TRUE(odFile_open(&file, "w", file_name_str));

	ASSERT_TRUE(odFile_write(&file, test_str, test_string_size));

	odFile_close(&file);
	ASSERT_TRUE(odFile_open(&file, "r", file_name_str));

	uint32_t read_size = 0;
	char read_buffer[test_string_size] = {};
	ASSERT_TRUE(odFile_read(&file, read_buffer, test_string_size, &read_size));
	ASSERT_EQ(read_size, test_string_size);
	ASSERT_EQ(strncmp(test_str, read_buffer, test_string_size), 0);
	odFile_close(&file);

	ASSERT_TRUE(odFilePath_delete(file_name_str));
}

TEST(odFilePath, write_read_delete) {
	odString file_name = odFile_test_create_random_name();
	const char* file_name_str = odString_get_const(&file_name, 0);

	const char test_str[] = "hello";
	const uint32_t test_string_size = sizeof(test_str);

	ASSERT_TRUE(odFilePath_write_all(file_name_str, "w", test_str, test_string_size));

	odAllocation read_allocation;
	uint32_t read_size = 0;

	ASSERT_TRUE(odFilePath_read_all(file_name_str, "r", &read_allocation, &read_size));
	ASSERT_EQ(read_size, test_string_size);
	const char* read_allocation_str = static_cast<const char*>(odAllocation_get_const(&read_allocation));
	ASSERT_NE(read_allocation_str, nullptr);
	ASSERT_EQ(strncmp(test_str, read_allocation_str, test_string_size), 0);

	ASSERT_TRUE(odFilePath_delete(file_name_str));
}
