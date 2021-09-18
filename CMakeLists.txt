cmake_minimum_required(VERSION 3.19)

project(overworld LANGUAGES CXX)

set(OD_BUILD_LOG_DEFAULT 1)
set(OD_BUILD_TESTS_DEFAULT 0)
if(${CMAKE_BUILD_TYPE} STREQUAL DEBUG)
	set(OD_BUILD_DEBUG_LOG_DEFAULT 1)
else()
	set(OD_BUILD_DEBUG_LOG_DEFAULT 0)
endif()
if(${WIN32} AND (${CMAKE_BUILD_TYPE} STREQUAL DEBUG))
	set(OD_BUILD_DLL_DEFAULT 1)
else()
	set(OD_BUILD_DLL_DEFAULT 0)
endif()

set(OD_BUILD_LOG ${OD_BUILD_LOG_DEFAULT} CACHE BOOL "Build logging")
set(OD_BUILD_DEBUG_LOG ${OD_BUILD_DEBUG_LOG_DEFAULT} CACHE BOOL "Build debug logging")
set(OD_BUILD_DLL ${OD_BUILD_DLL_DEFAULT} CACHE BOOL "Build engine DLLs")
set(OD_BUILD_TESTS ${OD_BUILD_TESTS_DEFAULT} CACHE BOOL "Build tests")

set(OD_BUILD_MODULE_LINKAGE "STATIC")
if(${OD_BUILD_DLL})
	set(OD_BUILD_MODULE_LINKAGE "SHARED")
endif()

find_package(SDL2 REQUIRED)
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(GTest REQUIRED)
include(GoogleTest)

add_library(od_engine_core ${OD_BUILD_MODULE_LINKAGE})
add_library(od_engine_platform ${OD_BUILD_MODULE_LINKAGE})

add_executable(od_test)
add_executable(od_client)

set(od_libs od_engine_core od_engine_platform)
set(od_binaries od_test od_client)
set(od_targets ${od_libs} ${od_binaries})

foreach(od_target ${od_targets})
	set_target_properties(
		${od_target}
		PROPERTIES
		CXX_STANDARD 11
		CXX_STANDARD_REQUIRED 1
		CXX_EXTENSIONS 0
		LINKER_LANGUAGE CXX
		LINK_DEPENDS_NO_SHARED 1
		WINDOWS_EXPORT_ALL_SYMBOLS 0
		UNITY_BUILD 1)

	target_compile_definitions(
		${od_target}
		PUBLIC
		OD_BUILD_DLL=${OD_BUILD_DLL}
		OD_BUILD_LOG=${OD_BUILD_LOG}
		OD_BUILD_DEBUG_LOG=${OD_BUILD_DEBUG_LOG})

	# Optional gcc-specific warnings and static analysis tools
	if(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
		if(${OD_BUILD_DLL} AND (${od_target} IN_LIST od_libs))
			foreach(od_target_inner ${od_targets})
				string(TOUPPER ${od_target} od_target_uppercase)

				if(${od_target_inner} STREQUAL ${od_target})
					set(dod_target_linkage "__declspec(dllexport)")
				else()
					set(dod_target_linkage "__declspec(dllimport)")
				endif()

				target_compile_definitions(${od_target_inner} PRIVATE "${od_target_uppercase}_MODULE=${dod_target_linkage}")
			endforeach()
		endif()

		target_compile_options(
			${od_target}
			PUBLIC
			-Wall -Wextra -pedantic -Werror=vla -Winvalid-pch -fno-exceptions -fno-rtti
			-Wold-style-cast -Wregister -Wshadow -Wvirtual-inheritance -Wnamespaces
			-Wzero-as-null-pointer-constant -Wpacked -Wredundant-decls -Wvolatile -Wvla
			-Wlarger-than=16777216 -Walloc-size-larger-than=16777216
			-Walloc-zero -Warray-bounds=2 -Wcast-align=strict -Wconversion -Wundef
			-Wformat=2 -Wformat-overflow=2 -Wformat-signedness -Wformat-truncation=2
			-Wdisabled-optimization -Wfloat-conversion -Wlogical-op -Winline -Wmultichar
			-Wplacement-new=2 -Wnon-virtual-dtor -Wnull-dereference -Woverloaded-virtual -Wstringop-overflow=4)

		if(${CMAKE_BUILD_TYPE} STREQUAL DEBUG)
			target_compile_options(${od_target} PUBLIC -ggdb3)
			target_compile_definitions(${od_target} PRIVATE _FORTIFY_SOURCE=2)

			if(CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 10.0.0)
				target_compile_options(${od_target} PUBLIC -fanalyzer)
			endif()

			if(NOT ${WIN32})
				target_compile_options(
					${od_target}
					PUBLIC
					-fsanitize=address,leak,undefined -fsanitize-recover=address -fsanitize-address-use-after-scope -fstack-protector-all)
			endif()
		endif()

		if(${CMAKE_BUILD_TYPE} STREQUAL RELEASE)
			target_compile_options(${od_target} PUBLIC -Os -s -ffast-math -flto)
			target_link_options(${od_target} PUBLIC -Os -s -flto -fwhole-program)

			if(${WIN32})
				target_link_options(${od_target} PUBLIC -mwindows)
			endif()

			if(${OD_BUILD_MODULE_LINKAGE} STREQUAL STATIC)
				target_compile_options(${od_target} PUBLIC -static-libstdc++ -static-libgcc)
				target_link_options(${od_target} PUBLIC -static-libstdc++ -static-libgcc)
			endif()
		endif()
	endif()
endforeach()

if(${MINGW})
	target_link_libraries(od_engine_platform PRIVATE mingw32)  # mingw32 must be before sdl2main
endif()
target_link_libraries(od_engine_platform PRIVATE SDL2::SDL2main SDL2::SDL2 PNG::PNG luajit-5.1 ZLIB::ZLIB od_engine_core)
# each CU has separate lib dependencies which mitigate the gains vs parallel and add cost to iterative builds
set_target_properties(od_engine_platform PROPERTIES UNITY_BUILD 0)

# gtest tests generate a lot of cruft; each test is extremely costly in compile-time
set_target_properties(od_test PROPERTIES UNITY_BUILD 0)
if(NOT ${OD_BUILD_TESTS})
	set_target_properties(od_test PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
endif()
target_link_libraries(od_test PRIVATE GTest::GTest GTest::Main od_engine_core od_engine_platform)
if(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
	target_compile_options(od_test PRIVATE -Wno-global-constructors -Wno-inline)
endif()
enable_testing()

target_link_libraries(od_client PRIVATE od_engine_core od_engine_platform)

add_subdirectory(include)
add_subdirectory(src)
