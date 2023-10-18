/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  efficient mipmap generation for use by opengl
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2015 by Sean D'Epagnier                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */

#include <string.h>
#include "mipmap.h"

#if defined(__SSSE3__) || (defined(__MSVC__) &&  (_MSC_VER >= 1700))
#include <tmmintrin.h>

// typically 5 times faster than generic
void MipMap_24_ssse3( int width, int height, unsigned char *source, unsigned char *target )
{
    if(width < 32) {
        MipMap_24_generic(width, height, source, target);
        return;
    }

    int newwidth = width / 2;
    int newheight = height / 2;
    int stride = width * 3;

    unsigned char *s = target;
    unsigned char *t = source;
    unsigned char *u = t+stride;

    int y, x;
    for( y = 0; y < newheight; y++ ) {
        for( x = 0; x < newwidth; x+=16 ) {
            // we must operate on 16 output pixels (64 input pixels at a time)
            // to fully pack the 128 bit registers

            __m128i a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, aa, ab;
            __m128 *b6 = (__m128*)&a6, *b7 = (__m128*)&a7, *b8 = (__m128*)&a8, *b9 = (__m128*)&a9;
            memcpy(&a0, t,    16);
            memcpy(&a1, t+16, 16);
            memcpy(&a2, t+32, 16);
            memcpy(&a3, t+48, 16);
            memcpy(&a4, t+64, 16);
            memcpy(&a5, t+80, 16);
            memcpy(&a6, u,    16);
            memcpy(&a7, u+16, 16);
            memcpy(&a8, u+32, 16);
            memcpy(&a9, u+48, 16);
            memcpy(&aa, u+64, 16);
            memcpy(&ab, u+80, 16);

            // average first and second scan lines
            a0 = _mm_avg_epu8(a0, a6);
            a1 = _mm_avg_epu8(a1, a7);
            a2 = _mm_avg_epu8(a2, a8);
            a3 = _mm_avg_epu8(a3, a9);
            a4 = _mm_avg_epu8(a4, aa);
            a5 = _mm_avg_epu8(a5, ab);

            // shuffle table
            unsigned char Z = 0x80;

//          a00 a01 a02 a06 a07 a08 a0c a0d a0e a12 a13 a14 a18 a19 a1a a1e
//          a03 a04 a05 a09 a0a a0b a0f a10 a11 a15 a16 a17 a1b a1c a1d a21

            __m128i s0 = _mm_set_epi8(Z, Z, Z, Z, Z, Z, Z, 14, 13, 12, 8, 7, 6, 2, 1, 0);
            __m128i s1 = _mm_set_epi8(14, 10, 9, 8, 4, 3, 2, Z, Z, Z, Z, Z, Z, Z, Z, Z);

            a6 = _mm_shuffle_epi8(a0, s0);
            a7 = _mm_shuffle_epi8(a1, s1);

            *b6 = _mm_or_ps(*b6, *b7);

            __m128i s2 = _mm_set_epi8(Z, Z, Z, Z, Z, Z, Z, Z, Z, 15, 11, 10, 9, 5, 4, 3);
            __m128i s3 = _mm_set_epi8(Z, 13, 12, 11, 7, 6, 5, 1, 0, Z, Z, Z, Z, Z, Z, Z);
            __m128i s4 = _mm_set_epi8(1, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z);

            a7 = _mm_shuffle_epi8(a0, s2);
            a8 = _mm_shuffle_epi8(a1, s3);
            a9 = _mm_shuffle_epi8(a2, s4);

            *b7 = _mm_or_ps(*b7, *b8);
            *b7 = _mm_or_ps(*b7, *b9);

            a0 = _mm_avg_epu8(a6, a7);

//          a1f a20 a24 a25 a26 a2a a2b a2c a30 a31 a32 a36 a37 a38 a3c a3d
//          a22 a23 a27 a28 a29 a2d a2e a2f a33 a34 a35 a39 a3a a3b a3f a40

            __m128i s5 = _mm_set_epi8(Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, 15);
            __m128i s6 = _mm_set_epi8(Z, Z, Z, Z, Z, Z, Z, Z, 12, 11, 10, 6, 5, 4, 0, Z);
            __m128i s7 = _mm_set_epi8(13, 12, 8, 7, 6, 2, 1, 0, Z, Z, Z, Z, Z, Z, Z, Z);

            a6 = _mm_shuffle_epi8(a1, s5);
            a7 = _mm_shuffle_epi8(a2, s6);
            a8 = _mm_shuffle_epi8(a3, s7);

            *b6 = _mm_or_ps(*b6, *b7);
            *b6 = _mm_or_ps(*b6, *b8);

            __m128i s8 = _mm_set_epi8(Z, Z, Z, Z, Z, Z, Z, Z, 15, 14, 13, 9, 8, 7, 3, 2);
            __m128i s9 = _mm_set_epi8(Z, 15, 11, 10, 9, 5, 4, 3, Z, Z, Z, Z, Z, Z, Z, Z);
            __m128i sa = _mm_set_epi8(0, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z);

            a7 = _mm_shuffle_epi8(a2, s8);
            a8 = _mm_shuffle_epi8(a3, s9);
            a9 = _mm_shuffle_epi8(a4, sa);

            *b7 = _mm_or_ps(*b7, *b8);
            *b7 = _mm_or_ps(*b7, *b9);

            a1 = _mm_avg_epu8(a6, a7);

//          a3e a42 a43 a44 a48 a49 a4a a4e a4f a50 a54 a55 a56 a5a a5b a5c
//          a41 a45 a46 a47 a4b a4c a4d a51 a52 a53 a57 a58 a59 a5d a5e a5f

            __m128i sb = _mm_set_epi8( Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, 14);
            __m128i sc = _mm_set_epi8( Z, Z, Z, Z, Z, Z, Z, 15, 14, 10, 9, 8, 4, 3, 2, Z);
            __m128i sd = _mm_set_epi8( 12, 11, 10, 6, 5, 4, 0, Z, Z, Z, Z, Z, Z, Z, Z, Z);

            a6 = _mm_shuffle_epi8(a3, sb);
            a7 = _mm_shuffle_epi8(a4, sc);
            a8 = _mm_shuffle_epi8(a5, sd);

            *b6 = _mm_or_ps(*b6, *b7);
            *b6 = _mm_or_ps(*b6, *b8);

            __m128i se = _mm_set_epi8(Z, Z, Z, Z, Z, Z, Z, Z, Z, 13, 12, 11, 7, 6, 5, 1);
            __m128i sf = _mm_set_epi8(15, 14, 13, 9, 8, 7, 3, 2, 1, Z, Z, Z, Z, Z, Z, Z);

            a7 = _mm_shuffle_epi8(a4, se);
            a8 = _mm_shuffle_epi8(a5, sf);

            *b7 = _mm_or_ps(*b7, *b8);

            a2 = _mm_avg_epu8(a6, a7);

            memcpy(s,    &a0, 16);
            memcpy(s+16, &a1, 16);
            memcpy(s+32, &a2, 16);

            s+=48;
            t+=96;
            u+=96;
        }
        t += stride;
        u += stride;
    }
}
#endif
