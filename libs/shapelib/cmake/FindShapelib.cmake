# Find the shapelib a k a shp libraries
#
# Defines:
#     SHAPELIB_FOUND - true if shapelib libs and headers found
#
# Exports:
#     shapelib::shapelib transitive link target if SHAPELIB_FOUND is true
#
# Copyright (c) 2024 Alec Leamas
# License: GPL 2+

find_path(SHAPELIB_INCLUDE_DIR shapefil.h)

find_library(SHAPELIB_LIBRARY NAMES shp shapelib)

find_package_handle_standard_args(Shapelib
    DEFAULT_MSG SHAPELIB_LIBRARY SHAPELIB_INCLUDE_DIR
)

if (Shapelib_FOUND)
  add_library(_shapelib INTERFACE)
  target_link_libraries(_shapelib INTERFACE ${SHAPELIB_LIBRARY})
  target_include_directories(_shapelib INTERFACE ${SHAPELIB_INCLUDE_DIR})
  add_library(shapelib::shapelib ALIAS _shapelib)
endif()

mark_as_advanced(SHAPELIB_INCLUDE_DIR SHAPELIB_LIBRARY)
