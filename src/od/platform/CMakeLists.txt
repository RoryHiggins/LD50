target_sources(od_engine PRIVATE file.cpp window.cpp image.cpp)
target_sources(od_test PRIVATE file_test.cpp window_test.cpp image_test.cpp)

add_subdirectory(debug_gui)
