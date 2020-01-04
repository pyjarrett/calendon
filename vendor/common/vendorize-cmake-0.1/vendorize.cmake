# For when you want to ship multiple versions of a library, for different
# platforms or using different versions at different times.
if(CMAKE_SYSTEM_NAME STREQUAL Linux)
    set(VENDOR_OS "linux")
else()
    set(VENDOR_OS "windows")
# TODO: I don't have a Mac to test.
endif()
set(VENDOR_ROOT "${CMAKE_SOURCE_DIR}/vendor")

# Some libraries have platform agnostic distributions.
set(VENDOR_COMMON_ROOT "${VENDOR_ROOT}/common")

# Providing our own packages as part of the source tree reduces variables in the
# build process.
#
# Sets VENDOR_${IDENTIFIER}_ROOT to target the appropriate vendored library
# version.
#
# Vendored libraries must reside in
# ${VENDOR_ROOT}/${platform}/${identifier}-${version}
#
# Example:
# - With VENDOR_ROOT set to ./vendor/
# vendorize(linux foo 1.4.2)
# - creates variable VENDOR_FOO_ROOT set to "./vendor/linux/foo-1.4.2
# include_directories(SYSTEM ${VENDOR_FOO_ROOT}/include)
# - adds include directories based on the picked foo version and platform.
#
# Using a function for this prevents needing to drag around and update the
# version name for libraries, or even easily update the libraries used on
# different platforms.
#
function(vendorize platform identifier version)
	string(TOUPPER ${identifier} IDENTIFIER)
	set(VENDOR_${IDENTIFIER}_ROOT ${VENDOR_ROOT}/${platform}/${identifier}-${version} PARENT_SCOPE)
	message("Using ${identifier}-${version} vendored for ${platform}")
endfunction()

