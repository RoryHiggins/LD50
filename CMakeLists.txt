cmake_minimum_required(VERSION 3.19)

project(overworld LANGUAGES CXX)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)
find_package(GTest REQUIRED)
include(GoogleTest)

enable_testing()
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED 1)
set(CMAKE_CXX_EXTENSIONS false)

add_library(od_engine SHARED)

add_executable(od_client)
add_executable(od_test)

# Optional gcc-specific warnings and static analysis tools
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options(-Wall -Wextra -pedantic -Werror=vla -Winvalid-pch -fno-exceptions -fno-rtti)
	add_compile_options(
		-Wold-style-cast -Wregister -Wshadow -Wvirtual-inheritance -Wnamespaces
		-Wzero-as-null-pointer-constant -Wpacked -Wredundant-decls -Wvolatile -Wvla)
	add_compile_options(-Wlarger-than=16777216 -Walloc-size-larger-than=16777216)
	add_compile_options(
		-fanalyzer
		-Walloc-zero -Warray-bounds=2 -Wcast-align=strict -Wconversion -Wundef
		-Wformat=2 -Wformat-overflow=2 -Wformat-signedness -Wformat-truncation=2
		-Wdisabled-optimization -Wfloat-conversion -Wlogical-op -Winline -Wmultichar
		-Wplacement-new=2 -Wnon-virtual-dtor -Wnull-dereference -Woverloaded-virtual -Wstringop-overflow=4)
endif()

set_target_properties(od_engine PROPERTIES LINKER_LANGUAGE CXX WINDOWS_EXPORT_ALL_SYMBOLS 0)
if(MINGW)
	target_link_libraries(od_engine PRIVATE mingw32)
endif()
if(WIN32)
	target_compile_definitions(od_engine PUBLIC OD_BUILD_DLL=1 PRIVATE OD_BUILD_DLL_EXPORT=1)
endif()
target_link_libraries(od_engine PRIVATE SDL2::SDL2main SDL2::SDL2 OpenGL::GL OpenGL::GLU GLEW::glew)

set_target_properties(od_client PROPERTIES LINKER_LANGUAGE CXX LINK_DEPENDS_NO_SHARED 1)
target_link_libraries(od_client PRIVATE od_engine)

set_target_properties(od_test PROPERTIES LINKER_LANGUAGE CXX LINKER_LANGUAGE CXX LINK_DEPENDS_NO_SHARED 1 EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
target_link_libraries(od_test PRIVATE GTest::GTest GTest::Main od_engine)
target_compile_options(od_test PRIVATE -Wno-global-constructors)  # for gtest TEST macros

add_subdirectory(include)
add_subdirectory(src)
