# Describe a demo and link it against knell-lib, and add it to the demo list
# so that knell will depend on it and force it to be build when knell is built.
#
# Format:
#   add_demo(NAME test_name SOURCES a.cpp b.cpp)
#   add_demo(NAME test_name SOURCES a.cpp b.cpp LIBS GL)
#
function(add_demo)
	set(options "")
	set(oneValueArgs NAME)
	set(multiValueArgs SOURCES LIBS)
	cmake_parse_arguments(DEMO "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	add_library(${DEMO_NAME} SHARED ${DEMO_SOURCES})
	target_link_libraries(${DEMO_NAME} ${DEMO_LIBS} knell)
	add_dependencies(${DEMO_NAME} knell)

	set(KN_ALL_DEMOS ${KN_ALL_DEMOS} ${DEMO_NAME} CACHE INTERNAL "KN_ALL_DEMOS")
	message("Adding ${DEMO_NAME} to demo list.")
endfunction()

remove_definitions(-DKN_LIBRARY)
add_definitions(-DKN_LIBRARY=0)

add_demo(NAME sample SOURCES sample.c)
add_demo(NAME koch-curve SOURCES koch_curve.c)
add_demo(NAME mouse-tracker SOURCES mouse_tracker.c)
add_demo(NAME interpolated-transitions SOURCES interpolated_transitions.c)
add_demo(NAME animation-loop SOURCES animation_loop.c)
add_demo(NAME texture-drawing SOURCES texture_drawing.c)
add_demo(NAME image-animation-loop SOURCES image_animation_loop.c)
add_demo(NAME graphics-sample SOURCES graphics_sample.c)
add_demo(NAME planets SOURCES planets.c)
