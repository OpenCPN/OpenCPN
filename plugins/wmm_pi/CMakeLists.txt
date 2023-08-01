# ---------------------------------------------------------------------------
# Author:      nohal aka. Pavel Kalian Copyright: License:     wxWidgets License
# ---------------------------------------------------------------------------

# define minimum cmake version
cmake_minimum_required(VERSION 3.1.1)
cmake_policy(SET CMP0043 NEW)

project(wmm_pi)

set(PACKAGE_NAME wmm_pi)
set(VERBOSE_NAME Wmm)
set(TITLE_NAME WMM)
set(CPACK_PACKAGE_CONTACT "Pavel Kalian")

set(VERSION_MAJOR "1")
set(VERSION_MINOR "1")
set(VERSION_PATCH "2")
set(VERSION_DATE "12/09/2019")

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE
      Release
      CACHE
        STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
        FORCE
  )
endif (NOT CMAKE_BUILD_TYPE)

include_directories(${CMAKE_SOURCE_DIR}/include)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# SET(CMAKE_BUILD_TYPE Debug)
set(CMAKE_MACOSX_RPATH "ON")

include("cmake/PluginConfigure.cmake")

set(SRC_WMM
    src/wmm_pi.h
    src/wmm_pi.cpp
    src/MagneticPlotMap.h
    src/MagneticPlotMap.cpp
    src/icons.h
    src/icons.cpp
    src/GeomagnetismHeader.h
    src/GeomagnetismLibrary.c
    src/EGM9615.h
    src/WmmUIDialog.h
    src/WmmUIDialog.cpp
    src/pi_TexFont.cpp
    src/pi_TexFont.h
    src/pi_ocpndc.cpp
    src/pi_ocpndc.h
)

if (QT_ANDROID)
  set(SRC_WMM ${SRC_WMM} src/pi_shaders.h)
  set(SRC_WMM ${SRC_WMM} src/pi_shaders.cpp)
endif (QT_ANDROID)

set(SRC_JSON
    src/json_defs.h
    src/jsonreader.h
    src/jsonval.h
    src/jsonwriter.h
    src/jsonreader.cpp
    src/jsonval.cpp
    src/jsonwriter.cpp
)

add_library(${PACKAGE_NAME} SHARED ${SRC_WMM} ${SRC_JSON})

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|AppleClang")
  target_compile_options(
    ${PACKAGE_NAME} PRIVATE -Wno-unknown-pragmas -Wno-overloaded-virtual
  )
endif ()

if (NOT QT_ANDROID)
  target_link_libraries(
    ${PACKAGE_NAME} PRIVATE ${wxWidgets_LIBRARIES} ocpn::wxjson ocpn::opencpn
  )
else (NOT QT_ANDROID)
  target_link_libraries(
    ${PACKAGE_NAME}
    PRIVATE ocpn::wxjson
            "-lGLESv1_CM"
            "-lGLESv2"
            ${Qt_Base}/${Qt_Build}/lib/libQt5Gui.so
            ${Qt_Base}/${Qt_Build}/lib/libQt5Core.so
            ${Qt_Base}/${Qt_Build}/lib/libQt5Widgets.so
            ${Qt_Base}/${Qt_Build}/lib/libQt5AndroidExtras.so
            ${CMAKE_CURRENT_BINARY_DIR}/../../libgorp.so
  )

  set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-soname,lib${PACKAGE_NAME}.so ")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-s")
endif (NOT QT_ANDROID)

if (WIN32)
  target_link_libraries(${PACKAGE_NAME} PRIVATE "GLU_static")
endif (WIN32)

if (APPLE)
  target_compile_options(
    ${PACKAGE_NAME}
    PRIVATE "-Wall"
            "-Wno-unused"
            "-fexceptions"
            "-Wno-overloaded-virtual"
            "-Wno-deprecated"
            "-Wno-deprecated-declarations"
            "-Wno-unknown-pragmas"
            "-D_WCHAR_H_CPLUSPLUS_98_CONFORMANCE_"
  )
endif (APPLE)

# target_link_libraries(${PACKAGE_NAME} ocpn::opencpn )

# Copy shared library to execution folder for debugging if not CI build
if (NOT OCPN_CI_BUILD)
  if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DEBUG /OPT:REF /OPT:ICF")
    add_custom_command(
      TARGET ${PACKAGE_NAME}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E echo "Copying dll file"
      COMMAND ${CMAKE_COMMAND} -E echo "$<TARGET_FILE:${PACKAGE_NAME}>"
                "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/plugins/${PACKAGE_NAME}.dll"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:${PACKAGE_NAME}>"
                "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/plugins/${PACKAGE_NAME}.dll"
    )
    add_custom_command(
      TARGET ${PACKAGE_NAME}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E echo "Copying pdb file"
      COMMAND ${CMAKE_COMMAND} -E echo "$<TARGET_PDB_FILE:${PACKAGE_NAME}>"
                "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/plugins/${PACKAGE_NAME}.pdb"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_PDB_FILE:${PACKAGE_NAME}>"
                "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/plugins/${PACKAGE_NAME}.pdb"
    )
  endif (MSVC)
endif (NOT OCPN_CI_BUILD)

include("cmake/PluginInstall.cmake")
include("cmake/PluginLocalization.cmake")
include("cmake/PluginPackage.cmake")
