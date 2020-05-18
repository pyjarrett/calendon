# Demos are single source files which demonstrate capabilities within or
# with Knell.  These are not intented to be large prototypes, or have external
# dependencies.
#
# Describe a demo and link it against knell-lib, and add it to the demo list
# so that knell will depend on it and force it to be build when knell is built.
#
function(add_demo)
	set(options "")
	set(oneValueArgs NAME)
	set(multiValueArgs SOURCES LIBS)
	cmake_parse_arguments(DEMO "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	add_library(${DEMO_NAME} SHARED ${DEMO_SOURCES})
	target_compile_definitions(${DEMO_NAME} PRIVATE KN_LIBRARY=0)
	target_link_libraries(${DEMO_NAME} ${DEMO_LIBS} knell)
	add_dependencies(${DEMO_NAME} knell)

	foreach(config ${CMAKE_CONFIGURATION_TYPES})
		string(TOUPPER ${config} CONFIG)
		set_target_properties(${DEMO_NAME}
			PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY_${CONFIG} "${DEMO_DIR}"
			LIBRARY_OUTPUT_DIRECTORY_${CONFIG} "${DEMO_DIR}"
			ARCHIVE_OUTPUT_DIRECTORY_${CONFIG} "${DEMO_DIR}")
	endforeach()

	set(KN_ALL_DEMOS ${KN_ALL_DEMOS} ${DEMO_NAME} CACHE INTERNAL "KN_ALL_DEMOS")
	message("Adding ${DEMO_NAME} to demo list.")
endfunction()

file(GLOB_RECURSE KNELL_DEMO_SRCS ./*.c)
foreach(DEMO_SRC_FILE ${KNELL_DEMO_SRCS})
	get_filename_component(DEMO_NAME ${DEMO_SRC_FILE} NAME_WE)
	add_demo(NAME ${DEMO_NAME} SOURCES ${DEMO_SRC_FILE})
endforeach()
