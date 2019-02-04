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

#if defined(__SSE2__) || (defined(__MSVC__) &&  (_MSC_VER >= 1700))

#include <emmintrin.h>

// typically 8 times faster than generic
void MipMap_32_sse2( int width, int height, unsigned char *source, unsigned char *target )
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
            __m128i a0, a1, a2, a3;

            memcpy(&a0, t,    16);
            memcpy(&a1, t+16, 16);
            memcpy(&a2, u,    16);
            memcpy(&a3, u+16, 16);

            // average first and second scan lines
            a0 = _mm_avg_epu8(a0, a2);
            a1 = _mm_avg_epu8(a1, a3);
#if 1
            // shuffle (somehow this is slightly faster than unpack in some cases why?)
            __m128 *b0 = (__m128*)&a0, *b1 = (__m128*)&a1, *b2 = (__m128*)&a2, *b3 = (__m128*)&a3;
            *b2 = _mm_shuffle_ps(*b1, *b0, _MM_SHUFFLE(3, 1, 3, 1));
            *b3 = _mm_shuffle_ps(*b1, *b0, _MM_SHUFFLE(2, 0, 2, 0));
#else
            a2 = _mm_unpacklo_epi64(a0, a1);
            a3 = _mm_unpackhi_epi64(a0, a1);
#endif
            // average even and odd pixels
            a0 = _mm_avg_epu8(a2, a3);

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
