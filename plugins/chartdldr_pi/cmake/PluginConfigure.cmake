##---------------------------------------------------------------------------
## Author:      Pavel Kalian (Based on the work of Sean D'Epagnier)
## Copyright:   2014
## License:     GPLv3+
##---------------------------------------------------------------------------

SET(PLUGIN_SOURCE_DIR .)

# This should be 2.8.0 to have FindGTK2 module

MESSAGE (STATUS "*** Staging to build ${PACKAGE_NAME} ***")

#configure_file(cmake/version.h.in ${PROJECT_SOURCE_DIR}/src/version.h)
#  Do the version.h configuration into the build output directory,
#  thereby allowing building from a read-only source tree.
IF(NOT SKIP_VERSION_CONFIG)
    SET(BUILD_INCLUDE_PATH ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY})
    configure_file(cmake/version.h.in ${BUILD_INCLUDE_PATH}/include/version.h)
    configure_file(cmake/wxWTranslateCatalog.h.in ${BUILD_INCLUDE_PATH}/include/wxWTranslateCatalog.h)
    INCLUDE_DIRECTORIES(${BUILD_INCLUDE_PATH}/include)
ENDIF(NOT SKIP_VERSION_CONFIG)

SET(PACKAGE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}" )

#SET(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_VERBOSE_MAKEFILE ON)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src)

# SET(PROFILING 1)

#  IF NOT DEBUGGING CFLAGS="-O2 -march=native"
IF(NOT MSVC)
 ADD_COMPILE_OPTIONS( "-fvisibility=hidden" )
 IF(PROFILING)
     ADD_COMPILE_OPTIONS(
         "-Wall" "-g" "-fprofile-arcs" "-ftest-coverage" "-fexceptions" )
 ELSE()
   #  ADD_DEFINITIONS( "-Wall -g -fexceptions" )
   ADD_COMPILE_OPTIONS(
       "-Wall" "-Wno-unused-result" "-g" "-O2" "-fexceptions")
 ENDIF()

 IF(CMAKE_SYSTEM_NAME MATCHES ".*Linux")
  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-Bsymbolic")
 ENDIF(CMAKE_SYSTEM_NAME MATCHES ".*Linux")
 IF(APPLE)
  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl -undefined dynamic_lookup")
 ENDIF(APPLE)

ENDIF(NOT MSVC)

# Add some definitions to satisfy MS
IF(MSVC)
    ADD_DEFINITIONS(-D__MSVC__)
    ADD_DEFINITIONS(-D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_DEPRECATE)
    ADD_DEFINITIONS(-DMAKING_PLUGIN)
ENDIF(MSVC)

SET_PROPERTY(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)
SET(BUILD_SHARED_LIBS "ON")


#  ANDROID is a cross-build, so the native FIND_PACKAGE(wxWidgets...) and wxWidgets_USE_FILE is not useful.
IF(NOT ANDROID)
IF(NOT DEFINED wxWidgets_USE_FILE)
  SET(wxWidgets_USE_LIBS base core net xml html adv)
  SET(BUILD_SHARED_LIBS TRUE)
  FIND_PACKAGE(wxWidgets REQUIRED)
ENDIF(NOT DEFINED wxWidgets_USE_FILE)

  INCLUDE(${wxWidgets_USE_FILE})
ENDIF(NOT ANDROID)

IF(MSYS)
# this is just a hack. I think the bug is in FindwxWidgets.cmake
STRING( REGEX REPLACE "/usr/local" "\\\\;C:/MinGW/msys/1.0/usr/local" wxWidgets_INCLUDE_DIRS ${wxWidgets_INCLUDE_DIRS} )
ENDIF(MSYS)

#  ANDROID is a cross-build, so the native FIND_PACKAGE(OpenGL) is not useful.
#
IF (NOT ANDROID )
FIND_PACKAGE(OpenGL)
IF(OPENGL_GLU_FOUND)

    SET(wxWidgets_USE_LIBS ${wxWidgets_USE_LIBS} gl)
    INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR})

    MESSAGE (STATUS "Found OpenGL..." )
    MESSAGE (STATUS "    Lib: " ${OPENGL_LIBRARIES})
    MESSAGE (STATUS "    Include: " ${OPENGL_INCLUDE_DIR})
    ADD_DEFINITIONS(-DocpnUSE_GL)
ELSE(OPENGL_GLU_FOUND)
    MESSAGE (STATUS "OpenGL not found..." )
ENDIF(OPENGL_GLU_FOUND)
ENDIF(NOT ANDROID)

SET(BUILD_SHARED_LIBS TRUE)

FIND_PACKAGE(Gettext REQUIRED)
