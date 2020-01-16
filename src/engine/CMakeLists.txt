file(GLOB_RECURSE KNELL_SRCS ./*.c)

set(KNELL_LIBS
		SDL2
		GL
		rt
		spa-fu
		${KNELL_LIBS}
		)

add_library(knell ${KNELL_SRCS})
target_link_libraries(knell ${KNELL_LIBS})

# TODO: Replace this with a file describing where to find assets.
if (UNIX)
	add_custom_target(sync_assets
			COMMAND rsync -avz "${CMAKE_SOURCE_DIR}/assets/" "${ASSET_DIR}"
			COMMENT "Copying assets")
	add_dependencies(knell sync_assets)
endif(UNIX)
