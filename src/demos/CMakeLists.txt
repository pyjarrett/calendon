
remove_definitions(-DKN_LIBRARY)
add_definitions(-DKN_LIBRARY=0)

add_demo(NAME sample SOURCES sample.c)
add_demo(NAME koch_curve SOURCES koch_curve.c)
add_demo(NAME mouse-tracker SOURCES mouse_tracker.c)
