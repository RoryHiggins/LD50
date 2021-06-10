foreach (od_engine_lib ${od_engine_libs})
	target_include_directories(${od_engine_lib} PUBLIC .)
endforeach()

add_subdirectory(od)
