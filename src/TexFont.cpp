/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  texture OpenGL text rendering built from wxFont
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

#include <wx/wx.h>

#include "TexFont.h"

#ifdef USE_ANDROID_GLES2
#include <gl2.h>
#include "linmath.h"
#include "shaders.h"
#else
 #include <GL/gl.h>
 #include <GL/glu.h>
#endif

#if 0
#define TXF_CACHE 8
static TexFontCache s_txf[TXF_CACHE];

TexFont *GetTexFont(wxFont *pFont)
{
    // rebuild font if needed
    TexFont *f_cache;
    unsigned int i;
    for (i = 0; i < TXF_CACHE && s_txf[i].key != nullptr; i++)
    {
        if (s_txf[i].key == pFont) {
            return &s_txf[i].cache;
        }
    }
    if (i == TXF_CACHE) {
        i = rand() & (TXF_CACHE -1);
    }
    s_txf[i].key = pFont;
    f_cache = &s_txf[i].cache;
    f_cache->Build(*pFont);
    return f_cache;
}
#endif

TexFont::TexFont( )
{
    texobj = 0;
    m_blur = false;
    m_built = false;
    m_color = wxColor(0,0,0);
}

TexFont::~TexFont( )
{
    Delete( );
}


void TexFont::Build( wxFont &font, bool blur )
{
    /* avoid rebuilding if the parameters are the same */
    if(m_built && (font == m_font) && (blur == m_blur))
        return;
    
    m_font = font;
    m_blur = blur;

    m_maxglyphw = 0;
    m_maxglyphh = 0;
    
    wxScreenDC sdc;

    sdc.SetFont( font );

    for( int i = MIN_GLYPH; i < MAX_GLYPH; i++ ) {
        wxCoord gw, gh;
        wxString text;
        if(i == DEGREE_GLYPH)
            text = wxString::Format(_T("%c"), 0x00B0); //_T("°");
        else
            text = wxString::Format(_T("%c"), i);
        wxCoord descent, exlead;
        sdc.GetTextExtent( text, &gw, &gh, &descent, &exlead, &font ); // measure the text

        tgi[i].width = gw;
        tgi[i].height = gh;

        tgi[i].advance = gw;
        
        
        m_maxglyphw = wxMax(tgi[i].width,  m_maxglyphw);
        m_maxglyphh = wxMax(tgi[i].height, m_maxglyphh);
    }

    /* add extra pixel to give a border between rows of characters
       without this, in some cases a faint line can be see on the edge
       from the character above */
    m_maxglyphh++;

    int w = COLS_GLYPHS * m_maxglyphw;
    int h = ROWS_GLYPHS * m_maxglyphh;

    wxASSERT(w < 2048 && h < 2048);

    /* make power of 2 */
    for(tex_w = 1; tex_w < w; tex_w *= 2);
    for(tex_h = 1; tex_h < h; tex_h *= 2);

    wxBitmap tbmp(tex_w, tex_h);
    wxMemoryDC dc;
    dc.SelectObject(tbmp);
    dc.SetFont( font );
    
    /* fill bitmap with black */
    dc.SetBackground( wxBrush( wxColour( 0, 0, 0 ) ) );
    dc.Clear();
        
    /* draw the text white */
    dc.SetTextForeground( wxColour( 255, 255, 255 ) );

 /*    wxPen pen(wxColour( 255, 255, 255 ));
     wxBrush brush(wxColour( 255, 255, 255 ), wxTRANSPARENT);
     dc.SetPen(pen);
     dc.SetBrush(brush);
  */  
    int row = 0, col = 0;
    for( int i = MIN_GLYPH; i < MAX_GLYPH; i++ ) {
        if(col == COLS_GLYPHS) {
            col = 0;
            row++;
        }

        tgi[i].x = col * m_maxglyphw;
        tgi[i].y = row * m_maxglyphh;

        wxString text;
        if(i == DEGREE_GLYPH)
            text = wxString::Format(_T("%c"), 0x00B0); //_T("°");
        else
            text = wxString::Format(_T("%c"), i);

        dc.DrawText(text, tgi[i].x, tgi[i].y );
        
//        dc.DrawRectangle(tgi[i].x, tgi[i].y, tgi[i].advance, tgi[i].height);
        col++;
    }

    dc.SelectObject(wxNullBitmap);
    
    wxImage image = tbmp.ConvertToImage();

    GLuint format, internalformat;
    int stride;

    format = GL_ALPHA;
    internalformat = format;
    stride = 1;

    if( m_blur )
        image = image.Blur(1);

    unsigned char *imgdata = image.GetData();
    
    if(imgdata){
        unsigned char *teximage = (unsigned char *) malloc( stride * tex_w * tex_h );

        for( int j = 0; j < tex_w*tex_h; j++ )
            for( int k = 0; k < stride; k++ )
                teximage[j * stride + k] = imgdata[3*j];

        Delete();

        glGenTextures( 1, &texobj );
        glBindTexture( GL_TEXTURE_2D, texobj );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST/*GL_LINEAR*/ );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

        glTexImage2D( GL_TEXTURE_2D, 0, internalformat, tex_w, tex_h, 0,
                    format, GL_UNSIGNED_BYTE, teximage );

        free(teximage);
    }
    
    m_built = true;
}

void TexFont::Delete( )
{
    if (texobj) {
        glDeleteTextures(1, &texobj);
        texobj = 0;
    }
    m_built = false;
}

void TexFont::GetTextExtent(const char *string, int *width, int *height)
{
    int w=0, h=0;

    for(int i = 0; string[i]; i++ ) {
        unsigned char c = string[i];
        if(c == '\n') {
            h += tgi[(int)'A'].height;
            continue;
        }
        if(c == 0xc2 && (unsigned char)string[i+1] == 0xb0) {
            c = DEGREE_GLYPH;
            i++;
        }
        if( c < MIN_GLYPH || c >= MAX_GLYPH)
            continue;

        TexGlyphInfo &tgisi = tgi[c];
        w += tgisi.advance;
        if(tgisi.height > h)
            h = tgisi.height;
    }
    if(width) *width = w;
    if(height) *height = h;
}

void TexFont::GetTextExtent(const wxString &string, int *width, int *height)
{
    GetTextExtent((const char*)string.ToUTF8(), width, height);
}

void TexFont::RenderGlyph( int c )
{
    if( c < MIN_GLYPH || c >= MAX_GLYPH)
        return;

    TexGlyphInfo &tgic = tgi[c];

    int x = tgic.x, y = tgic.y;
    float w = m_maxglyphw, h = m_maxglyphh;
    float tx1 = (float)x / (float)tex_w;
    float tx2 = (float)(x + w) / (float)tex_w;
    float ty1 = (float)y / (float)tex_h;
    float ty2 = (float)(y + h) / (float)tex_h;

#ifndef USE_ANDROID_GLES2    
    
    glBegin( GL_QUADS );

    glTexCoord2f( tx1, ty1 );  glVertex2i( 0, 0 );
    glTexCoord2f( tx2, ty1 );  glVertex2i( w, 0 );
    glTexCoord2f( tx2, ty2 );  glVertex2i( w, h );
    glTexCoord2f( tx1, ty2 );  glVertex2i( 0, h );

    glEnd();
    glTranslatef( tgic.advance, 0.0, 0.0 );
#else

    float uv[8];
    float coords[8];
    
    //normal uv
    uv[0] = tx1; uv[1] = ty1; uv[2] = tx2; uv[3] = ty1;
    uv[4] = tx2; uv[5] = ty2; uv[6] = tx1; uv[7] = ty2;
    
    // pixels
    coords[0] = 0; coords[1] = 0; coords[2] = w; coords[3] = 0;
    coords[4] = w; coords[5] = h; coords[6] = 0; coords[7] = h;
    
    glUseProgram( texture_2DA_shader_program );
    
    // Get pointers to the attributes in the program.
    GLint mPosAttrib = glGetAttribLocation( texture_2DA_shader_program, "aPos" );
    GLint mUvAttrib  = glGetAttribLocation( texture_2DA_shader_program, "aUV" );
    
    // Set up the texture sampler to texture unit 0
    GLint texUni = glGetUniformLocation( texture_2DA_shader_program, "uTex" );
    glUniform1i( texUni, 0 );
    
    // Disable VBO's (vertex buffer objects) for attributes.
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    
    // Set the attribute mPosAttrib with the vertices in the screen coordinates...
    glVertexAttribPointer( mPosAttrib, 2, GL_FLOAT, GL_FALSE, 0, coords );
    // ... and enable it.
    glEnableVertexAttribArray( mPosAttrib );
    
    // Set the attribute mUvAttrib with the vertices in the GL coordinates...
    glVertexAttribPointer( mUvAttrib, 2, GL_FLOAT, GL_FALSE, 0, uv );
    // ... and enable it.
    glEnableVertexAttribArray( mUvAttrib );
 
    float colorv[4];
    colorv[0] = m_color.Red() / float(256);
    colorv[1] = m_color.Green() / float(256);
    colorv[2] = m_color.Blue() / float(256);
    colorv[3] = 0;
    
    GLint colloc = glGetUniformLocation(texture_2DA_shader_program,"color");
    glUniform4fv(colloc, 1, colorv);
    
    // Rotate 
    float angle = 0;
    mat4x4 I, Q;
    mat4x4_identity(I);
    mat4x4_rotate_Z(Q, I, angle);
    
    // Translate
    Q[3][0] = m_dx;
    Q[3][1] = m_dy;
    
    
    //mat4x4 X;
    //mat4x4_mul(X, (float (*)[4])vp->vp_transform, Q);
    
    GLint matloc = glGetUniformLocation(texture_2DA_shader_program,"TransformMatrix");
    glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)Q); 
    
    // Select the active texture unit.
    glActiveTexture( GL_TEXTURE0 );
    
    // For some reason, glDrawElements is busted on Android
    // So we do this a hard ugly way, drawing two triangles...
    #if 0
    GLushort indices1[] = {0,1,3,2}; 
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, indices1);
    #else
    
    float co1[8];
    co1[0] = coords[0];
    co1[1] = coords[1];
    co1[2] = coords[2];
    co1[3] = coords[3];
    co1[4] = coords[6];
    co1[5] = coords[7];
    co1[6] = coords[4];
    co1[7] = coords[5];
    
    float tco1[8];
    tco1[0] = uv[0];
    tco1[1] = uv[1];
    tco1[2] = uv[2];
    tco1[3] = uv[3];
    tco1[4] = uv[6];
    tco1[5] = uv[7];
    tco1[6] = uv[4];
    tco1[7] = uv[5];
    
    glVertexAttribPointer( mPosAttrib, 2, GL_FLOAT, GL_FALSE, 0, co1 );
    glVertexAttribPointer( mUvAttrib, 2, GL_FLOAT, GL_FALSE, 0, tco1 );
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    #endif
    
    
    
    
    m_dx += tgic.advance;
    
#endif    
}

void TexFont::RenderString( const char *string, int x, int y )
{

#ifndef USE_ANDROID_GLES2
    
    glPushMatrix();
    glTranslatef(x, y, 0);

    glPushMatrix();
    glBindTexture( GL_TEXTURE_2D, texobj);

    for( int i = 0; string[i]; i++ ) {
        if(string[i] == '\n') {
            glPopMatrix();
            glTranslatef(0, tgi[(int)'A'].height, 0);
            glPushMatrix();
            continue;
        }
        /* degree symbol */
        if((unsigned char)string[i] == 0xc2 &&
           (unsigned char)string[i+1] == 0xb0) {
            RenderGlyph( DEGREE_GLYPH );
            i++;
            continue;
        }
        RenderGlyph( string[i] );
    }

    glPopMatrix();
    glPopMatrix();
#else
    m_dx = x;
    m_dy = y;
     
    glBindTexture( GL_TEXTURE_2D, texobj);
    
    for( int i = 0; string[i]; i++ ) {
        if(string[i] == '\n') {
            m_dy += tgi[(int)'A'].height;
            continue;
        }
        /* degree symbol */
        if((unsigned char)string[i] == 0xc2 &&
            (unsigned char)string[i+1] == 0xb0) {
            RenderGlyph( DEGREE_GLYPH );
            i++;
            continue;
        }
        RenderGlyph( string[i] );
    }
            
#endif    
}

void TexFont::RenderString( const wxString &string, int x, int y )
{
    RenderString((const char*)string.ToUTF8(), x, y);
}

//#endif     //#ifdef ocpnUSE_GL
