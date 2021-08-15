#
# Helper for dist_target.cmake
#

execute_process(
    COMMAND git rev-parse --short HEAD
    OUTPUT_VARIABLE  GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND git tag --contains HEAD
    OUTPUT_VARIABLE  GIT_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND git log --pretty=format:%cd --date=iso -1
    OUTPUT_VARIABLE GIT_DATE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "Creating ${SOURCE_DIR}/git-version")
configure_file(
    "${SOURCE_DIR}/cmake/version_git.in"
    "${SOURCE_DIR}/git-version"
    IMMEDIATE @ONLY
)
