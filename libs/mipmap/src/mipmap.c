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

#include <stdio.h>

#include <stdint.h>
#include <string.h>

#include "mipmap.h"

#ifdef __MSVC__

#include <Windows.h>
#include <intrin.h>

static void cpuid(int32_t out[4], int32_t x) {
    __cpuidex(out,x,0);
}

#else
# if defined(__x86_64__) || defined(__i686__)

static void cpuid(int32_t out[4], int32_t x){
    __asm__ __volatile__ (
        "cpuid":
        "=a" (out[0]),
        "=b" (out[1]),
        "=c" (out[2]),
        "=d" (out[3])
        : "a" (x), "c" (0)
    );
}

#if !defined( __WXOSX__ ) 
#include <cpuid.h>
#endif

# endif
#endif



void MipMap_24_generic( int width, int height, unsigned char *source, unsigned char *target )
{
    int newwidth = width / 2;
    int newheight = height / 2;
    int stride = width * 3;
    
    unsigned char *s = target;
    unsigned char *t = source;
    unsigned char *u = t+stride;

    int i, j, k;
    for( i = 0; i < newheight; i++ ) {
        for( j = 0; j < newwidth; j++ ) {
            for( k = 0; k < 3; k++)
                *s++ = ( t[k] + t[k+3] + u[k] + u[k+3] ) / 4;

            t += 6;
            u += 6;
        }
        t += stride;
        u += stride;
    }
}

// Note: does not blend alpha
void MipMap_32_generic( int width, int height, unsigned char *source, unsigned char *target )
{
    int newwidth = width / 2;
    int newheight = height / 2;
    int stride = width * 4;
    
    unsigned char *s = target;
    unsigned char *t = source;
    unsigned char *u = t+stride;

    int i, j, k;
    for( i = 0; i < newheight; i++ ) {
        for( j = 0; j < newwidth; j++ ) {
#if defined(__ARM_ARCH)||(MIPMAP_32_ALPHA) // better to always work at 32bites on arm
            for( k = 0; k < 4; k++)
                *s++ = ( t[k] + t[k+4] + u[k] + u[k+4] ) / 4;
#else            
            for( k = 0; k < 3; k++)
                *s++ = ( t[k] + t[k+4] + u[k] + u[k+4] ) / 4;
            s++;
#endif            
            t += 8;
            u += 8;
        }
        t += stride;
        u += stride;
    }
}

void (*MipMap_24)( int width, int height, unsigned char *source, unsigned char *target ) = MipMap_24_generic;
void (*MipMap_32)( int width, int height, unsigned char *source, unsigned char *target ) = MipMap_32_generic;

#define GCC_VERSION (__GNUC__ * 10000 \
+ __GNUC_MINOR__ * 100 \
+ __GNUC_PATCHLEVEL__)

void MipMap_ResolveRoutines()
{
#if defined(__x86_64__) || defined(__i686__) || (defined(__MSVC__) &&  (_MSC_VER >= 1700)) 
    int info[4];
    cpuid(info, 0);

    int nIds = info[0];

    //  Detect Features
    if (nIds >= 0x00000001) {
        cpuid(info,0x00000001);

        if(info[3] & bit_SSE2)
            MipMap_32 = MipMap_32_sse2;
        else
        if(info[3] & bit_SSE)
            MipMap_32 = MipMap_32_sse;

        if(info[2] & bit_SSSE3)
            MipMap_24 = MipMap_24_ssse3;
    }
    
#if defined(__AVX2__) || (defined(__MSVC__) &&  (_MSC_VER >= 1700))
    if (nIds >= 0x00000007) {
        cpuid(info,0x00000007);

        if(info[1] & bit_AVX2)
            MipMap_32 = MipMap_32_avx2;
    }
#endif

#endif

#if defined(__ARM_NEON) || defined(__ARM_NEON_FP)
    MipMap_24 = MipMap_24_neon;
    MipMap_32 = MipMap_32_neon;
#endif
}
