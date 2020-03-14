file(GLOB_RECURSE KNELL_SRCS ./*.c)
file(GLOB_RECURSE KNELL_HEADERS ./*.h)

# Add spng explicitly
set(KNELL_SRCS
	${KNELL_SRCS}
	${VENDOR_LIBSPNG_ROOT}/spng.c)

# Ignore compiler warnings on spng.
if (CMAKE_C_COMPILER_ID MATCHES "MSVC")
	# C4090: 'operation' : different 'modifier' qualifiers
	# C4267: 'var' : conversion from 'size_t' to 'type', possible loss of data
	set_property(SOURCE ${VENDOR_LIBSPNG_ROOT}/spng.c PROPERTY COMPILE_FLAGS
		"${CMAKE_C_FLAGS} /wd4090 /wd4267")
endif()

set(KNELL_LIBS
	SDL2
	${KNELL_LIBS}
	)

if (WIN32)
	set(KNELL_LIBS
		opengl32
		# Windows must link in SDL2main
		SDL2main
		${KNELL_LIBS}
		)
endif (WIN32)

if (UNIX)
	set(KNELL_LIBS
		GL
		rt
		z
		${KNELL_LIBS}
		)
endif ()

remove_definitions(-DKN_LIBRARY)
add_definitions(-DKN_LIBRARY=1)
add_library(knell SHARED ${KNELL_SRCS} ${KNELL_HEADERS})
target_link_libraries(knell ${KNELL_LIBS})

# Define a special version of knell to allow unit tests to actually verify
# that asserts (primarily preconditions) are actually checked.
remove_definitions(-DKN_LIBRARY)
remove_definitions(-DKN_TESTING)
add_definitions(-DKN_LIBRARY=1)
add_definitions(-DKN_TESTING=1)
add_library(knell-testable SHARED ${KNELL_SRCS} ${KNELL_HEADERS})
target_link_libraries(knell-testable ${KNELL_LIBS})
remove_definitions(-DKN_TESTING)

add_custom_command(TARGET knell-testable POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	$<TARGET_FILE:knell-testable>
	${BINARY_DIR}
	COMMENT "Copying knell-testable to binary directory")

# TODO: Replace this with a file describing where to find assets.
if (UNIX)
	add_custom_target(sync_assets
			COMMAND rsync -avz "${CMAKE_SOURCE_DIR}/assets/" "${ASSET_DIR}"
			COMMENT "Copying assets")
	add_dependencies(knell sync_assets)
	add_dependencies(knell-testable sync_assets)
endif(UNIX)
