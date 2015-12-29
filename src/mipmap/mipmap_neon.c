/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  efficient mipmap generation for use by opengl
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2015 by David S. Register                               *
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

#ifdef __ARM_NEON__
#include <arm_neon.h>

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
