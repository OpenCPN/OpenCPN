/*
  Modified for OpenCPN  by Sean D'Epagnier 2014

Copyright (c) 2013, Bartosz Taudul <wolf.pld@gmail.com>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <string.h>
#include <stdint.h>

#define sq(X) ((X)*(X))

typedef int8_t      int8;
typedef uint8_t     uint8;
typedef int16_t     int16;
typedef uint16_t    uint16;
typedef int32_t     int32;
typedef uint32_t    uint32;
typedef int64_t     int64;
typedef uint64_t    uint64;

typedef unsigned int uint;

static const int32 g_table[8][4] = {
    {  2,  8,   -2,   -8 },
    {  5, 17,   -5,  -17 },
    {  9, 29,   -9,  -29 },
    { 13, 42,  -13,  -42 },
    { 18, 60,  -18,  -60 },
    { 24, 80,  -24,  -80 },
    { 33, 106, -33, -106 },
    { 47, 183, -47, -183 }
};

static const int64 g_table256[8][4] = {
    {  2*256,  8*256,   -2*256,   -8*256 },
    {  5*256, 17*256,   -5*256,  -17*256 },
    {  9*256, 29*256,   -9*256,  -29*256 },
    { 13*256, 42*256,  -13*256,  -42*256 },
    { 18*256, 60*256,  -18*256,  -60*256 },
    { 24*256, 80*256,  -24*256,  -80*256 },
    { 33*256, 106*256, -33*256, -106*256 },
    { 47*256, 183*256, -47*256, -183*256 }
};

static const uint32 g_id[4][16] = {
    { 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 3, 3, 2, 2, 3, 3, 2, 2, 3, 3, 2, 2, 3, 3, 2, 2 },
    { 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4 },
    { 7, 7, 6, 6, 7, 7, 6, 6, 7, 7, 6, 6, 7, 7, 6, 6 }
};

static const uint32 g_avg2[16] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
};

static size_t GetLeastError( const uint64* err, size_t num )
{
    size_t idx = 0;
    for( size_t i=1; i<num; i++ )
    {
        if( err[i] < err[idx] )
        {
            idx = i;
        }
    }
    return idx;
}

static void Average( int32_t avg[3], const uint8* data )
{
    avg[0] = avg[1] = avg[2] = 0;
    for( int i=0; i<8; i++ )
    {
        avg[0] += *data++;
        avg[1] += *data++;
        avg[2] += *data++;
     }
    avg[0] /= 8;
    avg[1] /= 8;
    avg[2] /= 8;
}

static void CalcErrorBlock( const uint8* data, uint err[4] )
{
    for( int i=0; i<8; i++ )
    {
        uint d = *data++;
        err[0] += d;
        err[3] += d*d;
        d = *data++;
        err[1] += d;
        err[3] += d*d;
        d = *data++;
        err[2] += d;
        err[3] += d*d;
    }
}

static uint CalcError( const uint block[4], const int32_t average[3] )
{
    uint err = block[3];
    err -= block[0] * 2 * average[2];
    err -= block[1] * 2 * average[1];
    err -= block[2] * 2 * average[0];
    err += 8 * ( sq( average[0] ) + sq( average[1] ) + sq( average[2] ) );
    return err;
}

static void ProcessAverages( int32_t a[8][3] )
{
    for( int i=0; i<2; i++ )
    {
        for( int j=0; j<3; j++ )
        {
            int32 c1 = a[i*2+1][j] >> 3;
            int32 c2 = a[i*2][j] >> 3;

            int32 diff = c2 - c1;
            if( diff > 3 ) diff = 3;
            else if( diff < -4 ) diff = -4;

            int32 co = c1 + diff;

            a[5+i*2][j] = ( c1 << 3 ) | ( c1 >> 2 );
            a[4+i*2][j] = ( co << 3 ) | ( co >> 2 );
        }
    }
    for( int i=0; i<4; i++ )
    {
        a[i][0] = g_avg2[a[i][0] >> 4];
        a[i][1] = g_avg2[a[i][1] >> 4];
        a[i][2] = g_avg2[a[i][2] >> 4];
    }
}

static void EncodeAverages( uint64& _d, const int32_t a[8][3], size_t idx )
{
    uint64 d = _d;
    d |= ( idx << 24 );
    size_t base = idx << 1;

    if( ( idx & 0x2 ) == 0 )
    {
        for( int i=0; i<3; i++ )
        {
            d |= uint64( a[base+0][i] >> 4 ) << ( i*8 );
            d |= uint64( a[base+1][i] >> 4 ) << ( i*8 + 4 );
        }
    }
    else
    {
        for( int i=0; i<3; i++ )
        {
            d |= uint64( a[base+1][i] & 0xF8 ) << ( i*8 );
            int32 c = ( ( a[base+0][i] & 0xF8 ) - ( a[base+1][i] & 0xF8 ) ) >> 3;
            c &= ~0xFFFFFFF8;
            d |= ((uint64)c) << ( i*8 );
        }
    }
    _d = d;
}

static uint64 CheckSolid( const uint8* src )
{
    bool solid = true;
    const uint8* ptr = src + 3;
    for( int i=1; i<16; i++ )
    {
        if( memcmp( src, ptr, 3 ) != 0 )
        {
            return 0;
        }
        ptr += 3;
    }
    return 0x02000000 |
        ( uint( src[0] & 0xF8 ) ) |
        ( uint( src[1] & 0xF8 ) << 8 ) |
        ( uint( src[2] & 0xF8 ) << 16);
}

uint64 ProcessRGB( const uint8* src )
{
    uint64 d = CheckSolid( src );
    if( d != 0 ) return d;

    uint8 b23[2][24];
    const uint8* b[4] = { src+24, src, b23[0], b23[1] };

    for( int i=0; i<4; i++ )
    {
        memcpy( b23[1]+i*6, src+i*12,   6 );
        memcpy( b23[0]+i*6, src+i*12+6, 6 );
    }

    int32_t a[8][3];
    for( int i=0; i<4; i++ )
    {
        Average(a[i], b[i] );
    }
    ProcessAverages( a );

    uint64 err[4] = {};
    for( int i=0; i<4; i++ )
    {
        uint errblock[4] = {};
        CalcErrorBlock( b[i], errblock );
        err[i/2] += CalcError( errblock, a[i] );
        err[2+i/2] += CalcError( errblock, a[i+4] );
    }
    size_t idx = GetLeastError( err, 4 );

    EncodeAverages( d, a, idx );

    uint64 terr[2][8] = {};
    uint tsel[16][8];
    const uint32 *id = g_id[idx];
    const uint8* data = src;
    for( size_t i=0; i<16; i++ )
    {
        uint* sel = tsel[i];
        uint bid = id[i];
        uint64* ter = terr[bid%2];

        uint8 r = *data++;
        uint8 g = *data++;
        uint8 b = *data++;

        int dr = a[bid][0] - r;
        int dg = a[bid][1] - g;
        int db = a[bid][2] - b;

        int pix = dr * 77 + dg * 151 + db * 28;

        for( int t=0; t<8; t++ )
        {
            const int64* tab = g_table256[t];
            uint idx = 0;
            uint64 err = sq( tab[0] + pix );
            for( int j=1; j<4; j++ )
            {
                uint64 local = sq( tab[j] + pix );
                if( local < err )
                {
                    err = local;
                    idx = j;
                }
            }
            *sel++ = idx;
            *ter++ += err;
        }
    }
    size_t tidx[2];
    tidx[0] = GetLeastError( terr[0], 8 );
    tidx[1] = GetLeastError( terr[1], 8 );

    d |= tidx[0] << 26;
    d |= tidx[1] << 29;
    for( int i=0; i<16; i++ )
    {
        uint64 t = tsel[i][tidx[id[i]%2]];
        d |= ( t & 0x1 ) << ( i + 32 );
        d |= ( t & 0x2 ) << ( i + 47 );
    }

    d = ( ( d & 0x00000000FFFFFFFF ) ) |
        ( ( d & 0xFF00000000000000 ) >> 24 ) |
        ( ( d & 0x000000FF00000000 ) << 24 ) |
        ( ( d & 0x00FF000000000000 ) >> 8 ) |
        ( ( d & 0x0000FF0000000000 ) << 8 );

    return d;
}
