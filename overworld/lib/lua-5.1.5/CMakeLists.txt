project(lua C)
set(VERSION "5.1.5")

add_library(lua STATIC)
set_target_properties(lua PROPERTIES UNITY_BUILD 0)

add_subdirectory(src)
