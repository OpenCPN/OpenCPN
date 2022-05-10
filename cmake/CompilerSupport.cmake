#
# Check for supported compiler flags
#

include(CheckSymbolExists)
include(CheckCCompilerFlag)

check_symbol_exists(__SSE__ xmmintrin.h HAVE_MSSE)
check_symbol_exists(__SSE2__ emmintrin.h HAVE_MSSE2)
check_symbol_exists(__SSE3__ pmmintrin.h HAVE_MSSE3)
check_symbol_exists(__AVX2__ immintrin.h HAVE_MAVX2)
check_symbol_exists(__ARM_NEON arm_neon.h HAVE_ARM_NEON)
if (HAVE_ARM_NEON)
    check_c_compiler_flag(-mfpu=neon HAVE_MFPU_NEON)
endif()
