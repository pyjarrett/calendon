add_executable(test-utf8 test-utf8.c)
target_link_libraries(test-utf8 knell-lib)
add_test(NAME test-utf8 COMMAND test-utf8 test-utf8.c)
