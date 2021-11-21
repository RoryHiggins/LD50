foreach (od_target ${od_targets})
	message("target_include_directories(${od_target} PUBLIC .)")
	target_include_directories(${od_target} PUBLIC .)
endforeach()

add_subdirectory(od)
