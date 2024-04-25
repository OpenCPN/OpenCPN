#
# Export variables used in plugin setup: PKG_TARGET, PKG_TARGET_VERSION.
#
# For Debian/Ubuntu also export variables used in .de package generation
# like PACKAGE_FORMAT, PACKAGE_RECS, TENTATIVE_PREFIX

if (NOT OCPN_TARGET_TUPLE STREQUAL "")
    list(GET OCPN_TARGET_TUPLE 0 PKG_TARGET)
    list(GET OCPN_TARGET_TUPLE 1 PKG_TARGET_VERSION)
elseif (OCPN_FLATPAK)
    set(PKG_TARGET "flatpak-${ARCH}")
    set(manifest_path "${PROJECT_SOURCE_DIR}/flatpak/org.opencpn.OpenCPN.yaml")
    file(READ ${manifest_path} manifest)
    string(REPLACE "\n" ";" manifest_lines "${manifest}")
    foreach (_line ${manifest_lines})
      if (${_line} MATCHES  "^runtime-version")
        string(REGEX REPLACE ".*:" "" PKG_TARGET_VERSION "${_line}")
      endif ()
    endforeach ()
    message(STATUS  "Building for flatpak runtime ${PKG_TARGET_VERSION}")
elseif (MINGW)
  set(PKG_TARGET "mingw-${ARCH}")
    if (CMAKE_SYSTEM_VERSION)
      set(PKG_TARGET_VERSION ${CMAKE_SYSTEM_VERSION})
    else ()
      set(PKG_TARGET_VERSION 10)
    endif ()
elseif (MSVC)
    set(PKG_TARGET "msvc-wx32")
    if (CMAKE_SYSTEM_VERSION)
        set(PKG_TARGET_VERSION ${CMAKE_SYSTEM_VERSION})
    elseif (CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
        set(PKG_TARGET_VERSION ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION})
    else ()
        set(PKG_TARGET_VERSION 10)
    endif ()
elseif (APPLE)
    set(PKG_TARGET "darwin-wx32")
    execute_process(COMMAND "sw_vers" "-productVersion"
                    OUTPUT_VARIABLE PKG_TARGET_VERSION)
elseif (UNIX)
    find_program(LSB_RELEASE NAMES lsb_release)
    if (NOT LSB_RELEASE)
        message(FATAL_ERROR
                "Cannot find the lsb_release program, please install.")
    endif ()
    execute_process(COMMAND ${LSB_RELEASE} "-is"
                    OUTPUT_VARIABLE PKG_TARGET)
    execute_process(COMMAND ${LSB_RELEASE} "-rs"
                    OUTPUT_VARIABLE PKG_TARGET_VERSION)
    string(STRIP ${PKG_TARGET} PKG_TARGET)
    set(PKG_TARGET "${PKG_TARGET}-${ARCH}")
else ()
    set(PKG_TARGET "unknown")
    set(PKG_TARGET_VERSION 1)
endif ()

string(STRIP ${PKG_TARGET} PKG_TARGET)
string(TOLOWER ${PKG_TARGET} PKG_TARGET)
string(STRIP ${PKG_TARGET_VERSION} PKG_TARGET_VERSION)
string(TOLOWER ${PKG_TARGET_VERSION} PKG_TARGET_VERSION)
string(CONCAT _msg
  "Building for target (Platform\; version\; arch): "
  "${PKG_TARGET}\; ${PKG_TARGET_VERSION}\; ${ARCH}"
)
message(STATUS ${_msg})

# Set some target-related support variables, notably to build a .deb package:
set(LIB_INSTALL_DIR "lib")
if (${PKG_TARGET} MATCHES "ubuntu|debian|raspbian|linuxmint")
    set(PACKAGE_FORMAT "DEB")
    set(PACKAGE_RECS "xcalib,xdg-utils")
    set(TENTATIVE_PREFIX "/usr/local")
    message(STATUS "Setting up a Debian package.")
endif ()
