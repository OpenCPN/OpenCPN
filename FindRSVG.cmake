# - Find RSVG (https://en.wikipedia.org/wiki/Librsvg)
# Find the native librsvg includes and library
# Debian package: librsvg2-dev
#
#   RSVG_FOUND       - True if RSVG found.
#   RSVG_INCLUDE_DIR - where to find rsvg.h, etc.
#   RSVG_LIBRARY     - List of libraries when using RSVG.
#   RSVG_LIBRARY_DIR - Directory containing the rsvg library
#

IF( RSVG_INCLUDE_DIR )
    # Already in cache, be silent
    SET( RSVG_FIND_QUIETLY TRUE )
ENDIF( RSVG_INCLUDE_DIR )

FIND_PATH( RSVG_INCLUDE_DIR rsvg.h
              PATHS /usr/local/include /usr/include
              PATH_SUFFIXES librsvg-2.0/ librsvg/ librsvg-2.0/librsvg/ )

SET( RSVG_NAMES librsvg-2 rsvg-2 )
FIND_LIBRARY( RSVG_LIBRARY
              NAMES ${RSVG_NAMES}
              PATHS /usr/lib /usr/local/lib )

GET_FILENAME_COMPONENT( RSVG_LIBRARY_DIR ${RSVG_LIBRARY} PATH )


# handle the QUIETLY and REQUIRED arguments and set RSVG_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE( "FindPackageHandleStandardArgs" )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( "RSVG" DEFAULT_MSG RSVG_INCLUDE_DIR RSVG_LIBRARY )

MARK_AS_ADVANCED( RSVG_INCLUDE_DIR RSVG_LIBRARY )
