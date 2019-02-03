cmake_minimum_required(VERSION 3.1)

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
if(WIN32)
  set(SRC ${SRC} jeeps/gpsusbwin.c)
endif()

add_library(GARMINHOST STATIC ${SRC})
set_property(TARGET GARMINHOST PROPERTY COMPILE_FLAGS "${OBJ_VISIBILITY}")
if (OBJ_VISIBILITY)
    set_target_properties(GARMINHOST PROPERTIES
        COMPILE_FLAGS "${OBJ_VISIBILITY}"
    )
endif ()
file(GLOB GARMIN_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/build/*GARMINHOST*)
set(GARMIN_LIBRARIES ${GARMIN_LIBRARY} PARENT_SCOPE)
target_include_directories(GARMINHOST PRIVATE ${wxWidgets_INCLUDE_DIRS})

