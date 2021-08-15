#  Locates gpsd library and headers, returns linkable ocpn::gpsd if found
#
#  UNTESTED and UNUSED!

if (TARGET ocpn::gpsd)
  return ()
endif ()

find_package(gps)
if (GPS_FOUND)
  add_library(_GPSD INTERFACE)
  message (STATUS "Building with libgps includes")
  target_link_libraries(_GPSD INTERFACE ${GPS_LIBRARY})
  target_include_directories(_GPSD INTERFACE ${GPS_INCLUDE_DIR})
  target_compile_definitions(_GPSD
      INTERFACE -DBUILD_WITH_LIBGPS ${GPS_DEFINITIONS}
  )
  add_library(ocpn::gpsd ALIAS _GPSD)
ELSE (GPS_FOUND)
  message (STATUS "Gps library not found")
ENDIF (GPS_FOUND)

