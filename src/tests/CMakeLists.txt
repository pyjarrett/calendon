#
# Creates a unit test and adds it to the "check" target.
#
# Format:
#   unit_test(NAME test_name SOURCES a.cpp b.cpp)
#   unit_test(NAME test_name SOURCES a.cpp b.cpp LIBS GL)
# https://stackoverflow.com/questions/733475/cmake-ctest-make-test-doesnt-build-tests#736838
#
function(add_unit_test)
	set(options "")
	set(oneValueArgs NAME)
	set(multiValueArgs SOURCES LIBS)
	cmake_parse_arguments(UTEST "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	add_executable("${UTEST_NAME}" ${UTEST_SOURCES})
	target_compile_definitions(${UTEST_NAME} PRIVATE KN_TESTING=1)

	# Generate coverage data only under GCC
	if(CMAKE_BUILD_TYPE MATCHES Debug AND CMAKE_COMPILER_IS_GNUCC)
		target_link_libraries(${UTEST_NAME} pthread gcov)
		set_target_properties(${UTEST_NAME} PROPERTIES COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
		set_target_properties(${UTEST_NAME} PROPERTIES LINK_FLAGS "-fprofile-arcs -ftest-coverage")
	endif()
	target_link_libraries(${UTEST_NAME} ${UTEST_LIBS})
	add_test(NAME "${UTEST_NAME}" COMMAND "${UTEST_NAME}" WORKING_DIRECTORY ${BINARY_DIR})
	add_dependencies(testing ${UTEST_NAME})
	remove_definitions(-DKN_TESTING)
endfunction()

add_subdirectory(unit)
