##---------------------------------------------------------------------------
## Author:      Jean-Eudes Onfray
## Copyright:   
## License:     wxWidgets License
##---------------------------------------------------------------------------


# define minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)
# This should be 2.8.0 to have FindGTK2 module
IF (COMMAND cmake_policy)
  CMAKE_POLICY(SET CMP0003 OLD)
  CMAKE_POLICY(SET CMP0005 OLD)
  CMAKE_POLICY(SET CMP0011 OLD)
ENDIF (COMMAND cmake_policy)

PROJECT(vdr_pi)

SET(PACKAGE_NAME vdr_pi)
SET(PLUGIN_SOURCE_DIR .)
MESSAGE (STATUS "*** Building ${PACKAGE_NAME} ***")

#SET(CMAKE_BUILD_TYPE Debug)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)

# require proper c++
#ADD_DEFINITIONS( "-Wall -ansi -pedantic -Wno-variadic-macros" )
#TODO: Should we use  -fno-stack-protector
#  IF NOT DEBUGGING CFLAGS="-O2 -march=native"
IF(NOT WIN32)
 ADD_DEFINITIONS( "-Wall -g -fexceptions -fvisibility=hidden" )

 IF(NOT APPLE)
  SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,-Bsymbolic")
 ELSE(NOT APPLE)
  SET(CMAKE_SHARED_LINKER_FLAGS "-Wl")
 ENDIF(NOT APPLE)

ENDIF(NOT WIN32)

#SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,-Bsymbolic")

# Add some definitions to satisfy MS
IF(WIN32)
    ADD_DEFINITIONS(-D__MSVC__)
    ADD_DEFINITIONS(-D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_DEPRECATE)
ENDIF(WIN32)


#SET(wxWidgets_USE_LIBS base core net xml html adv aui)
#SET(BUILD_SHARED_LIBS TRUE)
#SET( wxWidgets_USE_DEBUG OFF)
#SET( wxWidgets_USE_UNICODE ON)
#FIND_PACKAGE(wxWidgets REQUIRED)

INCLUDE(${wxWidgets_USE_FILE})

FIND_PACKAGE(Gettext REQUIRED)

# For convenience we define the sources as a variable. You can add
# header files and cpp/c files and CMake will sort them out


SET(SRC_VDR
            src/icons.h
            src/icons.cpp
            src/vdr_pi.h
            src/vdr_pi.cpp
 	)

ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_VDR} )

IF(WIN32)
  SET(OPENCPN_IMPORT_LIB "../../${CMAKE_CFG_INTDIR}/${PARENT}")
  TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${OPENCPN_IMPORT_LIB} )
  ADD_DEPENDENCIES(${PACKAGE_NAME} ${PARENT})
ENDIF(WIN32)

TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${wxWidgets_LIBRARIES} )

IF(UNIX)
INSTALL(TARGETS ${PACKAGE_NAME} RUNTIME LIBRARY DESTINATION ${PREFIX_PLUGINS})
ENDIF(UNIX)

IF(WIN32)
INSTALL(TARGETS ${PACKAGE_NAME} RUNTIME DESTINATION "plugins")
ENDIF(WIN32)

# find src/ -name \*.cpp -or -name \*.h -or -name \*.hpp >po/POTFILES.in
FIND_PROGRAM(GETTEXT_XGETTEXT_EXECUTABLE xgettext)
IF (GETTEXT_XGETTEXT_EXECUTABLE)
ADD_CUSTOM_COMMAND(
   OUTPUT po/${PACKAGE_NAME}.pot.dummy
   COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE} --force-po --package-name=${PACKAGE_NAME} --package-version="${PACKAGE_VERSION}" --output=po/${PACKAGE_NAME}.pot  --keyword=_ --width=80 --files-from=${CMAKE_CURRENT_SOURCE_DIR}/po/POTFILES.in
   DEPENDS po/POTFILES.in po/${PACKAGE_NAME}.pot
   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
   COMMENT "vdr-pot-update [${PACKAGE_NAME}]: Generated pot file."
)
ADD_CUSTOM_TARGET(vdr-pot-update COMMENT "vdr-pot-update: Done." DEPENDS po/${PACKAGE_NAME}.pot.dummy)

ENDIF(GETTEXT_XGETTEXT_EXECUTABLE )

MACRO(GETTEXT_UPDATE_PO _potFile)
   SET(_poFiles ${_potFile})
   GET_FILENAME_COMPONENT(_absPotFile ${_potFile} ABSOLUTE)

   FOREACH (_currentPoFile ${ARGN})
      GET_FILENAME_COMPONENT(_absFile ${_currentPoFile} ABSOLUTE)
      GET_FILENAME_COMPONENT(_poBasename ${_absFile} NAME_WE)

      ADD_CUSTOM_COMMAND(
         OUTPUT ${_absFile}.dummy
         COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --width=80 --strict --quiet --update --backup=none --no-location -s ${_absFile} ${_absPotFile}
         DEPENDS ${_absPotFile} ${_absFile}
         COMMENT "vdr-po-update [${_poBasename}]: Updated po file."
      )

      SET(_poFiles ${_poFiles} ${_absFile}.dummy)

   ENDFOREACH (_currentPoFile )

   ADD_CUSTOM_TARGET(vdr-po-update COMMENT "vdr-po-update: Done." DEPENDS ${_poFiles})
ENDMACRO(GETTEXT_UPDATE_PO)

IF (GETTEXT_MSGMERGE_EXECUTABLE)
FILE(GLOB PACKAGE_PO_FILES po/*.po)
GETTEXT_UPDATE_PO(po/${PACKAGE_NAME}.pot ${PACKAGE_PO_FILES})
ENDIF(GETTEXT_MSGMERGE_EXECUTABLE)

SET(_gmoFiles)
MACRO(GETTEXT_BUILD_MO)
   FOREACH (_poFile ${ARGN})
      GET_FILENAME_COMPONENT(_absFile ${_poFile} ABSOLUTE)
      GET_FILENAME_COMPONENT(_poBasename ${_absFile} NAME_WE)
      SET(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${_poBasename}.mo)

      ADD_CUSTOM_COMMAND(
         OUTPUT ${_gmoFile}
         COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --check -o ${_gmoFile} ${_absFile}
         DEPENDS ${_absFile}
         COMMENT "vdr-i18n [${_poBasename}]: Created mo file."
      )

      INSTALL(FILES ${_gmoFile} DESTINATION ${PREFIX_DATA}/locale/${_poBasename}/LC_MESSAGES RENAME opencpn-${PACKAGE_NAME}.mo )
      SET(_gmoFiles ${_gmoFiles} ${_gmoFile})
   ENDFOREACH (_poFile )
ENDMACRO(GETTEXT_BUILD_MO)

if(GETTEXT_MSGFMT_EXECUTABLE)
   FILE(GLOB PACKAGE_PO_FILES po/*.po)
   GETTEXT_BUILD_MO(${PACKAGE_PO_FILES})
   ADD_CUSTOM_TARGET(vdr-i18n COMMENT "vdr-i18n: Done." DEPENDS ${_gmoFiles})
   #ADD_DEPENDENCIES(vdr-i18n ${PACKAGE_PO_FILES})
   ADD_DEPENDENCIES(${PACKAGE_NAME} vdr-i18n)
ENDIF(GETTEXT_MSGFMT_EXECUTABLE)

