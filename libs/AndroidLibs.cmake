#
# For armhf and arm64: Download precompiled libraries and set up
# linking
# Also setup preprocessor definitions
#
cmake_minimum_required(VERSION 3.1)

set(CMAKE_SKIP_RPATH true)

# Make sure we have downloaded and unpacked master.zip
set(
  OCPN_ANDROID_CACHEDIR "${CMAKE_SOURCE_DIR}/cache"
  CACHE STRING "Build download area"
)
set(_master_base ${OCPN_ANDROID_CACHEDIR}/OCPNAndroidCoreBuildSupport)
message(STATUS "Android Build support file base:  ${OCPN_ANDROID_CACHEDIR}/OCPNAndroidCoreBuildSupport")


if (TRUE) #(NOT EXISTS ${OCPN_ANDROID_CACHEDIR}/support.zip)
  file(
    DOWNLOAD
      https://github.com/bdbcat/OCPNAndroidCoreBuildSupport/releases/download/v1.1/OCPNAndroidCoreBuildSupport.zip
      ${OCPN_ANDROID_CACHEDIR}/support.zip
#    EXPECTED_HASH
#      SHA256=ac36afaf4f026e9b2624a963f5356f5b1fb2c45dec1134209333a8b46fb05ca0
    SHOW_PROGRESS
  )
endif ()
if (TRUE) #(NOT EXISTS ${_master_base})
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar -xzf ${OCPN_ANDROID_CACHEDIR}/support.zip
    WORKING_DIRECTORY "${OCPN_ANDROID_CACHEDIR}"
  )
endif ()

# testing
#set(_master_base "/home/dsr/Projects/OCPNAndroidCoreBuildSupport")

# Setup directories and libraries
if ("${OCPN_TARGET_TUPLE}" MATCHES "Android-arm64")
  file(GLOB _wx_setup
    ${_master_base}/wxWidgets/libs/arm64/lib/wx/include/arm-linux-*-static-*
  )
  set(_qt_include  ${_master_base}/qt5/build_arm64_O3/qtbase/include)
  set(_qtlibs  ${_master_base}/qt5/build_arm64_O3/qtbase/lib)
  set(_wxlibs  ${_master_base}/wxWidgets/libs/arm64/lib)
  set(Qt_Base ${_master_base}/qt5)
  set(Qt_Build build_arm64_O3/qtbase)
  set(base_include ${_master_base}/include)

else ()
  file(GLOB _wx_setup
    ${_master_base}/wxWidgets/libs/armhf/lib/wx/include/arm-linux-*-static-*
  )
  set(_qt_include ${_master_base}/qt5/build_arm32_19_O3/qtbase/include)
  set(_qtlibs  ${_master_base}/qt5/build_arm32_19_O3/qtbase/lib)
  set(_wxlibs  ${_master_base}/wxWidgets/libs/armhf/lib)
  set(Qt_Base ${_master_base}/qt5)
  set(Qt_Build build_arm32_19_O3/qtbase)
  set(base_include ${_master_base}/include)
endif ()

message(STATUS "Android Build wx include directories: support file base:  ${_wx_setup}")

include_directories(
  ${base_include}
  ${_qt_include}
  ${_qt_include}/QtWidgets
  ${_qt_include}/QtCore
  ${_qt_include}/QtGui
  ${_qt_include}/QtOpenGL
  ${_qt_include}/QtTest
  ${_master_base}/wxWidgets/include/
  ${_wx_setup}
)
target_link_libraries(${PACKAGE_NAME} PRIVATE
  ${_qtlibs}/libQt5Core.so
  ${_qtlibs}/libQt5OpenGL.so
  ${_qtlibs}/libQt5Widgets.so
  ${_qtlibs}/libQt5Gui.so
  ${_qtlibs}/libQt5AndroidExtras.so
)

target_link_libraries(${PACKAGE_NAME} PRIVATE
    # Link order is critical to avoid circular dependencies
    ${_wxlibs}/libwx_qtu_html-3.1-arm-linux-androideabi.a
    ${_wxlibs}/libwx_baseu_xml-3.1-arm-linux-androideabi.a
    ${_wxlibs}/libwx_qtu_qa-3.1-arm-linux-androideabi.a
    ${_wxlibs}/libwx_qtu_adv-3.1-arm-linux-androideabi.a
    ${_wxlibs}/libwx_qtu_core-3.1-arm-linux-androideabi.a
    ${_wxlibs}/libwx_baseu-3.1-arm-linux-androideabi.a
    ${_wxlibs}/libwx_qtu_aui-3.1-arm-linux-androideabi.a
    ${_wxlibs}/libwxexpat-3.1-arm-linux-androideabi.a
    ${_wxlibs}/libwxregexu-3.1-arm-linux-androideabi.a
    ${_wxlibs}/libwxjpeg-3.1-arm-linux-androideabi.a
    ${_wxlibs}/libwxpng-3.1-arm-linux-androideabi.a
    ${_wxlibs}/libwx_qtu_gl-3.1-arm-linux-androideabi.a
    ${_wxlibs}/libwx_baseu_net-3.1-arm-linux-androideabi.a
)

add_compile_definitions(
  __WXQT__
  __OCPN__ANDROID__
  ocpnUSE_GLES
  ocpnUSE_GL
  USE_ANDROID_GLES2
  USE_GLSL
  USE_GLU_TESS
)

  SET(OPENGLES_FOUND "YES")
  SET(OPENGL_FOUND "YES")
  SET(USE_GLES2 ON )

#if (NOT CMAKE_BUILD_TYPE STREQUAL Debug)
#  string(APPEND CMAKE_SHARED_LINKER_FLAGS " -s")
#endif ()
