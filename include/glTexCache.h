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

#ifndef __GLTEXTCACHE_H__
#define __GLTEXTCACHE_H__

#include <wx/glcanvas.h>
#include <wx/file.h>

#include "ocpn_types.h"
#include "glTextureDescriptor.h"

#define COMPRESSED_CACHE_MAGIC 0xf010  // change this when the format changes

struct CompressedCacheHeader
{
    uint32_t magic;
    uint32_t format;
    uint32_t chartdate;
    uint32_t m_nentries;
    uint32_t catalog_offset;    
};

class CatalogEntry
{
public:
    CatalogEntry();
    ~CatalogEntry();
    CatalogEntry(int level, int x0, int y0, ColorScheme colorscheme);
    int GetSerialSize();
    void Serialize(unsigned char *);
    void DeSerialize(unsigned char *);
    
    int         mip_level;
    int         x;
    int         y;
    ColorScheme tcolorscheme;
    int         texture_offset;
    uint32_t    compressed_size;
};

WX_DECLARE_OBJARRAY(CatalogEntry*, ArrayOfCatalogEntries);


class glTexFactory
{
public:
    glTexFactory(ChartBase *chart, GLuint raster_format);
    ~glTexFactory();

    void PrepareTexture( int base_level, const wxRect &rect, ColorScheme color_scheme );
    unsigned char *GetTextureLevel( const wxRect &rect, int level,  ColorScheme color_scheme );
    
    void DeleteTexture(const wxRect &rect);
    void DeleteAllTextures( void );
    void DeleteAllDescriptors( void );
    
    
private:
    bool LoadCatalog(void);
    bool LoadHeader(void);
    bool WriteCatalogAndHeader();
    
    bool UpdateCache(unsigned char *data, int data_size, glTextureDescriptor *ptd, int level,
                                   ColorScheme color_scheme);
    
    void DeleteSingleTexture( glTextureDescriptor *ptd );
    
    int         n_catalog_entries;
    ArrayOfCatalogEntries       m_catalog;
    ChartBase   *m_pchart;
    GLuint      m_raster_format;
    wxString    m_CompressedCacheFilePath;
    
    int         m_catalog_offset;
    bool        m_hdrOK;
    bool        m_catalogOK;
    wxFile      *m_fs;
    uint32_t    m_chart_date_binary;
    
    int         m_stride;
    int         m_ntex;
    int         m_tex_dim;
    int         m_size_X;
    int         m_size_Y;
    int         m_nx_tex;
    int         m_ny_tex;
    
    glTextureDescriptor  **m_td_array;
    
};

#endif
