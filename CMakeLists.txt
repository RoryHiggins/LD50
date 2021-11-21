cmake_minimum_required(VERSION 3.16)
project(overworld LANGUAGES CXX)

set(OD_BUILD_DEBUG_DEFAULT 1)
if(CMAKE_BUILD_TYPE STREQUAL "RELEASE")
	set(OD_BUILD_DEBUG_DEFAULT 0)
else()
endif()
set(OD_BUILD_DEBUG ${OD_BUILD_DEBUG_DEFAULT} CACHE BOOL "Build for debugging")
set(OD_BUILD_LOG ${OD_BUILD_DEBUG_DEFAULT} CACHE BOOL "Build with logging")
set(OD_BUILD_TESTS ${OD_BUILD_DEBUG_DEFAULT} CACHE BOOL "Build with tests")

# od_core
# ----------
add_library(od_core)
set_target_properties(od_core PROPERTIES UNITY_BUILD 1)

# od_platform
# ----------
add_library(od_platform)
# mingw32 must be before sdl2main
if("${MINGW}" EQUAL 1)
	target_link_libraries(od_platform PRIVATE mingw32)
endif()
if(NOT ("${EMSCRIPTEN}" EQUAL 1))
	find_package(SDL2 REQUIRED)
	find_package(OpenGL REQUIRED)
	find_package(GLEW REQUIRED)
	find_package(PNG REQUIRED)
	find_package(ZLIB REQUIRED)

	find_library(LUAJIT_LIBRARY
		NAMES luajit luajit-5.1
		HINTS ENV LUA_DIR
		PATH_SUFFIXES lib)

	find_path(LUAJIT_INCLUDE_DIR luajit.h
	  HINTS ENV LUA_DIR
	  PATH_SUFFIXES include/luajit include/luajit-2.1 include/lua51 include/lua5.1 include/lua-5.1 include)

	if ("${SDL2_LIBRARIES}" STREQUAL "")
		set(OD_SDL2_LIBRARIES SDL2::SDL2main SDL2::SDL2)
	else()
		set(OD_SDL2_LIBRARIES "${SDL2_LIBRARIES}")
	endif()

	set(OD_GL_LIBRARIES OpenGL::GL OpenGL::GLU GLEW::glew)

	if (NOT ("${LUAJIT_LIBRARY}" STREQUAL "") AND NOT ("${LUAJIT_INCLUDE_DIR}" STREQUAL ""))
		set(OD_LUA_LIBRARIES "${LUAJIT_LIBRARY}")
		include_directories(LUAJIT_INCLUDE_DIR)
		message("-- Found LuaJIT: ${LUAJIT_LIBRARY}")
	else()
		find_package(Lua51 REQUIRED)
		set(OD_LUA_LIBRARIES "${LUA_LIBRARY}")
	endif()

	target_link_libraries(od_platform PRIVATE ${OD_SDL2_LIBRARIES} ${OD_GL_LIBRARIES} ${OD_LUA_LIBRARIES} ZLIB::ZLIB PNG::PNG)
endif()
target_link_libraries(od_platform PRIVATE m od_core)
# each CU has separate lib dependencies which mitigate the gains vs parallel and add cost to iterative builds
set_target_properties(od_platform PROPERTIES UNITY_BUILD 0)

# od_engine
# ----------
add_library(od_engine)
set_target_properties(od_engine PROPERTIES UNITY_BUILD 1)
target_link_libraries(od_engine PRIVATE m od_core od_platform)

# od_test
# ----------
add_library(od_test)
set_target_properties(od_test PROPERTIES UNITY_BUILD 1 EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
target_link_libraries(od_test PRIVATE m od_core od_platform od_engine)

# od_client
# ----------
add_executable(od_client)
set_target_properties(od_client PROPERTIES UNITY_BUILD 1)
target_link_libraries(od_client PRIVATE m od_core od_platform)
if (${OD_BUILD_TESTS})
	set_target_properties(od_test PROPERTIES EXCLUDE_FROM_ALL 0 EXCLUDE_FROM_DEFAULT_BUILD 0)
	target_link_libraries(od_client PRIVATE od_test)
endif()

# shared parameters
# ----------
set(od_libs od_core od_platform od_engine od_test)
set(od_binaries od_client)
set(od_targets ${od_libs} ${od_binaries})

foreach(od_target ${od_targets})
	set_target_properties(
		${od_target} PROPERTIES
		CXX_STANDARD 11 CXX_STANDARD_REQUIRED 1 CXX_EXTENSIONS 0 LINKER_LANGUAGE CXX
		LINK_DEPENDS_NO_SHARED 1)

	# only set the UNITY_BUILD property if not set (allow explicit enabling/disabling per cu)
	get_property(od_unity_build_set TARGET ${od_target} PROPERTY UNITY_BUILD SET)
	if (NOT (${od_unity_build_set}))
		set_target_properties(${od_target} PROPERTIES UNITY_BUILD 1)
	endif()

	target_compile_definitions(
		${od_target} PUBLIC
		OD_BUILD_LOG=${OD_BUILD_LOG}
		OD_BUILD_DEBUG=${OD_BUILD_DEBUG}
		OD_BUILD_TESTS=${OD_BUILD_TESTS})
endforeach()

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

# optional gcc-specific warnings and static analysis tools
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	foreach(od_target ${od_targets})
		target_link_libraries(${od_target} PRIVATE m)
		target_compile_options(
			${od_target} PUBLIC
			-Wall -Wextra -pedantic -Werror=vla -Winvalid-pch -fno-exceptions -fno-rtti
			-Wold-style-cast -Wregister -Wshadow -Wvirtual-inheritance -Wnamespaces
			-Wzero-as-null-pointer-constant -Wpacked -Wredundant-decls -Wvla
			-Wlarger-than=16777216 -Walloc-size-larger-than=16777216 -Wstringop-overflow=4
			-Walloc-zero -Warray-bounds=2 -Wcast-align=strict -Wconversion -Wundef
			-Wformat=2 -Wformat-overflow=2 -Wformat-signedness -Wformat-truncation=2
			-Wdisabled-optimization -Wfloat-conversion -Wlogical-op -Winline -Wmultichar
			-Wplacement-new=2 -Wnon-virtual-dtor -Wnull-dereference -Woverloaded-virtual)

		if("${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
			target_compile_options(${od_target} PUBLIC -O0 -ggdb3)
			target_compile_definitions(${od_target} PRIVATE _FORTIFY_SOURCE=2)

			if(CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 10.0.0)
				target_compile_options(${od_target} PUBLIC -fanalyzer)
			endif()

			if(NOT ("${MINGW}" EQUAL 1))
				set(
					od_sanitizer_options
					-fsanitize=address,leak,undefined -fsanitize-recover=address
					-fsanitize-address-use-after-scope -fstack-protector-all
				)
				target_compile_options(${od_target} PUBLIC ${od_sanitizer_options})
				target_link_options(${od_target} PUBLIC ${od_sanitizer_options})
			endif()
		endif()

		if("${CMAKE_BUILD_TYPE}" STREQUAL "RELEASE")
			target_compile_options(${od_target} PUBLIC -Os -s -ffast-math -flto)
			target_link_options(${od_target} PUBLIC -Os -s -flto -fwhole-program)

			if("${MINGW}" EQUAL 1)
				target_link_options(${od_target} PUBLIC -mwindows)
			endif()

			if("${BUILD_SHARED_LIBS}" EQUAL 1)
				set(od_static_link_options -static-libstdc++ -static-libgcc)
				target_compile_options(${od_target} PUBLIC ${od_static_link_options})
				target_link_options(${od_target} PUBLIC ${od_static_link_options})
			endif()
		endif()
	endforeach()
endif()

add_subdirectory(overworld)

# emscripten-specific compiler and linker flags
if("${EMSCRIPTEN}" EQUAL 1)
	foreach(od_target ${od_targets})
		target_compile_definitions(
			${od_target}
			PRIVATE
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

	set_target_properties(od_client PROPERTIES OUTPUT_NAME "index" SUFFIX ".html")

	set(od_emscripten_shell "${PROJECT_SOURCE_DIR}/overworld/src/od/client/main.html")
	target_link_options(
		od_client
		PRIVATE
		"SHELL:--shell-file ${od_emscripten_shell}"
	)

	target_link_libraries(od_platform PRIVATE zlibstatic png_static)
	target_link_libraries(od_engine PRIVATE lua)
endif()
