cmake_minimum_required(VERSION 3.1)

if (TARGET ocpn::garminhost)
    return ()
endif ()

# Build the garmin lib, once upon a time copied from gpslib. Needs
# a -DwxWidgets_INCLUDE_DIRS if built  separately.

set( SRC
    jeeps/garminusb.h
    jeeps/gpsapp.cc
    jeeps/gpsapp.h
    jeeps/gpscom.cc
    jeeps/gpscom.h
    jeeps/gpsdatum.h
    jeeps/gpsdevice.cc
    jeeps/gpsdevice.h
    jeeps/gpsdevice_ser.cc
    jeeps/gpsdevice_usb.cc
    jeeps/gpsfmt.cc
    jeeps/gpsfmt.h
    jeeps/gps.h
    jeeps/gpslibusb.cc
    jeeps/gpsmath.cc
    jeeps/gpsmath.h
    jeeps/gpsmem.cc
    jeeps/gpsmem.h
    jeeps/gpsport.h
    jeeps/gpsproj.cc
    jeeps/gpsproj.h
    jeeps/gpsprot.cc
    jeeps/gpsprot.h
    jeeps/gpsread.cc
    jeeps/gpsread.h
    jeeps/gpsrqst.cc
    jeeps/gpsrqst.h
    jeeps/gpssend.cc
    jeeps/gpssend.h
    jeeps/gpsserial.cc
    jeeps/gpsserial.h
    jeeps/gpsusbcommon.cc
    jeeps/gpsusbcommon.h
    jeeps/gpsusbint.h
    jeeps/gpsusbread.cc
    jeeps/gpsusbsend.cc
    jeeps/gpsusbstub.cc
    jeeps/gpsusbwin.cc
    jeeps/gpsutil.h
    jeeps/jgpsutil.cc
    gbser.cc
    gbser.h
    gbser_posix.cc
    gbser_posix.h
    gbser_private.h
    gbser_win.cc
    gbser_win.h
)
if (WIN32)
  set(SRC ${SRC} jeeps/gpsusbwin.c)
  add_definitions(-D__WIN32__)
endif ()

add_library(GARMINHOST STATIC ${SRC})
add_library(ocpn::garminhost ALIAS GARMINHOST)
if (MINGW)
  target_compile_definitions(GARMINHOST PRIVATE "-D_UNICODE")
endif ()

set_property(TARGET GARMINHOST PROPERTY COMPILE_FLAGS "${OBJ_VISIBILITY}")
target_include_directories(GARMINHOST PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/jeeps)
target_include_directories(GARMINHOST PRIVATE ${wxWidgets_INCLUDE_DIRS})
