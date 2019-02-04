cmake_minimum_required(VERSION 3.1.0)

SET(SRC
    src/LatLong.hpp
    src/latlong.cpp
    src/long.cpp
    src/nmea0183.cpp
    src/nmea0183.hpp
    src/Response.hpp
    src/response.cpp
    src/RMB.hpp
    src/rmb.cpp
    src/Sentence.hpp
    src/sentence.cpp
    src/talkerid.cpp
    src/RMC.HPP
    src/rmc.cpp
    src/hexvalue.cpp
    src/lat.cpp
    src/expid.cpp
    src/wpl.hpp
    src/wpl.cpp
    src/rte.hpp
    src/rte.cpp
    src/hdt.hpp
    src/hdt.cpp
    src/hdg.hpp
    src/hdg.cpp
    src/hdm.hpp
    src/hdm.cpp
    src/gll.hpp
    src/gll.cpp
    src/vtg.hpp
    src/vtg.cpp
    src/gga.hpp
    src/gga.cpp
    src/gsv.hpp
    src/gsv.cpp
    src/GPwpl.cpp
    src/GPwpl.hpp
    src/apb.cpp
    src/apb.hpp
    src/xte.cpp
    src/xte.hpp
)
ADD_LIBRARY(NMEA0183 STATIC ${SRC})
set_property(TARGET NMEA0183 PROPERTY COMPILE_FLAGS "${OBJ_VISIBILITY}")
target_include_directories(NMEA0183 PRIVATE ${wxWidgets_INCLUDE_DIRS})
