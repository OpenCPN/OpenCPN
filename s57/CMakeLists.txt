set(
  SRC
  src/cm93.cpp
  src/Osenc.cpp
  src/s57chart.cpp
  src/s57obj.cpp
  src/SencManager.cpp
  src/ogrs57datasource.cpp
  src/ogrs57layer.cpp
  src/s57classregistrar.cpp
  src/s57featuredefns.cpp
  src/s57reader.cpp
  src/s57RegistrarMgr.cpp
)

set(
  HDRS
  include/cm93.h
  include/ogr_s57.h
  include/Osenc.h
  include/s57.h
  include/s57chart.h
  include/s57RegistrarMgr.h
  include/SencManager.h
)

add_library(S57ENC STATIC ${SRC})
add_library(ocpn::s57enc ALIAS S57ENC)
target_link_libraries(
  S57ENC
  PUBLIC
    ocpn::iso8211
    ocpn::nmea0183
    ocpn::gdal
    ocpn::geoprim
    ocpn::N2KParser
    ocpn::pugixml
    ocpn::tess2
    ocpn::s52plib
    ocpn::sound
    ocpn::wxjson
    ssl::sha1
    ocpn::tinyxml
    observable::observable
)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|GNU")  # Apple is AppleClang
  target_compile_options(S57ENC PRIVATE -fvisibility=default)
endif ()

if (OCPN_USE_LZMA)
  find_package(LibLZMA)
  if (LIBLZMA_FOUND)
    target_include_directories(S57ENC PRIVATE ${LIBLZMA_INCLUDE_DIR})
    target_link_libraries(S57ENC PUBLIC ${LIBLZMA_LIBRARY})
  endif ()
endif ()


target_include_directories(S57ENC PRIVATE ${CMAKE_SOURCE_DIR}/include)

if (OCPN_USE_CURL AND NOT QT_ANDROID)
  target_link_libraries(S57ENC PUBLIC ocpn::wxcurl)
endif ()
if (TARGET LibLZMA::LibLZMA)
  target_link_libraries(S57ENC PUBLIC LibLZMA::LibLZMA)
endif ()
target_include_directories(S57ENC PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
if (APPLE)
  target_include_directories(S57ENC PUBLIC /usr/local/include)
  target_link_libraries(S57ENC PUBLIC ocpn::filesystem)
endif ()

target_include_directories(
  S57ENC PUBLIC
    ${CMAKE_SOURCE_DIR}/gui/include/gui
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/model/include
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_BINARY_DIR}/include
)

if (QT_ANDROID)
  target_include_directories(S57ENC PUBLIC ${CMAKE_SOURCE_DIR}/android)
endif ()
