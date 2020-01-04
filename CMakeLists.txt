cmake_minimum_required(VERSION 3.10)
project(knell)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(BINARY_DIR "${PROJECT_BINARY_DIR}")
set(LIBRARY_DIR "${PROJECT_BINARY_DIR}/lib")
set(ARCHIVE_DIR "${PROJECT_BINARY_DIR}/lib")
set(ASSET_DIR "${PROJECT_BINARY_DIR}/assets")
set(CONFIG_DIR "${PROJECT_BINARY_DIR}/configs")
set(RESOURCE_DIR "${PROJECT_BINARY_DIR}/resources")

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	set(CMAKE_C_FLAGS "-Wall -g")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
endif()

include(vendor/common/vendorize-cmake-0.1/vendorize.cmake)

vendorize(common spartan_file_utils 0.0.1)
include_directories(SYSTEM "${VENDOR_SPARTAN_FILE_UTILS_ROOT}/include")
link_directories(SYSTEM "${VENDOR_SPARTAN_FILE_UTILS_ROOT}/build/lib")

# Change POSIX support to allow use of CLOCK_MONOTONIC.
add_definitions(-D_POSIX_C_SOURCE=199309L)

set(KNELL_SRCS
	src/control.c
	src/input.c
	src/kn.c
	src/main.c
	src/math.c
	src/render_hl.c
	src/render_ll.c
	src/ui.c
	)

set(KNELL_LIBS
	SDL2
	GL
	rt
	libspa-fu.a
	)

add_executable(knell ${KNELL_SRCS})
target_link_libraries(knell ${KNELL_LIBS})

