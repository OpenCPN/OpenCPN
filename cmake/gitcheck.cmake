#
#  dist_target helper
#
execute_process(
    COMMAND git status --porcelain
    OUTPUT_VARIABLE GIT_STATUS
    ERROR_VARIABLE GIT_STATUS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if (GIT_STATUS)
   message(FATAL_ERROR "Git repository is not clean. Aborting")
else ()
   message(STATUS "Git status: clean")
endif ()


