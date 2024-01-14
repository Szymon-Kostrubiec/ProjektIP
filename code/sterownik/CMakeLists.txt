add_executable(sterownik main.c ../shared_memory.c hash.c password_check.c)

target_link_libraries(sterownik crypt)

set(CMAKE_BUILD_TYPE Debug) # for gdb
