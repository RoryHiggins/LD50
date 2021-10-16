#include <od/platform/file.hpp>

#include <errno.h>
#include <cstdio>
#include <cstring>

#include <od/core/allocation.h>
#include <od/core/debug.h>
#include <od/core/type.hpp>

#define OD_FILE_ERROR(FILE, ...) \
	OD_ERROR("%s", odFile_get_debug_string(FILE)); \
	OD_ERROR(__VA_ARGS__)

const odType* odFile_get_type_constructor(void) {
	return odType_get<odFile>();
}
void odFile_swap(odFile* file1, odFile* file2) {
	void* native_file_swap = file1->native_file;

	file1->native_file = file2->native_file;

	file2->native_file = native_file_swap;
}
bool odFile_get_valid(const struct odFile* file) {
	if (file == nullptr) {
		return false;
	}

	if (file->native_file == nullptr) {
		return false;
	}

	return true;
}
const char* odFile_get_debug_string(const struct odFile* file) {
	if (file == nullptr) {
		return "odFile{this=nullptr}";
	}

	return odDebugString_format(
		"odFile{this=%p, native_file=%p}",
		static_cast<const void*>(file),
		static_cast<const void*>(file->native_file));
}
bool odFile_open(odFile* file, const char* mode, const char* file_path) {
	OD_DEBUG(
		"file=%s, mode=%s, file_path=%s",
		odFile_get_debug_string(file),
		mode ? mode : "<nullptr>",
		file_path ? file_path : "<nullptr>");

	if (!OD_DEBUG_CHECK(file_path != nullptr)
		|| !OD_DEBUG_CHECK(mode != nullptr)
		|| !OD_DEBUG_CHECK(file != nullptr)) {
		return false;
	}

	odFile_close(file);

	file->native_file = static_cast<void*>(fopen(file_path, mode));
	if (!OD_CHECK(file->native_file != nullptr)) {
		OD_ERROR(
			"failed to open file, file=%s, mode=%s, file_path=%s, errno_str=%s",
			odFile_get_debug_string(file), mode, file_path, strerror(errno));
		return false;
	}

	return true;
}
void odFile_close(odFile* file) {
	OD_DEBUG("file=%p", static_cast<const void*>(file));

	if (!OD_DEBUG_CHECK(file != nullptr)) {
		return;
	}

	if (file->native_file != nullptr) {
		if (!OD_CHECK(fclose(static_cast<FILE*>(file->native_file)) != EOF)) {
			OD_ERROR(
				"error closing file, file=%s, errno_str=%s",
				odFile_get_debug_string(file), strerror(errno));
		}
		file->native_file = nullptr;
	}
}
bool odFile_read_size(odFile* file, int32_t* out_size) {
	OD_TRACE("file=%p, out_size=%p", static_cast<const void*>(file), static_cast<const void*>(out_size));

	if (!OD_DEBUG_CHECK(file != nullptr)
		|| !OD_DEBUG_CHECK(out_size != nullptr)) {
		return false;
	}

	long old_pos = ftell(static_cast<FILE*>(file->native_file));

	if (!OD_CHECK(fseek(static_cast<FILE*>(file->native_file), 0, SEEK_END) == 0)) {
		OD_ERROR(
			"error seeking to end of file, file=%s, errno_str=%s",
			odFile_get_debug_string(file), strerror(errno));
		return false;
	}

	long size = ftell(static_cast<FILE*>(file->native_file));
	if (!OD_CHECK(size >= 0)) {
		OD_ERROR(
			"error getting position at end of file, file=%s, result=%ld, errno_str=%s",
			odFile_get_debug_string(file), size, strerror(errno));
		return false;
	}

	if (fseek(static_cast<FILE*>(file->native_file), old_pos, SEEK_SET) != 0) {
		OD_ERROR(
			"error seeking back to original position file, file=%s, errno_str=%s",
			odFile_get_debug_string(file), strerror(errno));
		return false;
	}

	*out_size = static_cast<int32_t>(size);
	return true;
}
bool odFile_read(odFile* file, void* out_buffer, int32_t buffer_size, int32_t* out_size) {
	OD_TRACE(
		"file=%p, out_buffer=%p, buffer_size=%d, out_size=%p",
		static_cast<const void*>(file),
		static_cast<const void*>(out_buffer),
		buffer_size,
		static_cast<const void*>(out_size));

	if (!OD_DEBUG_CHECK(odFile_get_valid(file))
		|| !OD_DEBUG_CHECK(out_buffer != nullptr)
		|| !OD_DEBUG_CHECK(out_size != nullptr)
		|| !OD_DEBUG_CHECK(buffer_size > 0)) {
		return false;
	}

	FILE* native_file = static_cast<FILE*>(file->native_file);
	*out_size = static_cast<int32_t>(fread(out_buffer, 1, static_cast<size_t>(buffer_size), native_file));

	int read_error = ferror(native_file);
	if (!OD_CHECK(read_error == 0)) {
		OD_ERROR(
			"Error reading from file, file=%s, read_error=%d, err_str=%s",
			odFile_get_debug_string(file), read_error, strerror(errno));
		return false;
	}

	return true;
}
bool odFile_write(odFile* file, const void* buffer, int32_t size) {
	OD_TRACE(
		"file=%p, buffer=%p, size=%d",
		static_cast<const void*>(file),
		static_cast<const void*>(buffer),
		size);

	if (!OD_DEBUG_CHECK(odFile_get_valid(file))
		|| !OD_DEBUG_CHECK(buffer != nullptr)
		|| !OD_DEBUG_CHECK(size > 0)) {
		return false;
	}

	int32_t writes = static_cast<int32_t>(fwrite(buffer, static_cast<size_t>(size), 1, static_cast<FILE*>(file->native_file)));
	if (!OD_CHECK(writes == 1)) {
		OD_ERROR(
			"failed to write to file, file=%s, err_str=%s",
			odFile_get_debug_string(file), strerror(errno));
		return false;
	}

	return true;
}

bool odFilePath_read_all(
	const char* file_path, const char* mode, struct odAllocation* out_allocation, int32_t* out_size) {
	OD_TRACE(
		"file_path=%s, mode=%s, out_allocation=%p, out_size=%p",
		file_path ? file_path : "<nullptr>",
		mode ? mode : "<nullptr>",
		static_cast<const void*>(out_allocation),
		static_cast<const void*>(out_size));

	if (!OD_DEBUG_CHECK(file_path != nullptr)
		|| !OD_DEBUG_CHECK(mode != nullptr)
		|| !OD_DEBUG_CHECK(out_allocation != nullptr)
		|| !OD_DEBUG_CHECK(out_size != nullptr)) {
		return false;
	}

	odFile file;
	if (!OD_CHECK(odFile_open(&file, mode, file_path))) {
		return false;
	}

	if (!OD_CHECK(file.native_file != nullptr)) {
		return false;
	}

	int32_t file_size = 0;
	if (!OD_CHECK(odFile_read_size(&file, &file_size))) {
		return false;
	}

	if (!OD_CHECK(odAllocation_allocate(out_allocation, file_size))) {
		return false;
	}

	void* allocation_buffer = odAllocation_get(out_allocation);
	if (!OD_CHECK(allocation_buffer != nullptr)) {
		return false;
	}

	if (!OD_CHECK(fseek(static_cast<FILE*>(file.native_file), 0, SEEK_SET) == 0)) {
		OD_ERROR(
			"error seeking to beginning of file, file=%s, errno_str=%s",
			odFile_get_debug_string(&file), strerror(errno));
		return false;
	}

	return odFile_read(&file, allocation_buffer, file_size, out_size);
}
bool odFilePath_write_all(const char* file_path, const char* mode, const void* buffer, int32_t size) {
	OD_TRACE(
		"file_path=%s, mode=%s, out_allocation=%p, size=%d",
		file_path ? file_path : "<nullptr>",
		mode ? mode : "<nullptr>",
		static_cast<const void*>(buffer),
		size);

	if (!OD_DEBUG_CHECK(file_path != nullptr)
		|| !OD_DEBUG_CHECK(mode != nullptr)
		|| !OD_DEBUG_CHECK(buffer != nullptr)
		|| !OD_DEBUG_CHECK(size > 0)) {
		return false;
	}

	odFile file;
	if (!OD_CHECK(odFile_open(&file, mode, file_path))) {
		return false;
	}

	if (!OD_CHECK(file.native_file != nullptr)) {
		return false;
	}

	return odFile_write(&file, buffer, size);
}
bool odFilePath_delete(const char* file_path) {
	OD_TRACE("file_path=%s", file_path ? file_path : "<nullptr>");

	if (!OD_DEBUG_CHECK(file_path != nullptr)) {
		return false;
	}

	if (!OD_CHECK(remove(file_path) == 0)) {
		OD_ERROR("failed to delete file, file_path=%s, err_str=%s", file_path, strerror(errno));
		return false;
	}

	return true;
}
bool odFilePath_get_exists(const char* file_path) {
	OD_TRACE("file_path=%s", file_path ? file_path : "<nullptr>");

	if (!OD_DEBUG_CHECK(file_path != nullptr)) {
		return false;
	}

	FILE* file = fopen(file_path, "r");
	if (!OD_CHECK(fclose(file) != EOF)) {
		OD_WARN("error closing file, file_path=%s, errno_str=%s", file_path, strerror(errno));
	}

	return (file != nullptr);
}

odFile::odFile() : native_file{nullptr} {
}
odFile::odFile(odFile&& other) : odFile{} {
	odFile_swap(this, &other);
}
odFile& odFile::operator=(odFile&& other) {
	odFile_swap(this, &other);
	return *this;
}
odFile::~odFile() {
	odFile_close(this);
}
