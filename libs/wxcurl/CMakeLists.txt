cmake_minimum_required(VERSION 3.5)

if (TARGET ocpn::wxcurl)
    return ()
endif ()

if (POLICY CMP0077)
    CMAKE_POLICY(SET CMP0077 OLD)
endif ()

# FixME (#3527) Dependency on main CMakeLists.txt
if (NOT CMAKE_MODULE_PATH)
  set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)
endif ()


set(SRC
    src/base.cpp
    src/dav.cpp
    src/davtool.cpp
    src/dialog.cpp
    src/ftp.cpp
    src/http.cpp
    src/include
    src/panel.cpp
    src/telnet.cpp
    src/thread.cpp
    src/utils.cpp
    include/wx/curl/base.h
    include/wx/curl/dav.h
    include/wx/curl/davtool.h
    include/wx/curl/dialog.h
    include/wx/curl/ftp.h
    include/wx/curl/http.h
    include/wx/curl/panel.h
    include/wx/curl/telnet.h
    include/wx/curl/thread.h
    include/wx/curl/utils.h
)

if (NOT wxWidgets_INCLUDE_DIRS)
    include(SimpleWxConfig)
    SimpleWxConfig()
endif ()

include(Curl)

add_library(WXCURL STATIC ${SRC})
add_library(ocpn::wxcurl ALIAS WXCURL)
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|GNU")  # Apple is AppleClang
  set_property(TARGET WXCURL PROPERTY COMPILE_FLAGS "-fvisibility=default")
endif ()
target_include_directories(WXCURL
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC ${CURL_INCLUDE_DIRS}
)
target_link_libraries(WXCURL ${CURL_LIBRARIES})# ${wxWidgets_LIBRARIES})
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|AppleClang|GNU")
  target_compile_options(WXCURL  PRIVATE "-Wno-unknown-pragmas")
endif ()
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|AppleClang")
    target_compile_options(WXCURL PRIVATE "-Wno-varargs")
endif()
