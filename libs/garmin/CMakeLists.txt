cmake_minimum_required(VERSION 3.1)

if (TARGET ocpn::garminhost)
    return ()
endif ()

# Build the garmin lib, once upon a time copied from gpslib. Needs
# a -DwxWidgets_INCLUDE_DIRS if built  separately.

set( SRC
    jeeps/gpsapp.c
    jeeps/gpscom.c
    jeeps/gpsmath.c
    jeeps/gpsmem.c
    jeeps/gpsprot.c
    jeeps/gpsread.c
    jeeps/gpsdevice.c
    jeeps/gpsdevice_ser.c
    jeeps/gpsdevice_usb.c
    jeeps/gpsrqst.c
    jeeps/gpssend.c
    jeeps/gpsserial.c
    jeeps/gpsutil.c
    jeeps/gpsusbread.c
    jeeps/gpsusbsend.c
    jeeps/gpsusbcommon.c
    jeeps/garmin_gps.h
    jeeps/gpsapp.h
    jeeps/gpsdatum.h
    jeeps/gpsfmt.h
    jeeps/gpsmath.h
    jeeps/gpsport.h
    jeeps/gpsprot.h
    jeeps/gpsrqst.h
    jeeps/gpsserial.h
    jeeps/gpsusbint.h
    jeeps/gps_wx_logging.h
    jeeps/garminusb.h
    jeeps/gpscom.h
    jeeps/gpsdevice.h
    jeeps/gpsinput.h
    jeeps/gpsmem.h
    jeeps/gpsproj.h
    jeeps/gpsread.h
    jeeps/gpssend.h
    jeeps/gpsusbcommon.h
    jeeps/gpsutil.h
    jeeps/garmin_wrapper_utils.h
    jeeps/garmin_wrapper_utils.c
)
if (WIN32)
  set(SRC ${SRC} jeeps/gpsusbwin.c)
  add_definitions(-D__WIN32__)
endif ()
set_source_files_properties(${SRC} PROPERTIES LANGUAGE CXX )

add_library(GARMINHOST STATIC ${SRC})
add_library(ocpn::garminhost ALIAS GARMINHOST)
if (MINGW)
  target_compile_definitions(GARMINHOST PRIVATE "-D_UNICODE")
endif ()

target_compile_definitions(GARMINHOST PRIVATE "-DUSE_WX_LOGGING")
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|GNU")  # Apple is AppleClang
  target_compile_options(GARMINHOST PRIVATE -fvisibility=default -fPIC)
endif ()

target_include_directories(GARMINHOST PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/jeeps)
target_include_directories(GARMINHOST PRIVATE ${wxWidgets_INCLUDE_DIRS})
