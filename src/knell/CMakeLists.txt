file(GLOB_RECURSE KNELL_SRCS ./*.c)
file(GLOB_RECURSE KNELL_HEADERS ./*.h)

# Add spng explicitly
set(KNELL_SRCS
	${KNELL_SRCS}
	${VENDOR_LIBSPNG_ROOT}/spng.c)

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
add_library(knell-lib SHARED ${KNELL_SRCS} ${KNELL_HEADERS})
target_link_libraries(knell-lib ${KNELL_LIBS})

# Define a special version of knell-lib to allow unit tests to actually verify
# that asserts (primarily preconditions) are actually checked.
remove_definitions(-DKN_LIBRARY)
add_definitions(-DKN_LIBRARY=1)
add_definitions(-DKN_TESTING=1)
add_library(knell-lib-test SHARED ${KNELL_SRCS} ${KNELL_HEADERS})
target_link_libraries(knell-lib-test ${KNELL_LIBS})
remove_definitions(-DKN_TESTING)

add_custom_command(TARGET knell-lib-test POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	$<TARGET_FILE:knell-lib-test>
	${BINARY_DIR}
	COMMENT "Copying knell-lib-test to binary directory")

# TODO: Replace this with a file describing where to find assets.
if (UNIX)
	add_custom_target(sync_assets
			COMMAND rsync -avz "${CMAKE_SOURCE_DIR}/assets/" "${ASSET_DIR}"
			COMMENT "Copying assets")
	add_dependencies(knell-lib sync_assets)
endif(UNIX)
