file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cc)

include_directories(.)

add_library(luavm chunk.cc)
add_executable(luac luac.cc)
target_link_libraries(luavm pthread)
target_link_libraries(luac luavm)