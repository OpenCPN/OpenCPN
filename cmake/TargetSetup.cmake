#
# Export variables used in plugin setup: PKG_TARGET, PKG_TARGET_VERSION
# and PKG_NVR

if (OCPN_FLATPAK)
    set(PKG_TARGET "flatpak")
    set(PKG_TARGET_VERSION "18.08")    # As of flatpak/*yaml
elseif (MINGW)
    set(PKG_TARGET "mingw")
    if (CMAKE_SYSTEM_VERSION)
        set(PKG_TARGET_VERSION ${CMAKE_SYSTEM_VERSION})
    else ()
	set(PKG_TARGET_VERSION 10)
    endif ()
elseif (MSVC)
    set(PKG_TARGET "msvc")
    if (CMAKE_SYSTEM_VERSION)
        set(PKG_TARGET_VERSION ${CMAKE_SYSTEM_VERSION})
    elseif (CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
        set(PKG_TARGET_VERSION ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION})
    else ()
        set(PKG_TARGET_VERSION 10)
    endif ()
elseif (APPLE)
    set(PKG_TARGET "darwin")
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
else ()
    set(PKG_TARGET "unknown")
    set(PKG_TARGET_VERSION 1)
endif ()


string(STRIP ${PKG_TARGET} PKG_TARGET)
string(TOLOWER ${PKG_TARGET} PKG_TARGET)
string(STRIP ${PKG_TARGET_VERSION} PKG_TARGET_VERSION)
string(TOLOWER ${PKG_TARGET_VERSION} PKG_TARGET_VERSION)
