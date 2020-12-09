# - Find Pixman
# Find the Pixman libraries
#
#  This module defines the following variables:
#     PIXMAN_FOUND        - true if PIXMAN_INCLUDE_DIR & PIXMAN_LIBRARY are found
#     PIXMAN_LIBRARIES    - Set when PIXMAN_LIBRARY is found
#     PIXMAN_INCLUDE_DIRS - Set when PIXMAN_INCLUDE_DIR is found
#
#     PIXMAN_INCLUDE_DIR  - where to find pixman.h, etc.
#     PIXMAN_LIBRARY      - the Pixman library
#


find_path(PIXMAN_INCLUDE_DIR NAMES pixman.h PATH_SUFFIXES pixman-1 pixman)

find_library(PIXMAN_LIBRARY NAMES pixman-1 pixman)

find_package_handle_standard_args(Pixman
  DEFAULT_MSG PIXMAN_LIBRARY PIXMAN_INCLUDE_DIR
)

if(Pixman_FOUND)
  set(PIXMAN_LIBRARIES ${PIXMAN_LIBRARY})
  set(PIXMAN_INCLUDE_DIRS ${PIXMAN_INCLUDE_DIR})
  add_library(_pixman INTERFACE)
  target_link_libraries(_pixman INTERFACE ${PIXMAN_LIBRARIES})
  target_include_directories(_pixman INTERFACE ${PIXMAN_INCLUDE_DIRS})
  add_library(Pixman::Pixman ALIAS _pixman)
endif()

mark_as_advanced(PIXMAN_INCLUDE_DIR PIXMAN_LIBRARY)
