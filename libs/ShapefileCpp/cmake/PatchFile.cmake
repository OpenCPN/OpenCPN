# use GNU Patch from any platform
#
# Source: https://github.com/scivision/cmake-patch-file
# License: MIT

if (WIN32)
  # prioritize Git Patch on Windows as other Patches may be very old and incompatible.
  find_package(Git)
  if (Git_FOUND)
    get_filename_component(GIT_DIR ${GIT_EXECUTABLE} DIRECTORY)
    get_filename_component(GIT_DIR ${GIT_DIR} DIRECTORY)
  endif ()
endif ()

find_program(PATCH NAMES gpatch patch HINTS ${GIT_DIR} PATH_SUFFIXES usr/bin)

if (NOT PATCH)
  message(FATAL_ERROR "Did not find GNU Patch")
endif()

execute_process(
  COMMAND ${PATCH} -p1 --ignore-whitespace
  INPUT_FILE ${patch_file}
  OUTPUT_FILE last_patch_output
  WORKING_DIRECTORY ${patch_dir}
  TIMEOUT 15
  RESULT_VARIABLE ret
)
if (NOT ret EQUAL 0)
  message(
    FATAL_ERROR "Failed to apply ${patch_file} in ${patch_dir} using ${PATCH}"
  )
endif()
