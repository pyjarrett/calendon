cmake_minimum_required(VERSION 3.10)

# ccache can speed up program compilation, especially if you wipe and regenerate
# your build directory.
if (CN_ENABLE_CCACHE)
	find_program(CCACHE_PROGRAM ccache)
	if (CCACHE_PROGRAM)
		message(STATUS "Using ccache")
		set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
	endif()
endif()

project(calendon C)

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
    set(CMAKE_CONFIGURATION_TYPES Debug;Release;RelWithDebInfo;Coverage;Future)
elseif (WIN32)
    # Coverage is not supported on Windows.
    set(CMAKE_CONFIGURATION_TYPES Debug;Release;RelWithDebInfo)
endif()
if(NOT CMAKE_BUILD_TYPE)
    # Visual studio might not correctly set CMAKEBUILD_TYPE
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build Type: ${CMAKE_CONFIGURATION_TYPES}." FORCE)
endif()
message("Configuration types: ${CMAKE_CONFIGURATION_TYPES}")

# Provide standardized output locations.  This particularly
# helps when dealing with additional directories such as for
# assets to ensure they are placed correctly relative to
# executables.
#
# https://stackoverflow.com/questions/543203/cmake-runtime-output-directory-on-windows
set(OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(BINARY_DIR ${OUTPUT_DIRECTORY})
set(DEMO_DIR ${OUTPUT_DIRECTORY}/demos)
message("Output directory: ${OUTPUT_DIRECTORY}")

foreach(config ${CMAKE_CONFIGURATION_TYPES})
	string(TOUPPER ${config} CONFIG)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG} "${OUTPUT_DIRECTORY}")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG} "${OUTPUT_DIRECTORY}")
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG} "${OUTPUT_DIRECTORY}")
endforeach()

# CALENDON_LIBS is the list of all libraries to link with the Calendon lib itself.
# Client should be required to do minimum linking themselves.
set(CALENDON_LIBS "")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	# Set the CN_DEBUG symbol to 1 at the top level within the build system
	# so no header needs to be included (or possibly forgotten) to get this
	# value.
	if(CMAKE_C_COMPILER_ID MATCHES "Clang")
		add_definitions(-DCN_DEBUG=1)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

		add_definitions(-DCN_ENABLE_CORE_DUMPS=1)
		# Enabling this prevents core dumps from working.
		#set(CALENDON_COMMON_C_FLAGS "${CALENDON_COMMON_C_FLAGS} -fsanitize=address -fsanitize=undefined")
	elseif(CMAKE_C_COMPILER_ID MATCHES "GNU")
		add_definitions(-DCN_DEBUG=1)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

		add_definitions(-DCN_ENABLE_CORE_DUMPS=1)
		# Enabling this prevents core dumps from working.
		#set(CALENDON_COMMON_C_FLAGS "${CALENDON_COMMON_C_FLAGS} -fsanitize=address -fsanitize=undefined")
    elseif(CMAKE_C_COMPILER_ID MATCHES "MSVC")
        # Allow symbol resolution when querying for symbols.
        add_definitions(/DPSAPI_VERSION=1)
        add_definitions(/DCN_DEBUG=1)
		set(CALENDON_LIBS ${CALENDON_LIBS} psapi)

		# Adds MiniDump reporting.
		add_definitions(/DCN_ENABLE_CORE_DUMPS=1)
		set(CALENDON_LIBS ${CALENDON_LIBS} dbghelp)
	endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		message(FATAL_ERROR "Coverage is not available in Clang.")
	elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
		set(CALENDON_LIBS ${CALENDON_LIBS} gcov)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
		set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
	endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Future")
	# Omit deprecated functionality, causing build breakages.
	add_definitions(-DCN_DEPRECATION_BREAK=0)
	add_definitions(-DCN_DEPRECATION_OMIT=1)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
	# Make a breakpoint on deprecated behavior.
	add_definitions(-DCN_DEPRECATION_BREAK=1)
	add_definitions(-DCN_DEPRECATION_OMIT=0)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
	# Release may need to ship with deprecated features to make a ship date.
	add_definitions(-DCN_DEPRECATION_BREAK=0)
	add_definitions(-DCN_DEPRECATION_OMIT=0)
endif()

# Bring in extra warnings to prevent errors.
# https://caiorss.github.io/C-Cpp-Notes/compiler-flags-options.html
set(CALENDON_COMMON_C_FLAGS "${CALENDON_COMMON_C_FLAGS} -Wall -Wextra")
#set(CALENDON_COMMON_C_FLAGS "${CALENDON_COMMON_C_FLAGS} -Wpedantic")
set(CALENDON_COMMON_C_FLAGS "${CALENDON_COMMON_C_FLAGS} -Wshadow -Wdouble-promotion -Wundef -Wconversion -fno-common")
set(CALENDON_COMMON_C_FLAGS "${CALENDON_COMMON_C_FLAGS} -fvisibility=hidden")

# Temporary workaround until strcpy, strlen, usages get replaced.
if (WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS=1)
    set(CALENDON_COMMON_C_FLAGS "${CALENDON_COMMON_C_FLAGS} /W4")
	set(CALENDON_COMMON_C_FLAGS "${CALENDON_COMMON_C_FLAGS} /MP")      # Enable parallel build
endif (WIN32)

if (UNIX)
	set(CALENDON_LIBS ${CALENDON_LIBS} m)
endif (UNIX)

# Eliminate extra dead code
#set(CALENDON_COMMON_C_FLAGS "-ffunction-sections -fdata-sections")

# https://interrupt.memfault.com/blog/best-and-worst-gcc-clang-compiler-flags
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	# TODO: It'd be nice to use -Weverything, but it generates pointer sign
	# errors on uint8_t* <-> char* and I don't have time to investigate.
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CALENDON_COMMON_C_FLAGS}")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CALENDON_COMMON_C_FLAGS}")
endif()

# Introduce the vendorize(platform library semantic_version) syntax for
# providing libraries from known local sources (vendoring).
include(vendor/common/vendorize-cmake-0.1/vendorize.cmake)

# libspng provides extremely simplified PNG loading.
vendorize(common libspng 0.6.3)
include_directories(SYSTEM ${VENDOR_LIBSPNG_ROOT}/spng)

if (WIN32)
	vendorize(windows sdl2 2.0.10)
	include_directories(SYSTEM "${VENDOR_SDL2_ROOT}/include")
	link_directories(SYSTEM "${VENDOR_SDL2_ROOT}/lib/x64")

	# glew (GL Extension Wranger) is used to load OpenGL extension functions so we
	# don't need to manually load function pointers.
	vendorize(windows glew 2.1.0)
	include_directories(SYSTEM "${VENDOR_GLEW_ROOT}/include")
	link_directories(SYSTEM "${VENDOR_GLEW_ROOT}/lib/Release/x64")
	set(CALENDON_LIBS
		glew32.lib
		${CALENDON_LIBS})

	# zlib is required by libspng
	vendorize(windows zlib 1.2.11)
	include_directories(SYSTEM ${VENDOR_ZLIB_ROOT}/include)
	link_directories(${VENDOR_ZLIB_ROOT}/Release/x64)
	set(CALENDON_LIBS ${CALENDON_LIBS} zlibwapi.dll)

	# dbghelp.dll isn't provided normally, it's part of the Debugging tools in
	# the SDK, so an x64 version is included for creating minidumps.
	vendorize(windows windows_sdk 10)
endif (WIN32)

add_subdirectory(src)
