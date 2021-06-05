#pragma once

#include <od/core/api.h>

struct odFile;


OD_API_C void odFile_construct(struct odFile* file);
OD_API_C void odFile_destruct(struct odFile* file);
OD_API_C bool odFile_open(struct odFile* file, const char* mode, const char* file_path);
OD_API_C void odFile_close(struct odFile* file);
OD_API_C bool odFile_read_size(struct odFile* file, uint64_t* out_size);
OD_API_C bool odFile_read_all(struct odFile* file, struct odAllocation *out_data);
OD_API_C bool odFile_write_all(struct odFile* file, const struct odAllocation *data);
OD_API_C bool odFile_get_exists(const char* file_path);
