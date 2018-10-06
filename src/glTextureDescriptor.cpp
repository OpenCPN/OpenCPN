/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#include "glTextureDescriptor.h"
#include <wx/thread.h>

wxCriticalSection gs_critSect;

glTextureDescriptor::glTextureDescriptor()
{
    for( int i = 0; i < 10; i++ ){
        map_array[i] = NULL;
        comp_array[i] = NULL;
        compcomp_array[i] = NULL;
        compcomp_size[i] = 0;
    }

    tex_name = 0;
    nGPU_compressed = GPU_TEXTURE_UNKNOWN;
    tex_mem_used = 0;
    compdata_ticks = 0;
}

glTextureDescriptor::~glTextureDescriptor()
{
    FreeAll();
}

void glTextureDescriptor::FreeAll()
{
    FreeMap();
    FreeComp();
    FreeCompComp();
}

void glTextureDescriptor::FreeMap()
{
    for( int i = 0; i < 10; i++ ){
        free( map_array[i] );
        map_array[i] = 0;
    }
}

void glTextureDescriptor::FreeComp()
{
    for( int i = 0; i < 10; i++ ){
        free( comp_array[i] );
        comp_array[i] = NULL;
    }
}
    
void glTextureDescriptor::FreeCompComp()
{
    for( int i = 0; i < 10; i++ ){
        free( compcomp_array[i] );
        compcomp_array[i] = NULL;
        compcomp_size[i] = 0;
    }
}

size_t glTextureDescriptor::GetMapArrayAlloc(void)
{
    
    size_t size = 512 * 512 * 3;
    size_t ret = 0;
    for( int i = 0; i < 10; i++ ){
        if( map_array[i] ){
            ret += size;
        }
        size /= 4;
    }
    
    return ret;
}

size_t glTextureDescriptor::GetCompArrayAlloc(void)
{
    size_t size = (512 * 512 * 3) / 6;
    size_t ret = 0;
    for( int i = 0; i < 10; i++ ){
        if( comp_array[i] ){
            ret += size;
        }
        size /= 4;
    }
    
    return ret;
}

size_t glTextureDescriptor::GetCompCompArrayAlloc(void)
{
    size_t ret = 0;
    for( int i = 0; i < 10; i++ ){
        if( compcomp_size[i] )
            ret += compcomp_size[i];
    }
    
    return ret;
}

bool glTextureDescriptor::IsCompCompArrayComplete( int base_level )
{
    extern int g_mipmap_max_level;
    for(int level = base_level; level < g_mipmap_max_level+1; level++ )
        if(!compcomp_array[level])
            return false;

    return true;
}
