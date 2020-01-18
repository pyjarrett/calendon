file(GLOB_RECURSE KNELL_SRCS ./*.c)

set(KNELL_LIBS
	SDL2
	spa-fu
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
		${KNELL_LIBS}
		)
endif ()

add_definitions(-DKN_LIBRARY=1)
add_library(knell-lib SHARED ${KNELL_SRCS})
target_link_libraries(knell-lib ${KNELL_LIBS})

# TODO: Replace this with a file describing where to find assets.
if (UNIX)
	add_custom_target(sync_assets
			COMMAND rsync -avz "${CMAKE_SOURCE_DIR}/assets/" "${ASSET_DIR}"
			COMMENT "Copying assets")
	add_dependencies(knell-lib sync_assets)
endif(UNIX)
