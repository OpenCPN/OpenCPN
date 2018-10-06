aux_source_directory(src CLIENT_SOURCES)
add_library(proxyclient SHARED EXCLUDE_FROM_ALL ${CLIENT_SOURCES})
include_directories(.)

set_target_properties(proxyclient PROPERTIES
    VERSION "2.0.0"
    SOVERSION "2"
)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-version-script=Versions")

if(CMAKE_SHARED_LIBRARY_SUFFIX MATCHES ".so")
    set_target_properties(proxyclient PROPERTIES SUFFIX ".so.1")
endif()
