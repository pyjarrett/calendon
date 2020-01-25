remove_definitions(-DKN_LIBRARY)
add_definitions(-DKN_LIBRARY=0)
add_library(sample SHARED sample.c)
target_link_libraries(sample knell-lib)

add_library(koch_curve SHARED koch_curve.c)
target_link_libraries(koch_curve knell-lib)

add_library(mouse-tracker SHARED mouse_tracker.c)
target_link_libraries(mouse-tracker knell-lib)

set(SAMPLES_SHARED
    sample
    koch_curve
	mouse-tracker
    PARENT_SCOPE
    )
