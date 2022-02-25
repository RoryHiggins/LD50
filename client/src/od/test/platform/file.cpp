#include <od/platform/file.hpp>

#include <cstring>

#include <od/core/allocation.hpp>
#include <od/core/string.hpp>
#include <od/test/test.hpp>

OD_TEST(odTest_odFile_open) {
	odString filename;
	OD_ASSERT(odTest_get_random_filename(&filename));
	odScopedTempFile temp_file{filename};
	const char* filename_str = filename.get_c_str();

	const int32_t write_modes_count = 6;
	const char write_modes[write_modes_count][8] = {"w", "wb", "w+", "wb+", "a", "a+"};
	for (int32_t mode = 0; mode < write_modes_count; mode++) {
		odFile file;
		OD_ASSERT(odFile_open(&file, write_modes[mode], filename_str));
	}

	const int32_t read_modes_count = 4;
	const char read_modes[read_modes_count][8] = {"r", "rb", "r+", "rb+"};
	for (int32_t mode = 0; mode < read_modes_count; mode++) {
		odFile file;
		OD_ASSERT(odFile_open(&file, read_modes[mode], filename_str));
	}

	OD_ASSERT(odFile_delete(filename_str));
}
OD_TEST(odTest_odFile_write_read_delete_buffered) {
	odString filename;
	OD_ASSERT(odTest_get_random_filename(&filename));
	odScopedTempFile temp_file{filename};
	const char* filename_str = filename.get_c_str();

	const char test_str[] = "hello";
	const int32_t test_string_size = sizeof(test_str);

	odFile file;
	OD_ASSERT(odFile_open(&file, "w", filename_str));

	OD_ASSERT(odFile_write(&file, test_str, test_string_size));

	odFile_close(&file);
	OD_ASSERT(odFile_open(&file, "r", filename_str));

	int32_t read_size = 0;
	char read_buffer[test_string_size] = {};
	OD_ASSERT(odFile_read(&file, read_buffer, test_string_size, &read_size));
	OD_ASSERT(read_size == test_string_size);
	OD_ASSERT(strncmp(test_str, read_buffer, test_string_size) == 0);
	odFile_close(&file);

	OD_ASSERT(odFile_delete(filename_str));
}
OD_TEST(odTest_odFile_write_read_delete_all) {
	odString filename;
	OD_ASSERT(odTest_get_random_filename(&filename));
	odScopedTempFile temp_file{filename};
	const char* filename_str = filename.get_c_str();

	const char test_str[] = "hello";
	const int32_t test_string_size = sizeof(test_str);

	OD_ASSERT(odFile_write_all(filename_str, "w", test_str, test_string_size));

	odAllocation read_allocation;
	int32_t read_size = 0;

	OD_ASSERT(odFile_read_all(filename_str, "r", &read_allocation, &read_size));
	OD_ASSERT(read_size == test_string_size);
	const char* read_allocation_str = static_cast<const char*>(odAllocation_get_const(&read_allocation));
	OD_ASSERT(read_allocation_str != nullptr);
	OD_ASSERT(strncmp(test_str, read_allocation_str, test_string_size) == 0);

	OD_ASSERT(odFile_delete(filename_str));
}

OD_TEST_SUITE(
	odTestSuite_odFile,
	odTest_odFile_open,
	odTest_odFile_write_read_delete_buffered,
	odTest_odFile_write_read_delete_all,
)
