##---------------------------------------------------------------------------
## Author:      bdbcat aka. dsr (Dave Register)
## Copyright:   
## License:     wxWidgets License
## Update:      sethdart (Jean-Eudes Onfray)
##              with parts from balp (Anders Arnholm)
## DEB config from: antonm - Anton Martchukov <anton@martchukov.com>
##---------------------------------------------------------------------------
 
#TODO:
# - cross platform compiling
# - test with Win & OSX
# USE_GLU_TESS
# USE_GLU_DLL
# I also find it deficient in some areas. For instance, I cannot make it output a VS project with certain compile switches set as desired, namely /MT vs /MD. This means I must manually set this and other compiler options after the CMake step. Sometimes I forget. Grrr...
# set /MT for Release build, and /MTd for debug.

# define minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)
# This should be 2.8.0 to have FindGTK2 module
IF (COMMAND cmake_policy)
  CMAKE_POLICY(SET CMP0003 OLD)
  CMAKE_POLICY(SET CMP0005 OLD)
  CMAKE_POLICY(SET CMP0011 OLD)
ENDIF (COMMAND cmake_policy)

PROJECT(OpenCPN)

#SET(CMAKE_VERBOSE_MAKEFILE ON)

SET(PACKAGE_NAME opencpn)
INCLUDE( ${CMAKE_SOURCE_DIR}/VERSION.cmake )
SET(PACKAGE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )


MESSAGE (STATUS "*** Building ${PACKAGE_NAME} ${PACKAGE_VERSION} ***")

SET(wxWidgets_USE_LIBS base core net xml html adv aui)

# Find wxWidgets here, and the setting get inherited by all plugins.
# These options can be used to set the linux widgets build type
SET( wxWidgets_USE_DEBUG OFF)
SET( wxWidgets_USE_UNICODE ON)
SET( wxWidgets_USE_UNIVERSAL OFF)
SET( wxWidgets_USE_STATIC OFF)

FIND_PACKAGE(wxWidgets REQUIRED)
INCLUDE(${wxWidgets_USE_FILE})
#INCLUDE_DIRECTORIES(${wxWidgets_INCLUDE_DIRS})
#ADD_DEFINITIONS(${wxWidgets_DEFINITIONS})

#  Search for wxWidgets 2.9
STRING(REGEX MATCHALL "2.9" wx_Version "${wxWidgets_INCLUDE_DIRS}")

MESSAGE (STATUS "")



IF(NOT SKIP_PLUGINS)
 add_subdirectory (plugins)
ENDIF(NOT SKIP_PLUGINS)

MESSAGE (STATUS "")
MESSAGE (STATUS "*** Building ${PACKAGE_NAME}  ***")

#SET(CMAKE_BUILD_TYPE Debug)

IF (PREFIX)
	SET(CMAKE_INSTALL_PREFIX ${PREFIX})
ENDIF (PREFIX)
SET(PREFIX_BIN bin)
SET(PREFIX_INCLUDE include)
SET(PREFIX_DATA share)
SET(PREFIX_PKGDATA ${PREFIX_DATA}/${PACKAGE_NAME})
IF (NOT ARCH)
	SET(ARCH "i386")
ENDIF (NOT ARCH)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)

# require proper c++
#ADD_DEFINITIONS( "-Wall -ansi -pedantic -Wno-variadic-macros" )
#TODO: Should we use  -fno-stack-protector
#  IF NOT DEBUGGING CFLAGS="-O2 -march=native"
IF(NOT WIN32)
  ADD_DEFINITIONS( "-Wall -g -fexceptions -rdynamic" )
  ADD_DEFINITIONS( "-O3 -fno-strict-aliasing")
ENDIF(NOT WIN32)

# Add some definitions to satisfy MS
ADD_DEFINITIONS(-DUSE_GLU_TESS)
IF(WIN32)
    ADD_DEFINITIONS(-D__MSVC__)
    ADD_DEFINITIONS(-D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_DEPRECATE)
ENDIF(WIN32)


# Location where cmake first looks for cmake modules.
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/build)

# Let cmake find additional modules private to opencpn
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/")

FIND_PACKAGE(GTK2 2.6)
IF (GTK2_FOUND)
    INCLUDE_DIRECTORIES(${GTK2_INCLUDE_DIRS})
    ADD_DEFINITIONS(-DocpnUSE_GTK_OPTIMIZE)
    SET(EXTRA_LIBS ${EXTRA_LIBS} ${GTK2_LIBRARIES})
ENDIF (GTK2_FOUND)

FIND_PACKAGE(OpenGL REQUIRED)
# OPENGL_GLU_FOUND
INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR})

FIND_PACKAGE(Gettext REQUIRED)

IF(UNIX)
    FIND_PACKAGE(BZip2 REQUIRED)
    INCLUDE_DIRECTORIES(${BZIP2_INCLUDE_DIR})
    FIND_PACKAGE(ZLIB REQUIRED)
    INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
ENDIF(UNIX)

# For convenience we define the sources as a variable. You can add 
# header files and cpp/c files and CMake will sort them out

SET(SRC_NMEA0183
		include/nmea.h
		src/nmea.cpp
		src/nmea0183/LatLong.hpp
		src/nmea0183/latlong.cpp
		src/nmea0183/long.cpp
		src/nmea0183/nmea0183.cpp
		src/nmea0183/nmea0183.hpp
		src/nmea0183/Response.hpp
		src/nmea0183/response.cpp
		src/nmea0183/RMB.hpp
		src/nmea0183/rmb.cpp
		src/nmea0183/Sentence.hpp
		src/nmea0183/sentence.cpp
		src/nmea0183/talkerid.cpp
		src/nmea0183/RMC.HPP
		src/nmea0183/rmc.cpp
		src/nmea0183/hexvalue.cpp
		src/nmea0183/lat.cpp
		src/nmea0183/expid.cpp
		src/nmea0183/wpl.hpp
		src/nmea0183/wpl.cpp
		src/nmea0183/rte.hpp
		src/nmea0183/rte.cpp
		src/nmea0183/hdt.hpp
		src/nmea0183/hdt.cpp
		src/nmea0183/hdg.hpp
		src/nmea0183/hdg.cpp
		src/nmea0183/hdm.hpp
		src/nmea0183/hdm.cpp
		src/nmea0183/gll.hpp
		src/nmea0183/gll.cpp
		src/nmea0183/vtg.hpp
		src/nmea0183/vtg.cpp
		src/nmea0183/gga.hpp
		src/nmea0183/gga.cpp
		src/nmea0183/gsv.hpp
		src/nmea0183/gsv.cpp
	)
ADD_LIBRARY(NMEA0183 ${SRC_NMEA0183})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/nmea0183)


IF(NOT WIN32)
OPTION (USE_GPSD "Enable GPSD Library" ON) 
IF (USE_GPSD)
      message (STATUS "GPSD Library support: enabled")

 #     FIND_PACKAGE(gps QUIET)
     FIND_PACKAGE(gps)
      IF (GPS_FOUND)
            message (STATUS "Building with libgps includes")
            INCLUDE_DIRECTORIES(${GPS_INCLUDE_DIR})
            ADD_DEFINITIONS(${GPS_DEFINITIONS})
            ADD_DEFINITIONS(-DBUILD_WITH_LIBGPS)
#            message (STATUS "GPS library is ${GPS_LIBRARY}")
#            SET(EXTRA_LIBS ${EXTRA_LIBS} ${GPS_LIBRARY})
      ELSE (GPS_FOUND)
            message (STATUS "Gps library not found")
      ENDIF (GPS_FOUND)
ENDIF (USE_GPSD)
ENDIF (NOT WIN32)




#FILE(GLOB_RECURSE SRCS *.cpp *.c)
SET(HDRS
                include/ocpn_types.h
		include/chart1.h
		include/bbox.h
		include/ocpn_pixel.h
		include/chartdb.h
		include/chartdbs.h
		include/chartimg.h
		include/chcanv.h
		include/concanv.h
		include/cutil.h
		include/georef.h
		include/navutil.h
		include/routeman.h
		include/routemanagerdialog.h
		include/routeprop.h
		include/statwin.h
		include/tcmgr.h
		include/thumbwin.h
		include/options.h
		include/wvschart.h
		include/about.h
		include/ais.h
                include/pluginmanager.h
                include/ocpn_plugin.h
		include/tinystr.h
		include/tinyxml.h
		include/gpxdocument.h
)

SET(SRCS 
		src/chart1.cpp
		src/bbox.cpp
		src/ocpn_pixel.cpp
		src/chartdb.cpp
		src/chartdbs.cpp
		src/chartimg.cpp
		src/chcanv.cpp
		src/concanv.cpp
		src/cutil.c
		src/georef.c
		src/navutil.cpp
		src/routeman.cpp
		src/routemanagerdialog.cpp
		src/routeprop.cpp
		src/statwin.cpp
		src/tcmgr.cpp
		src/thumbwin.cpp
		src/options.cpp
		src/wvschart.cpp
		src/about.cpp
		src/ais.cpp
                src/pluginmanager.cpp
		src/tinyxml.cpp
		src/tinyxmlerror.cpp
		src/tinyxmlparser.cpp
		src/tinystr.cpp
		src/gpxdocument.cpp
    )

IF(wx_Version MATCHES "2.9")
ELSE(wx_Version MATCHES "2.9")
  SET(HDRS ${HDRS} include/scrollingdialog.h )
  SET(SRCS ${SRCS} src/scrollingdialog.cpp)
ENDIF(wx_Version MATCHES "2.9")

SET(PACKAGE_HEADERS ""
#	include/ais.h
)

OPTION (USE_S57 "Enable S57 ENC support" ON)
IF (USE_S57)
	message (STATUS "S57 ENC support: enabled")
	ADD_DEFINITIONS(-DUSE_S57)

	SET(SRC_S57ENC
		include/s52plib.h
		include/s52utils.h
		include/s57chart.h
		include/mygeom.h
		src/s52cnsy.cpp
		src/s52plib.cpp
		src/s52utils.cpp
		src/s57chart.cpp
		src/cm93.cpp
		src/mygeom.cpp
		include/cm93.h
		src/tri.c
		src/myiso8211/ddffielddefn.cpp
		src/myiso8211/ddfmodule.cpp
		src/myiso8211/ddfrecord.cpp
		src/myiso8211/ddfsubfielddefn.cpp
		src/myiso8211/ddffield.cpp
		src/myiso8211/ddfutils.cpp
		src/mygdal/cpl_config.h
		src/mygdal/gdal_priv.h
		src/mygdal/cpl_conv.h
		src/mygdal/iso8211.h
		src/mygdal/cpl_csv.h
		src/mygdal/ogr_api.h
		src/mygdal/cpl_error.h
		src/mygdal/ogr_attrind.h
		src/mygdal/cpl_list.h
		src/mygdal/ogr_core.h
		src/mygdal/cpl_minixml.h
		src/mygdal/ogr_feature.h
		src/mygdal/cpl_multiproc.h
		src/mygdal/ogr_geometry.h
		src/mygdal/cpl_odbc.h
		src/mygdal/ogr_p.h
		src/mygdal/cpl_port.h
		src/mygdal/ogr_s57.h
		src/mygdal/cpl_string.h
		src/mygdal/ogr_spatialref.h
		src/mygdal/cpl_vsi.h
		src/mygdal/ogrsf_frmts.h
		src/mygdal/gdal.h
		src/mygdal/s57.h
		src/mygdal/gdal_frmts.h
		src/mygdal/ddfrecordindex.cpp
		src/mygdal/cpl_conv.cpp
		src/mygdal/cpl_csv.cpp
		src/mygdal/cpl_dir.cpp
		src/mygdal/cpl_error.cpp
		src/mygdal/cpl_findfile.cpp
		src/mygdal/cpl_minixml.cpp
		src/mygdal/cpl_path.cpp
		src/mygdal/cpl_string.cpp
		src/mygdal/cpl_vsisimple.cpp
		src/mygdal/cplgetsymbol.cpp
		src/mygdal/ograssemblepolygon.cpp
		src/mygdal/ogrcurve.cpp
		src/mygdal/ogrfeature.cpp
		src/mygdal/ogrfeaturedefn.cpp
		src/mygdal/ogrfielddefn.cpp
		src/mygdal/ogrgeometry.cpp 
		src/mygdal/ogrgeometrycollection.cpp
		src/mygdal/ogrgeometryfactory.cpp
		src/mygdal/ogrlinearring.cpp
		src/mygdal/ogrlinestring.cpp
		src/mygdal/ogrmultilinestring.cpp
		src/mygdal/ogrmultipoint.cpp
		src/mygdal/ogrmultipolygon.cpp
		src/mygdal/ogrpoint.cpp
		src/mygdal/ogrpolygon.cpp
		src/mygdal/ogrs57datasource.cpp
		src/mygdal/ogrlayer.cpp
		src/mygdal/ogrs57layer.cpp
		src/mygdal/ogrutils.cpp
		src/mygdal/s57classregistrar.cpp
		src/mygdal/s57featuredefns.cpp
		src/mygdal/s57reader.cpp
		src/mygdal/gdal_misc.cpp
	)
	ADD_LIBRARY(S57ENC ${SRC_S57ENC})
	INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/mygdal)
	SET(EXTRA_LIBS ${EXTRA_LIBS} S57ENC)
ELSE (USE_S57)
	message (STATUS "S57 ENC support: disabled")
ENDIF (USE_S57)

OPTION (USE_GARMINHOST "Enable Garmin Host Mode support" ON)
IF (USE_GARMINHOST)
	message (STATUS "Garmin Host Mode support: enabled")
	ADD_DEFINITIONS(-DUSE_GARMINHOST)

	SET(SRC_GARMINHOST
                  src/garmin/jeeps/gpsapp.c
                  src/garmin/jeeps/gpscom.c
	          src/garmin/jeeps/gpsmath.c
                  src/garmin/jeeps/gpsmem.c
	          src/garmin/jeeps/gpsprot.c
                  src/garmin/jeeps/gpsread.c
	          src/garmin/jeeps/gpsdevice.c
                  src/garmin/jeeps/gpsdevice_ser.c
                  src/garmin/jeeps/gpsdevice_usb.c
                  src/garmin/jeeps/gpsrqst.c
                  src/garmin/jeeps/gpssend.c
                  src/garmin/jeeps/gpsserial.c
                  src/garmin/jeeps/gpsutil.c
                  src/garmin/jeeps/gpsusbread.c
                  src/garmin/jeeps/gpsusbsend.c
                  src/garmin/jeeps/gpsusbcommon.c
                  src/garmin/jeeps/garmin_wrapper_utils.c
                  src/garmin/jeeps/garmin_wrapper.cpp
                  src/garmin/jeeps/garmin_gps.h
                  src/garmin/jeeps/garmin_wrapper.h
                  src/garmin/jeeps/gpsapp.h
                  src/garmin/jeeps/gpsdatum.h
                  src/garmin/jeeps/gpsfmt.h
                  src/garmin/jeeps/gpsmath.h
                  src/garmin/jeeps/gpsport.h
                  src/garmin/jeeps/gpsprot.h
                  src/garmin/jeeps/gpsrqst.h
                  src/garmin/jeeps/gpsserial.h
                  src/garmin/jeeps/gpsusbint.h
                  src/garmin/jeeps/garminusb.h
                  src/garmin/jeeps/garmin_wrapper_utils.h
                  src/garmin/jeeps/gpscom.h
                  src/garmin/jeeps/gpsdevice.h
                  src/garmin/jeeps/gpsinput.h
                  src/garmin/jeeps/gpsmem.h
                  src/garmin/jeeps/gpsproj.h
                  src/garmin/jeeps/gpsread.h
                  src/garmin/jeeps/gpssend.h
                  src/garmin/jeeps/gpsusbcommon.h
                  src/garmin/jeeps/gpsutil.h
                  
	)
        IF(WIN32)
           SET(SRC_GARMINHOST ${SRC_GARMINHOST} src/garmin/jeeps/gpsusbwin.c)
        ENDIF(WIN32)

	ADD_LIBRARY(GARMINHOST ${SRC_GARMINHOST})
	INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/garmin/jeeps)
	SET(EXTRA_LIBS ${EXTRA_LIBS} GARMINHOST)
        IF(WIN32)
           ADD_DEFINITIONS(-D__WIN32__)
        ENDIF(WIN32)
ELSE (USE_GARMINHOST)
	message (STATUS "Garmin Host Mode support: disabled")
ENDIF (USE_GARMINHOST)

#TODO
#dnl
#dnl Use OpenGL tesselator or Internal tesselator
#dnl
#tess_internal=false
#tess_glu=true
#AC_ARG_WITH(tess_internal,
#          [[  --with-tess-internal    use Internal Polygon Tesselator]],
#          [tess_internal=true]
#           )
#
#if [[ "$tess_internal" = "false" ]] ; then
#        dnl  Look for and qualify an external GLU library
#        echo "checking for useable OpenGLU Library"
#        AC_CHECK_LIB(GLU, gluNewTess, true, dnl here true is just a nop
#           AC_MSG_ERROR([*** libGLU not found.  Run configure using --with-tess-internal.]))
#
#        GL_LIBS="-lGL -lGLU"
#        AC_SUBST(GL_LIBS)
#
#        GL_CFLAGS="-DUSE_GLU_TESS"
#        AC_SUBST(GL_CFLAGS)
#fi


# If we build for windows systems, we also include the resource file
# containing the manifest, icon and other resources
IF(WIN32)
  SET(SRCS ${SRCS} src/opencpn.rc)
ENDIF(WIN32)
IF(APPLE)
  INCLUDE_DIRECTORIES(/usr/X11/include /usr/X11/include/GL)
  SET(CMAKE_C_FLAGS_DEBUG "-O2 -arch ${ARCH}")
  SET(CMAKE_C_FLAGS_MINSIZEREL "-O2 -arch ${ARCH}")
  SET(CMAKE_C_FLAGS_RELEASE "-O2 -arch ${ARCH}")
  SET(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -arch ${ARCH}")
  SET(CMAKE_CXX_FLAGS_DEBUG "-O2 -arch ${ARCH}")
  SET(CMAKE_CXX_FLAGS_MINSIZEREL "-O2 -arch ${ARCH}")
  SET(CMAKE_CXX_FLAGS_RELEASE "-O2 -arch ${ARCH}")
  SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -arch ${ARCH}")

  SET(SRCS ${SRCS}
    include/macsercomm.h
    src/macsercomm.cpp
    include/macutils.h
    src/macutils.c
    )
ENDIF(APPLE)


ADD_EXECUTABLE(${PACKAGE_NAME} WIN32 MACOSX_BUNDLE ${HDRS} ${SRCS})

IF(WIN32)
    IF(MSVC)
        SET(CMAKE_C_FLAGS_DEBUG               "/MDd /Ob0 /Od  /D_DEBUG  /Zi /RTC1" )
        SET(CMAKE_C_FLAGS_MINSIZEREL          "/MD  /O1  /Ob1 /D NDEBUG")
        SET(CMAKE_C_FLAGS_RELEASE             "/MD  /O2  /Ob2 /D NDEBUG")
        SET(CMAKE_C_FLAGS_RELWITHDEBINFO      "/MD  /O2  /Ob1 /D NDEBUG /Zi")
        SET(CMAKE_CXX_FLAGS_DEBUG             "/MDd /Ob0 /Od  /D_DEBUG  /Zi /RTC1")
        SET(CMAKE_CXX_FLAGS_MINSIZEREL        "/MD  /O1  /Ob1 /D NDEBUG")
        SET(CMAKE_CXX_FLAGS_RELEASE           "/MD  /O2  /Ob2 /D NDEBUG")
        SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO    "/MD  /O2  /Ob1 /D NDEBUG" /Zi )
    ENDIF(MSVC)
#    SET_TARGET_PROPERTIES(${PACKAGE_NAME} PROPERTIES COMPILE_FLAGS /Zp44)
#    SET_TARGET_PROPERTIES(${PACKAGE_NAME} PROPERTIES LINK_FLAGS /MANIFEST:NO)
ENDIF(WIN32)

TARGET_LINK_LIBRARIES(${PACKAGE_NAME}
      ${wxWidgets_LIBRARIES}
      NMEA0183
      ${EXTRA_LIBS}
)

IF(WIN32)
TARGET_LINK_LIBRARIES(${PACKAGE_NAME}
	setupapi.lib
	gdiplus.lib
	glu32.lib
)
ENDIF(WIN32)

IF(UNIX)
TARGET_LINK_LIBRARIES(${PACKAGE_NAME}
      ${OPENGL_LIBRARIES}
      ${GETTEXT_LIBRARY}
      ${BZIP2_LIBRARIES}
      ${ZLIB_LIBRARY}
)
ENDIF(UNIX)

#   Certain older Cmake FindGTK2 modules ( e.g. cmake-2.8.0-2) do not yield all of the required link libraries
#   So, add them manually.
IF(UNIX)
TARGET_LINK_LIBRARIES(${PACKAGE_NAME}
      dl
      gobject-2.0
)
ENDIF(UNIX)



IF(NOT APPLE)

IF(WIN32)
INSTALL(TARGETS ${PACKAGE_NAME} RUNTIME DESTINATION ".")
ELSE(WIN32)
INSTALL(TARGETS ${PACKAGE_NAME} RUNTIME DESTINATION ${PREFIX_BIN})
ENDIF(WIN32)

#ADD_CUSTOM_TARGET(dummy ALL DEPENDS ${PACKAGE_NAME} i18n)

#INSTALL(FILES ${PACKAGE_HEADERS} DESTINATION ${PREFIX_INCLUDE})

IF(WIN32)
SET(PREFIX_PKGDATA ".")
ENDIF(WIN32)

SET(wvsData
        data/wvsdata/readmewvs.txt
        data/wvsdata/wvs1.dat
        data/wvsdata/wvs43.dat
   )
INSTALL(FILES ${wvsData} DESTINATION ${PREFIX_PKGDATA}/wvsdata )

IF (USE_S57)
	SET(s57data
		data/s57data/attdecode.csv
		data/s57data/S52RAZDS.RLE
		data/s57data/s57attributes.csv
		data/s57data/s57expectedinput.csv
		data/s57data/s57objectclasses.csv
	   )
	INSTALL(FILES ${s57data} DESTINATION ${PREFIX_PKGDATA}/s57data )
ENDIF (USE_S57)

SET(tcData
        data/tcdata/HARMONIC
        data/tcdata/HARMONIC.IDX
        data/tcdata/README.harmonics
   )
INSTALL(FILES ${tcData} DESTINATION ${PREFIX_PKGDATA}/tcdata )

INSTALL(FILES data/license.txt DESTINATION ${PREFIX_PKGDATA} )

INSTALL(DIRECTORY data/sounds/ DESTINATION ${PREFIX_PKGDATA}/sounds )

INSTALL(DIRECTORY data/doc/ DESTINATION ${PREFIX_PKGDATA}/doc )

IF(UNIX)
  INSTALL(FILES data/opencpn.png DESTINATION ${PREFIX_DATA}/icons/hicolor/48x48/apps )
  INSTALL(FILES src/bitmaps/other_svg_src/opencpn.svg DESTINATION ${PREFIX_DATA}/icons/hicolor/scalable/apps )
  INSTALL(FILES data/opencpn.desktop DESTINATION ${PREFIX_DATA}/applications )
ENDIF(UNIX)

#  Fix up time_t definition
# Checks for 32-bit version. And always use 32-bit time_t for compatibility


#uninstall-local:
#        rm -fr $(DESTDIR)$(pkgdatadir)/wvsdata/*
#        rm -fr $(DESTDIR) $(pkgdatadir)/s57data/*
#        rm -fr $(DESTDIR)$(pkgdatadir)/tcdata/*
#        rmdir $(DESTDIR)$(pkgdatadir)/wvsdata
#        rmdir $(DESTDIR)$(pkgdatadir)/s57data
#        rmdir $(DESTDIR)$(pkgdatadir)/tcdata
#        rmdir $(DESTDIR)$(pkgdatadir)
#

FIND_PROGRAM(GETTEXT_XGETTEXT_EXECUTABLE xgettext)
IF (GETTEXT_XGETTEXT_EXECUTABLE)

ADD_CUSTOM_COMMAND(
   OUTPUT ${CMAKE_SOURCE_DIR}/po/${PACKAGE_NAME}.pot.dummy
   COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE} --force-po --package-name=${PACKAGE_NAME} --package-version="${PACKAGE_VERSION}" --output=${CMAKE_SOURCE_DIR}/po/${PACKAGE_NAME}.pot  --keyword=_ --width=80 --files-from=${CMAKE_SOURCE_DIR}/po/POTFILES.in 
   DEPENDS ${CMAKE_SOURCE_DIR}/po/POTFILES.in ${CMAKE_SOURCE_DIR}/po/${PACKAGE_NAME}.pot
   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
   COMMENT "pot-update [${PACKAGE_NAME}]: Generated pot file."
)
ADD_CUSTOM_TARGET(pot-update COMMENT "pot-update: Done." DEPENDS ${CMAKE_SOURCE_DIR}/po/${PACKAGE_NAME}.pot.dummy)

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
         COMMENT "po-update [${_poBasename}]: Updated po file."
      )

      SET(_poFiles ${_poFiles} ${_absFile}.dummy)

   ENDFOREACH (_currentPoFile )

   ADD_CUSTOM_TARGET(po-update COMMENT "po-update: Done." DEPENDS ${_poFiles})
ENDMACRO(GETTEXT_UPDATE_PO)

IF (GETTEXT_MSGMERGE_EXECUTABLE)
FILE(GLOB PACKAGE_PO_FILES ${CMAKE_SOURCE_DIR}/po/*.po)
GETTEXT_UPDATE_PO(${CMAKE_SOURCE_DIR}/po/${PACKAGE_NAME}.pot ${PACKAGE_PO_FILES})
ENDIF(GETTEXT_MSGMERGE_EXECUTABLE)

SET(_gmoFiles)
MACRO(GETTEXT_BUILD_MO _poFile _lang)

   GET_FILENAME_COMPONENT(_absFile ${_poFile} ABSOLUTE)
   GET_FILENAME_COMPONENT(_poBasename ${_absFile} NAME_WE)
   SET(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${_poBasename}.mo)

   ADD_CUSTOM_COMMAND(
      OUTPUT ${_gmoFile}
      COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --check -o ${_gmoFile} ${_absFile}
      DEPENDS ${_absFile}
      COMMENT "i18n [${_poBasename}]: Created mo file."
   )

   INSTALL(FILES ${_gmoFile} DESTINATION ${PREFIX_DATA}/locale/${_lang}/LC_MESSAGES RENAME ${PACKAGE_NAME}.mo )
   SET(_gmoFiles ${_gmoFiles} ${_gmoFile})
ENDMACRO(GETTEXT_BUILD_MO)

if(GETTEXT_MSGFMT_EXECUTABLE)
GETTEXT_BUILD_MO(po/opencpn_cs_CZ.po cs)
GETTEXT_BUILD_MO(po/opencpn_da_DK.po da)
GETTEXT_BUILD_MO(po/opencpn_de_DE.po de)
GETTEXT_BUILD_MO(po/opencpn_es_ES.po es)
GETTEXT_BUILD_MO(po/opencpn_fr_FR.po fr)
GETTEXT_BUILD_MO(po/opencpn_it_IT.po it)
GETTEXT_BUILD_MO(po/opencpn_nl_NL.po nl)
GETTEXT_BUILD_MO(po/opencpn_pl_PL.po pl)
GETTEXT_BUILD_MO(po/opencpn_ru_RU.po ru)
GETTEXT_BUILD_MO(po/opencpn_sv_SE.po sv)
GETTEXT_BUILD_MO(po/opencpn_et_ET.po et)
GETTEXT_BUILD_MO(po/opencpn_pt_PT.po pt_PT)
GETTEXT_BUILD_MO(po/opencpn_pt_BR.po pt_BR)
GETTEXT_BUILD_MO(po/opencpn_nb_NO.po nb_NO)
GETTEXT_BUILD_MO(po/opencpn_tr_TR.po tr_TR)
GETTEXT_BUILD_MO(po/opencpn_fi_FI.po fi_FI)
GETTEXT_BUILD_MO(po/opencpn_el_GR.po el_GR)
GETTEXT_BUILD_MO(po/opencpn_zh_TW.po zh_TW)
ADD_CUSTOM_TARGET(i18n COMMENT "i18n: Done." DEPENDS ${_gmoFiles})
#ADD_DEPENDENCIES(i18n ${PACKAGE_PO_FILES})
ADD_DEPENDENCIES(${PACKAGE_NAME} i18n)
ENDIF(GETTEXT_MSGFMT_EXECUTABLE)

ENDIF(NOT APPLE)


# On Windows builds, copy the wxWidgets dlls and  to the executable directory
IF(WIN32 AND NOT UNIX)
INSTALL(DIRECTORY wxWidgets/ DESTINATION "." )
INSTALL(DIRECTORY buildwin/vc9/ DESTINATION ".\\\\vc9" )
#INSTALL(DIRECTORY buildwin/NSIS_Unicode/Bitmaps/ DESTINATION ".\\\\NSISBmps" ) #Gunther
ENDIF(WIN32 AND NOT UNIX)


# build a CPack driven installer package
#include (InstallRequiredSystemLibraries)
SET(CPACK_PACKAGE_NAME "OpenCPN")
SET(CPACK_PACKAGE_VENDOR "opencpn.org")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenCPN ${PACKAGE_VERSION}")
SET(CPACK_PACKAGE_VERSION ${PACKAGE_VERSION})
SET(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
SET(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_CURRENT_BINARY_DIR};${PACKAGE_NAME};ALL;/")
SET(CPACK_PACKAGE_EXECUTABLES ${PACKAGE_NAME} "OpenCPN")



IF(WIN32 AND NOT UNIX)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.

  SET(CPACK_NSIS_INSTALLED_ICON_NAME "opencpn.exe")
  SET(CPACK_NSIS_PACKAGE_NAME_LC "opencpn")

#  These lines set the name of the Windows Start Menu shortcut and the icon that goes with it
  SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_NAME} Version ${PACKAGE_VERSION}")
  SET(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/src/bitmaps\\\\opencpn.ico")

  SET(CPACK_PACKAGE_FILE_NAME "${PACKAGE_NAME}_${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}_setup" )
  
  SET(CPACK_NSIS_DIR "${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode")  #Gunther
  SET(CPACK_BUILDWIN_DIR "${CMAKE_SOURCE_DIR}/buildwin")  #Gunther
  
# Install the VC90 Redistributable
  SET(vc9_redist "vcredist_x86.exe")

    STRING( REPLACE "/" "\\\\" Temp_vc9_redist ${vc9_redist} )
    GET_FILENAME_COMPONENT( VC9_FILE_NAME ${vc9_redist} NAME )
    SET( CPACK_NSIS_EXTRA_INSTALL_COMMANDS ${CPACK_NSIS_EXTRA_INSTALL_COMMANDS}
#      " ;Execute silent and wait\\n  ExecWait '\\\"$INSTDIR\\\\vc9\\\\${VC9_FILE_NAME} /q:a /c:msiexec /i vcredist.msi /qn /l*v %temp%\\\\vcredist_x86.log\\\"' $0\\n"
#      "ExecWait '\\\"$INSTDIR\\\\vc9\\\\${VC9_FILE_NAME}\\\" /q:a /c:\\\"VCREDI~1.EXE /q:a /c:\\\"\\\"msiexec /i vcredist.msi /qn\\\"\\\" \\\"' $0\\n"
      "ExecWait '\\\"$INSTDIR\\\\vc9\\\\${VC9_FILE_NAME}\\\" /q' $0\\n"
      " Remove folder\\n  RMDir /r \\\"$INSTDIR\\\\vc9\\\"\\n\\n" )


ELSE(WIN32 AND NOT UNIX)
  SET(CPACK_PACKAGE_INSTALL_DIRECTORY ${PACKAGE_NAME})
  SET(CPACK_STRIP_FILES "bin/opencpn.exe")
  SET(CPACK_SOURCE_STRIP_FILES "")
  SET(CPACK_PACKAGE_FILE_NAME "${PACKAGE_NAME}_${PACKAGE_VERSION}-1_${ARCH}" )

ENDIF(WIN32 AND NOT UNIX)

#SET(CPACK_PACKAGE_EXECUTABLES ${PACKAGE_NAME})
#SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${PACKAGE_NAME} ${PACKAGE_VERSION}")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/data/license.txt")
#SET(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/data/README")
#SET(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_CURRENT_SOURCE_DIR}/data/Welcome.txt")
IF (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/README")
#    MESSAGE(STATUS "Using generic cpack package description file.")
    SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
    SET(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README")
ENDIF ()
SET(CPACK_SOURCE_GENERATOR "TGZ")

# The following components are regex's to match anywhere (unless anchored)
# in absolute path + filename to find files or directories to be excluded
# from source tarball.
set(CPACK_SOURCE_IGNORE_FILES
"\\\\.cvsignore$"
"^${CMAKE_CURRENT_SOURCE_DIR}.*/CVS/"
"^${CMAKE_CURRENT_SOURCE_DIR}/build*"
"^${CPACK_PACKAGE_INSTALL_DIRECTORY}/*"
)


IF(UNIX)
#    INCLUDE(UseRPMTools)
#    IF(RPMTools_FOUND)
#        RPMTools_ADD_RPM_TARGETS(packagename ${CMAKE_SOURCE_DIR}/package.spec)
#    ENDIF(RPMTools_FOUND)

 
    SET(CPACK_GENERATOR "DEB;RPM")
    SET(CPACK_PACKAGE_CONTACT "Dave Register <bdbcat@yahoo.com>")
    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libwxgtk2.8-0 (>= 2.8.7.1), libglu1-mesa (>= 7.0.0), libgl1-mesa-glx (>= 7.0.0), zlib1g, bzip2")
    SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${ARCH})
    SET(CPACK_DEBIAN_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}")
    SET(CPACK_DEBIAN_PACKAGE_SECTION "Miscellaneous")
    SET(CPACK_RPM_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}")
    SET(CPACK_RPM_PACKAGE_ARCHITECTURE  ${ARCH})
    SET(CPACK_RPM_PACKAGE_REQUIRES  "wxGTK mesa-libGLU mesa-libGL gettext zlib bzip2")
    SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "An OpenSource Chart Plotter/Navigator")
    SET(CPACK_PACKAGE_DESCRIPTION "OpenCPN is a concise ChartPlotter/Navigator. The application supports: GPS/GPDS Position Input, BSB Raster Chart Display, S57 Vector ENChart Display, AIS Input Decoding, Waypoint Autopilot Navigation .")
    SET(CPACK_SET_DESTDIR ON)
    SET(CPACK_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

ENDIF(UNIX)

IF(WIN32 AND NOT UNIX)
 configure_file(${CMAKE_SOURCE_DIR}/src/opencpn.rc.in ${CMAKE_SOURCE_DIR}/src/opencpn.rc)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_GERMAN.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_GERMAN.nsh" @ONLY)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_FRENCH.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_FRENCH.nsh" @ONLY)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_CZECH.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_CZECH.nsh" @ONLY)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_DANISH.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_DANISH.nsh" @ONLY)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_SPANISH.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_SPANISH.nsh" @ONLY)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_ITALIAN.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_ITALIAN.nsh" @ONLY)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_DUTCH.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_DUTCH.nsh" @ONLY)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_POLISH.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_POLISH.nsh" @ONLY)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_PORTUGUESEBR.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_PORTUGUESEBR.nsh" @ONLY)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_PORTUGUESE.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_PORTUGUESE.nsh" @ONLY)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_RUSSIAN.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_RUSSIAN.nsh" @ONLY)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_SWEDISH.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_SWEDISH.nsh" @ONLY)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_FINNISH.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_FINNISH.nsh" @ONLY)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_NORWEGIAN.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_NORWEGIAN.nsh" @ONLY)
ENDIF(WIN32 AND NOT UNIX)

configure_file(version.h.in ${CMAKE_SOURCE_DIR}/include/version.h) #Gunther

# this dummy target is necessary to make sure the ADDITIONAL_MAKE_CLEAN_FILES directive is executed.
# apparently, the base CMakeLists.txt file must have "some" target to activate all the clean steps.
ADD_CUSTOM_TARGET(dummy COMMENT "dummy: Done." DEPENDS ${PACKAGE_NAME})

SET_DIRECTORY_PROPERTIES(dummy ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_SOURCE_DIR}/include/version.h)

INCLUDE(CPack)

MESSAGE(STATUS "Writing spec file...")

 	  	 
