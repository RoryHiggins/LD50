target_sources(od_platform PRIVATE platform.cpp timer.cpp vertex.cpp primitive.cpp file.cpp image.cpp gl.h gl.cpp texture.cpp renderer.cpp window.cpp)

# most CUs have their own lib dependencies; separate those out in unity builds to greatly improve build time
set_target_properties(od_platform PROPERTIES UNITY_BUILD_MODE BATCH)
set_source_files_properties(timer.cpp vertex.cpp primitive.cpp file.cpp PROPERTIES UNITY_GROUP "od_platform_core")
set_source_files_properties(platform.cpp UNITY_GROUP "od_platform_os")
set_source_files_properties(gl.h gl.cpp texture.cpp renderer.cpp window.cpp UNITY_GROUP "od_platform_sdl_gl")
set_source_files_properties(image.cpp UNITY_GROUP "od_platform_libpng")
