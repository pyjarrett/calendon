cmake_minimum_required(VERSION 3.10)
project(knell)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CONFIGURATION_TYPES Debug;Release;Coverage)

set(BINARY_DIR "${PROJECT_BINARY_DIR}")
set(LIBRARY_DIR "${PROJECT_BINARY_DIR}/lib")
set(ARCHIVE_DIR "${PROJECT_BINARY_DIR}/lib")
set(ASSET_DIR "${PROJECT_BINARY_DIR}/assets")
set(CONFIG_DIR "${PROJECT_BINARY_DIR}/configs")
set(RESOURCE_DIR "${PROJECT_BINARY_DIR}/resources")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
	elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
	endif()
elseif(CMAKE_BUILD_TYPE STREQUAL "Coverage")
	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		message(FATAL_ERROR "Coverage is not available in Clang.")
	elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
		set(KNELL_LIBS ${KNELL_LIBS} gcov)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
		set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
	endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g")
endif()

include(vendor/common/vendorize-cmake-0.1/vendorize.cmake)

vendorize(linux spartan_file_utils 0.0.1)
include_directories(SYSTEM "${VENDOR_SPARTAN_FILE_UTILS_ROOT}/include")
link_directories(SYSTEM "${VENDOR_SPARTAN_FILE_UTILS_ROOT}/lib")

# Change POSIX support to allow use of CLOCK_MONOTONIC.
add_definitions(-D_POSIX_C_SOURCE=199309L)

set(KNELL_SRCS
	src/assets.c
	src/control.c
	src/input.c
	src/kn.c
	src/log.c
	src/main.c
	src/math.c
	src/render.c
	src/render-ll.c
	src/ui.c
	)

set(KNELL_LIBS
	SDL2
	GL
	rt
	spa-fu
	${KNELL_LIBS}
	)

add_executable(knell ${KNELL_SRCS})
target_link_libraries(knell ${KNELL_LIBS})

# TODO: Replace this with a file describing where to find assets.
if (UNIX)
	add_custom_target(sync_assets
			COMMAND rsync -avz "${CMAKE_SOURCE_DIR}/assets/" "${ASSET_DIR}"
			COMMENT "Copying assets")
	add_dependencies(knell sync_assets)
endif(UNIX)
