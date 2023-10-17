file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cc)

include_directories(.)

add_library(luavm ${SRC_FILES})
target_link_libraries(luavm pthread rt)