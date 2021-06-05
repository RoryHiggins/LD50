#include <od/core.h>
#include <od/platform/file.hpp>

#include <stdio.h>
#include <string.h>
#include <errno.h>

void odFile_construct(odFile* file) {
	if (file == nullptr) {
		OD_ERROR("file=nullptr");
		return;
	}

	file->native_file = nullptr;
}
void odFile_destruct(odFile* file) {
	if (file == nullptr) {
		OD_ERROR("file=nullptr");
		return;
	}

	if (file->native_file != nullptr) {
		odFile_close(file);
	}

	file->native_file = nullptr;
}
bool odFile_open(odFile* file, const char* mode, const char* file_path) {
	if (file == nullptr) {
		OD_ERROR("file=nullptr");
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
		OD_ERROR("failed to open file, file_path=%s, errno_str=%s", file_path, strerror(errno));
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
bool odFile_read_size(odFile* file, uint64_t* out_size) {
	if (file == nullptr) {
		OD_ERROR("file=nullptr");
		return false;
	}

	if (out_size == nullptr) {
		OD_ERROR("out_size=nullptr");
		return false;
	}

	if (fseek(static_cast<FILE*>(file->native_file), 0, SEEK_END) == EOF) {
		OD_ERROR("error seeking to end of file, errno_str=%s", strerror(errno));
		return false;
	}

	long size = ftell(static_cast<FILE*>(file->native_file));
	if (size < 0) {
		OD_ERROR("error getting position at end of file, result=%ld, errno_str=%s", size, strerror(errno));
		return false;
	}

	*out_size = static_cast<uint32_t>(size);
	return true;
}
// bool odFile_read(odFile* file, void *out_data, uint64_t size) {
// 	if (file == nullptr) {
// 		OD_ERROR("file=nullptr");
// 		return false;
// 	}

// 	if (out_data == nullptr) {
// 		OD_ERROR("out_data_ptr=nullptr");
// 		return false;
// 	}

// 	if (file->native_file == nullptr) {
// 		OD_ERROR("no file to read");
// 		return false;
// 	}

// 	if (fseek(static_cast<FILE*>(file->native_file), 0, SEEK_SET) == EOF) {
// 		OD_ERROR("error seeking to beginning of file, errno_str=%s", strerror(errno));
// 		return false;
// 	}


// }
// bool odFile_write_all(struct odFile* file, const void *data) {
// 	if (file == nullptr) {
// 		OD_ERROR("file=nullptr");
// 		return false;
// 	}

// 	if (data == nullptr) {
// 		OD_ERROR("out_data_ptr=nullptr");
// 		return false;
// 	}

// }
bool odFile_get_exists(const char* file_path) {
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
