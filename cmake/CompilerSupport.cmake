#
# Check for supported compiler flags
#

include(CheckSymbolExists)
include(CheckCXXCompilerFlag)

check_symbol_exists(__SSE__ xmmintrin.h HAVE_MSSE)
check_symbol_exists(__ARM_NEON__ arm_neon.h HAVE_ARM_NEON)
if(HAVE_ARM_NEON)
  check_cxx_compiler_flag(-mfpu=neon HAVE_MFPU_NEON)
endif()

#set(CMAKE_REQUIRED_INCLUDES /usr/lib/gcc/x86_64-redhat-linux/13/include/)
set(CMAKE_REQUIRED_FLAGS "-march=native")

include(CheckCSourceCompiles)
check_c_source_compiles(
        "#include <emmintrin.h>
        int main(void) {
            __m128i zero = _mm_setzero_si128();
            (void)zero;
            return 0;
        }"
        HAVE_MSSE2
)

check_c_source_compiles(
         "#include <tmmintrin.h>
         int main(void) {
             __m128i u, v, w;
             u = _mm_set1_epi32(1);
             v = _mm_set1_epi32(2);
             w = _mm_hadd_epi32(u, v);
             (void)w;
             return 0;
         }"
         HAVE_MSSSE3
)

check_c_source_compiles(
        "#include <immintrin.h>
        int main(void) {
            __m256i x = _mm256_set1_epi16(2);
            const __m256i y = _mm256_set1_epi16(1);
            x = _mm256_subs_epu16(x, y);
            (void)x;
            return 0;
        }"
        HAVE_MAVX2
)
