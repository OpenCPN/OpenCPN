cmake_minimum_required(VERSION 3.5.0)

if (TARGET ocpn::java-jvm)
  return ()
endif ()
set(SRC include/android_jvm.h src/android_jvm.cpp)
add_library(JAVA_JVM STATIC ${SRC})
target_include_directories(JAVA_JVM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(ocpn::java-jvm ALIAS JAVA_JVM)
