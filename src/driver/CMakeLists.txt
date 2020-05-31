file(GLOB_RECURSE CALENDON_DRIVER_SRCS *.c)
file(GLOB_RECURSE CALENDON_DRIVER_HEADERS *.h)

remove_definitions(-DCN_LIBRARY)
add_definitions(-DCN_LIBRARY=0)

add_executable(calendon-driver ${CALENDON_DRIVER_SRCS} ${CALENDON_DRIVER_HEADERS})
target_link_libraries(calendon-driver calendon)
