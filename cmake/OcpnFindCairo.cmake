#
# Find the cairo libraries. On exit, sets:
#
#    CAIRO_INCLUDE_DIRS  - header directories.
#    CAIRO_LIBRARIES  - libraries to link

if (CAIRO_INCLUDE_DIRS)
  # Already in cache, be silent
  set(CAIRO_FIND_QUIETLY TRUE)
endif ()

set(CAIRO_INC_LOOK_PATHS /usr/local/include /usr/include)

if (APPLE)
  if (NOT APPLE_MODERN)
      SET(CAIRO_INC_LOOK_PATHS /usr/local/Cellar/cairo/1.14.6/include)
      SET(LOOK_OPTION NO_DEFAULT_PATH)
  endif ()
endif (APPLE)

find_path(CAIRO_INCLUDE_DIRS cairo.h
          PATHS ${CAIRO_INC_LOOK_PATHS}
          PATH_SUFFIXES cairo/ libcairo/ cairo/libcairo/
          ${LOOK_OPTION})
message(STATUS " Cairo include directory found: ${CAIRO_INCLUDE_DIRS}")

set(CAIRO_LIB_LOOK_PATHS ${LINUX_LIB_PATHS})
if (APPLE)
  if (NOT APPLE_MODERN)
      set(CAIRO_LIB_LOOK_PATHS /usr/local/Cellar/cairo/1.14.6/lib)
      set(LOOK_OPTION NO_DEFAULT_PATH)
  endif (NOT APPLE_MODERN)
endif (APPLE)

set( CAIRO_NAMES libcairo cairo )
find_library(CAIRO_LIBRARIES
    NAMES ${CAIRO_NAMES}
    PATHS ${CAIRO_LIB_LOOK_PATHS}
    ${LOOK_OPTION}
)
message(STATUS " Cairo library found: ${CAIRO_LIBRARIES}")

get_filename_component(CAIRO_LIBRARY_DIR ${CAIRO_LIBRARIES} PATH)

# handle the QUIETLY and REQUIRED arguments and set CAIRO_FOUND to TRUE if
# all listed variables are TRUE
include( "FindPackageHandleStandardArgs" )
find_package_handle_standard_args("CAIRO"
    DEFAULT_MSG CAIRO_INCLUDE_DIRS CAIRO_LIBRARIES
)
mark_as_advanced(CAIRO_INCLUDE_DIRS CAIRO_LIBRARIES )
if (NOT CAIRO_FOUND)
    message(FATAL_ERROR "Cairo component required, but not found!")
endif()

# Some systems (e.g ARMHF RPI2) require some extra libraries
# This is not exactly a general solution, but probably harmless where
# not needed.
if (NOT APPLE)
    find_library(
        PANGOCAIRO_LIBRARY
        NAMES pangocairo-1.0 PATHS ${LINUX_LIB_PATHS}
    )
    find_library(
        PANGOFT2_LIBRARY NAMES pangoft2-1.0 PATHS ${LINUX_LIB_PATHS})
    find_library(
        PANGOXFT_LIBRARY NAMES pangoxft-1.0 PATHS ${LINUX_LIB_PATHS})
    find_library(
        GDK_PIXBUF_LIBRARY
        NAMES gdk_pixbuf-2.0 PATHS ${LINUX_LIB_PATHS}
    )
    find_package_handle_standard_args(
        "CAIRO_EXTRAS" DEFAULT_MSG PANGOCAIRO_LIBRARY
        PANGOFT2_LIBRARY PANGOXFT_LIBRARY GDK_PIXBUF_LIBRARY
    )
endif (NOT APPLE)

if (CAIRO_EXTRAS_FOUND)
    set(CAIRO_EXTRA_LIBRARIES
        ${PANGOCAIRO_LIBRARY}
        ${PANGOFT2_LIBRARY} ${PANGOXFT_LIBRARY}
        ${GDK_PIXBUF_LIBRARY}
    )
    message(STATUS "Cairo Extra Libraries: ${CAIRO_EXTRA_LIBRARIES}")
endif ()
set(CAIRO_LIBRARIES ${CAIRO_LIBRARIES} ${CAIRO_EXTRA_LIBRARIES})
