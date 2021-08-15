#
# After inclusion, export the target ocpn::expat
#

if (TARGET ocpn::expat)
    return()
endif ()

add_library(_EXPAT INTERFACE)
if (CMAKE_HOST_WIN32)
  target_include_directories(_EXPAT
      INTERFACE ${CMAKE_SOURCE_DIR}/buildwin/expat-2.2.5/include
  )
  target_link_libraries(_EXPAT
      INTERFACE ${CMAKE_SOURCE_DIR}/buildwin/expat-2.2.5/expat.lib
  )
else ()
  FIND_PACKAGE(EXPAT REQUIRED)
  target_include_directories(_EXPAT INTERFACE ${EXPAT_INCLUDE_DIRS})
  target_link_libraries(_EXPAT INTERFACE ${EXPAT_LIBRARIES})
endif()
add_library(ocpn::expat ALIAS _EXPAT)


