cmake_minimum_required(VERSION 3.10)
project(knell)

# Using basic C99 for this engine until reaching technical and complexity
# limits with the language.  C11 was also considered, but C99 should be
# readily supported by the most number of compilers.
#
# Why not C++?  Using C99 aims to use a low level language, which readily maps
# to the generated assembly and  generates consistent binary symbols across
# platforms.  C99 limits overdesign and simplifies design choices by providing
# a subset of functionality of other languages, and hopes to boost productivity
# by providing clear compile errors.
#
# As the lingua franca of programming bindings to other languages should able
# to be auto-generated or readily written by hand when appropriate.
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Debug: Include debugging symbols and most warnings.
# Release: No debugging symbol and optimized for release.
# Coverage: Flags for generated code coverage under GCC.
#
# Future configurations:
# Profile: Add -pg flag for GCC to use with gprof
# RelWIthDebugInfo: Optimized build with debug symbols included.
set(CMAKE_CONFIGURATION_TYPES Debug;Release;Coverage)
if(NOT CMAKE_BUILD_TYPE)
    # Visual studio might not correctly set CMAKEBUILD_TYPE
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build Type: (Debug, Release)." FORCE)
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

# Bring in extra warnings to prevent errors.
# https://caiorss.github.io/C-Cpp-Notes/compiler-flags-options.html
set(KNELL_COMMON_C_FLAGS "${KNELL_COMMON_C_FLAGS} -Wall -Wextra")
#set(KNELL_COMMON_C_FLAGS "${KNELL_COMMON_C_FLAGS} -Wpedantic")
set(KNELL_COMMON_C_FLAGS "${KNELL_COMMON_C_FLAGS} -Wshadow -Wdouble-promotion -Wundef -Wconversion -fno-common")
set(KNELL_COMMON_C_FLAGS "${KNELL_COMMON_C_FLAGS} -fvisibility=hidden")

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

	# spartan file utils in an in-house library for cross platform quick file
	# operations such as seeing if a file exists, or if a path is a dir or a file.
	vendorize(windows spartan_file_utils 0.0.1)
	include_directories(SYSTEM "${VENDOR_SPARTAN_FILE_UTILS_ROOT}/include")
	link_directories(SYSTEM "${VENDOR_SPARTAN_FILE_UTILS_ROOT}/lib")

	# glew (GL Extension Wranger) is used to load OpenGL extension functions so we
	# don't need to manually load function pointers.
	vendorize(windows glew 2.1.0)
	include_directories(SYSTEM "${VENDOR_GLEW_ROOT}/include")
	link_directories(SYSTEM "${VENDOR_GLEW_ROOT}/lib/Release/x64")
	set(KNELL_LIBS
		glew32.lib
		${KNELL_LIBS})

	# dbghelp.dll isn't provided normally, it's part of the Debugging tools in
	# the SDK, so an x64 version is included for creating minidumps.
	vendorize(windows windows_sdk 10)
endif (WIN32)

if (UNIX)
	# spartan file utils in an in-house library for cross platform quick file
	# operations such as seeing if a file exists, or if a path is a dir or a file.
	vendorize(linux spartan_file_utils 0.0.1)
	include_directories(SYSTEM "${VENDOR_SPARTAN_FILE_UTILS_ROOT}/include")
	link_directories(SYSTEM "${VENDOR_SPARTAN_FILE_UTILS_ROOT}/lib")
endif (UNIX)

# Use "feature test macro" to change POSIX support to allow use of
# CLOCK_MONOTONIC.
# http://man7.org/linux/man-pages/man7/feature_test_macros.7.html
add_definitions(-D_POSIX_C_SOURCE=199309L)

remove_definitions(-DKN_LIBRARY)
add_definitions(-DKN_LIBRARY=0)
add_subdirectory(src/knell)
include_directories(SYSTEM src/)

remove_definitions(-DKN_LIBRARY)
add_definitions(-DKN_LIBRARY=0)
add_subdirectory(src/demos)

remove_definitions(-DKN_LIBRARY)
add_definitions(-DKN_LIBRARY=0)
add_executable(knell src/main.c)

target_link_libraries(knell knell-lib ${SAMPLES_SHARED})
add_dependencies(knell knell-lib ${SAMPLES_SHARED})
    
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
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:knell-lib> $<TARGET_FILE_DIR:knell>)

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
