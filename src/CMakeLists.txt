# The knell driver should depend on all demos to ensure that they are built
# before running the driver to prevent problems where you're trying to
# iterate but your shared lib isn't being built.
set(KN_ALL_DEMOS "" CACHE INTERNAL "KN_ALL_DEMOS")

# Use "feature test macro" to change POSIX support to allow use of
# CLOCK_MONOTONIC.
# http://man7.org/linux/man-pages/man7/feature_test_macros.7.html
add_definitions(-D_POSIX_C_SOURCE=199309L)

include_directories(SYSTEM .)
add_subdirectory(knell)
add_subdirectory(demos)
add_subdirectory(driver)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT knell-driver)

add_dependencies(knell-driver knell ${KN_ALL_DEMOS})
message("Demos are ${KN_ALL_DEMOS}")

###############################################################################
# Testing
###############################################################################
enable_testing()

#
# A custom target is always stale and will be "rebuilt".  `testing` is the
# custom target for force DLL copy on Windows.
#
add_custom_target(testing)

#
# Provides a command to run all tests, since CLion can't do that on it's own.
# https://stackoverflow.com/questions/733475/cmake-ctest-make-test-doesnt-build-tests#736838
#
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} -V -C Debug)

#
# check-iterate is the target to use when iterating when using tests,
# such as TDD's red->green pattern.
#
add_custom_target(check-iterate COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failed --rerun-failed)

add_dependencies(check testing)
add_dependencies(check-iterate testing)
add_dependencies(testing knell-testable)

add_subdirectory(tests)

###############################################################################
# Windows DLL copies
###############################################################################
if (WIN32)
	add_custom_target(windows-dll-copy)

	add_custom_command(TARGET windows-dll-copy POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${VENDOR_SDL2_ROOT}/lib/x64/SDL2.dll
		${BINARY_DIR}
		)

	add_custom_command(TARGET windows-dll-copy POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${VENDOR_GLEW_ROOT}/bin/Release/x64/glew32.dll
		${BINARY_DIR}
		)

	add_custom_command(TARGET windows-dll-copy POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${VENDOR_ZLIB_ROOT}/Release/x64/zlibwapi.dll
		${BINARY_DIR}
		)

    add_custom_command(TARGET windows-dll-copy POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
		$<TARGET_FILE:knell>
		${BINARY_DIR}
		COMMENT "Copying knell shared lib to driver directory")

	add_custom_command(TARGET windows-dll-copy POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		$<TARGET_FILE:knell-testable>
		${BINARY_DIR}
		COMMENT "Copying knell-testable shared lib to driver directory")

	# Only create dumps in Debug.
	# Uses a x64 version of the dll.
	# TODO: Provide an x86 and an ARM version.
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		add_custom_command(TARGET windows-dll-copy POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${VENDOR_WINDOWS_SDK_ROOT}/dbghelp.dll
			${BINARY_DIR}
			)
	endif()

	add_dependencies(testing windows-dll-copy)
	add_dependencies(knell-driver windows-dll-copy)

endif (WIN32)
