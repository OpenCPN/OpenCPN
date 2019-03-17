message(STATUS "Installing bundled mingw libraries")

if ("${CMAKE_BINARY_DIR}" STREQUAL "")
  set(CMAKE_BINARY_DIR ".")
endif ()

set (mingw_bundle_dlls "${CMAKE_BINARY_DIR}/../mingw/tools/mingw-bundle-dlls")
file(GLOB opencpn_exe "${CMAKE_BINARY_DIR}/*opencpn.exe")
execute_process(
    COMMAND python3 ${mingw_bundle_dlls} ${opencpn_exe}
    OUTPUT_VARIABLE MINGW_LIBS
)
string(REPLACE "\n" ";" MINGW_LIBS ${MINGW_LIBS})

if ("$ENV{DESTDIR}" STREQUAL "")
    set (install_dir ${CMAKE_INSTALL_PREFIX})
else ()
    set (install_dir $ENV{DESTDIR}/${CMAKE_INSTALL_PREFIX})
endif ()

message(STATUS "Installing mingw runtime libs in ${install_dir}")
file (COPY ${MINGW_LIBS} DESTINATION "${install_dir}")
