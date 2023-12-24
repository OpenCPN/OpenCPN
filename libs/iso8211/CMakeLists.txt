cmake_minimum_required(VERSION 3.5.0)

if (TARGET ocpn::iso8211)
    return ()
endif ()

if (NOT CMAKE_MODULE_PATH)
  set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)
endif ()

SET(SRC
    include/iso8211.h
    src/ddffield.cpp
    src/ddffielddefn.cpp
    src/ddfmodule.cpp
    src/ddfrecord.cpp
    src/ddfrecordindex.cpp
    src/ddfsubfielddefn.cpp
    src/ddfutils.cpp
)

add_library(ISO8211 STATIC ${SRC})
add_library(ocpn::iso8211 ALIAS ISO8211)
set_property(TARGET ISO8211 PROPERTY COMPILE_FLAGS "${OBJ_VISIBILITY}")
target_include_directories(ISO8211 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

#FIXME (#3527) Dependency on main openpcn
target_include_directories(ISO8211 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

#FIXME (#3527) MIssing linkage
target_include_directories(ISO8211 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../gdal/include)
