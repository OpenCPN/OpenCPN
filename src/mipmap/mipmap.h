/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Efficiently generate mipmaps for use by opengl
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

#ifndef __MIPMAP_H__
#define __MIPMAP_H__

#if defined(__MSVC__) || defined(__WXOSX__)
#define bit_SSE         (1 << 25)
#define bit_SSE2        (1 << 26)
#define bit_SSE3        (1 << 0)
#define bit_SSSE3       (1 << 9)
#define bit_AVX2        (1 << 5)
#endif


#ifdef  __cplusplus
extern "C" {
#endif

extern void (*MipMap_24)( int width, int height, unsigned char *source, unsigned char *target );
extern void (*MipMap_32)( int width, int height, unsigned char *source, unsigned char *target );

void MipMap_ResolveRoutines();

void MipMap_24_generic( int width, int height, unsigned char *source, unsigned char *target );
void MipMap_32_generic( int width, int height, unsigned char *source, unsigned char *target );

void MipMap_32_sse( int width, int height, unsigned char *source, unsigned char *target );
void MipMap_32_sse2( int width, int height, unsigned char *source, unsigned char *target );
void MipMap_24_ssse3( int width, int height, unsigned char *source, unsigned char *target );
void MipMap_32_avx2( int width, int height, unsigned char *source, unsigned char *target );

#ifdef  __cplusplus
}
#endif
#endif
