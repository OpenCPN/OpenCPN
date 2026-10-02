# libaec 1.1.5 (https://gitlab.dkrz.de/k202009/libaec), decoder only
add_library(AEC STATIC src/decode.c src/vector.c)
set_property(TARGET AEC PROPERTY COMPILE_FLAGS "${OBJ_VISIBILITY}")
set_property(TARGET AEC PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(AEC PUBLIC include PRIVATE src)
