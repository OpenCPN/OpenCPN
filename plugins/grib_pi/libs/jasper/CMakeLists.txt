cmake_minimum_required(VERSION 3.5.0)

SET(SRC
    src/base/jas_cm.c
    src/base/jas_debug.c
    src/base/jas_getopt.c
    src/base/jas_icc.c
    src/base/jas_iccdata.c
    src/base/jas_image.c
    src/base/jas_init.c
    src/base/jas_malloc.c
    src/base/jas_seq.c
    src/base/jas_stream.c
    src/base/jas_string.c
    src/base/jas_tmr.c
    src/base/jas_tvp.c
    src/base/jas_version.c
    src/include/jasper/jas_cm.h
    src/include/jasper/jas_config2.h
    src/include/jasper/jas_config.h
    src/include/jasper/jas_config.h.in
    src/include/jasper/jas_debug.h
    src/include/jasper/jas_fix.h
    src/include/jasper/jas_getopt.h
    src/include/jasper/jas_icc.h
    src/include/jasper/jas_image.h
    src/include/jasper/jas_init.h
    src/include/jasper/jas_malloc.h
    src/include/jasper/jas_math.h
    src/include/jasper/jasper.h
    src/include/jasper/jas_seq.h
    src/include/jasper/jas_stream.h
    src/include/jasper/jas_string.h
    src/include/jasper/jas_tmr.h
    src/include/jasper/jas_tvp.h
    src/include/jasper/jas_types.h
    src/include/jasper/jas_version.h
    src/jp2/jp2_cod.c
    src/jp2/jp2_cod.h
    src/jp2/jp2_dec.c
    src/jp2/jp2_dec.h
    src/jp2/jp2_enc.c
    src/jpc/jpc_bs.c
    src/jpc/jpc_bs.h
    src/jpc/jpc_cod.h
    src/jpc/jpc_cs.c
    src/jpc/jpc_cs.h
    src/jpc/jpc_dec.c
    src/jpc/jpc_dec.h
    src/jpc/jpc_enc.c
    src/jpc/jpc_enc.h
    src/jpc/jpc_fix.h
    src/jpc/jpc_flt.h
    src/jpc/jpc_math.c
    src/jpc/jpc_math.h
    src/jpc/jpc_mct.c
    src/jpc/jpc_mct.h
    src/jpc/jpc_mqcod.c
    src/jpc/jpc_mqcod.h
    src/jpc/jpc_mqdec.c
    src/jpc/jpc_mqdec.h
    src/jpc/jpc_mqenc.c
    src/jpc/jpc_mqenc.h
    src/jpc/jpc_qmfb.c
    src/jpc/jpc_qmfb.h
    src/jpc/jpc_t1cod.c
    src/jpc/jpc_t1cod.h
    src/jpc/jpc_t1dec.c
    src/jpc/jpc_t1dec.h
    src/jpc/jpc_t1enc.c
    src/jpc/jpc_t1enc.h
    src/jpc/jpc_t2cod.c
    src/jpc/jpc_t2cod.h
    src/jpc/jpc_t2dec.c
    src/jpc/jpc_t2dec.h
    src/jpc/jpc_t2enc.c
    src/jpc/jpc_t2enc.h
    src/jpc/jpc_tagtree.c
    src/jpc/jpc_tagtree.h
    src/jpc/jpc_tsfb.c
    src/jpc/jpc_tsfb.h
    src/jpc/jpc_util.c
    src/jpc/jpc_util.h
)

set(CMAKE_POSITION_INDEPENDENT_CODE "ON")
add_library(JASPER STATIC ${SRC})
set_property(TARGET JASPER PROPERTY COMPILE_FLAGS "${OBJ_VISIBILITY}")
target_include_directories(JASPER BEFORE
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/include
)
target_compile_definitions(JASPER
  PRIVATE
    -DEXCLUDE_MIF_SUPPORT
    -DEXCLUDE_PNM_SUPPORT
    -DEXCLUDE_BMP_SUPPORT
    -DEXCLUDE_RAS_SUPPORT
    -DEXCLUDE_JPG_SUPPORT
    -DEXCLUDE_PGX_SUPPORT
  PUBLIC
    -DJASPER
)

# FIXME: this hides some nasty warnings, some of which fixed upstream.
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    target_compile_definitions(JASPER PRIVATE "-D_GNU_SOURCE=1")
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|AppleClang")
    target_compile_options(JASPER PRIVATE
        "-Wno-format"
        "-Wno-tautological-constant-out-of-range-compare"
    )
endif ()
