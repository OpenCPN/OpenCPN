cmake_minimum_required(VERSION 3.1.0)

if (TARGET ocpn::serial)
    return ()
endif ()

#    DESCRIPTION "Cross-platform, Serial Port library written in C++"
#    HOMEPAGE_URL "http://wjwwood.io/serial/"

SET(PROJECT_NAME "SERIAL")
SET(PROJECT_VERSION "1.2.1")

include(GNUInstallDirs)

# Test does not work as of current.
option(CATKIN_ENABLE_TESTING "Enable catkin unit tests" OFF)
option(ENABLE_TEST_PROGRAM "Build test program" OFF)
option(ENABLE_API_DOCS "Build Doxygen API documentation" OFF)

# Find catkin, required for tests

if(APPLE)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(FOUNDATION_LIBRARY Foundation)
endif()

if (NOT ${CATKIN_ENABLE_TESTING} MATCHES "OFF")
    find_package(catkin)
    if(UNIX AND NOT APPLE)
        # If Linux, add rt and pthread
        set(rt_LIBRARIES rt)
        set(pthread_LIBRARIES pthread)
        catkin_package(
            LIBRARIES ${PROJECT_NAME}
            INCLUDE_DIRS include
            DEPENDS rt pthread
        )
    else()
        # Otherwise normal call
        catkin_package(
            LIBRARIES ${PROJECT_NAME}
            INCLUDE_DIRS include
        )
    endif()
endif ()


## Sources
set(serial_SRCS
    src/serial.cc
    include/serial/serial.h
    include/serial/v8stdint.h
)
if(APPLE)
    list(APPEND serial_SRCS src/impl/unix.cc)
    list(APPEND serial_SRCS src/impl/list_ports/list_ports_osx.cc)
elseif(UNIX)
    list(APPEND serial_SRCS src/impl/unix.cc)
    list(APPEND serial_SRCS src/impl/list_ports/list_ports_linux.cc)
else()
    # windows
    list(APPEND serial_SRCS src/impl/win.cc)
    list(APPEND serial_SRCS src/impl/list_ports/list_ports_win.cc)
endif()

set(serial_HEADERS
    include/serial/serial.h
    include/serial/v8stdint.h
)

# Build, link and install main library
add_library(${PROJECT_NAME} STATIC ${serial_SRCS})
add_library(ocpn::serial ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PACKAGE_VERSION}
    PUBLIC_HEADER "${serial_HEADERS}"
)
if (OBJ_VISIBILITY)
    set_target_properties(${PROJECT_NAME} PROPERTIES 
        COMPILE_FLAGS "${OBJ_VISIBILITY}"
    )
endif ()
target_include_directories(${PROJECT_NAME} PUBLIC include)
if(APPLE)
    find_library(IOKIT_LIB IOKit)
    find_library(FOUNDATION_LIB Foundation)
    target_link_libraries(${PROJECT_NAME} ${FOUNDATION_LIB} ${IOKIT_LIB})
elseif(UNIX)
    set(rt_LIBRARIES rt)
    set(pthread_LIBRARIES pthread)
    target_link_libraries(${PROJECT_NAME} rt pthread)
else()
    target_link_libraries(${PROJECT_NAME} setupapi)
endif()

# Other targets: test program, pkg-config and tests.
if (ENABLE_TEST_PROGRAM)
    add_executable(serial_example examples/serial_example.cc)
    add_dependencies(serial_example ${PROJECT_NAME})
    target_link_libraries(serial_example ${PROJECT_NAME})
endif()


if (CATKIN_ENABLE_TESTING)
    add_subdirectory(tests)
endif ()

if (ENABLE_API_DOCS)
    find_package(Doxygen COMPONENTS dot)
    if (DOXYGEN_FOUND)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile)
        add_custom_target(doc ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
        install(DIRECTORY  ${CMAKE_BINARY_DIR}/doc/html
            DESTINATION ${CMAKE_INSTALL_DOCDIR}
        )
    endif (DOXYGEN_FOUND)
endif (ENABLE_API_DOCS)
