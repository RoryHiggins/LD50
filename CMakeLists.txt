cmake_minimum_required(VERSION 3.19)

project(overworld LANGUAGES CXX)


# build parameters
# -------------------------------
set(OD_BUILD_LOG_DEFAULT 1)
set(OD_BUILD_TESTS_DEFAULT 0)
if(${CMAKE_BUILD_TYPE} STREQUAL DEBUG)
	set(OD_BUILD_DEBUG_LOG_DEFAULT 1)
else()
	set(OD_BUILD_DEBUG_LOG_DEFAULT 0)
endif()
if((${WIN32}) AND (${CMAKE_BUILD_TYPE} STREQUAL DEBUG))
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

if(NOT ("${EMSCRIPTEN}" EQUAL 1))
	find_package(SDL2 REQUIRED)
	find_package(PNG REQUIRED)
	find_package(ZLIB REQUIRED)
	find_package(GTest REQUIRED)
	include(GoogleTest)
endif()

set(od_libs)
set(od_binaries)
set(od_targets)


# od_engine_core
# -------------------------------
add_library(od_engine_core ${OD_BUILD_MODULE_LINKAGE})
list(APPEND od_libs od_engine_core)


# od_engine_platform
# -------------------------------
add_library(od_engine_platform ${OD_BUILD_MODULE_LINKAGE})
list(APPEND od_libs od_engine_platform)
# mingw32 must be before sdl2main
if("${MINGW}" EQUAL 1)
	target_link_libraries(od_engine_platform PRIVATE mingw32)
endif()
# these modules wont work for emcmake, must add -s USE_* args instead
if("${EMSCRIPTEN}" EQUAL 1)
else()
	target_link_libraries(od_engine_platform PRIVATE SDL2::SDL2main SDL2::SDL2 PNG::PNG luajit-5.1 ZLIB::ZLIB)
endif()
target_link_libraries(od_engine_platform PRIVATE od_engine_core)
# each CU has separate lib dependencies which mitigate the gains vs parallel and add cost to iterative builds
set_target_properties(od_engine_platform PROPERTIES UNITY_BUILD 0)


# od_client
# -------------------------------
add_executable(od_client)
list(APPEND od_binaries od_client)
target_link_libraries(od_client PRIVATE ${od_libs})


# od_test
# -------------------------------
add_executable(od_test)
list(APPEND od_binaries od_test)
# gtest tests generate a lot of cruft; each test is extremely costly in compile-time
set_target_properties(od_test PROPERTIES UNITY_BUILD 0)
# these modules wont work for emcmake, must add -s USE_* args instead
if(NOT ("${EMSCRIPTEN}" EQUAL 1))
	target_link_libraries(od_test PRIVATE GTest::GTest GTest::Main)
endif()
target_link_libraries(od_test PRIVATE ${od_libs})
if(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
	target_compile_options(od_test PRIVATE -Wno-global-constructors -Wno-inline)
endif()
if(${OD_BUILD_TESTS})
	enable_testing()	
else()
	set_target_properties(od_test PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
endif()


# multi-target properties
# -------------------------------
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
endforeach()


# gcc-specific warnings and static analysis tools
# -------------------------------
if(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
	foreach(od_target ${od_targets})
		# If building a dll, define for each module whether to import or export:
		if(${OD_BUILD_DLL} AND (${od_target} IN_LIST od_libs))
			foreach(od_target_inner ${od_targets})
				string(TOUPPER ${od_target} od_target_uppercase)

				if(${od_target_inner} STREQUAL ${od_target})
					set(od_target_linkage "__declspec(dllexport)")
				else()
					set(od_target_linkage "__declspec(dllimport)")
				endif()

				target_compile_definitions(
					${od_target_inner}
					PRIVATE
					"${od_target_uppercase}_MODULE=${od_target_linkage}")
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
			-Wplacement-new=2 -Wnon-virtual-dtor -Wnull-dereference -Woverloaded-virtual
			-Wstringop-overflow=4)

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
					-fsanitize=address,leak,undefined -fsanitize-recover=address
					-fsanitize-address-use-after-scope -fstack-protector-all)
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
	endforeach()
endif()


# emscripten-specific compiler and linker flags
if("${EMSCRIPTEN}" EQUAL 1)
	foreach(od_target ${od_targets})
		target_compile_definitions(
			${od_target}
			PRIVATE
			OD_BUILD_RENDERER=2
			OD_BUILD_EMSCRIPTEN
		)
		set(
			od_emscripten_common_params
			-O0 -g -Wall
			"SHELL:-s USE_SDL=2" "SHELL:-s DISABLE_EXCEPTION_CATCHING=1")
		target_compile_options(
			${od_target}
			PRIVATE
			${od_emscripten_common_params}
		)
		target_link_options(
			${od_target}
			PRIVATE
			${od_emscripten_common_params}
			"SHELL:-s WASM=1" "SHELL:-s ALLOW_MEMORY_GROWTH=1" "SHELL:-s NO_EXIT_RUNTIME=0"
			"SHELL:-s NO_FILESYSTEM=1"
		)
		if(${CMAKE_BUILD_TYPE} STREQUAL DEBUG)
			target_link_options(
				${od_target}
				PRIVATE
				"SHELL:-s ASSERTIONS=1" "SHELL:-s SAFE_HEAP=1" "SHELL:-s STACK_OVERFLOW_CHECK=1"
				"SHELL:-s DEMANGLE_SUPPORT=1"
			)
		endif()
	endforeach()

	# set(CMAKE_EXECUTABLE_SUFFIX ".html")
	set_target_properties(od_client PROPERTIES OUTPUT_NAME "index" SUFFIX ".html")

	set(od_emscripten_shell "${PROJECT_SOURCE_DIR}/client/src/od/client/main.html")
	target_link_options(
		od_client
		PRIVATE
		"SHELL:--shell-file ${od_emscripten_shell}"
	)
	# configure_file("${od_emscripten_shell}" "${CMAKE_BINARY_DIR}/index.html")
endif()


add_subdirectory(client)
