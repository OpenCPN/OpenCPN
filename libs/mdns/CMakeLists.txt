cmake_minimum_required(VERSION 3.5.0)

if (TARGET ocpn::mdns)
  return ()
endif ()

set(MDNS_ROOT ${CMAKE_CURRENT_LIST_DIR}/mdns-1.4.3)
file(COPY ${MDNS_ROOT}/mdns.c DESTINATION ${CMAKE_BINARY_DIR})

set(MDNS_PATCH ${CMAKE_CURRENT_SOURCE_DIR}/mdns.c.patch)
execute_process(
  COMMAND
    ${CMAKE_COMMAND} -Dpatch:FILEPATH=${MDNS_PATCH}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/PatchFile.cmake
  WORKING_DIRECTORY
    ${CMAKE_BINARY_DIR}
)

set(MDNS_SRC ${CMAKE_BINARY_DIR}/mdns.c ${MDNS_ROOT}/mdns.h)
add_library(_MDNS STATIC ${MDNS_SRC})
#target_compile_definitions(_MDNS PRIVATE "main(argc,argv)=foobar(argc,argv)")
target_include_directories(_MDNS PUBLIC ${MDNS_ROOT})

set(SRC src/mdns_util.cpp include/mdns_util.h)
add_library(_MDNS_UTILS STATIC ${SRC})
target_link_libraries(_MDNS_UTILS PRIVATE _MDNS)
add_library(ocpn::mdns ALIAS _MDNS_UTILS)
target_include_directories(
  _MDNS_UTILS PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${MDNS_ROOT}
)
if (QT_ANDROID)
  target_include_directories(
    _MDNS_UTILS PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/android
  )
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|GNU")  # Apple is AppleClang
  target_compile_options(_MDNS_UTILS PRIVATE -fvisibility=default -fPIC)
endif ()
