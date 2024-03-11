# For platforms without a complete set of OpenGL headers add those
# to the link target
#
# Exports: ocpn::gl-headers transitive link target

cmake_minimum_required(VERSION 3.5.0)

if (TARGET ocpn::gl-headers)
  return ()
endif ()

add_library(_GL_HEADERS INTERFACE)
add_library(ocpn::gl-headers ALIAS _GL_HEADERS)

if (ANDROID OR ANDROID)
  target_include_directories(_GL_HEADERS INTERFACE ${CMAKE_CURRENT_LIST_DIR}/android)
elseif (WIN32)
  target_include_directories(_GL_HEADERS INTERFACE ${CMAKE_CURRENT_LIST_DIR}/windows)
endif ()
