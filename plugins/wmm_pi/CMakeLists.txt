##---------------------------------------------------------------------------
## Author:      nohal aka. Pavel Kalian
## Copyright:   
## License:     wxWidgets License
##---------------------------------------------------------------------------
 
# define minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)

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

MESSAGE (STATUS "Build type: ${CMAKE_BUILD_TYPE}")

INCLUDE("cmake/PluginConfigure.cmake")

SET(SRC_WMM
            src/wmm_pi.h
            src/wmm_pi.cpp
            src/MagneticPlotMap.h
            src/MagneticPlotMap.cpp
            src/icons.h
            src/icons.cpp
	    src/WMMHeader.h
	    src/WMM_SubLibrary.c
	    src/WmmUIDialog.h
	    src/WmmUIDialog.cpp
	    src/WMM_COF.h
            src/TexFont.cpp
            src/TexFont.h
	)

IF(WIN32)
SET(SRC_JSON
	    src/json_defs.h
	    src/jsonreader.h
	    src/jsonval.h
	    src/jsonwriter.h
	    src/jsonreader.cpp
	    src/jsonval.cpp
	    src/jsonwriter.cpp
        )
ENDIF(WIN32)

ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_WMM} ${SRC_JSON})

TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${wxWidgets_LIBRARIES} )

IF (QT_ANDROID )
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${OCPN_Core_LIBRARIES} )
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} "-lgnustl_shared")
ENDIF(QT_ANDROID)

INCLUDE("cmake/PluginInstall.cmake")
INCLUDE("cmake/PluginLocalization.cmake")
INCLUDE("cmake/PluginPackage.cmake")
