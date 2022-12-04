MACRO (COMMIT_ID RESULT)
    # Get the latest abbreviated commit hash of the working branch
    execute_process(
      COMMAND git log -1 --format=%h
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      OUTPUT_VARIABLE COMMIT_HASH
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    SET(${RESULT} ${COMMIT_HASH})
ENDMACRO (COMMIT_ID)

MACRO (BUILD_NUM RESULT)
    # Get the current Travis/CircleCI build number, possibly ""
    execute_process(
      COMMAND /usr/bin/sh -c "echo $CIRCLE_BUILD_NUM$TRAVIS_BUILD_NUM$BUILD_NUMBER"
      OUTPUT_VARIABLE _BUILD_NUM
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
SET(${RESULT} ${_BUILD_NUM})
ENDMACRO (BUILD_NUM)
