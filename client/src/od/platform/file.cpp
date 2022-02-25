#include <od/platform/file.hpp>

#include <errno.h>
#include <cstdio>
#include <cstring>

#include <od/core/debug.h>
#include <od/core/allocation.hpp>
#include <od/core/type.hpp>

#define OD_FILE_ERROR(FILE, ...) \
	OD_ERROR("%s", odFile_get_debug_string(FILE)); \
	OD_ERROR(__VA_ARGS__)

const odType* odFile_get_type_constructor(void) {
	return odType_get<odFile>();
}
void odFile_swap(odFile* file1, odFile* file2) {
	if (!OD_CHECK(file1 != nullptr)
		|| !OD_CHECK(file2 != nullptr)) {
		return;
	}

	void* native_file_swap = file1->native_file;
	file1->native_file = file2->native_file;
	file2->native_file = native_file_swap;
}
bool odFile_check_valid(const struct odFile* file) {
	if (!OD_CHECK(file != nullptr)
		|| !OD_CHECK(file->native_file != nullptr)) {
		return false;
	}

	return true;
}
const char* odFile_get_debug_string(const struct odFile* file) {
	if (file == nullptr) {
		return "null";
	}

	return odDebugString_format("\"%p\"", static_cast<const void*>(file->native_file));
}
bool odFile_open(odFile* file, const char* mode, const char* filename) {
	OD_DEBUG(
		"file=%s, mode=%s, filename=%s",
		odFile_get_debug_string(file),
		mode ? mode : "<nullptr>",
		filename ? filename : "<nullptr>");

	if (!OD_CHECK(filename != nullptr)
		|| !OD_CHECK(mode != nullptr)
		|| !OD_CHECK(file != nullptr)) {
		return false;
	}

	odFile_close(file);

	file->native_file = static_cast<void*>(fopen(filename, mode));
	if (!OD_CHECK(file->native_file != nullptr)) {
		OD_ERROR(
			"failed to open file, file=%s, mode=%s, filename=%s, errno_str=%s",
			odFile_get_debug_string(file), mode, filename, strerror(errno));
		return false;
	}

	return true;
}
void odFile_close(odFile* file) {
	OD_DEBUG("file=%p", static_cast<const void*>(file));

	if (!OD_CHECK(file != nullptr)) {
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

	if (!OD_CHECK(file != nullptr)
		|| !OD_CHECK(out_size != nullptr)) {
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

	if (!OD_CHECK(odFile_check_valid(file))
		|| !OD_CHECK(out_buffer != nullptr)
		|| !OD_CHECK(out_size != nullptr)
		|| !OD_CHECK(buffer_size > 0)) {
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

	if (!OD_CHECK(odFile_check_valid(file))
		|| !OD_CHECK(buffer != nullptr)
		|| !OD_CHECK(size > 0)) {
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

bool odFile_read_all(
	const char* filename, const char* mode, struct odAllocation* out_allocation, int32_t* out_size) {
	OD_TRACE(
		"filename=%s, mode=%s, out_allocation=%p, out_size=%p",
		filename ? filename : "<nullptr>",
		mode ? mode : "<nullptr>",
		static_cast<const void*>(out_allocation),
		static_cast<const void*>(out_size));

	if (!OD_CHECK(filename != nullptr)
		|| !OD_CHECK(mode != nullptr)
		|| !OD_CHECK(out_allocation != nullptr)
		|| !OD_CHECK(out_size != nullptr)) {
		return false;
	}

	odFile file;
	if (!OD_CHECK(odFile_open(&file, mode, filename))) {
		return false;
	}

	if (!OD_CHECK(file.native_file != nullptr)) {
		return false;
	}

	int32_t file_size = 0;
	if (!OD_CHECK(odFile_read_size(&file, &file_size))) {
		return false;
	}

	if (!OD_CHECK(odAllocation_init(out_allocation, file_size))) {
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
bool odFile_write_all(const char* filename, const char* mode, const void* buffer, int32_t size) {
	OD_TRACE(
		"filename=%s, mode=%s, out_allocation=%p, size=%d",
		filename ? filename : "<nullptr>",
		mode ? mode : "<nullptr>",
		static_cast<const void*>(buffer),
		size);

	if (!OD_CHECK(filename != nullptr)
		|| !OD_CHECK(mode != nullptr)
		|| !OD_CHECK(buffer != nullptr)
		|| !OD_CHECK(size > 0)) {
		return false;
	}

	odFile file;
	if (!OD_CHECK(odFile_open(&file, mode, filename))) {
		return false;
	}

	if (!OD_CHECK(file.native_file != nullptr)) {
		return false;
	}

	return odFile_write(&file, buffer, size);
}
bool odFile_delete(const char* filename) {
	OD_TRACE("filename=%s", filename ? filename : "<nullptr>");

	if (!OD_CHECK(filename != nullptr)) {
		return false;
	}

	if (!OD_CHECK(remove(filename) == 0)) {
		OD_ERROR("failed to delete file, filename=%s, err_str=%s", filename, strerror(errno));
		return false;
	}

	return true;
}
bool odFile_get_exists(const char* filename) {
	OD_TRACE("filename=%s", filename ? filename : "<nullptr>");

	if (!OD_CHECK(filename != nullptr)) {
		return false;
	}

	FILE* file = fopen(filename, "r");
	if ((file != nullptr) && !OD_CHECK(fclose(file) != EOF)) {
		OD_WARN("error closing file, filename=%s, errno_str=%s", filename, strerror(errno));
		return false;
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

odScopedTempFile::odScopedTempFile(odString in_filename)
: filename{static_cast<odString&&>(in_filename)} {
}
odScopedTempFile::~odScopedTempFile() {
	const char* filename_c_str = filename.get_c_str();
	if (odFile_get_exists(filename_c_str)) {
		OD_DISCARD(OD_CHECK(odFile_delete(filename_c_str)));
	}
	OD_DISCARD(filename.set_count(0));
}
