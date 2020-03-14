file(GLOB_RECURSE KNELL_DRIVER_SRCS *.c)
file(GLOB_RECURSE KNELL_DRIVER_HEADERS *.h)

remove_definitions(-DKN_LIBRARY)
add_definitions(-DKN_LIBRARY=0)

add_executable(knell-driver ${KNELL_DRIVER_SRCS} ${KNELL_DRIVER_HEADERS})
target_link_libraries(knell-driver knell)
