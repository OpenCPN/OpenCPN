#
# Add linkage to the rapidjson header-only library
#
# On hosts having the package installed in system locations found by
# cmake use the system code. If not, download sources from rapidjson
# github site.
#
# Exports: ocpn::rapidjson link target

cmake_minimum_required(VERSION 3.1.20)

set(src_url "https://github.com/Tencent/rapidjson/archive/refs/tags/v1.1.0.tar.gz")
set(src_hash MD5=badd12c511e081fec6c89c43a7027bce)

if (TARGET ocpn::rapidjson)
    return ()
endif ()

find_path(RAPIDJSON_BASE NAME rapidjson.h PATH_SUFFIXES rapidjson)
if (RAPIDJSON_BASE)
  cmake_path(GET RAPIDJSON_BASE PARENT_PATH RAPIDJSON_BASE)   # drop rapidjson/
  message(STATUS "Using system rapidjson at ${RAPIDJSON_BASE}")
  cmake_path(GET RAPIDJSON_BASE PARENT_PATH RAPIDJSON_BASE)   # drop include/
  set(SRC_DIR ${RAPIDJSON_BASE})
else ()
  include(FetchContent)
  FetchContent_Declare(
    rapidjson
    URL  ${src_url}
    URL_HASH ${src_hash}
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  )
  FetchContent_Populate(rapidjson)
  FetchContent_GetProperties(rapidjson SOURCE_DIR SRC_DIR)
  message(STATUS "Using downloaded rapidjson at ${SRC_DIR}/include")
endif ()

set(SRC
  ${SRC_DIR}/include/rapidjson
  ${SRC_DIR}/include/rapidjson/allocators.h
  ${SRC_DIR}/include/rapidjson/document.h
  ${SRC_DIR}/include/rapidjson/encodedstream.h
  ${SRC_DIR}/include/rapidjson/encodings.h
  ${SRC_DIR}/include/rapidjson/error
  ${SRC_DIR}/include/rapidjson/filereadstream.h
  ${SRC_DIR}/include/rapidjson/filewritestream.h
  ${SRC_DIR}/include/rapidjson/fwd.h
  ${SRC_DIR}/include/rapidjson/internal
  ${SRC_DIR}/include/rapidjson/istreamwrapper.h
  ${SRC_DIR}/include/rapidjson/memorybuffer.h
  ${SRC_DIR}/include/rapidjson/memorystream.h
  ${SRC_DIR}/include/rapidjson/msinttypes
  ${SRC_DIR}/include/rapidjson/ostreamwrapper.h
  ${SRC_DIR}/include/rapidjson/pointer.h
  ${SRC_DIR}/include/rapidjson/prettywriter.h
  ${SRC_DIR}/include/rapidjson/rapidjson.h
  ${SRC_DIR}/include/rapidjson/reader.h
  ${SRC_DIR}/include/rapidjson/schema.h
  ${SRC_DIR}/include/rapidjson/stream.h
  ${SRC_DIR}/include/rapidjson/stringbuffer.h
  ${SRC_DIR}/include/rapidjson/writer.h
  ${SRC_DIR}/include/rapidjson/error/en.h
  ${SRC_DIR}/include/rapidjson/error/error.h
  ${SRC_DIR}/include/rapidjson/internal/biginteger.h
  ${SRC_DIR}/include/rapidjson/internal/diyfp.h
  ${SRC_DIR}/include/rapidjson/internal/dtoa.h
  ${SRC_DIR}/include/rapidjson/internal/ieee754.h
  ${SRC_DIR}/include/rapidjson/internal/itoa.h
  ${SRC_DIR}/include/rapidjson/internal/meta.h
  ${SRC_DIR}/include/rapidjson/internal/pow10.h
  ${SRC_DIR}/include/rapidjson/internal/regex.h
  ${SRC_DIR}/include/rapidjson/internal/stack.h
  ${SRC_DIR}/include/rapidjson/internal/strfunc.h
  ${SRC_DIR}/include/rapidjson/internal/strtod.h
  ${SRC_DIR}/include/rapidjson/internal/swap.h
  ${SRC_DIR}/include/rapidjson/msinttypes/inttypes.h
  ${SRC_DIR}/include/rapidjson/msinttypes/stdint.h
)

add_library(_RAPIDJSON STATIC ${SRC})
add_library(ocpn::rapidjson ALIAS _RAPIDJSON)

target_include_directories(_RAPIDJSON PUBLIC "${SRC_DIR}/include")

set_target_properties(_RAPIDJSON PROPERTIES LINKER_LANGUAGE CXX)
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|GNU")  # Apple is AppleClang
  target_compile_options(
    _RAPIDJSON PRIVATE -fvisibility=default -Wno-unknown-pragmas -fPIC
  )
endif ()
