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

	if (file_path == nullptr) {
		OD_FILE_ERROR(file, "file_path=nullptr");
		return false;
	}

	if (mode == nullptr) {
		OD_FILE_ERROR(file, "mode=nullptr, file_path=%s", file_path);
		return false;
	}

	if (file == nullptr) {
		OD_FILE_ERROR(file, "file=nullptr, mode=%s, file_path=%s", mode, file_path);
		return false;
	}

	if (file->native_file != nullptr) {
		OD_FILE_ERROR(file, "already has an open file, mode=%s, file_path=%s", mode, file_path);
		return false;
	}

	file->native_file = static_cast<void*>(fopen(file_path, mode));
	if (file->native_file == nullptr) {
		OD_FILE_ERROR(file, "failed to open file, mode=%s, file_path=%s, errno_str=%s", mode, file_path, strerror(errno));
		return false;
	}

	return true;
}
void odFile_close(odFile* file) {
	OD_DEBUG("file=%p", static_cast<const void*>(file));

	if (file == nullptr) {
		OD_FILE_ERROR(file, "file=nullptr");
		return;
	}

	if (file->native_file != nullptr) {
		if (fclose(static_cast<FILE*>(file->native_file)) == EOF) {
			OD_FILE_ERROR(file, "error closing file, errno_str=%s", strerror(errno));
		}
		file->native_file = nullptr;
	}
}
bool odFile_read_size(odFile* file, int32_t* out_size) {
	OD_TRACE("file=%p, out_size=%p", static_cast<const void*>(file), static_cast<const void*>(out_size));

	if (file == nullptr) {
		OD_FILE_ERROR(file, "file=nullptr");
		return false;
	}

	if (out_size == nullptr) {
		OD_FILE_ERROR(file, "out_size=nullptr");
		return false;
	}

	long old_pos = ftell(static_cast<FILE*>(file->native_file));

	if (fseek(static_cast<FILE*>(file->native_file), 0, SEEK_END) != 0) {
		OD_FILE_ERROR(file, "error seeking to end of file, errno_str=%s", strerror(errno));
		return false;
	}

	long size = ftell(static_cast<FILE*>(file->native_file));
	if (size < 0) {
		OD_FILE_ERROR(file, "error getting position at end of file, result=%ld, errno_str=%s", size, strerror(errno));
		return false;
	}

	if (fseek(static_cast<FILE*>(file->native_file), old_pos, SEEK_SET) != 0) {
		OD_FILE_ERROR(file, "error seeking back to original position file, errno_str=%s", strerror(errno));
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

	if (!odFile_get_valid(file)) {
		OD_FILE_ERROR(file, "not valid");
		return false;
	}

	if (out_buffer == nullptr) {
		OD_FILE_ERROR(file, "out_buffer=nullptr");
		return false;
	}

	if (out_size == nullptr) {
		OD_FILE_ERROR(file, "out_size=nullptr");
		return false;
	}

	if (buffer_size <= 0) {
		OD_FILE_ERROR(file, "buffer_size<=0");
		return false;
	}

	FILE* native_file = static_cast<FILE*>(file->native_file);
	*out_size = static_cast<int32_t>(fread(out_buffer, 1, static_cast<size_t>(buffer_size), native_file));

	int read_error = ferror(native_file);
	if (read_error != 0) {
		OD_ERROR("Error reading from file, read_error=%d, err_str=%s", read_error, strerror(errno));
		clearerr(native_file);
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

	if (!odFile_get_valid(file)) {
		OD_FILE_ERROR(file, "not valid");
		return false;
	}

	if (buffer == nullptr) {
		OD_FILE_ERROR(file, "out_allocation=nullptr");
		return false;
	}

	if (size <= 0) {
		OD_FILE_ERROR(file, "size<=0");
		return false;
	}

	int32_t writes = static_cast<int32_t>(fwrite(buffer, static_cast<size_t>(size), 1, static_cast<FILE*>(file->native_file)));
	if (writes != 1) {
		OD_FILE_ERROR(file, "failed to write to file, err_str=%s", strerror(errno));
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

	if (file_path == nullptr) {
		OD_ERROR("file_path=nullptr");
		return false;
	}

	if (mode == nullptr) {
		OD_ERROR("mode=nullptr");
		return false;
	}

	if (out_allocation == nullptr) {
		OD_ERROR("out_allocation=nullptr");
		return false;
	}

	if (out_size == nullptr) {
		OD_ERROR("out_size=nullptr");
		return false;
	}

	odFile file;
	if (!odFile_open(&file, mode, file_path)) {
		return false;
	}

	if (file.native_file == nullptr) {
		OD_FILE_ERROR(&file, "no file to read");
		return false;
	}

	int32_t file_size = 0;
	if (!odFile_read_size(&file, &file_size)) {
		OD_FILE_ERROR(&file, "failed to read size");
		return false;
	}

	if (!odAllocation_allocate(out_allocation, file_size)) {
		OD_FILE_ERROR(&file, "failed to allocate output buffer");
		return false;
	}

	void* allocation_buffer = odAllocation_get(out_allocation);
	if (allocation_buffer == nullptr) {
		OD_FILE_ERROR(&file, "no buffer allocated");
		return false;
	}

	if (fseek(static_cast<FILE*>(file.native_file), 0, SEEK_SET) != 0) {
		OD_FILE_ERROR(&file, "error seeking to beginning of file, errno_str=%s", strerror(errno));
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

	if (file_path == nullptr) {
		OD_ERROR("file_path=nullptr");
		return false;
	}

	if (mode == nullptr) {
		OD_ERROR("mode=nullptr");
		return false;
	}

	if (buffer == nullptr) {
		OD_ERROR("out_allocation=nullptr");
		return false;
	}

	if (size <= 0) {
		OD_ERROR("size<=0");
		return false;
	}

	odFile file;
	if (!odFile_open(&file, mode, file_path)) {
		OD_FILE_ERROR(&file, "failed to open file");
		return false;
	}

	if (file.native_file == nullptr) {
		OD_FILE_ERROR(&file, "no file to write");
		return false;
	}

	return odFile_write(&file, buffer, size);
}
bool odFilePath_delete(const char* file_path) {
	OD_TRACE("file_path=%s", file_path ? file_path : "<nullptr>");

	if (file_path == nullptr) {
		OD_ERROR("file_path=nullptr");
		return false;
	}

	if (remove(file_path) != 0) {
		OD_ERROR("failed to delete file, file_path=%s, err_str=%s", file_path, strerror(errno));
		return false;
	}

	return true;
}
bool odFilePath_get_exists(const char* file_path) {
	OD_TRACE("file_path=%s", file_path ? file_path : "<nullptr>");

	if (file_path == nullptr) {
		OD_WARN("file_path=nullptr");
		return false;
	}

	FILE* file = fopen(file_path, "r");
	if (fclose(file) == EOF) {
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
