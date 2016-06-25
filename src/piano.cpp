/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Chart Bar Window
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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
 *
 *
 */

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers
#include "dychart.h"

#include "chcanv.h"
#include "piano.h"
#include "chartdb.h"
#include "chart1.h"
#include "chartbase.h"
#include "styles.h"
#include "ocpndc.h"
#include "cutil.h"
#include "wx28compat.h"

#ifdef __OCPN__ANDROID__
#include "androidUTIL.h"
#endif

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(RectArray);

//------------------------------------------------------------------------------
//    External Static Storage
//------------------------------------------------------------------------------
extern ChartDB *ChartData;
extern ocpnStyle::StyleManager* g_StyleManager;
extern MyFrame *gFrame;
extern bool g_btouch;
extern int  g_GUIScaleFactor;

extern ChartCanvas               *cc1;
extern Piano                     *g_Piano;

//------------------------------------------------------------------------------
//          Piano Window Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(Piano, wxEvtHandler)
    EVT_TIMER ( PIANO_EVENT_TIMER, Piano::onTimerEvent )
END_EVENT_TABLE()

// Define a constructor
Piano::Piano()
{
    m_index_last = -1;
    m_iactive = -1;

    m_hover_icon_last = -1;
    m_hover_last = -1;
    m_brounded = false;
    m_bBusy = false;
    
    m_nRegions = 0;

//>    SetBackgroundStyle( wxBG_STYLE_CUSTOM ); // on WXMSW, this prevents flashing on color scheme change

    m_pVizIconBmp = NULL;
    m_pInVizIconBmp = NULL;
    m_pPolyIconBmp = NULL;
    m_pSkewIconBmp = NULL;
    m_pTmercIconBmp = NULL;
    
    m_eventTimer.SetOwner( this, PIANO_EVENT_TIMER );
    
    m_tex = m_tex_piano_height = 0;
}

Piano::~Piano()
{
    if( m_pInVizIconBmp ) delete m_pInVizIconBmp;
    if( m_pPolyIconBmp ) delete m_pPolyIconBmp;
    if( m_pSkewIconBmp ) delete m_pSkewIconBmp;
    if( m_pTmercIconBmp ) delete m_pTmercIconBmp;
    if( m_pVizIconBmp ) delete m_pVizIconBmp;
}

void Piano::Paint( int y, wxDC& dc, wxDC *shapeDC )
{
    ocpnDC odc(dc);
    Paint(y, odc, shapeDC);
}

void Piano::Paint( int y, ocpnDC& dc, wxDC *shapeDC )
{
    if(shapeDC) {
        shapeDC->SetBackground( *wxBLACK_BRUSH);
        shapeDC->SetBrush( *wxWHITE_BRUSH);
        shapeDC->SetPen( *wxWHITE_PEN);
        shapeDC->Clear();
    }

    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
    if(!style->chartStatusWindowTransparent) {
        dc.SetPen( *wxTRANSPARENT_PEN );
        dc.SetBrush( m_backBrush );
        dc.DrawRectangle( 0, y, cc1->GetClientSize().x, GetHeight() );
    }

//    Create the Piano Keys

    int nKeys = m_key_array.GetCount();

    wxPen ppPen( GetGlobalColor( _T("CHBLK") ), 1, wxPENSTYLE_SOLID );
    dc.SetPen( ppPen );

    for( int i = 0; i < nKeys; i++ ) {
        int key_db_index = m_key_array.Item( i );

        if( -1 == key_db_index ) continue;

        bool selected = InArray(m_active_index_array, key_db_index);

        if( ChartData->GetDBChartType( key_db_index ) == CHART_TYPE_CM93 ||
            ChartData->GetDBChartType( key_db_index ) == CHART_TYPE_CM93COMP ) {
            if(selected)
                dc.SetBrush( m_scBrush );
            else
                dc.SetBrush( m_cBrush );
        } else if( ChartData->GetDBChartFamily( key_db_index ) == CHART_FAMILY_VECTOR ) {
            if(selected)
                dc.SetBrush( m_svBrush );
            else
                dc.SetBrush( m_vBrush );
        } else { // Raster Chart
            if(selected)
                dc.SetBrush( m_slBrush );
            else
                dc.SetBrush( m_tBrush );
        }

#if 0
        // Check to see if this box appears in the sub_light array
        // If so, add a crosshatch pattern to the brush
        if(InArray(m_eclipsed_index_array, key_db_index)) {
            wxBrush ebrush( dc.GetBrush().GetColour(), wxCROSSDIAG_HATCH );
            dc.SetBrush(ebrush);
        }
#endif

        if(m_bBusy)
            dc.SetBrush( m_uvBrush );
            
        wxRect box = KeyRect.Item( i );
        box.y += y;

        if( m_brounded ) {
            dc.DrawRoundedRectangle( box.x, box.y, box.width, box.height, 4 );
            if(shapeDC)
                shapeDC->DrawRoundedRectangle( box.x, box.y, box.width, box.height, 4 );
        } else {
            dc.DrawRectangle( box.x, box.y, box.width, box.height );
            if(shapeDC)
                shapeDC->DrawRectangle( box );
        }

        if(InArray(m_eclipsed_index_array, key_db_index)) {
            dc.SetBrush( m_backBrush );
            int w = 3;
            dc.DrawRoundedRectangle( box.x + w, box.y + w, box.width - ( 2 * w ),
                                     box.height - ( 2 * w ), 3 );
        }

        //    Look in the current noshow array for this index
        if(InArray(m_noshow_index_array, key_db_index) &&
           m_pInVizIconBmp && m_pInVizIconBmp->IsOk() )
            dc.DrawBitmap(ConvertTo24Bit( dc.GetBrush().GetColour(), *m_pInVizIconBmp ),
                          box.x + 4, box.y + 3, false );

        //    Look in the current skew array for this index
        if(InArray(m_skew_index_array, key_db_index) &&
           m_pSkewIconBmp && m_pSkewIconBmp->IsOk())
            dc.DrawBitmap(ConvertTo24Bit( dc.GetBrush().GetColour(), *m_pSkewIconBmp ),
                          box.x + box.width - m_pSkewIconBmp->GetWidth() - 4, box.y + 2, false );

        //    Look in the current tmerc array for this index
        if(InArray(m_tmerc_index_array, key_db_index) &&
           m_pTmercIconBmp && m_pTmercIconBmp->IsOk() )
            dc.DrawBitmap(ConvertTo24Bit( dc.GetBrush().GetColour(), *m_pTmercIconBmp ),
                          box.x + box.width - m_pTmercIconBmp->GetWidth() - 4, box.y + 2, false );

        //    Look in the current poly array for this index
        if(InArray(m_poly_index_array, key_db_index) &&
           m_pPolyIconBmp && m_pPolyIconBmp->IsOk() )
            dc.DrawBitmap(ConvertTo24Bit( dc.GetBrush().GetColour(), *m_pPolyIconBmp ),
                          box.x + box.width - m_pPolyIconBmp->GetWidth() - 4, box.y + 2, false );
    }
}

static void SetColor(unsigned char color[4], const wxBrush &brush)
{
    const wxColour &c = brush.GetColour();
    color[0] = c.Red(), color[1] = c.Green(), color[2] = c.Blue(), color[3] = 255;
}

#if 0 // alternate implementation, it is much faster most frames (only a single quad is rendered),
// but requires updating the texture if the piano changes which is slower (although still quite fast)
// the other problem is the code is difficult to follow and maintain
// can remove this when we know we aren't going to ever be using it
void Piano::UpdateGLTexture()
{
    extern GLenum       g_texture_rectangle_format;
    int w = cc1->GetClientSize().x, h = GetHeight(), tex_w, tex_h;
    if(g_texture_rectangle_format == GL_TEXTURE_2D)
        tex_w = w, tex_h = h;
    else
        tex_w = NextPow2(w), tex_h = NextPow2(h);
    m_texcoords[0] = (float)w / tex_w;
    m_texcoords[1] = (float)h / tex_h;

    // this isn't very pretty but should at least be fast enough
    unsigned char bgcolor[4], tbgcolor[4];
    SetColor(bgcolor, m_backBrush);
    SetColor(tbgcolor, m_backBrush);

    wxColour b = GetGlobalColor( _T("CHBLK") );
    unsigned char bcolor[4] = {b.Red(), b.Green(), b.Blue(), 255};

    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
    if(style->chartStatusWindowTransparent)
        tbgcolor[3] = 0;

    unsigned char *data = new unsigned char[4*w*h];

    // fill to background color
    for(int x = 0; x < w; x++) {
        unsigned char *pos = data + 4*x;
        memcpy(pos, tbgcolor, 4);
    }

    for(int y = 1; y < 8; y++) {
        unsigned char *pos = data + 4*(y*w);
        memcpy(pos, data, 4*w);
    }

    int nKeys = m_key_array.GetCount();
    // draw the keys
    for( int i = 0; i < nKeys; i++ ) {
        int key_db_index = m_key_array.Item( i );

        if( -1 == key_db_index ) continue;

        bool selected = InArray(m_active_index_array, key_db_index);
        unsigned char color[4];
        if( ChartData->GetDBChartType( key_db_index ) == CHART_TYPE_CM93 ||
            ChartData->GetDBChartType( key_db_index ) == CHART_TYPE_CM93COMP ) {
            if(selected)
                SetColor(color, m_scBrush );
            else
                SetColor(color, m_cBrush );
        } else if( ChartData->GetDBChartFamily( key_db_index ) == CHART_FAMILY_VECTOR ) {
            if(selected)
                SetColor(color, m_svBrush );
            else
                SetColor(color, m_vBrush );
        } else { // Raster Chart
            if(selected)
                SetColor(color, m_slBrush );
            else
                SetColor(color, m_tBrush );
        }

#if 0
        // Check to see if this box appears in the sub_light array
        // If so, add a crosshatch pattern to the brush
        if(InArray(m_eclipsed_index_array, key_db_index)) {
            wxBrush ebrush( dc.GetBrush().GetColour(), wxCROSSDIAG_HATCH );
            dc.SetBrush(ebrush);
        }
#endif
        if(m_bBusy)
            SetColor(color, m_uvBrush );
            
        wxRect box = KeyRect.Item( i );

        bool eclipsed = InArray(m_eclipsed_index_array, key_db_index);

        int sympoint =  m_brounded ? (eclipsed ? 5 : 3) : 1;
        for(int y = 0; y < 6; y++) {
            unsigned char *pos = data + 4*((box.y+y)*w + box.x);
            if(y > sympoint) {
                int line;
                if(y < box.height - sympoint)
                    line = sympoint;
                else
                    line = box.height-y-1; // shouldn't hit
                memcpy(pos, data + 4*((box.y+line)*w + box.x), 4*box.width);
                continue;
            }
            
            for(int x = 0; x < box.width; x++, pos+=4) {
                if(y == 0) {
                    if( m_brounded && (x <= 2 || x >= box.width - 3) )
                        memcpy(pos, tbgcolor, 4);
                    else
                        memcpy(pos, bcolor, 4);
                } else if(m_brounded) {
                    if(y == 1) {
                        if(x == 0 || x == box.width - 1)
                            memcpy(pos, tbgcolor, 4);
                        else if(x == 1 || x == 2 || x == box.width - 3 || x == box.width - 2)
                            memcpy(pos, bcolor, 4);
                        else
                            memcpy(pos, color, 4);
                    } else if(y == 2) {
                        if(x == 0 || x == box.width - 1)
                            memcpy(pos, tbgcolor, 4);
                        else if(x == 1 || x == box.width - 2)
                            memcpy(pos, bcolor, 4);
                        else
                            memcpy(pos, color, 4);
                    } else if(eclipsed) {
                        if(x == 0 || x == box.width - 1)
                            memcpy(pos, bcolor, 4);
                        else {
                            if(y == 3) {
                                if(x <= 4 || x >= box.width - 5)
                                    memcpy(pos, color, 4);
                                else
                                    memcpy(pos, bcolor, 4);
                            } else if(y == 4) {
                                if(x <= 3 || x >= box.width - 4)
                                    memcpy(pos, color, 4);
                                else if(x == 4 || x == box.width - 5)
                                    memcpy(pos, bcolor, 4);
                                else
                                    memcpy(pos, bgcolor, 4);
                            } else {
                                if(x <= 2 || x >= box.width - 3)
                                    memcpy(pos, color, 4);
                                else if(x == 3 || x == box.width - 4)
                                    memcpy(pos, bcolor, 4);
                                else
                                    memcpy(pos, bgcolor, 4);
                            }
                        }
                    } else
                        goto def;
                } else {
                def:
                    if(x == 0 || x == box.width - 1)
                        memcpy(pos, bcolor, 4);
                    else
                        memcpy(pos, color, 4);
                }
            }
        }
    }

    // quickly fill the rest via symmetry
    for(int y = 8; y < h; y++) {
        int line;
        if(y < h - 7)
            line = 7;
        else
            line = h-y-1;
        memcpy(data + 4*(y*w), data + 4*(line*w), 4*w);
    }

    if(!m_tex)
        glGenTextures( 1, &m_tex );

    glBindTexture(GL_TEXTURE_2D, m_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 

    if(g_texture_rectangle_format == GL_TEXTURE_2D)
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
    else {
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, tex_w, tex_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data );
    }

    delete [] data;
}

void Piano::DrawGL(int off)
{
    OCPNStopWatch sw;
    int w = cc1->GetClientSize().x, h = GetHeight();

    printf("t0: %f\n", sw.Time());

    glBindTexture(GL_TEXTURE_2D, m_tex);
    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

    if(style->chartStatusWindowTransparent) {
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
        glColor4ub(255, 255, 255, 200); // perhaps we could allow the style to set this
        glEnable(GL_BLEND);
    } else
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);

    for(int i=0; i<30; i++) {
        glTexCoord2f(0,                           0), glVertex2f(0, off);
        glTexCoord2f(m_texcoords[0],              0), glVertex2f(w, off);
        glTexCoord2f(m_texcoords[0], m_texcoords[1]), glVertex2f(w, off+h);
        glTexCoord2f(0,              m_texcoords[1]), glVertex2f(0, off+h);
    }
    glEnd();

    printf("t1: %f\n", sw.Time());
    int iw = 9, ih = 13;
    if(!m_icontex) {
        glGenTextures( 1, &m_icontex );
        glBindTexture(GL_TEXTURE_2D, m_icontex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 

        iw = m_pPolyIconBmp->GetWidth();
        ih = m_pPolyIconBmp->GetHeight();

        wxImage im = m_pPolyIconBmp->ConvertToImage();
        unsigned char *data = new unsigned char[4*iw*ih], *d = data, *e = im.GetData(), *a = im.GetAlpha();
        for(int i=0; i<iw*ih; i++) {
            memcpy(d, e, 3), d+=3, e+=3;
            *d = *a, d++, a++;
        }

        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, iw, ih, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
        delete [] data;
    }
    printf("t2: %f\n", sw.Time());

    glBindTexture(GL_TEXTURE_2D, m_icontex);
    glEnable(GL_BLEND);
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

    printf("t3: %f\n", sw.Time());
    glBegin(GL_QUADS);

    int nKeys = m_key_array.GetCount();
    // draw the bitmaps
    for( int i = 0; i < nKeys; i++ ) {
        int key_db_index = m_key_array.Item( i );

        if( -1 == key_db_index ) continue;

        wxRect box = KeyRect.Item( i );

        float u1 = 0, u2 = 1, v = 1;

        int x1, y1;
        if(InArray(m_noshow_index_array, key_db_index))
            x1 = box.x + 4, y1 = box.y + 3;
        else {
            x1 = box.x + box.width - iw - 4, y1 = box.y + 2;
            if(InArray(m_skew_index_array, key_db_index))
                ;
            else if(InArray(m_tmerc_index_array, key_db_index))
                ;
            else if(InArray(m_poly_index_array, key_db_index))
                ;
            else
                continue;
        }

        y1 += off;
        int x2 = x1 + iw, y2 = y1 + ih;

        glTexCoord2f(u1, 0), glVertex2i(x1, y1);
        glTexCoord2f(u2, 0), glVertex2i(x2, y1);
        glTexCoord2f(u2, v), glVertex2i(x2, y2);
        glTexCoord2f(u1, v), glVertex2i(x1, y2);
    }
    glEnd();
    printf("t4: %f\n", sw.Time());
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}
#endif

// build a texture to hold minimum sized rectangles and icons used to render the chart bar
// this texture is only updated if the color scheme or chart bar height change
void Piano::BuildGLTexture()
{
    int h = GetHeight();

    wxBrush tbackBrush; // transparent back brush
    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
    if(style->chartStatusWindowTransparent)
        tbackBrush = wxColour(1,1,1);
    else
        tbackBrush = m_backBrush;

    wxBrush brushes[] = { m_scBrush, m_cBrush, m_svBrush, m_vBrush, m_slBrush, m_tBrush, m_uvBrush };

    m_tex_piano_height = h;
    m_texw = 64;
    m_texh = ((sizeof brushes) / (sizeof *brushes)) * h;
    m_texh += 4*16; // for icons;

    m_texh = NextPow2(m_texh);

    if(!m_tex)
        glGenTextures( 1, &m_tex );

    glBindTexture(GL_TEXTURE_2D, m_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 

    wxBitmap bmp(m_texw, m_texh);
    wxMemoryDC dc(bmp);

    dc.SetPen( *wxTRANSPARENT_PEN );
    dc.SetBrush(tbackBrush);
    dc.DrawRectangle(0, 0, m_texw, m_texh);

    // draw the needed rectangles with minimal width
    wxPen ppPen( GetGlobalColor( _T("CHBLK") ), 1, wxPENSTYLE_SOLID );
    dc.SetPen( ppPen );
    for(unsigned int b = 0; b < (sizeof brushes) / (sizeof *brushes); b++) {
        unsigned int x = 0, y = h * b;

        dc.SetBrush(brushes[b]);

        int u = 3, v = 2;
        dc.DrawRectangle(x+u, y+v, 3, h-2*v);
        x+=3+2*u;
        
        dc.DrawRoundedRectangle(x+u, y+v, 9, h-2*v, 4);
        x+=9+2*u;

        int w = 3;
        dc.DrawRoundedRectangle(x+u, y+v, 12, h-2*v, 4);
        dc.SetBrush( m_backBrush );
        dc.DrawRoundedRectangle(x+u+w, y+v+w, 12-2*w, h-2*v-2*w, 3);
        x+=12+2*u;

        if(x >= m_texw)
            printf("texture too small\n");
    }
    dc.SelectObject( wxNullBitmap );

    wxImage image = bmp.ConvertToImage();

    unsigned char *data = new unsigned char[4*m_texw*m_texh], *d = data, *e = image.GetData(), *a = image.GetAlpha();
    for(unsigned int i=0; i<m_texw*m_texh; i++) {
        if(style->chartStatusWindowTransparent &&
           e[0] == 1 && e[1] == 1 && e[2] == 1)
            d[3] = 0;
        else
            d[3] = 255;

        memcpy(d, e, 3), d+=4, e+=3;
    }

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_texw, m_texh, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
    delete [] data;

    // put the bitmaps in below
    wxBitmap *bitmaps[] = {m_pInVizIconBmp, m_pTmercIconBmp, m_pSkewIconBmp, m_pPolyIconBmp};

    for(unsigned int i = 0; i < (sizeof bitmaps) / (sizeof *bitmaps); i++) {
        int iw = bitmaps[i]->GetWidth(), ih = bitmaps[i]->GetHeight();
        wxASSERT(ih <= 16);

        wxImage im = bitmaps[i]->ConvertToImage();
        unsigned char *data = new unsigned char[4*iw*ih], *d = data, *e = im.GetData(), *a = im.GetAlpha();
        for(int j = 0; j<iw*ih; j++) {
            memcpy(d, e, 3), d+=3, e+=3;
            *d = *a, d++, a++;
        }

        int off = ((sizeof brushes) / (sizeof *brushes))*h + 16*i;
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, off, iw, ih, GL_RGBA, GL_UNSIGNED_BYTE, data );
        delete [] data;
    }
}

void Piano::DrawGL(int off)
{
    unsigned int w = cc1->GetClientSize().x, h = GetHeight(), endx = 0;
 
    if(m_tex_piano_height != h)
        BuildGLTexture();

    int y1 = off, y2 = y1 + h;

    int nKeys = m_key_array.GetCount();

    // we could cache the coordinates and recompute only when the piano hash changes,
    // but the performance is already fast enough at this point
    float *texcoords = new float[(nKeys*3+1)*4*2], *coords = new float[(nKeys*3+1)*4*2];
    
    int tc = 0, vc = 0;

    // draw the keys
    for( int i = 0; i < nKeys; i++ ) {
        int key_db_index = m_key_array.Item( i );

        int b;
        if( ChartData->GetDBChartType( key_db_index ) == CHART_TYPE_CM93 ||
            ChartData->GetDBChartType( key_db_index ) == CHART_TYPE_CM93COMP )
            b = 0;
        else if( ChartData->GetDBChartFamily( key_db_index ) == CHART_FAMILY_VECTOR )
            b = 2;
        else // Raster Chart
            b = 4;

        if(!InArray(m_active_index_array, key_db_index))
            b++;

        wxRect box = KeyRect.Item( i );
        float y = h*b, v1 = (y+.5)/m_texh, v2 = (y+h-.5)/m_texh;
        // u contains the pixel coordinates in the texture for the three possible rectangles
        const float u[3][6] = {{0, 3, 4, 4, 5, 8},
                               {9, 14, 15, 15, 18, 23},
                               {24, 31, 32, 32, 34, 41}};
        int uindex;
        if(m_brounded) {
            if(InArray(m_eclipsed_index_array, key_db_index))
                uindex = 2;
            else
                uindex = 1;
        } else
            uindex = 0;

        // if the chart is too narrow.. we maybe render the "wrong" rectangle because it can be thinner
        int x1 = box.x, x2 = x1+box.width, w = 2*uindex+1;
        while(x1 + w > x2 - w && uindex > 0)
            uindex--, w -= 2;

        // the minimal width rectangles are texture mapped to the
        // width needed by mapping 3 quads: left middle and right
        const int x[6] = {x1 - 3, x1 + w, x2 - w, x2+3};

        for(int i=0; i<3; i++ ) {
            float u1 = (u[uindex][2*i]+.5)/m_texw, u2 = (u[uindex][2*i+1]+.5)/m_texw;
            int x1 = x[i], x2 = x[i+1];
            texcoords[tc++] = u1, texcoords[tc++] = v1, coords[vc++] = x1, coords[vc++] = y1;
            texcoords[tc++] = u2, texcoords[tc++] = v1, coords[vc++] = x2, coords[vc++] = y1;
            texcoords[tc++] = u2, texcoords[tc++] = v2, coords[vc++] = x2, coords[vc++] = y2;
            texcoords[tc++] = u1, texcoords[tc++] = v2, coords[vc++] = x1, coords[vc++] = y2;
        }
        endx = x[3];
    }

    // if not transparent, fill the rest of the chart bar with the background
    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
    if(!style->chartStatusWindowTransparent && endx < w) {
        texcoords[tc++] = 0, texcoords[tc++] = 0, coords[vc++] = endx, coords[vc++] = y1;
        texcoords[tc++] = 0, texcoords[tc++] = 0, coords[vc++] = w,    coords[vc++] = y1;
        texcoords[tc++] = 0, texcoords[tc++] = 0, coords[vc++] = w,    coords[vc++] = y2;
        texcoords[tc++] = 0, texcoords[tc++] = 0, coords[vc++] = endx, coords[vc++] = y2;
    }

    glBindTexture(GL_TEXTURE_2D, m_tex);

    if(style->chartStatusWindowTransparent) {
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
        glColor4ub(255, 255, 255, 200); // perhaps we could allow the style to set this
        glEnable(GL_BLEND);
    } else
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
    glVertexPointer(2, GL_FLOAT, 0, coords);
    glDrawArrays(GL_QUADS, 0, vc/2);

    // draw the bitmaps
    vc = tc = 0;
    for( int i = 0; i < nKeys; i++ ) {
        int key_db_index = m_key_array.Item( i );

        if( -1 == key_db_index ) continue;

        wxRect box = KeyRect.Item( i );

        wxBitmap *bitmaps[] = {m_pInVizIconBmp, m_pTmercIconBmp, m_pSkewIconBmp, m_pPolyIconBmp};
        int index;
        if(InArray(m_noshow_index_array, key_db_index))
            index = 0;
        else {
            if(InArray(m_skew_index_array, key_db_index))
                index = 2;
            else if(InArray(m_tmerc_index_array, key_db_index))
                index = 1;
            else if(InArray(m_poly_index_array, key_db_index))
                index = 3;
            else
                continue;
        }

        int x1, y1, iw = bitmaps[index]->GetWidth(), ih = bitmaps[index]->GetHeight();
        if(InArray(m_noshow_index_array, key_db_index))
            x1 = box.x + 4, y1 = box.y + 3;
        else
            x1 = box.x + box.width - iw - 4, y1 = box.y + 2;

        y1 += off;
        int x2 = x1 + iw, y2 = y1 + ih;

        wxBrush brushes[] = { m_scBrush, m_cBrush, m_svBrush, m_vBrush, m_slBrush, m_tBrush, m_uvBrush };

        float yoff = ((sizeof brushes) / (sizeof *brushes))*h + 16*index;
        float u1 = 0, u2 = (float)iw / m_texw;
        float v1 = yoff / m_texh, v2 = (yoff + ih) / m_texh;

        texcoords[tc++] = u1, texcoords[tc++] = v1, coords[vc++] = x1, coords[vc++] = y1;
        texcoords[tc++] = u2, texcoords[tc++] = v1, coords[vc++] = x2, coords[vc++] = y1;
        texcoords[tc++] = u2, texcoords[tc++] = v2, coords[vc++] = x2, coords[vc++] = y2;
        texcoords[tc++] = u1, texcoords[tc++] = v2, coords[vc++] = x1, coords[vc++] = y2;
    }
    glEnable(GL_BLEND);
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

    glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
    glVertexPointer(2, GL_FLOAT, 0, coords);
    glDrawArrays(GL_QUADS, 0, vc/2);

    glDisable(GL_BLEND);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    delete [] texcoords;
    delete [] coords;

    glDisable(GL_TEXTURE_2D);
}

void Piano::SetColorScheme( ColorScheme cs )
{

    //    Recreate the local brushes

    m_backBrush = wxBrush( GetGlobalColor( _T("UIBDR") ), wxBRUSHSTYLE_SOLID );

    m_tBrush = wxBrush( GetGlobalColor( _T("BLUE2") ), wxBRUSHSTYLE_SOLID );    // Raster Chart unselected
    m_slBrush = wxBrush( GetGlobalColor( _T("BLUE1") ), wxBRUSHSTYLE_SOLID );    // and selected

    m_vBrush = wxBrush( GetGlobalColor( _T("GREEN2") ), wxBRUSHSTYLE_SOLID );    // Vector Chart unselected
    m_svBrush = wxBrush( GetGlobalColor( _T("GREEN1") ), wxBRUSHSTYLE_SOLID );    // and selected

    m_cBrush = wxBrush( GetGlobalColor( _T("YELO2") ), wxBRUSHSTYLE_SOLID );     // CM93 Chart unselected
    m_scBrush = wxBrush( GetGlobalColor( _T("YELO1") ), wxBRUSHSTYLE_SOLID );    // and selected


    m_uvBrush = wxBrush( GetGlobalColor( _T("UINFD") ), wxBRUSHSTYLE_SOLID );    // and unavailable

    m_tex_piano_height = 0; // force texture to update
}

void Piano::ShowBusy( bool busy )
{
    m_bBusy = busy;
//    Refresh( true );
//    Update();
}

void Piano::SetKeyArray( ArrayOfInts array )
{
    m_key_array = array;
    FormatKeys();
}

void Piano::SetNoshowIndexArray( ArrayOfInts array )
{
    m_noshow_index_array = array;
}

void Piano::SetActiveKeyArray( ArrayOfInts array )
{
    m_active_index_array = array;
}

void Piano::SetEclipsedIndexArray( ArrayOfInts array )
{
    m_eclipsed_index_array = array;
}

void Piano::SetSkewIndexArray( ArrayOfInts array )
{
    m_skew_index_array = array;
}

void Piano::SetTmercIndexArray( ArrayOfInts array )
{
    m_tmerc_index_array = array;
}

void Piano::SetPolyIndexArray( ArrayOfInts array )
{
    m_poly_index_array = array;
}

bool Piano::InArray(ArrayOfInts &array, int key)
{
    for( unsigned int ino = 0; ino < array.GetCount(); ino++ )
        if( array.Item( ino ) == key )
            return true;
    return false;
}

wxString Piano::GetStateHash()
{
    wxString hash;

    for(unsigned int i=0 ; i < m_key_array.GetCount() ; i++){
        wxString a;
        a.Printf(_T("%dK"), m_key_array.Item(i));
        hash += a;
    }
    for(unsigned int i=0 ; i < m_noshow_index_array.GetCount() ; i++){
        wxString a;
        a.Printf(_T("%dN"), m_noshow_index_array.Item(i));
        hash += a;
    }
    for(unsigned int i=0 ; i < m_active_index_array.GetCount() ; i++){
        wxString a;
        a.Printf(_T("%dA"), m_active_index_array.Item(i));
        hash += a;
    }
    for(unsigned int i=0 ; i < m_eclipsed_index_array.GetCount() ; i++){
        wxString a;
        a.Printf(_T("%dE"), m_eclipsed_index_array.Item(i));
        hash += a;
    }
    for(unsigned int i=0 ; i < m_skew_index_array.GetCount() ; i++){
        wxString a;
        a.Printf(_T("%dW"), m_skew_index_array.Item(i));
        hash += a;
    }
    for(unsigned int i=0 ; i < m_tmerc_index_array.GetCount() ; i++){
        wxString a;
        a.Printf(_T("%dM"), m_tmerc_index_array.Item(i));
        hash += a;
    }
    for(unsigned int i=0 ; i < m_poly_index_array.GetCount() ; i++){
        wxString a;
        a.Printf(_T("%dP"), m_poly_index_array.Item(i));
        hash += a;
    }

    return hash;
}

wxString &Piano::GenerateAndStoreNewHash()
{
    m_hash = GetStateHash();
    return m_hash;
}

wxString &Piano::GetStoredHash()
{
    return m_hash;
}

void Piano::FormatKeys( void )
{
    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
    int width = cc1->GetClientSize().x, height = GetHeight();
    width *= g_btouch ? 0.98f : 0.6f;

    int nKeys = m_key_array.GetCount();
    if( nKeys ) {
        int kw = style->chartStatusIconWidth;
        if( !kw ) kw = width / nKeys;

//    Build the Key Regions

        KeyRect.Empty();
        KeyRect.Alloc( nKeys );
        for( int i = 0; i < nKeys; i++ ) {
            wxRect r( ( i * kw ) + 3, 2, kw - 6, height - 4 );
            KeyRect.Add( r );
        }
    }
    m_nRegions = nKeys;
}

wxPoint Piano::GetKeyOrigin( int key_index )
{
    if( ( key_index >= 0 ) && ( key_index <= (int) m_key_array.GetCount() - 1 ) ) {
        wxRect box = KeyRect.Item( key_index );
        return wxPoint( box.x, box.y );
    } else
        return wxPoint( -1, -1 );
}

bool Piano::MouseEvent( wxMouseEvent& event )
{

    int x, y;
    event.GetPosition( &x, &y );

    if(event.Leaving() || y < cc1->GetCanvasHeight() - GetHeight()) {
        if(m_bleaving)
            return false;
        m_bleaving = true;
    } else
        m_bleaving = false;

//    Check the regions

    int sel_index = -1;
    int sel_dbindex = -1;

    for( int i = 0; i < m_nRegions; i++ ) {
        if( KeyRect.Item( i ).Contains( x, 6 ) ) {
            sel_index = i;
            sel_dbindex = m_key_array.Item( i );
            break;
        }
    }

    if(g_btouch){
        if( event.LeftDown() ) {
            if( -1 != sel_index ){
                m_action = DEFERRED_KEY_CLICK_DOWN;
                ShowBusy( true );
 #ifdef __OCPN__ANDROID__
                androidShowBusyIcon();
 #endif                
                m_eventTimer.Start(10, wxTIMER_ONE_SHOT);
            }
        } if( event.LeftUp() ) {
            if( -1 != sel_index ){
                m_click_sel_index = sel_index;
                m_click_sel_dbindex = sel_dbindex;
                if(!m_eventTimer.IsRunning()){
                    m_action = DEFERRED_KEY_CLICK_UP;
                    m_eventTimer.Start(10, wxTIMER_ONE_SHOT);
                }
            }
        } else if( event.RightDown() ) {
            if( sel_index != m_hover_last ) {
                gFrame->HandlePianoRollover( sel_index, sel_dbindex );
                m_hover_last = sel_index;
                
//                m_action = INFOWIN_TIMEOUT;
//                m_eventTimer.Start(3000, wxTIMER_ONE_SHOT);
                
            }
        } else if( event.ButtonUp() ) {
            gFrame->HandlePianoRollover( -1, -1 );
            ResetRollover();
        }
    }
    else{
        if( m_bleaving ) {
            gFrame->HandlePianoRollover( -1, -1 );
            ResetRollover();
        } else if( event.LeftDown() ) {
            if( -1 != sel_index ) {
                gFrame->HandlePianoClick( sel_index, sel_dbindex );
                gFrame->Raise();
            } else
                return false;
        } else if( event.RightDown() ) {
            if( -1 != sel_index ) {
                gFrame->HandlePianoRClick( x, y, sel_index, sel_dbindex );
                gFrame->Raise();
            } else
                return false;
        } else if(!event.ButtonUp()){
            if( sel_index != m_hover_last ) {
                gFrame->HandlePianoRollover( sel_index, sel_dbindex );
                m_hover_last = sel_index;
            }
        }
    }

    return true;

    /*
     Todo:
     Could do something like this to better encapsulate the pianowin
     Allows us to get rid of global statics...

     wxCommandEvent ev(MyPianoEvent);    // Private event
     ..set up event to specify action...SelectChart, SetChartThumbnail, etc
     ::PostEvent(pEventReceiver, ev);    // event receiver passed to ctor

     */

}

void Piano::ResetRollover( void )
{
    m_index_last = -1;
    m_hover_icon_last = -1;
    m_hover_last = -1;
}

int Piano::GetHeight()
{
    int height = 22;
    if(g_btouch){
        double size_mult =  exp( g_GUIScaleFactor * 0.0953101798043 ); //ln(1.1)
        height *= size_mult;
        height = wxMin(height, 50);     // absolute boundaries
        height = wxMax(height, 10);
    }
    return height;
}

void Piano::onTimerEvent(wxTimerEvent &event)
{
    switch(m_action){
        case DEFERRED_KEY_CLICK_DOWN:
            break;
        case DEFERRED_KEY_CLICK_UP:
            if(m_hover_last >= 0){              // turn it off, and return
                gFrame->HandlePianoRollover( -1, -1 );
                ResetRollover();
            }
            else{
                gFrame->HandlePianoClick( m_click_sel_index, m_click_sel_dbindex );
//            ShowBusy( false );
            }
            break;
        case INFOWIN_TIMEOUT:
            gFrame->HandlePianoRollover( -1, -1 );
            ResetRollover();
            break;
        default:
            break;
    }
}
