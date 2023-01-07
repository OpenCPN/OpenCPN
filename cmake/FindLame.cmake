# - Find  lame
# Find the mp3lame libraries
#
#  Defines:
#     LAME_FOUND        - true if LAME_INCLUDE_DIR & LAME_LIBRARY are found
#     lame::lame        - transitive link target.
#
#     LAME_LIBRARIES    - Set when LAME_LIBRARY is found
#     LAME_INCLUDE_DIRS - Set when LAME_INCLUDE_DIR is found


find_path(LAME_INCLUDE_DIR NAMES lame.h PATH_SUFFIXES lame)

find_library(LAME_LIBRARY NAMES mp3lame)

find_package_handle_standard_args(
  LAME DEFAULT_MSG LAME_LIBRARY LAME_INCLUDE_DIR
)

if(LAME_FOUND)
  set(LAME_LIBRARIES ${LAME_LIBRARY})
  set(LAME_INCLUDE_DIRS ${LAME_INCLUDE_DIR})
  add_library(_lame INTERFACE)
  target_link_libraries(_lame INTERFACE ${LAME_LIBRARIES})
  target_include_directories(_lame INTERFACE ${LAME_INCLUDE_DIRS})
  add_library(lame::lame ALIAS _lame)
endif()

mark_as_advanced(LAME_INCLUDE_DIR LAME_LIBRARY)
