##---------------------------------------------------------------------------
## Author:      Pavel Kalian (Based on the work of Sean D'Epagnier)
## Copyright:   2014
## License:     GPLv3+
##---------------------------------------------------------------------------

SET(PLUGIN_SOURCE_DIR .)

# This should be 2.8.0 to have FindGTK2 module
IF (COMMAND cmake_policy)
  CMAKE_POLICY(SET CMP0003 OLD)
  CMAKE_POLICY(SET CMP0005 OLD)
  CMAKE_POLICY(SET CMP0011 OLD)
ENDIF (COMMAND cmake_policy)

MESSAGE (STATUS "*** Staging to build ${PACKAGE_NAME} ***")

configure_file(cmake/version.h.in ${PROJECT_SOURCE_DIR}/src/version.h)
SET(PACKAGE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}" )

#SET(CMAKE_BUILD_TYPE Debug)
#SET(CMAKE_VERBOSE_MAKEFILE ON)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src)

# SET(PROFILING 1)

#  IF NOT DEBUGGING CFLAGS="-O2 -march=native"
IF(NOT MSVC)
 IF(PROFILING)
  ADD_DEFINITIONS( "-Wall -g -fprofile-arcs -ftest-coverage -fexceptions" )
 ELSE(PROFILING)
#  ADD_DEFINITIONS( "-Wall -g -fexceptions" )
 ADD_DEFINITIONS( "-Wall -Wno-unused-result -g -O2 -fexceptions" )
 ENDIF(PROFILING)

 IF(NOT APPLE)
  SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,-Bsymbolic")
 ELSE(NOT APPLE)
  SET(CMAKE_SHARED_LINKER_FLAGS "-Wl -undefined dynamic_lookup")
 ENDIF(NOT APPLE)

ENDIF(NOT MSVC)

# Add some definitions to satisfy MS
IF(MSVC)
    ADD_DEFINITIONS(-D__MSVC__)
    ADD_DEFINITIONS(-D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_DEPRECATE)
ENDIF(MSVC)

SET(wxWidgets_USE_LIBS base core net xml html adv)
SET(BUILD_SHARED_LIBS TRUE)

FIND_PACKAGE(wxWidgets REQUIRED)

IF(MSYS)
# this is just a hack. I think the bug is in FindwxWidgets.cmake
STRING( REGEX REPLACE "/usr/local" "\\\\;C:/MinGW/msys/1.0/usr/local" wxWidgets_INCLUDE_DIRS ${wxWidgets_INCLUDE_DIRS} )
ENDIF(MSYS)

INCLUDE(${wxWidgets_USE_FILE})

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

SET(BUILD_SHARED_LIBS TRUE)

FIND_PACKAGE(Gettext REQUIRED)

