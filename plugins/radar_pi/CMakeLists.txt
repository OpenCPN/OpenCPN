##---------------------------------------------------------------------------
## Plagiarized by: Dave Cowell for WINDOWS
##
## Author:      bdbcat aka. dsr (Dave Register)
## Copyright:
## License:     wxWidgets License
##---------------------------------------------------------------------------

# define minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)

PROJECT(radar_pi)

SET(PACKAGE_NAME radar_pi)
SET(VERBOSE_NAME Radar)
SET(TITLE_NAME Radar)
SET(CPACK_PACKAGE_CONTACT "kees@verruijt.net")
#SET(CMAKE_BUILD_TYPE Debug)
SET(VERSION_MAJOR "5")
SET(VERSION_MINOR "0")
SET(VERSION_PATCH "5")
SET(VERSION_DATE "2020-04-19")
SET(OCPN_MIN_VERSION "ov50")

# require proper c++
#ADD_DEFINITIONS( "-Wall -ansi -pedantic -Wno-variadic-macros" )
#TODO: Should we use  -fno-stack-protector
#  IF NOT DEBUGGING CFLAGS="-O2 -march=native"
IF(NOT WIN32 AND NOT APPLE)
    ADD_DEFINITIONS( "-Wall -O2 -fexceptions -fvisibility=hidden" )
    IF(CMAKE_SYSTEM_NAME MATCHES ".*Linux")
      SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-Bsymbolic")
    ENDIF(CMAKE_SYSTEM_NAME MATCHES ".*Linux")
ENDIF(NOT WIN32 AND NOT APPLE)

IF( APPLE )
    ADD_COMPILE_OPTIONS( "-O3" "-fexceptions" "-fvisibility=hidden" )
    ADD_COMPILE_OPTIONS("-Wno-unused" "-Wno-overloaded-virtual")
   SET(CMAKE_SHARED_LINKER_FLAGS "-Wl -undefined dynamic_lookup")
ENDIF( APPLE )

# Add some definitions to satisfy MSVC
IF(MSVC)
    ADD_DEFINITIONS(-D__MSVC__)
    ADD_DEFINITIONS(-D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_DEPRECATE)
ENDIF(MSVC)



#  QT_ANDROID is a cross-build, so the native FIND_PACKAGE(wxWidgets...) and wxWidgets_USE_FILE is not useful.
IF(NOT QT_ANDROID)
IF(NOT DEFINED wxWidgets_USE_FILE)
  SET(wxWidgets_USE_LIBS base core net xml html adv)
  SET(BUILD_SHARED_LIBS TRUE)
  FIND_PACKAGE(wxWidgets REQUIRED)
ENDIF(NOT DEFINED wxWidgets_USE_FILE)

  INCLUDE(${wxWidgets_USE_FILE})
ENDIF(NOT QT_ANDROID)

FIND_PACKAGE(Gettext REQUIRED)

INCLUDE("cmake/PluginConfigure.cmake")

# For convenience we define the sources as a variable. You can add
# header files and cpp/c files and CMake will sort them out

SET(SRC_EMULATOR
            src/emulator/EmulatorControl.cpp        
            src/emulator/EmulatorControl.h          
            src/emulator/EmulatorControlSet.h       
            src/emulator/EmulatorControlsDialog.cpp 
            src/emulator/EmulatorControlsDialog.h   
            src/emulator/EmulatorReceive.cpp        
            src/emulator/EmulatorReceive.h          
            src/emulator/emulatortype.h
)

SET(SRC_GARMIN_HD
            src/garminhd/GarminHDControl.cpp        
            src/garminhd/GarminHDControl.h          
            src/garminhd/GarminHDControlSet.h       
            src/garminhd/GarminHDControlsDialog.cpp 
            src/garminhd/GarminHDControlsDialog.h   
            src/garminhd/GarminHDReceive.cpp        
            src/garminhd/GarminHDReceive.h          
            src/garminhd/garminhdtype.h
)

SET(SRC_GARMIN_XHD
            src/garminxhd/GarminxHDControl.cpp        
            src/garminxhd/GarminxHDControl.h          
            src/garminxhd/GarminxHDControlSet.h       
            src/garminxhd/GarminxHDControlsDialog.cpp 
            src/garminxhd/GarminxHDControlsDialog.h   
            src/garminxhd/GarminxHDReceive.cpp        
            src/garminxhd/GarminxHDReceive.h          
            src/garminxhd/garminxhdtype.h
)

SET(SRC_NAVICO
            src/navico/br24type.h
            src/navico/br3gtype.h
            src/navico/br4gatype.h
            src/navico/br4gbtype.h
            src/navico/haloatype.h
            src/navico/halobtype.h
            src/navico/NavicoCommon.h
            src/navico/NavicoReceive.cpp
            src/navico/NavicoReceive.h
            src/navico/NavicoControl.cpp
            src/navico/NavicoControl.h
            src/navico/NavicoControlSet.h
            src/navico/NavicoControlsDialog.cpp
            src/navico/NavicoControlsDialog.h
            src/navico/NavicoLocate.cpp
            src/navico/NavicoLocate.h
            src/navico/NavicoRadarInfo.h
)

SET(SRC_RADAR
            src/ControlsDialog.cpp
            src/ControlsDialog.h
            src/GuardZone.cpp
            src/GuardZone.h
            src/GuardZoneBogey.cpp
            src/GuardZoneBogey.h
            src/Kalman.cpp
            src/Kalman.h
            src/Matrix.h
            src/MessageBox.cpp
            src/MessageBox.h
            src/OptionsDialog.cpp
            src/OptionsDialog.h
            src/RadarCanvas.cpp
            src/RadarCanvas.h
            src/RadarControl.h
            src/RadarControlItem.h
            src/RadarDraw.cpp
            src/RadarDraw.h
            src/RadarDrawShader.cpp
            src/RadarDrawShader.h
            src/RadarDrawVertex.cpp
            src/RadarDrawVertex.h
            src/RadarFactory.cpp
            src/RadarFactory.h
            src/RadarInfo.cpp
            src/RadarInfo.h
            src/RadarMarpa.cpp
            src/RadarMarpa.h
            src/RadarPanel.cpp
            src/RadarPanel.h
            src/RadarReceive.h
            src/RadarType.h
            src/SelectDialog.cpp
            src/SelectDialog.h
            src/SoftwareControlSet.h
            src/TextureFont.cpp
            src/TextureFont.h
            src/TrailBuffer.h
            src/TrailBuffer.cpp
            src/ControlsDialog.cpp
            src/ControlsDialog.h
            src/drawutil.cpp
            src/drawutil.h
            src/icons.cpp
            src/icons.h
            src/pi_common.h
            src/radar_pi.cpp
            src/radar_pi.h
            src/shaderutil.cpp
            src/shaderutil.h
            src/socketutil.cpp
            src/socketutil.h

)

SET(SRC_NMEA0183
        src/nmea0183/nmea0183.cpp
        src/nmea0183/nmea0183.hpp
        src/nmea0183/Response.hpp
        src/nmea0183/response.cpp
        src/nmea0183/Sentence.hpp
        src/nmea0183/sentence.cpp
        src/nmea0183/talkerid.cpp
        src/nmea0183/hexvalue.cpp
        src/nmea0183/expid.cpp
        src/nmea0183/hdt.hpp
        src/nmea0183/hdt.cpp
        src/nmea0183/hdg.hpp
        src/nmea0183/hdg.cpp
        src/nmea0183/hdm.hpp
        src/nmea0183/hdm.cpp
)
SET(SRC_JSON
        src/wxJSON/jsonreader.cpp
        src/wxJSON/jsonval.cpp
        # We don't use jsonwriter.cpp yet ...
)
INCLUDE_DIRECTORIES(src/nmea0183)
INCLUDE_DIRECTORIES(src/wxJSON)
INCLUDE_DIRECTORIES(src)

ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_RADAR} ${SRC_NMEA0183} ${SRC_JSON} ${SRC_EMULATOR} ${SRC_GARMIN_HD} ${SRC_GARMIN_XHD} ${SRC_NAVICO})

target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::opencpn )

INCLUDE("cmake/PluginInstall.cmake")
INCLUDE("cmake/PluginLocalization.cmake")
INCLUDE("cmake/PluginPackage.cmake")
