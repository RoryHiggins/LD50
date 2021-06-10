foreach (od_target ${od_targets})
	target_include_directories(${od_target} PRIVATE .)
endforeach()

add_subdirectory(od)
