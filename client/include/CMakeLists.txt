foreach (od_target ${od_targets})
	target_include_directories(${od_target} PUBLIC .)
endforeach()

add_subdirectory(od)
