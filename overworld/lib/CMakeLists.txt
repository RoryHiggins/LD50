set(BUILD_SHARED_LIBS 0)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(PNG_SHARED 0)
set(PNG_EXECUTABLES 0)
set(PNG_TESTS 0)

add_subdirectory(zlib)
add_subdirectory(libpng)
