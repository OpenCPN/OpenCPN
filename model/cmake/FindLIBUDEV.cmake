#
# Try to find the udev library, sets UDEV_FOUND and a linkable ocpn::libudev
# target.
#

if (TARGET ocpn::libudev)
  return()
endif ()


add_library(_LIBUDEV INTERFACE)
add_library(ocpn::libudev ALIAS _LIBUDEV)

set(UDEV_ROOT_DIR "${UDEV_ROOT_DIR}" CACHE PATH "Directory to search for udev")

find_library(UDEV_LIBRARY
  NAMES udev
  HINTS "${UDEV_ROOT_DIR}"
  PATH_SUFFIXES lib lib64
)

get_filename_component(_libdir "${UDEV_LIBRARY}" PATH)

find_path(UDEV_INCLUDE_DIR
  NAMES libudev.h
  HINTS "${_libdir}" "${_libdir}/.." "${UDEV_ROOT_DIR}"
  PATH_SUFFIXES include
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  LIBUDEV DEFAULT_MSG UDEV_LIBRARY UDEV_INCLUDE_DIR
)

if (LIBUDEV_FOUND)
  list(APPEND UDEV_LIBRARIES ${UDEV_LIBRARY})
  list(APPEND UDEV_INCLUDE_DIRS ${UDEV_INCLUDE_DIR})
  mark_as_advanced(UDEV_ROOT_DIR)
  target_include_directories(_LIBUDEV INTERFACE ${UDEV_INCLUDE_DIRS})
  target_link_libraries(_LIBUDEV INTERFACE ${UDEV_LIBRARIES})
endif ()

mark_as_advanced(UDEV_INCLUDE_DIR UDEV_LIBRARY)
