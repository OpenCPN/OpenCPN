cmake_minimum_required(VERSION 3.5)

if (TARGET ocpn::wxjson)
    return ()
endif ()

set(SRC
    src/jsonreader.cpp
    src/jsonval.cpp
    src/jsonwriter.cpp
    include/wx/json_defs.h
    include/wx/jsonreader.h
    include/wx/jsonval.h
    include/wx/jsonwriter.h
)

if (NOT wxWidgets_INCLUDE_DIRS)
    include(SimpleWxConfig)
    SimpleWxConfig()
endif ()

# Library is used also in the plugins, so:
set(CMAKE_POSITION_INDEPENDENT_CODE "ON")

add_library(WXJSON STATIC ${SRC})
add_library(ocpn::wxjson ALIAS WXJSON)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|GNU")  # Apple is AppleClang
  target_compile_options(WXJSON PRIVATE -fvisibility=default -fPIC)
endif ()

target_include_directories(WXJSON PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(WXJSON PRIVATE ${wxWidgets_LIBRARIES})
