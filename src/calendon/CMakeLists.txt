# I shouldn't be using GLOB, but until I decide to support a CMake version > 3.10
# so I can use CONFIGURE_DEPENDS.
# https://stackoverflow.com/questions/1027247/is-it-better-to-specify-source-files-with-glob-or-each-file-individually-in-cmak/1060061#1060061
file(GLOB_RECURSE CALENDON_SRCS ./*.c)
file(GLOB_RECURSE CALENDON_HEADERS ./*.h)

# Add spng explicitly
set(VENDOR_LIBSPNG_SRC_DIR ${VENDOR_LIBSPNG_ROOT}/spng)
set(CALENDON_SRCS
	${CALENDON_SRCS}
	${VENDOR_LIBSPNG_SRC_DIR}/spng.c)

# Ignore compiler warnings on spng.
if (CMAKE_C_COMPILER_ID MATCHES "MSVC")
	# C4090: 'operation' : different 'modifier' qualifiers
	# C4267: 'var' : conversion from 'size_t' to 'type', possible loss of data
	set_property(SOURCE ${VENDOR_LIBSPNG_ROOT}/spng.c PROPERTY COMPILE_FLAGS
		"${CMAKE_C_FLAGS} /wd4090 /wd4267")
endif()

set(CALENDON_LIBS
	SDL2
	${CALENDON_LIBS}
	)

if (WIN32)
	set(CALENDON_LIBS
		opengl32
		# Windows must link in SDL2main
		SDL2main
		${CALENDON_LIBS}
		)
endif (WIN32)

if (UNIX)
	set(CALENDON_LIBS
		GL
		rt
		z
		${CALENDON_LIBS}
		)
endif ()

add_library(calendon SHARED ${CALENDON_SRCS} ${CALENDON_HEADERS})
target_compile_definitions(calendon PRIVATE CN_TESTING=0 CN_LIBRARY=1)
target_link_libraries(calendon ${CALENDON_LIBS})

# Define a special version of calendon to allow unit tests to actually verify
# that asserts (primarily preconditions) are actually checked.
add_library(calendon-testable SHARED ${CALENDON_SRCS} ${CALENDON_HEADERS})
target_compile_definitions(calendon-testable PRIVATE CN_TESTING=1 CN_LIBRARY=1)
target_link_libraries(calendon-testable ${CALENDON_LIBS})
remove_definitions(-DCN_TESTING)
