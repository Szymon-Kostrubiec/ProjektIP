add_executable(szyba main.c ../attach_to_shared.c ../shared_memory.c)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

target_link_libraries(szyba ${CURSES_LIBRARIES})
