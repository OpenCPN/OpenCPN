/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  efficient mipmap generation for use by opengl
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2016 by Sean D'Epagnier                                 *
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


#if defined(__ARM_NEON) || defined(__ARM_NEON_FP)
#include <arm_neon.h>

// about twice as fast as generic
void MipMap_24_neon( int width, int height, unsigned char *source, unsigned char *target )
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
            uint8x16_t a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, aa, ab;

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
            a0 = vhaddq_u8(a0, a6);
            a1 = vhaddq_u8(a1, a7);
            a2 = vhaddq_u8(a2, a8);
            a3 = vhaddq_u8(a3, a9);
            a4 = vhaddq_u8(a4, aa);
            a5 = vhaddq_u8(a5, ab);

            // at this point, we have averaged the two scanlines,
            // in 24bit it's a bit of a pain to average the pixels
            // because they are aligned to 3 bytes

#if 0  // simple readable version finishing without neon (slower)
            uint8_t b[96], c[96];
            memcpy(b,    &a0, 16);
            memcpy(b+16, &a1, 16);
            memcpy(b+32, &a2, 16);
            memcpy(b+48, &a3, 16);
            memcpy(b+64, &a4, 16);
            memcpy(b+80, &a5, 16);

            int i,j;
            for( i=0; i<16; i++)
                for( j=0; j<3; j++)
                    s[3*i+j] = (b[3*2*i+j] + b[3*(2*i+1)+j]) / 2;
#else
            // full neon version with swizzel (ugly but fast)
            uint8x8_t r0, r1;

            uint8x8x4_t z;
            uint8x8x2_t *z2 = (uint8x8x2_t*)&z, *z3 = (uint8x8x2_t*)&z+1;

//          a00 a01 a02 a06 a07 a08 a0c a0d a0e a12 a13 a14 a18 a19 a1a a1e
//          a03 a04 a05 a09 a0a a0b a0f a10 a11 a15 a16 a17 a1b a1c a1d a21
#define int8x16_to_8x8x2(v) ((int8x8x2_t) { vget_low_s8(v), vget_high_s8(v) })

            uint8x8_t s0l = {0, 1, 2, 6, 7, 8, 12, 13};
            memcpy(&z, &a0, sizeof a0);
            r0 = vtbl2_u8(*z2, s0l);

            memcpy(z3, &a1, sizeof a1);
            uint8x8_t s0h = {14, 16+2, 16+3, 16+4, 16+8, 16+9, 16+10, 16+14};
            r1 = vtbl4_u8(z, s0h);
            a6 = vcombine_u8 (r0, r1);

            uint8x8_t s1l = {3, 4, 5, 9, 10, 11, 15, 16+0};
            r0 = vtbl4_u8(z, s1l);
            memcpy(&z, &a2, sizeof a2);
            uint8x8_t s1h = {16+1, 16+5, 16+6, 16+7, 16+11, 16+12, 16+13, 1};
            r1 = vtbl4_u8(z, s1h);
            a7 = vcombine_u8 (r0, r1);

            a0 = vhaddq_u8(a6, a7);

//          a1f a20 a24 a25 a26 a2a a2b a2c a30 a31 a32 a36 a37 a38 a3c a3d
//          a22 a23 a27 a28 a29 a2d a2e a2f a33 a34 a35 a39 a3a a3b a3f a40

            uint8x8_t s2l = {16+15, 0, 4, 5, 6, 10, 11, 12};
            r0 = vtbl4_u8(z, s2l);
            uint8x8_t s2h = {0, 1, 2, 6, 7, 8, 12, 13};
            memcpy(z3, &a3, sizeof a3);
            r1 = vtbl2_u8(*z3, s2h);
            a6 = vcombine_u8 (r0, r1);

            uint8x8_t s3l = {2, 3, 7, 8, 9, 13, 14, 15};
            r0 = vtbl2_u8(*z2, s3l);
            memcpy(&z, &a4, sizeof a4);
            uint8x8_t s3h = {16+3, 16+4, 16+5, 16+9, 16+10, 16+11, 16+15, 0};
            r1 = vtbl4_u8(z, s3h);
            a7 = vcombine_u8 (r0, r1);

            a1 = vhaddq_u8(a6, a7);

//          a3e a42 a43 a44 a48 a49 a4a a4e a4f a50 a54 a55 a56 a5a a5b a5c
//          a41 a45 a46 a47 a4b a4c a4d a51 a52 a53 a57 a58 a59 a5d a5e a5f

            uint8x8_t s4l = {16+14, 2, 3, 4, 8, 9, 10, 14};
            r0 = vtbl4_u8(z, s4l);
            memcpy(z3, &a5, sizeof a5);
            uint8x8_t s4h = {15, 16+0, 16+4, 16+5, 16+6, 16+10, 16+11, 16+12};
            r1 = vtbl4_u8(z, s4h);
            a6 = vcombine_u8 (r0, r1);

            uint8x8_t s5l = {1, 5, 6, 7, 11, 12, 13, 16+1};
            r0 = vtbl4_u8(z, s5l);
            uint8x8_t s5h = {2, 3, 7, 8, 9, 13, 14, 15};
            r1 = vtbl2_u8(*z3, s5h);
            a7 = vcombine_u8 (r0, r1);

            a2 = vhaddq_u8(a6, a7);

            memcpy(s,    &a0, 16);
            memcpy(s+16, &a1, 16);
            memcpy(s+32, &a2, 16);
#endif
            s+=48;
            t+=96;
            u+=96;
        }
        t += stride;
        u += stride;
    }
}

// about twice as fast as generic
void MipMap_32_neon( int width, int height, unsigned char *source, unsigned char *target )
{
    if(width < 8) {
        MipMap_32_generic(width, height, source, target);
        return;
    }

    int newwidth = width / 2;
    int newheight = height / 2;
    int stride = width * 4;

    unsigned char *s = target;
    unsigned char *t = source;
    unsigned char *u = t+stride;

    int y, x;
    for( y = 0; y < newheight; y++ ) {
        for( x = 0; x < newwidth; x+=4 ) {
            uint8x16_t a0, a1, a2, a3;

            memcpy(&a0, t,    16);
            memcpy(&a1, t+16, 16);
            memcpy(&a2, u,    16);
            memcpy(&a3, u+16, 16);

            // average first and second scan lines
            a0 = vhaddq_u8(a0, a2);
            a1 = vhaddq_u8(a1, a3);

            // repack
            uint32x4x2_t z = vuzpq_u32(vreinterpretq_u32_u8(a0), vreinterpretq_u32_u8(a1));
            uint8x16_t d0, d1;
            memcpy(&d0, &z.val[0], 16), memcpy(&d1, &z.val[1], 16);

            // average even and odd x pixels
            a0 = vhaddq_u8(vreinterpretq_u8_u32(z.val[0]), vreinterpretq_u8_u32(z.val[1]));

            memcpy(s, &a0, 16);

            s+=16;
            t+=32;
            u+=32;
        }
        t += stride;
        u += stride;
    }
}
#endif
