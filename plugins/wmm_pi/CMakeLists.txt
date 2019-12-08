##---------------------------------------------------------------------------
## Author:      nohal aka. Pavel Kalian
## Copyright:   
## License:     wxWidgets License
##---------------------------------------------------------------------------
 
# define minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 3.1.1)
CMAKE_POLICY(SET CMP0043 NEW)

PROJECT(wmm_pi)

SET(PACKAGE_NAME wmm_pi)
SET(VERBOSE_NAME Wmm)
SET(TITLE_NAME WMM)
SET(CPACK_PACKAGE_CONTACT "Pavel Kalian")

SET(VERSION_MAJOR "1")
SET(VERSION_MINOR "1")
SET(VERSION_PATCH "1")
SET(VERSION_DATE "01/03/2016")

IF( NOT CMAKE_BUILD_TYPE )
  SET( CMAKE_BUILD_TYPE Release CACHE STRING
       "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
       FORCE )
ENDIF( NOT CMAKE_BUILD_TYPE )

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)

MESSAGE (STATUS "Build type: ${CMAKE_BUILD_TYPE}")

#SET(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_MACOSX_RPATH "ON")

INCLUDE("cmake/PluginConfigure.cmake")

SET(SRC_WMM
    src/wmm_pi.h
    src/wmm_pi.cpp
    src/MagneticPlotMap.h
    src/MagneticPlotMap.cpp
    src/icons.h
    src/icons.cpp
    src/GeomagnetismHeader.h
    src/GeomagnetismLibrary.c
    src/EGM9615.h
    src/WmmUIDialog.h
    src/WmmUIDialog.cpp
    src/pi_TexFont.cpp
    src/pi_TexFont.h
    src/pi_ocpndc.cpp
    src/pi_ocpndc.h
    )

IF (QT_ANDROID )
    SET(SRC_WMM ${SRC_WMM} src/pi_shaders.h)
    SET(SRC_WMM ${SRC_WMM} src/pi_shaders.cpp)
ENDIF (QT_ANDROID )

SET(SRC_JSON
    src/json_defs.h
    src/jsonreader.h
    src/jsonval.h
    src/jsonwriter.h
    src/jsonreader.cpp
    src/jsonval.cpp
    src/jsonwriter.cpp
    )

ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_WMM} ${SRC_JSON})

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|AppleClang")
    target_compile_options(${PACKAGE_NAME} PRIVATE
        -Wno-unknown-pragmas
        -Wno-overloaded-virtual
    )
endif ()


if (TARGET ocpn::wxjson)
    target_link_libraries(${PACKAGE_NAME} ocpn::wxjson)
endif() 

if (TARGET ocpn::wxsvg)
    target_link_libraries(${PACKAGE_NAME} ocpn::wxsvg)
endif()

IF (QT_ANDROID )
    ADD_DEFINITIONS("-Wno-inconsistent-missing-override -Wno-potentially-evaluated-expression")
    
#    add_library(ocpn  SHARED IMPORTED)
#    set_property(TARGET ocpn PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/../../libopencpn.so)
#    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME}  ocpn )

    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME}  ${CMAKE_CURRENT_BINARY_DIR}/../../libopencpn.so )
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} GLESv2 EGL z)
    SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,-soname,lib${PACKAGE_NAME}.so ")
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-s")

ENDIF(QT_ANDROID)

IF( APPLE )
    target_compile_options(${PACKAGE_NAME} PRIVATE
        "-Wall -Wno-unused" "-fexceptions"
        "-Wno-overloaded-virtual"
        "-Wno-deprecated" "-Wno-deprecated-declarations"
        "-Wno-unknown-pragmas"
        "-D_WCHAR_H_CPLUSPLUS_98_CONFORMANCE_"
    )
ENDIF(APPLE)

target_link_libraries(${PACKAGE_NAME} ocpn::opencpn )

INCLUDE("cmake/PluginInstall.cmake")
INCLUDE("cmake/PluginLocalization.cmake")
INCLUDE("cmake/PluginPackage.cmake")
