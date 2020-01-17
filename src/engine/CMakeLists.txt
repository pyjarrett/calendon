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

add_executable(knell ${KNELL_SRCS})
target_link_libraries(knell ${KNELL_LIBS})

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
endif (WIN32)

# TODO: Replace this with a file describing where to find assets.
if (UNIX)
	add_custom_target(sync_assets
			COMMAND rsync -avz "${CMAKE_SOURCE_DIR}/assets/" "${ASSET_DIR}"
			COMMENT "Copying assets")
	add_dependencies(knell sync_assets)
endif(UNIX)
