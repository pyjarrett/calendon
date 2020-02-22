cmake_minimum_required(VERSION 3.10)
project(knell)

# Using basic C99 for this engine until reaching technical and complexity
# limits with the language.  C11 was also considered, but C99 should be
# readily supported by the most number of compilers.

# Why not C++ (or Rust, or Nim, or Go)?  Using C99 aims to use a low level
# language, which readily maps to the generated assembly and generates consistent
# simple binary symbols. C99 limits overdesign and simplifies design choices by
# providing a subset of functionality of other languages, and hopes to boost
# productivity by providing clear compile errors.

# As the lingua franca of programming, bindings to other languages should able
# to be auto-generated or readily written by hand when appropriate.
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# For sourcetrail and other tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Debug: Include debugging symbols and most warnings.
# Release: No debugging symbol and optimized for release.
# Coverage: Flags for generated code coverage under GCC.
# Future: Release mode which omits deprecated functionality.
#
# Future configurations:
# Profile: Add -pg flag for GCC to use with gprof
# RelWIthDebugInfo: Optimized build with debug symbols included.
if (UNIX)
    set(CMAKE_CONFIGURATION_TYPES Debug;Release;Coverage;Future)
elseif (WIN32)
    # Coverage is not supported on Windows.
    set(CMAKE_CONFIGURATION_TYPES Debug;Release;Future)
endif()
if(NOT CMAKE_BUILD_TYPE)
    # Visual studio might not correctly set CMAKEBUILD_TYPE
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build Type: (Debug, Release, Future)." FORCE)
endif()

# Provide standardized output locations.  This particularly
# helps when dealing with additional directories such as for
# assets to ensure they are placed correctly relative to
# executables.
set(BINARY_DIR "${PROJECT_BINARY_DIR}")
set(LIBRARY_DIR "${PROJECT_BINARY_DIR}/lib")
set(ARCHIVE_DIR "${PROJECT_BINARY_DIR}/lib")
set(ASSET_DIR "${PROJECT_BINARY_DIR}/assets")
set(CONFIG_DIR "${PROJECT_BINARY_DIR}/configs")
set(RESOURCE_DIR "${PROJECT_BINARY_DIR}/resources")

message(STATUS "BINARY_DIR:   ${BINARY_DIR}")
message(STATUS "LIBRARY_DIR:  ${LIBRARY_DIR}")
message(STATUS "ARCHIVE_DIR:  ${ARCHIVE_DIR}")
message(STATUS "ASSET_DIR:    ${ASSET_DIR}")
message(STATUS "CONFIG_DIR:   ${CONFIG_DIR}")
message(STATUS "RESOURCE_DIR: ${RESOURCE_DIR}")

# KNELL_LIBS is the list of all libraries to link with the Knell lib itself.
# Client should be required to do minimum linking themselves.
set(KNELL_LIBS "")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	# Set the KN_DEBUG symbol to 1 at the top level within the build system
	# so no header needs to be included (or possibly forgotten) to get this
	# value.
	if(CMAKE_C_COMPILER_ID MATCHES "Clang")
		add_definitions(-DKN_DEBUG=1)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

		add_definitions(-DKN_ENABLE_CORE_DUMPS=1)
		# Enabling this prevents core dumps from working.
		#set(KNELL_COMMON_C_FLAGS "${KNELL_COMMON_C_FLAGS} -fsanitize=address -fsanitize=undefined")
	elseif(CMAKE_C_COMPILER_ID MATCHES "GNU")
		add_definitions(-DKN_DEBUG=1)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

		add_definitions(-DKN_ENABLE_CORE_DUMPS=1)
		# Enabling this prevents core dumps from working.
		#set(KNELL_COMMON_C_FLAGS "${KNELL_COMMON_C_FLAGS} -fsanitize=address -fsanitize=undefined")
    elseif(CMAKE_C_COMPILER_ID MATCHES "MSVC")
        # Allow symbol resolution when querying for symbols.
        add_definitions(/DPSAPI_VERSION=1)
        add_definitions(/DKN_DEBUG=1)
		set(KNELL_LIBS ${KNELL_LIBS} psapi)

		# Adds MiniDump reporting.
		add_definitions(/DKN_ENABLE_CORE_DUMPS=1)
		set(KNELL_LIBS ${KNELL_LIBS} dbghelp)
	endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		message(FATAL_ERROR "Coverage is not available in Clang.")
	elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
		set(KNELL_LIBS ${KNELL_LIBS} gcov)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
		set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
	endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Future")
	# Omit deprecated functionality, causing build breakages.
	add_definitions(-DKN_DEPRECATION_BREAK=0)
	add_definitions(-DKN_DEPRECATION_OMIT=1)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
	# Make a breakpoint on deprecated behavior.
	add_definitions(-DKN_DEPRECATION_BREAK=1)
	add_definitions(-DKN_DEPRECATION_OMIT=0)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
	# Release may need to ship with deprecated features to make a ship date.
	add_definitions(-DKN_DEPRECATION_BREAK=0)
	add_definitions(-DKN_DEPRECATION_OMIT=0)
endif()

# Bring in extra warnings to prevent errors.
# https://caiorss.github.io/C-Cpp-Notes/compiler-flags-options.html
set(KNELL_COMMON_C_FLAGS "${KNELL_COMMON_C_FLAGS} -Wall -Wextra")
#set(KNELL_COMMON_C_FLAGS "${KNELL_COMMON_C_FLAGS} -Wpedantic")
set(KNELL_COMMON_C_FLAGS "${KNELL_COMMON_C_FLAGS} -Wshadow -Wdouble-promotion -Wundef -Wconversion -fno-common")
set(KNELL_COMMON_C_FLAGS "${KNELL_COMMON_C_FLAGS} -fvisibility=hidden")

# Temporary workaround until strcpy, strlen, usages get replaced.
if (WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS=1)
    set(KNELL_COMMON_C_FLAGS "${KNELL_COMMON_C_FLAGS} /W4")
	set(KNELL_COMMON_C_FLAGS "${KNELL_COMMON_C_FLAGS} /MP")      # Enable parallel build
endif (WIN32)

# Eliminate extra dead code
#set(KNELL_COMMON_C_FLAGS "-ffunction-sections -fdata-sections")

# https://interrupt.memfault.com/blog/best-and-worst-gcc-clang-compiler-flags
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${KNELL_COMMON_C_FLAGS} -Weverything")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${KNELL_COMMON_C_FLAGS}")
endif()

# Introduce the vendorize(platform library semantic_version) syntax for
# providing libraries from known local sources (vendoring).
include(vendor/common/vendorize-cmake-0.1/vendorize.cmake)

# libspng provides extremely simplified PNG loading.
vendorize(common libspng 0.5.0)
include_directories(SYSTEM ${VENDOR_LIBSPNG_ROOT})

if (WIN32)
	vendorize(windows sdl2 2.0.10)
	include_directories(SYSTEM "${VENDOR_SDL2_ROOT}/include")
	link_directories(SYSTEM "${VENDOR_SDL2_ROOT}/lib/x64")

	# glew (GL Extension Wranger) is used to load OpenGL extension functions so we
	# don't need to manually load function pointers.
	vendorize(windows glew 2.1.0)
	include_directories(SYSTEM "${VENDOR_GLEW_ROOT}/include")
	link_directories(SYSTEM "${VENDOR_GLEW_ROOT}/lib/Release/x64")
	set(KNELL_LIBS
		glew32.lib
		${KNELL_LIBS})

	# zlib is required by libspng
	vendorize(windows zlib 1.2.11)
	include_directories(SYSTEM ${VENDOR_ZLIB_ROOT}/include)
	link_directories(${VENDOR_ZLIB_ROOT}/Release/x64)
	set(KNELL_LIBS ${KNELL_LIBS} zlibwapi.dll)

	# dbghelp.dll isn't provided normally, it's part of the Debugging tools in
	# the SDK, so an x64 version is included for creating minidumps.
	vendorize(windows windows_sdk 10)
endif (WIN32)

# The knell driver should depend on all demos to ensure that they are built
# before running the driver to prevent problems where you're trying to
# iterate but your shared lib isn't being built.
set(KN_ALL_DEMOS "" CACHE INTERNAL "KN_ALL_DEMOS")
# Describe a demo and link it against knell-lib, and add it to the demo list
# so that knell will depend on it and force it to be build when knell is built.
#
# Format:
#   add_demo(NAME test_name SOURCES a.cpp b.cpp)
#   add_demo(NAME test_name SOURCES a.cpp b.cpp LIBS GL)
#
function(add_demo)
	set(options "")
	set(oneValueArgs NAME)
	set(multiValueArgs SOURCES LIBS)
	cmake_parse_arguments(DEMO "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	add_library(${DEMO_NAME} SHARED ${DEMO_SOURCES})
	target_link_libraries(${DEMO_NAME} ${DEMO_LIBS} knell-lib)
	add_dependencies(${DEMO_NAME} knell-lib)

	set(KN_ALL_DEMOS ${KN_ALL_DEMOS} ${DEMO_NAME} CACHE INTERNAL "KN_ALL_DEMOS")
	message("Adding ${DEMO_NAME} to demo list.")
endfunction()

# Use "feature test macro" to change POSIX support to allow use of
# CLOCK_MONOTONIC.
# http://man7.org/linux/man-pages/man7/feature_test_macros.7.html
add_definitions(-D_POSIX_C_SOURCE=199309L)

include_directories(SYSTEM src/)

add_subdirectory(src/knell)
add_subdirectory(src/demos)
add_subdirectory(src/build_tools)

add_compile_definitions(KN_LIBRARY=0)
add_executable(knell src/main.c)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT knell)

target_link_libraries(knell knell-lib)
add_dependencies(knell knell-lib ${KN_ALL_DEMOS})
message("Demos are ${KN_ALL_DEMOS}")


#
# Creates a unit test and add it to our registered list.
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

	# Generate coverage data only under GCC
	if(CMAKE_BUILD_TYPE MATCHES Debug AND CMAKE_COMPILER_IS_GNUCC)
		target_link_libraries(${UTEST_NAME} pthread gcov)
		set_target_properties(${UTEST_NAME} PROPERTIES COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
		set_target_properties(${UTEST_NAME} PROPERTIES LINK_FLAGS "-fprofile-arcs -ftest-coverage")
	endif()
	target_link_libraries(${UTEST_NAME} ${UTEST_LIBS})
	add_test(NAME "${UTEST_NAME}" COMMAND "${UTEST_NAME}")
	list(APPEND UNIT_TEST_REGISTER ${UTEST_NAME})
	add_dependencies(check ${UTEST_NAME})
	message("Adding ${UTEST_NAME} to unit tests.")
endfunction()

#
# Provides a command to run all tests, since CLion can't do that on it's own.
# https://stackoverflow.com/questions/733475/cmake-ctest-make-test-doesnt-build-tests#736838
#
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} -V)
add_subdirectory(src/tests)
    
if (WIN32)
	add_custom_command(TARGET knell POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${VENDOR_SDL2_ROOT}/lib/x64
		$<TARGET_FILE_DIR:knell>
		)

	add_custom_command(TARGET knell POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${VENDOR_GLEW_ROOT}/bin/Release/x64
		$<TARGET_FILE_DIR:knell>
		)

	add_custom_command(TARGET knell POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${VENDOR_ZLIB_ROOT}/Release/x64
		$<TARGET_FILE_DIR:knell>
		)

	# Fix a bug where knell-lib was not being copied into the working
	# directory where knell.exe lives, leading to usage of stale versions
	# of knell-lib in the binary.
	#
	# None of touch, type, and $null> work properly under CLion or Visual
	# Studio, so Knell provides a simple executable for writing a 0 byte
	# file (stamp_file.exe) which the Knell driver (knell) depends upon
	# which triggers a rebuild, even if no changes have been made to Knell
	# to ensure the post build step fires.
	#
	# https://nickdademo.blogspot.com/2015/03/cmake-always-running-post-build-step.html
    add_custom_command(TARGET knell POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
		$<TARGET_FILE:knell-lib>
		$<TARGET_FILE_DIR:knell>
		COMMENT "Copying knell-lib shared lib to driver directory")
	add_custom_target(force-knell-post-build
		COMMAND $<TARGET_FILE:stamp_file> ${CMAKE_SOURCE_DIR}/src/post_build_dummy.h
		COMMENT "Updating timestamp to force knell post build."
		)
	add_dependencies(force-knell-post-build stamp_file)
	add_dependencies(knell force-knell-post-build)

	# Only create dumps in Debug.
	# Uses a x64 version of the dll.
	# TODO: Provide an x86 and an ARM version.
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		add_custom_command(TARGET knell POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_directory
			${VENDOR_WINDOWS_SDK_ROOT}
			$<TARGET_FILE_DIR:knell>
			)
	endif()

endif (WIN32)
