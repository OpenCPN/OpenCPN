cmake_minimum_required(VERSION 3.1.0)

if (TARGET ocpn::tinyxml)
  return ()
endif ()

if (APPLE)
  # TinyXML from Homebrew is not good enough for our GpxDocument needs We
  # should probably replace it with pugixml completely anyway...
  set(USE_BUNDLED_TINYXML ON)
elseif (COMMAND use_bundled_lib)
  use_bundled_lib(USE_BUNDLED_TINYXML tinyxml)
else ()
  set (USE_BUNDLED_TINYXML OFF)
endif ()

if (NOT USE_BUNDLED_TINYXML)
  find_package(TinyXML)
endif ()

if (TINYXML_FOUND)
  message(STATUS "Building with system tinyxml")
  add_library(TINYXML INTERFACE)
  target_include_directories(TINYXML INTERFACE ${TINYXML_INCLUDE_DIR})
  target_link_libraries(TINYXML INTERFACE ${TINYXML_LIBRARIES})
else ()
  message(STATUS "Building with bundled tinyxml")
  set (SRC
    src/tinyxml.cpp
    src/tinyxmlerror.cpp
    src/tinyxmlparser.cpp
    include/tinyxml.h
  )
  add_library(TINYXML STATIC ${SRC})
  target_compile_definitions(TINYXML PUBLIC -DTIXML_USE_STL)
  if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|GNU")   # Apple is AppleClang
    target_compile_options(TINYXML PRIVATE -fvisibility=default -fPIC)
  endif ()
  target_include_directories(TINYXML PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif ()
add_library(ocpn::tinyxml ALIAS TINYXML)
