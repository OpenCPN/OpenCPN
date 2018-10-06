aux_source_directory(host HOST_SOURCES)
add_library(proxyhost SHARED EXCLUDE_FROM_ALL ${HOST_SOURCES})
target_link_libraries(proxyhost GL asound)

add_library(proxyhost_static STATIC EXCLUDE_FROM_ALL ${HOST_SOURCES})
set_target_properties(proxyhost_static PROPERTIES OUTPUT_NAME proxyhost)

add_subdirectory(client)
