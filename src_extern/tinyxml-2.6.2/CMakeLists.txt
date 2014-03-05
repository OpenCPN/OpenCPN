project(tinyxml)
cmake_minimum_required(VERSION 2.8)

option(USE_STL "Use STL" ON)

message(STATUS "CMAKE_BUILD_TYPE       : ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_C_COMPILER       : ${CMAKE_C_COMPILER}")
message(STATUS "CMAKE_C_FLAGS          : ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_C_FLAGS_DEBUG    : ${CMAKE_C_FLAGS_DEBUG}")
message(STATUS "CMAKE_C_FLAGS_RELEASE  : ${CMAKE_C_FLAGS_RELEASE}")
message(STATUS "CMAKE_CXX_COMPILER     : ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_CXX_FLAGS        : ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG  : ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")

set(SOURCES
	tinyxml.h
	tinystr.h
	tinyxml.cpp
	tinyxmlparser.cpp
	tinyxmlerror.cpp
	tinystr.cpp
	)

if(USE_STL)
	message(STATUS "tinyxml with STL")
	add_definitions("-DTIXML_USE_STL")
else()
	message(STATUS "tinyxml without STL")
endif()

add_library(tinyxml STATIC ${SOURCES})

set_target_properties(tinyxml PROPERTIES VERSION 2.6.2 SOVERSION 2.6.2)

install(TARGETS tinyxml LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(FILES tinyxml.h DESTINATION include/tinyxml)

if(NOT USE_STL)
	install(FILES tinystr.h DESTINATION include/tinyxml)
endif()

