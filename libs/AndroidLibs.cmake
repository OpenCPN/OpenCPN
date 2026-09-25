#
# For armhf and arm64: locate the prebuilt Qt, wxWidgets and OpenSSL staged by
# the builder image, and set up linking.
# Also setup preprocessor definitions
#
cmake_minimum_required(VERSION 3.1)

set(CMAKE_SKIP_RPATH true)

# Base directory of the Android dependencies staged by OpenCPN-builder's
# Dockerfile.android, one subtree per component:
#
#   ${OCPN_ANDROID_DEPS}/qt5/<armhf|arm64>/{include,lib}      official upstream Qt
#   ${OCPN_ANDROID_DEPS}/wxWidgets/{include,libs/<abi>/lib}   wxQt, built from source
#   ${OCPN_ANDROID_DEPS}/openssl/<armhf|arm64>/{include,lib}  built from source
#
# This replaces downloading the ~1.3GB OCPNAndroidCoreBuildSupport bundle on
# every configure. That bundle carried an EOL Qt and OpenSSL, baked absolute
# paths from the packager's machine into its pkg-config files, and had no
# reproducible build recipe.
set(
  OCPN_ANDROID_DEPS "/usr/src"
  CACHE STRING "Base directory of the staged Android dependencies"
)
message(STATUS "Android staged dependency base: ${OCPN_ANDROID_DEPS}")

if ("${OCPN_TARGET_TUPLE}" MATCHES "Android-arm64")
  set(_abi arm64)
else ()
  set(_abi armhf)
endif ()

set(_wx_base ${OCPN_ANDROID_DEPS}/wxWidgets)

file(GLOB _wx_setup
  ${_wx_base}/libs/${_abi}/lib/wx/include/arm-linux-*-static-*
)
set(_qt_include ${OCPN_ANDROID_DEPS}/qt5/${_abi}/include)
set(_qtlibs ${OCPN_ANDROID_DEPS}/qt5/${_abi}/lib)
set(_wxlibs ${_wx_base}/libs/${_abi}/lib)
# Qt_Base/Qt_Build are also consumed by plugins/*/CMakeLists.txt, which link
# ${Qt_Base}/${Qt_Build}/lib/libQt5*.so — keep that path shape valid.
set(Qt_Base ${OCPN_ANDROID_DEPS}/qt5)
set(Qt_Build ${_abi})
set(openssl_include ${OCPN_ANDROID_DEPS}/openssl/${_abi}/include)

# Fail early and clearly rather than deep in a link line full of missing files.
foreach (_dir ${_qt_include} ${_qtlibs} ${_wxlibs})
  if (NOT EXISTS ${_dir})
    message(FATAL_ERROR
      "Staged Android dependency not found: ${_dir}\n"
      "Build inside the OpenCPN-builder image, or point -DOCPN_ANDROID_DEPS=<dir> "
      "at a tree containing qt5/, wxWidgets/ and openssl/."
    )
  endif ()
endforeach ()

message(STATUS "Android Build wx include directories: support file base:  ${_wx_setup}")

include_directories(
  ${base_include}
  ${_qt_include}
  ${_qt_include}/QtWidgets
  ${_qt_include}/QtCore
  ${_qt_include}/QtGui
  ${_qt_include}/QtOpenGL
  ${_qt_include}/QtTest
  ${_wx_base}/include/
  ${_wx_setup}
)
target_link_libraries(${PACKAGE_NAME} PRIVATE
  ${_qtlibs}/libQt5Core.so
  ${_qtlibs}/libQt5OpenGL.so
  ${_qtlibs}/libQt5Widgets.so
  ${_qtlibs}/libQt5Gui.so
  ${_qtlibs}/libQt5AndroidExtras.so
)

set(_all_wx_libs
    # Link order is critical to avoid circular dependencies
    ${_wxlibs}/libwx_qtu_html-3.2-arm-linux-androideabi.a
    ${_wxlibs}/libwx_baseu_xml-3.2-arm-linux-androideabi.a
    ${_wxlibs}/libwx_qtu_qa-3.2-arm-linux-androideabi.a
    ${_wxlibs}/libwx_qtu_adv-3.2-arm-linux-androideabi.a
    ${_wxlibs}/libwx_qtu_core-3.2-arm-linux-androideabi.a
    ${_wxlibs}/libwx_baseu-3.2-arm-linux-androideabi.a
    ${_wxlibs}/libwx_qtu_aui-3.2-arm-linux-androideabi.a
    ${_wxlibs}/libwxexpat-3.2-arm-linux-androideabi.a
    ${_wxlibs}/libwxregexu-3.2-arm-linux-androideabi.a
    ${_wxlibs}/libwxjpeg-3.2-arm-linux-androideabi.a
    ${_wxlibs}/libwxpng-3.2-arm-linux-androideabi.a
    ${_wxlibs}/libwx_qtu_gl-3.2-arm-linux-androideabi.a
    ${_wxlibs}/libwx_baseu_net-3.2-arm-linux-androideabi.a
)
target_link_libraries(${PACKAGE_NAME} PRIVATE ${_all_wx_libs})

if ("${OCPN_TARGET_TUPLE}" MATCHES "Android-armhf")
  target_link_libraries(${PACKAGE_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/buildandroid/ndk/linux-atomic.o")
endif ()

add_compile_definitions(
  __WXQT__
  __OCPN__ANDROID__
  ocpnUSE_GLES
  ocpnUSE_GL
  USE_ANDROID_GLES2
  USE_GLSL
  USE_GLU_TESS
)

set(OPENGLES_FOUND "YES")
set(OPENGL_FOUND "YES")
set(USE_GLES2 ON )

set(ANDROID_WX_INCLUDES ${_wx_setup})
set(ANDROID_WX_LIBS ${_all_wx_libs})

#if (NOT CMAKE_BUILD_TYPE STREQUAL Debug)
#  string(APPEND CMAKE_SHARED_LINKER_FLAGS " -s")
#endif ()
