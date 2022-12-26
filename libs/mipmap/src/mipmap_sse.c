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

#if defined(__SSE__)
#include <xmmintrin.h>

// typically 4 times faster than generic
void MipMap_32_sse( int width, int height, unsigned char *source, unsigned char *target )
{
    if(width < 4) {
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
        for( x = 0; x < newwidth; x+=2 ) {
            __m64 a0, a1, a2, a3;

            memcpy(&a0, t,   8);
            memcpy(&a1, t+8, 8);
            memcpy(&a2, u,   8);
            memcpy(&a3, u+8, 8);

            // average first and second scan lines
            a0 = _mm_avg_pu8(a0, a2);
            a1 = _mm_avg_pu8(a1, a3);

            // shuffle
            a2 = _mm_unpacklo_pi32(a0, a1);
            a3 = _mm_unpackhi_pi32(a0, a1);

            // average even and odd pixels
            a0 = _mm_avg_pu8(a2, a3);
            memcpy(s, &a0, 8);

            s+=8;
            t+=16;
            u+=16;
        }
        t += stride;
        u += stride;
    }
    _mm_empty();
}
#endif
