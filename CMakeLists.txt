cmake_minimum_required(VERSION 3.19)

project(overworld LANGUAGES CXX)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(GTest REQUIRED)
include(GoogleTest)

enable_testing()
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED 1)
set(CMAKE_CXX_EXTENSIONS 0)
set(OD_BUILD_DLL 0)
set(OD_LINK_MODE "STATIC")

if(CMAKE_BUILD_TYPE STREQUAL "DEBUG")
	if(WIN32)
		set(OD_BUILD_DLL 1)
		set(OD_LINK_MODE "SHARED")
	endif()
endif()

add_library(od_engine_core ${OD_LINK_MODE})
add_library(od_engine_debug_gui ${OD_LINK_MODE})
add_library(od_engine_platform ${OD_LINK_MODE})

add_executable(od_client)
add_executable(od_test)

set(od_engine_libs od_engine_core od_engine_debug_gui od_engine_platform)
set(od_engine_binaries od_client od_test)
set(od_targets ${od_engine_libs} ${od_engine_binaries})

foreach (od_engine_lib ${od_engine_libs})
	set_target_properties(${od_engine_lib} PROPERTIES LINKER_LANGUAGE CXX WINDOWS_EXPORT_ALL_SYMBOLS 0)
endforeach()
if(OD_BUILD_DLL)
	foreach (od_target ${od_targets})
		target_compile_definitions(${od_target} PUBLIC OD_BUILD_DLL=1)
	endforeach()
	target_compile_definitions(od_engine_core PRIVATE OD_BUILD_DLL_ENGINE_CORE=1)
	target_compile_definitions(od_engine_debug_gui PRIVATE OD_BUILD_DLL_ENGINE_DEBUG_GUI=1)
	target_compile_definitions(od_engine_platform PRIVATE OD_BUILD_DLL_ENGINE_PLATFORM=1)
endif()
if(MINGW)
	target_link_libraries(od_engine_platform PRIVATE mingw32)
	target_link_libraries(od_engine_debug_gui PRIVATE gdi32 shell32 imm32 opengl32)
endif()
target_link_libraries(od_engine_debug_gui PRIVATE SDL2::SDL2 OpenGL::GL OpenGL::GLU GLEW::glew od_engine_core)
target_link_libraries(od_engine_platform PRIVATE SDL2::SDL2main SDL2::SDL2 OpenGL::GL OpenGL::GLU GLEW::glew PNG::PNG luajit-5.1 ZLIB::ZLIB od_engine_core od_engine_debug_gui)

foreach (od_engine_binary ${od_engine_binaries})
	set_target_properties(${od_engine_binary} PROPERTIES LINKER_LANGUAGE CXX LINK_DEPENDS_NO_SHARED 1)
endforeach()
target_link_libraries(od_client PRIVATE ${od_engine_libs})
set_target_properties(od_test PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
target_link_libraries(od_test PRIVATE GTest::GTest GTest::Main ${od_engine_libs})
target_compile_options(od_test PRIVATE "-Wno-global-constructors")  # for gtest TEST macros

add_subdirectory(lib)
add_subdirectory(include)
add_subdirectory(src)

# Optional gcc-specific warnings and static analysis tools
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	function(od_add_compile_link_options)
		foreach (od_target ${od_targets})
			add_compile_options(${od_target} ${ARGV})
			add_link_options(${od_target} ${ARGV})
		endforeach()
	endfunction()

	od_add_compile_link_options(-Wall -Wextra -pedantic -Werror=vla -Winvalid-pch -fno-exceptions -fno-rtti)
	od_add_compile_link_options(
		-Wold-style-cast -Wregister -Wshadow -Wvirtual-inheritance -Wnamespaces
		-Wzero-as-null-pointer-constant -Wpacked -Wredundant-decls -Wvolatile -Wvla)
	od_add_compile_link_options(-Wlarger-than=16777216 -Walloc-size-larger-than=16777216)
	od_add_compile_link_options(
		-Walloc-zero -Warray-bounds=2 -Wcast-align=strict -Wconversion -Wundef
		-Wformat=2 -Wformat-overflow=2 -Wformat-signedness -Wformat-truncation=2
		-Wdisabled-optimization -Wfloat-conversion -Wlogical-op -Winline -Wmultichar
		-Wplacement-new=2 -Wnon-virtual-dtor -Wnull-dereference -Woverloaded-virtual -Wstringop-overflow=4)

	if (OD_LINK_MODE STREQUAL "STATIC")
		od_add_compile_link_options(-static-libstdc++ -static-libgcc)
	endif()

	if(CMAKE_BUILD_TYPE STREQUAL "DEBUG")
		od_add_compile_link_options(-Og -ggdb3 -pg)

		if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 10.0.0)
			od_add_compile_link_options(-fanalyzer)
		endif()

		if(NOT WIN32)
			od_add_compile_link_options(-fsanitize=address,leak,undefined -fsanitize-recover=address -fsanitize-address-use-after-scope -fstack-protector)
		endif()
	endif()

	if(CMAKE_BUILD_TYPE STREQUAL "RELEASE")
		od_add_compile_link_options(-Os -s -ffast-math -flto -fwhole-program)
		if (WIN32)
			od_add_compile_link_options(-mwindows)
		endif()
	endif()
endif()
