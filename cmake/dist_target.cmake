#
# Add a custom 'dist' target. At a minimum, it performs
# 'make package_source'. If a tar command is found the
# source tarball is repacked to avoid incompatibilities
# with cmake's tar (#1849)
#
#
# Usage:
#    file(GLOB DIST_TARBALL opencpn*.tar.*)
#    include(dist_target)
#
#    The DIST_TARBALL file is the file to repack, defaults to
#    build/ *.tar.* (the definition could usually be omitted).
#
# Sets global DIST_TARBALL if not already defined.

file(WRITE ${CMAKE_BINARY_DIR}/gitcheck.sh
    "
    if output=$(git status --porcelain) && [ -z \"$output\" ]; then
        echo 'Git status: clean'
    else
        echo 'WARNING: git status not clean' >&2
    fi"
)
add_custom_target(gitcheck
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/cmake/gitcheck.cmake
)
if (GIT_STATUS)
   message(FATAL_ERROR "Git repository is not clean. Aborting")
else ()
   message(STATUS "Git status: clean")
endif ()

find_program(TAR NAMES tar)
if (TAR)
    if (NOT TARBALL)
        file(GLOB TARBALL ${CMAKE_BINARY_DIR}/*.tar.*)
    endif()
    file(WRITE ${CMAKE_BINARY_DIR}/repack.sh
      "
      tmpdir=repack.$$
      rm -rf $tmpdir && mkdir repack.$$
      tar -C $tmpdir -xf $1
      tar -C $tmpdir -cjf $1 .
      rm -rf $tmpdir"
    )
    add_custom_target(repack
        COMMAND bash repack.sh ${TARBALL}
        COMMENT "Repacking ${TARBALL} using external tar(1)"
    )
else ()
    add_custom_target(repack
        COMMAND bash :
        COMMENT "Cannot repack (no tar found)"
    )
endif ()


add_custom_target(dist
    COMMAND ${CMAKE_MAKE_PROGRAM} gitcheck
    COMMAND ${CMAKE_COMMAND}
        -D SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_SOURCE_DIR}/cmake/version_git.cmake
    COMMAND ${CMAKE_MAKE_PROGRAM} package_source
    COMMAND ${CMAKE_MAKE_PROGRAM} repack
    COMMAND ${CMAKE_COMMAND} -E  remove ${CMAKE_SOURCE_DIR}/git-version
)
