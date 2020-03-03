/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenGL text rendering
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2014 Sean D'Epagnier                                    *
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
 **************************************************************************/

#ifndef __TEXFONT_H__
#define __TEXFONT_H__

/* support ascii plus degree symbol for now pack font in a single texture 16x8 */
#define DEGREE_GLYPH 127
#define MIN_GLYPH 32
#define MAX_GLYPH 128

#define NUM_GLYPHS (MAX_GLYPH - MIN_GLYPH)

#define COLS_GLYPHS 16
#define ROWS_GLYPHS ((NUM_GLYPHS / COLS_GLYPHS)+1)


struct TexGlyphInfo {
    int x, y, width, height;
    float advance;
};

class TexFont {
public:
    TexFont();
    ~TexFont();
    
    void Build( wxFont &font, bool blur = false );
    void Delete();

    void GetTextExtent( const wxString &string, int *width, int *height);
    void RenderString( const char *string, int x=0, int y=0 );
    void RenderString( const wxString &string, int x=0, int y=0 );
    bool IsBuilt(){ return m_built; }
    void SetColor(wxColor &color){ m_color = color;}

private:
    void GetTextExtent( const char *string, int *width, int *height);
    void RenderGlyph( int c );

    wxFont m_font;
    bool m_blur;

    TexGlyphInfo tgi[MAX_GLYPH];

    unsigned  int texobj;
    int tex_w, tex_h;
    int m_maxglyphw;
    int m_maxglyphh;
    bool m_built;
    
    float m_dx;
    float m_dy;
    wxColor m_color;
};

TexFont *GetTexFont(wxFont *key);

#endif  //guard