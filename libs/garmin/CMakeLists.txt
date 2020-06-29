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
    jeeps/gpsutil.h
    jeeps/jgpsutil.cc
    opencpn/garmin_wrapper_utils.h
    opencpn/garmin_wrapper_utils.cc
    opencpn/gpsinput.h
    gbser.cc
    gbser.h
    gbser_private.h
)
if (WIN32)
  set(SRC ${SRC}
    jeeps/gpsusbwin.c
    gbser_win.cc
    gbser_win.h
  )
  add_definitions(-D__WIN32__)
else ()
  set(SRC ${SRC}
    gbser_posix.cc
    gbser_posix.h
  )
endif ()

add_library(GARMINHOST STATIC ${SRC})
add_library(ocpn::garminhost ALIAS GARMINHOST)
target_compile_definitions(GARMINHOST PRIVATE "-DLIBRARY_BUILD")
if (MINGW)
  target_compile_definitions(GARMINHOST PRIVATE "-D_UNICODE")
endif ()
if (WIN32)
  target_compile_definitions(GARMINHOST PRIVATE "-DZLIB_INHIBITED")
endif ()
 
set_property(TARGET GARMINHOST PROPERTY COMPILE_FLAGS "${OBJ_VISIBILITY}")
target_include_directories(
  GARMINHOST
    PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR}/jeeps
      ${CMAKE_CURRENT_SOURCE_DIR}/opencpn
      ${CMAKE_CURRENT_SOURCE_DIR}
)
target_include_directories(GARMINHOST PRIVATE ${wxWidgets_INCLUDE_DIRS})
