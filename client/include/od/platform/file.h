#pragma once

#include <od/platform/module.h>

struct odAllocation;
struct odFile;

OD_API_C OD_ENGINE_PLATFORM_MODULE const struct odType* odFile_get_type_constructor(void);
OD_API_C OD_ENGINE_PLATFORM_MODULE void odFile_swap(struct odFile* file1, struct odFile* file2);
OD_API_C OD_ENGINE_PLATFORM_MODULE bool odFile_get_valid(const struct odFile* file);
OD_API_C OD_ENGINE_PLATFORM_MODULE const char* odFile_get_debug_string(const struct odFile* file);
OD_API_C OD_ENGINE_PLATFORM_MODULE bool odFile_open(struct odFile* file, const char* mode, const char* file_path);
OD_API_C OD_ENGINE_PLATFORM_MODULE void odFile_close(struct odFile* file);
OD_API_C OD_ENGINE_PLATFORM_MODULE bool odFile_read_size(struct odFile* file, int32_t* out_size);
OD_API_C OD_ENGINE_PLATFORM_MODULE bool
odFile_read(struct odFile* file, void* out_buffer, int32_t buffer_size, int32_t* out_size);
OD_API_C OD_ENGINE_PLATFORM_MODULE bool odFile_write(struct odFile* file, const void* buffer, int32_t size);

OD_API_C OD_ENGINE_PLATFORM_MODULE bool
odFilePath_read_all(const char* file_path, const char* mode, struct odAllocation* out_allocation, int32_t* out_size);
OD_API_C OD_ENGINE_PLATFORM_MODULE bool
odFilePath_write_all(const char* file_path, const char* mode, const void* buffer, int32_t size);
OD_API_C OD_ENGINE_PLATFORM_MODULE bool odFilePath_delete(const char* file_path);
OD_API_C OD_ENGINE_PLATFORM_MODULE bool odFilePath_get_exists(const char* file_path);
