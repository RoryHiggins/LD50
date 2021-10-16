target_sources(od_test PUBLIC test.h test.hpp)
target_sources(od_test PRIVATE test.cpp)


add_subdirectory(core)
add_subdirectory(platform)
