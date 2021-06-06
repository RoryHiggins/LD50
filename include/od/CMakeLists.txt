add_subdirectory(core)
add_subdirectory(platform)

target_sources(od_engine PUBLIC core.h)
target_precompile_headers(od_engine PRIVATE core.h)
target_precompile_headers(od_test PRIVATE test.hpp)
