cmake_minimum_required(VERSION 3.20)

project(projekt C)

set(CMAKE_C_STANDARD 23)
add_compile_options(-Wall -Wextra -pedantic)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

add_subdirectory(krancowki)
add_subdirectory(przyciski)
add_subdirectory(silnik)
add_subdirectory(sterownik)
add_subdirectory(szyba)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

set(CMAKE_BUILD_TYPE Debug) # for gdb

