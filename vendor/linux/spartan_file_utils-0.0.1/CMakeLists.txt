# Probably not the true minimum version required.
cmake_minimum_required(VERSION 3.10)

project(spartan_file_utils)

set(CMAKE_CONFIGURATION_TYPES Debug;Release)
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: ${CMAKE_CONFIGURATION_TYPES}" FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g")
	endif()
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(BINARY_DIR "${PROJECT_BINARY_DIR}")
set(LIBRARY_DIR "${PROJECT_BINARY_DIR}/lib")
set(ARCHIVE_DIR "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${LIBRARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${ARCHIVE_DIR}")

set(LIBRARY_DIR "${PROJECT_BINARY_DIR}/lib")

include_directories(include)
set(SPA_SRCS src/spa_fu.c)

add_library(spa-fu ${SPA_SRCS})

