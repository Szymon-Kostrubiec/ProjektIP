add_executable(silnik main.c ../attach_to_shared.c ../shared_memory.c)

set(CMAKE_BUILD_TYPE Debug) # for gdb
