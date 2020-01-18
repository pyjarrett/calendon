add_definitions(-DKN_LIBRARY=0)
add_library(sample SHARED sample.c)
target_link_libraries(sample knell-lib)

add_library(sample2 SHARED sample2.c)
target_link_libraries(sample2 knell-lib)
