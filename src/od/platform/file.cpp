#include <od/platform/file.hpp>

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <od/core/allocation.h>
#include <od/core/debug.h>
#include <od/core/type.hpp>

const odType* odFile_get_type_constructor(void) {
	return odType_get<odFile>();
}
void odFile_swap(odFile* file1, odFile* file2) {
	void* native_file_swap = file1->native_file;

	file1->native_file = file2->native_file;

	file2->native_file = native_file_swap;
}
bool odFile_open(odFile* file, const char* mode, const char* file_path) {
	if (file == nullptr) {
		OD_ERROR("file=nullptr");
		return false;
	}

	if (mode == nullptr) {
		OD_ERROR("mode=nullptr");
		return false;
	}

	if (file_path == nullptr) {
		OD_ERROR("file_path=nullptr");
		return false;
	}

	if (file->native_file != nullptr) {
		OD_WARN("already has an open file, closing");
		odFile_close(file);
	}

	file->native_file = static_cast<void*>(fopen(file_path, mode));
	if (file->native_file == nullptr) {
		OD_ERROR("failed to open file, mode=%s, file_path=%s, errno_str=%s", mode, file_path, strerror(errno));
		return false;
	}

	return true;
}
void odFile_close(odFile* file) {
	if (file == nullptr) {
		OD_ERROR("file=nullptr");
		return;
	}

	if (file->native_file == nullptr) {
		OD_WARN("no file to close, errno_str=%s", strerror(errno));
		return;
	}

	if (fclose(static_cast<FILE*>(file->native_file)) == EOF) {
		OD_ERROR("error closing file, errno_str=%s", strerror(errno));
	}

	file->native_file = nullptr;
}
bool odFile_read_size(odFile* file, uint32_t* out_size) {
	if (file == nullptr) {
		OD_ERROR("file=nullptr");
		return false;
	}

	if (out_size == nullptr) {
		OD_ERROR("out_size=nullptr");
		return false;
	}

	long old_pos = ftell(static_cast<FILE*>(file->native_file));

	if (fseek(static_cast<FILE*>(file->native_file), 0, SEEK_END) != 0) {
		OD_ERROR("error seeking to end of file, errno_str=%s", strerror(errno));
		return false;
	}

	long size = ftell(static_cast<FILE*>(file->native_file));
	if (size < 0) {
		OD_ERROR("error getting position at end of file, result=%ld, errno_str=%s", size, strerror(errno));
		return false;
	}

	if (fseek(static_cast<FILE*>(file->native_file), old_pos, SEEK_SET) != 0) {
		OD_ERROR("error seeking back to original position file, errno_str=%s", strerror(errno));
		return false;
	}

	*out_size = static_cast<uint32_t>(size);
	return true;
}
bool odFile_read(odFile* file, void* out_buffer, uint32_t buffer_size, uint32_t* out_size) {
	if (file == nullptr) {
		OD_ERROR("file=nullptr");
		return false;
	}

	if (out_buffer == nullptr) {
		OD_ERROR("out_buffer=nullptr");
		return false;
	}

	if (out_size == nullptr) {
		OD_ERROR("out_size=nullptr");
		return false;
	}

	if (file->native_file == nullptr) {
		OD_ERROR("no file to read");
		return false;
	}

	if (buffer_size == 0) {
		return true;
	}

	FILE* native_file = static_cast<FILE*>(file->native_file);
	*out_size = static_cast<uint32_t>(fread(out_buffer, 1, buffer_size, native_file));

	int read_error = ferror(native_file);
	if (read_error != 0) {
		OD_ERROR("Error reading from file, read_error=%d, err_str=%s", read_error, strerror(errno));
		clearerr(native_file);
		return false;
	}

	return true;
}
bool odFile_write(odFile* file, const void* buffer, uint32_t size) {
	if (file == nullptr) {
		OD_ERROR("file=nullptr");
		return false;
	}

	if (buffer == nullptr) {
		OD_ERROR("out_allocation=nullptr");
		return false;
	}

	if (file->native_file == nullptr) {
		OD_ERROR("no file to write");
		return false;
	}

	uint32_t write_size = static_cast<uint32_t>(fwrite(buffer, size, 1, static_cast<FILE*>(file->native_file)));
	if ((write_size != 1) && (size > 0)) {
		OD_ERROR("failed to write to file, err_str=%s", strerror(errno));
		return false;
	}

	return true;
}

bool odFilePath_read_all(
	const char* file_path, const char* mode, struct odAllocation* out_allocation, uint32_t* out_size) {
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
		OD_ERROR("no file to read");
		return false;
	}

	uint32_t file_size = 0;
	if (!odFile_read_size(&file, &file_size)) {
		return false;
	}

	if (!odAllocation_allocate(out_allocation, file_size)) {
		return false;
	}

	void* allocation_buffer = odAllocation_get(out_allocation);
	if (allocation_buffer == nullptr) {
		OD_ERROR("no buffer allocated");
		return false;
	}

	if (fseek(static_cast<FILE*>(file.native_file), 0, SEEK_SET) != 0) {
		OD_ERROR("error seeking to beginning of file, errno_str=%s", strerror(errno));
		return false;
	}

	return odFile_read(&file, allocation_buffer, file_size, out_size);
}
bool odFilePath_write_all(const char* file_path, const char* mode, const void* buffer, uint32_t size) {
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

	odFile file;
	if (!odFile_open(&file, mode, file_path)) {
		return false;
	}

	if (file.native_file == nullptr) {
		OD_ERROR("no file to write");
		return false;
	}

	return odFile_write(&file, buffer, size);
}
bool odFilePath_delete(const char* file_path) {
	if (remove(file_path) != 0) {
		OD_ERROR("failed to delete file, file_path=%s, err_str=%s", file_path, strerror(errno));
		return false;
	}

	return true;
}
bool odFilePath_get_exists(const char* file_path) {
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
	if (native_file != nullptr) {
		odFile_close(this);
	}
}
