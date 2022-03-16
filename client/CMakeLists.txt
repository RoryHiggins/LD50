cmake_minimum_required(VERSION 3.16)
project(overworld LANGUAGES CXX)

set(OD_BUILD_LOGS_DEFAULT 0)
set(OD_BUILD_DEBUG_DEFAULT 0)
set(OD_BUILD_TESTS_DEFAULT 0)
set(OD_BUILD_PROFILE_DEFAULT 0)

set(OD_BUILD_LUAJIT_DEFAULT 0)
set(OD_BUILD_EMSCRIPTEN_DEFAULT 0)
if("${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
	set(OD_BUILD_DEBUG_DEFAULT 1)
	set(OD_BUILD_LOGS_DEFAULT 1)
	set(OD_BUILD_TESTS_DEFAULT 1)
endif()
if("${EMSCRIPTEN}" EQUAL 1)
	set(OD_BUILD_EMSCRIPTEN_DEFAULT 1)
else()
	set(OD_BUILD_LUAJIT_DEFAULT 1)
endif()


set(OD_BUILD_LOGS ${OD_BUILD_LOGS_DEFAULT} CACHE BOOL
	"Build with logging enabled")
set(OD_BUILD_DEBUG ${OD_BUILD_DEBUG_DEFAULT} CACHE BOOL
	"Build with debugging runtime checks enabled")
set(OD_BUILD_TESTS ${OD_BUILD_TESTS_DEFAULT} CACHE BOOL
	"Build with tests enabled")
set(OD_BUILD_PROFILE ${OD_BUILD_PROFILE_DEFAULT} CACHE BOOL
	"Build with profiling enabled")
set(OD_BUILD_LUAJIT ${OD_BUILD_LUAJIT_DEFAULT} CACHE BOOL
	"Build with luajit instead of lua")
set(OD_BUILD_EMSCRIPTEN ${OD_BUILD_EMSCRIPTEN_DEFAULT} CACHE BOOL
	"Build with emscripten support enabled")

if((${OD_BUILD_PROFILE}) AND ("${BUILD_SHARED_LIBS}" EQUAL 1))
	message(FATAL_ERROR "profile build must be statically linked")
endif()
if(NOT ("${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG") AND ("${OD_BUILD_DEBUG}" EQUAL 1))
	message(WARNING "debug checks with non-debug build may not be intentional")
endif()
if(("${CMAKE_BUILD_TYPE}" STREQUAL "RELEASE") AND ("${OD_BUILD_TESTS}" EQUAL 1))
	message(WARNING "release build with tests may not be intentional")
endif()
if(("${OD_BUILD_EMSCRIPTEN}" EQUAL 1) AND NOT ("${EMSCRIPTEN}" EQUAL 1))
	message(WARNING "emscripten support for non-emscripten platform may not be intentional")
endif()
if(("${OD_BUILD_EMSCRIPTEN}" EQUAL 1) AND ("${OD_BUILD_LUAJIT}" EQUAL 1))
	message(WARNING "emscripten with luajit will likely fail, and is unlikely to ever be supported")
endif()

message("OD_BUILD_LOGS=${OD_BUILD_LOGS}")
message("OD_BUILD_DEBUG=${OD_BUILD_DEBUG}")
message("OD_BUILD_TESTS=${OD_BUILD_TESTS}")
message("OD_BUILD_PROFILE=${OD_BUILD_PROFILE}")

message("OD_BUILD_EMSCRIPTEN=${OD_BUILD_EMSCRIPTEN}")
message("OD_BUILD_LUAJIT=${OD_BUILD_LUAJIT}")
message("BUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}")
message("CMAKE_UNITY_BUILD=${CMAKE_UNITY_BUILD}")

# od_core
add_library(od_core)

# od_platform
add_library(od_platform)
target_link_libraries(od_platform PRIVATE m od_core)
# most CUs have their own lib dependencies; separating those out in unity builds can greatly improve build time
set_target_properties(od_platform PROPERTIES UNITY_BUILD 0)

# od_engine
add_library(od_engine)
target_link_libraries(od_engine PRIVATE m od_core od_platform)

# od_test
add_library(od_test)
target_link_libraries(od_test PRIVATE m od_core od_platform od_engine)
if (NOT (${OD_BUILD_TESTS}))
	set_target_properties(od_test PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
endif()

# od_client
add_executable(od_client)
target_link_libraries(od_client PRIVATE m od_core od_platform od_engine)
if (${OD_BUILD_TESTS})
	target_link_libraries(od_client PRIVATE od_test)
endif()


set(od_libs od_core od_platform od_engine od_test)
set(od_binaries od_client)
set(od_targets ${od_libs} ${od_binaries})

# shared parameters
foreach(od_target ${od_targets})
	set_target_properties(
		${od_target} PROPERTIES
		CXX_STANDARD 11 CXX_STANDARD_REQUIRED 1 CXX_EXTENSIONS 0 LINKER_LANGUAGE CXX
		LINK_DEPENDS_NO_SHARED 1)

	target_compile_definitions(
		${od_target} PUBLIC
		OD_BUILD_LOGS=${OD_BUILD_LOGS}
		OD_BUILD_DEBUG=${OD_BUILD_DEBUG}
		OD_BUILD_TESTS=${OD_BUILD_TESTS}
		OD_BUILD_PROFILE=${OD_BUILD_PROFILE}
		OD_BUILD_LUAJIT=${OD_BUILD_LUAJIT}
		OD_BUILD_EMSCRIPTEN=${OD_BUILD_EMSCRIPTEN})
endforeach()

# parameters for non-emscripten targets
if(NOT("${EMSCRIPTEN}" EQUAL 1))
	# native (non-emscripten) parameters
	find_package(SDL2 REQUIRED)
	find_package(OpenGL REQUIRED)
	find_package(GLEW REQUIRED)
	find_package(PNG REQUIRED)
	find_package(ZLIB REQUIRED)

	find_package(Backtrace)

	find_library(OD_SDL2_MIXER_LIBRARY NAMES SDL2_mixer PATH_SUFFIXES lib bin REQUIRED)
	find_path(OD_SDL2_MIXER_INCLUDE_DIR SDL2/SDL_mixer.h PATH_SUFFIXES include/SDL2 REQUIRED)

	find_library(LUAJIT_LIBRARY NAMES luajit luajit-5.1 HINTS ENV LUA_DIR PATH_SUFFIXES lib bin)
	find_path(LUAJIT_INCLUDE_DIR luajit.h HINTS ENV LUA_DIR PATH_SUFFIXES include/luajit include/luajit-2.1)

	if ("${SDL2_LIBRARIES}" STREQUAL "")
		set(OD_SDL2_LIBRARIES SDL2::SDL2main SDL2::SDL2 ${OD_SDL2_MIXER_LIBRARY})
	else()
		set(OD_SDL2_LIBRARIES ${SDL2_LIBRARIES} ${OD_SDL2_MIXER_LIBRARY})
	endif()

	if (("${OD_SDL2_MIXER_LIBRARY}" STREQUAL "OD_SDL2_MIXER_LIBRARY-NOTFOUND")
		OR ("${OD_SDL2_MIXER_INCLUDE_DIR}" STREQUAL "OD_SDL2_MIXER_INCLUDE_DIR-NOTFOUND"))
		message(WARNING "Failed to find SDL2_mixer, may fail to link")
	else()
		message("-- Found SDL2_mixer: ${OD_SDL2_MIXER_LIBRARY}, ${OD_SDL2_MIXER_INCLUDE_DIR}")
		set(OD_SDL2_MIXER_LIBRARY)
		set(OD_SDL2_MIXER_INCLUDE_DIR)
	endif()

	# mingw32 must be linked before sdl2main
	if("${MINGW}" EQUAL 1)
		list(PREPEND OD_SDL2_LIBRARIES mingw32)
	endif()

	if (${OD_BUILD_LUAJIT} AND NOT ("${LUAJIT_LIBRARY}" STREQUAL "") AND NOT ("${LUAJIT_INCLUDE_DIR}" STREQUAL ""))
		set(OD_LUA_LIBRARY "${LUAJIT_LIBRARY}")
		set(OD_LUA_INCLUDE_DIR "${LUAJIT_INCLUDE_DIR}")
		message("-- Found LuaJIT: ${LUAJIT_LIBRARY}")
	else()
		find_package(Lua51 REQUIRED)
		set(OD_LUA_LIBRARY "${LUA_LIBRARY}")
		set(OD_LUA_INCLUDE_DIR "${LUA_INCLUDE_DIR}")
	endif()

	target_link_libraries(od_platform PRIVATE ${OD_SDL2_LIBRARIES} OpenGL::GL OpenGL::GLU GLEW::glew ZLIB::ZLIB PNG::PNG)
	target_compile_definitions(od_platform PRIVATE GLEW_STATIC)

	target_link_libraries(od_engine PRIVATE ${OD_LUA_LIBRARY})
	target_include_directories(od_engine PRIVATE ${OD_LUA_INCLUDE_DIR})

	target_link_libraries(od_engine PRIVATE ${OD_SDL2_MIXER_LIBRARY})
	target_include_directories(od_engine PRIVATE ${OD_SDL2_MIXER_INCLUDE_DIR})
# parameters for emscripten target
else()
	foreach(od_target ${od_targets})
		set(
			od_emscripten_common_params
			-Wall "SHELL:-s USE_SDL=2" "SHELL:-s USE_SDL_MIXER=2" "SHELL:-s DISABLE_EXCEPTION_CATCHING=1"
		)
		set(
			od_emscripten_debug_params
			-O0 -g2 -gsource-map "SHELL:--source-map-base http://localhost:8000/"
			"SHELL:-s ASSERTIONS=1" "SHELL:-s SAFE_HEAP=1" "SHELL:-s STACK_OVERFLOW_CHECK=1"
			"SHELL:-s DEMANGLE_SUPPORT=1" "SHELL:-s ASYNCIFY=1"
		)
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
			"SHELL:-s NO_FILESYSTEM=0" "SHELL:--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/../examples/@examples/"
		)
		if("${CMAKE_BUILD_TYPE}" STREQUAL DEBUG)
			target_link_options(
				${od_target}
				PRIVATE
				${od_emscripten_debug_params}
			)
			target_link_options(
				${od_target}
				PRIVATE
				${od_emscripten_debug_params}
			)
		endif()
	endforeach()

	set_target_properties(od_client PROPERTIES OUTPUT_NAME "index" SUFFIX ".html")

	set(od_emscripten_shell "${PROJECT_SOURCE_DIR}/src/od/client/main.html")
	target_link_options(
		od_client
		PRIVATE
		"SHELL:--shell-file ${od_emscripten_shell}"
	)

	target_link_libraries(od_platform PRIVATE zlibstatic png_static)
	target_link_libraries(od_engine PRIVATE lua)
endif()

# decorate module api macros with dllimport/dllexport if building dlls for windows
if(("${BUILD_SHARED_LIBS}" EQUAL 1) AND ("${WIN32}" EQUAL 1))
	foreach(od_target ${od_target})
		get_target_property(od_lib_dependencies ${od_target} LINK_LIBRARIES)

		foreach(od_target_compare ${od_libs})
			string(TOUPPER ${od_target_compare} od_target_compare_uppercase)
			set(od_target_compare_macro "${od_target_compare_uppercase}_MODULE") # e.g. OD_CORE_MODULE

			if ("${od_target}" STREQUAL "${od_target_compare}")
				target_compile_definitions(${od_target} PRIVATE "${od_target_compare_macro}=__declspec(dllexport)")
			elseif("${od_target_compare}" IN_LIST "${od_lib_dependencies}")
				target_compile_definitions(${od_target} PRIVATE "${od_target_compare_macro}=__declspec(dllimport)")
			endif()
		endforeach()
	endforeach()
endif()

# optional gcc-specific parameters, if using gcc
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	foreach(od_target ${od_targets})
		set(od_warning_options
			-Wall -Wextra -pedantic -Werror=vla -Winvalid-pch -fno-exceptions -fno-rtti
			-Wold-style-cast -Wregister -Wshadow -Wvirtual-inheritance -Wnamespaces
			-Wzero-as-null-pointer-constant -Wpacked -Wredundant-decls -Wvla
			-Wlarger-than=16777216 -Walloc-size-larger-than=16777216 -Wstringop-overflow=4
			-Walloc-zero -Warray-bounds=2 -Wcast-align=strict -Wconversion -Wundef
			-Wformat=2 -Wformat-overflow=2 -Wformat-signedness
			-Wdisabled-optimization -Wfloat-conversion -Wlogical-op -Wmultichar
			-Wplacement-new=2 -Wnon-virtual-dtor -Woverloaded-virtual
			-Wno-stringop-truncation -Wno-missing-profile)

		target_link_libraries(${od_target} PRIVATE m)
		target_compile_options(${od_target} PUBLIC ${od_warning_options} ${od_profile_generation_options})
		target_link_options(${od_target} PUBLIC ${od_warning_options} ${od_profile_generation_options})

		if("${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
			set(od_debug_options -O0 -ggdb3 -fno-omit-frame-pointer -fno-inline)
			target_compile_options(${od_target} PUBLIC ${od_debug_options})
			target_link_options(${od_target} PUBLIC ${od_debug_options})

			if(CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 10.0.0)
				target_compile_options(${od_target} PUBLIC -fanalyzer)
			endif()

			if("${WIN32}" EQUAL 1)
				target_link_libraries(${od_target} PRIVATE imagehlp)
				target_compile_definitions(${od_target} PRIVATE OD_BUILD_DBGHELP=1)
			else()
				set(od_sanitizer_options -fsanitize=address,leak,undefined -fsanitize-recover=address
					-fsanitize-address-use-after-scope -fstack-protector-all)
				target_compile_options(${od_target} PUBLIC ${od_sanitizer_options})
				target_link_options(${od_target} PUBLIC ${od_sanitizer_options})

				if (Backtrace_FOUND)
					target_link_libraries(${od_target} PRIVATE backtrace)
					target_compile_definitions(${od_target} PRIVATE OD_BUILD_LIBBACKTRACE=1)
				endif()
			endif()
		endif()

		if("${CMAKE_BUILD_TYPE}" STREQUAL "RELEASE")
			target_compile_options(${od_target} PUBLIC -ffast-math -flto -Os)
			target_link_options(${od_target} PUBLIC -flto -fwhole-program -Os)

			if(NOT(${OD_BUILD_PROFILE}))
				target_compile_options(${od_target} PUBLIC -s)
				target_link_options(${od_target} PUBLIC -s)
			endif()

			if("${MINGW}" EQUAL 1)
				target_link_options(${od_target} PUBLIC -mwindows)
			endif()
		endif()

		if (${OD_BUILD_COVERAGE})
			set(od_coverage_options -fprofile-arcs -ftest-coverage)
			target_compile_options(${od_target} PUBLIC ${od_coverage_options})
			target_link_options(${od_target} PUBLIC ${od_coverage_options})
		endif()

		if(${OD_BUILD_PROFILE})
			target_compile_options(${od_target} PUBLIC -g -pg)
			target_link_options(${od_target} PUBLIC -g -pg)
		endif()

	endforeach()
endif()

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(include)
