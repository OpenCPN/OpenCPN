/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Main wxWidgets Program
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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
 */
#include "wx/wxprec.h"
#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers
#include "ocpn_types.h"
#include "compass.h"
#include "chcanv.h"
#include "styles.h"


extern ocpnStyle::StyleManager* g_StyleManager;
extern ChartCanvas *cc1;
extern bool bGPSValid;
extern bool g_bSatValid;
extern int g_SatsInView;
extern bool g_bCourseUp;
extern bool g_bskew_comp;
extern MyFrame *gFrame;
extern bool g_bopengl;

ocpnCompass::ocpnCompass()
{
    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
    _img_compass = style->GetIcon( _T("CompassRose") );
    _img_gpsRed = style->GetIcon( _T("gpsRed") );

    m_rose_angle = -999;  // force a refresh when first used

    m_pStatBoxToolStaticBmp = NULL;

    m_rect = wxRect(style->GetCompassXOffset(), style->GetCompassYOffset(),
            _img_compass.GetWidth() + _img_gpsRed.GetWidth() + style->GetCompassLeftMargin() * 2
                    + style->GetToolSeparation(),
                    _img_compass.GetHeight() + style->GetCompassTopMargin() + style->GetCompassBottomMargin() );
}

ocpnCompass::~ocpnCompass()
{
    delete m_pStatBoxToolStaticBmp;
}

void ocpnCompass::Paint( ocpnDC& dc )
{
    if(m_shown && m_StatBmp.IsOk()){
# ifdef ocpnUSE_GLES  // GLES does not do ocpnDC::DrawBitmap(), so use texture
        if(g_bopengl){
            glBindTexture( GL_TEXTURE_2D, texobj );
            glEnable( GL_TEXTURE_2D );
            
            glBegin( GL_QUADS );
            
            glTexCoord2f( 0, 0 );  glVertex2i( m_rect.x, m_rect.y );
            glTexCoord2f( 1, 0 );  glVertex2i( m_rect.x + m_rect.width, m_rect.y );
            glTexCoord2f( 1, 1 );  glVertex2i( m_rect.x + m_rect.width, m_rect.y + m_rect.height );
            glTexCoord2f( 0, 1 );  glVertex2i( m_rect.x, m_rect.y + m_rect.height );
            
            glEnd();
            glDisable( GL_TEXTURE_2D );
            
        }
#else        
        dc.DrawBitmap( m_StatBmp, m_rect.x, m_rect.y, true/*false*/ );
#endif        
    }
}

bool ocpnCompass::MouseEvent( wxMouseEvent& event )
{
    if(!m_shown || !m_rect.Contains(event.GetPosition()) || !event.LeftDown())
        return false;

    gFrame->ToggleCourseUp();
    return true;
}

void ocpnCompass::SetColorScheme( ColorScheme cs )
{
    UpdateStatus( true );
}

void ocpnCompass::UpdateStatus( bool bnew )
{
    if( bnew ) m_lastgpsIconName.Clear();        // force an update to occur

    CreateBmp( bnew );
    if(bnew)
        m_StatBmp.SetMask(new wxMask(m_MaskBmp, *wxWHITE));
}

void ocpnCompass::SetScaleFactor( float factor)
{
    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
    
    if(factor > 0.1)
        m_scale = factor;
    else
        m_scale = 1.0;
    
    m_rect = wxRect(style->GetCompassXOffset(), style->GetCompassYOffset(),
                    m_scale * (_img_compass.GetWidth() + _img_gpsRed.GetWidth()) + style->GetCompassLeftMargin() * 2
                    + style->GetToolSeparation(),
                    m_scale * (_img_compass.GetHeight() + style->GetCompassTopMargin()) + style->GetCompassBottomMargin() );
    
}


void ocpnCompass::CreateBmp( bool newColorScheme )
{
    if(!m_shown)
        return;

    wxString gpsIconName;
    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

    // In order to draw a horizontal compass window when the toolbar is vertical, we
    // need to save away the sizes and backgrounds for the two icons.

    static wxBitmap compassBg, gpsBg;
    static wxSize toolsize;
    static int topmargin, leftmargin, radius;

    if( ! compassBg.IsOk() || newColorScheme ) {
        int orient = style->GetOrientation();
        style->SetOrientation( wxTB_HORIZONTAL );
        if( style->HasBackground() ) {
            compassBg = style->GetNormalBG();
            style->DrawToolbarLineStart( compassBg );
            compassBg = style->SetBitmapBrightness( compassBg );
            gpsBg = style->GetNormalBG();
            style->DrawToolbarLineEnd( gpsBg );
            gpsBg = style->SetBitmapBrightness( gpsBg );
        }

        if(fabs(m_scale-1.0) > 0.1){
            wxImage bg_img = compassBg.ConvertToImage();
            bg_img.Rescale(compassBg.GetWidth() * m_scale, compassBg.GetHeight() *m_scale, wxIMAGE_QUALITY_NORMAL);
            compassBg = wxBitmap( bg_img );
            
            bg_img = gpsBg.ConvertToImage();
            bg_img.Rescale(gpsBg.GetWidth() * m_scale, gpsBg.GetHeight() *m_scale, wxIMAGE_QUALITY_NORMAL);
            gpsBg = wxBitmap( bg_img );
        }
    
        leftmargin = style->GetCompassLeftMargin();
        topmargin = style->GetCompassTopMargin();
        toolsize = style->GetToolSize();
        toolsize.x *= 2;
        radius = style->GetCompassCornerRadius();

        if( orient ) style->SetOrientation( wxTB_VERTICAL );
    }

    bool b_need_refresh = false;

    if( bGPSValid ) {
        if( g_bSatValid ) {
            gpsIconName = _T("gps3Bar");
            if( g_SatsInView <= 8 ) gpsIconName = _T("gps2Bar");
            if( g_SatsInView <= 4 ) gpsIconName = _T("gps1Bar");
            if( g_SatsInView < 0 ) gpsIconName = _T("gpsGry");

        } else
            gpsIconName = _T("gpsGrn");
    } else
        gpsIconName = _T("gpsRed");

    if( m_lastgpsIconName != gpsIconName ) b_need_refresh = true;

    double rose_angle = -999.;

    if( ( fabs( cc1->GetVPRotation() ) > .01 ) || ( fabs( cc1->GetVPSkew() ) > .01 ) ) {
        rose_angle = -cc1->GetVPRotation();

        if( !g_bCourseUp && !g_bskew_comp )
            rose_angle -= cc1->GetVPSkew();

    } else
        rose_angle = 0.;

    if( fabs( m_rose_angle - rose_angle ) > .1 ) b_need_refresh = true;

    if( !b_need_refresh )
        return;

     m_StatBmp.Create(
        m_scale * ( ( _img_compass.GetWidth() + _img_gpsRed.GetWidth() ) + style->GetCompassLeftMargin() * 2
        + style->GetToolSeparation()),
                   m_scale * (_img_compass.GetHeight() + style->GetCompassTopMargin() + style->GetCompassBottomMargin()) );
    
    if( !m_StatBmp.IsOk() )
        return;

    wxMemoryDC mdc;
    mdc.SelectObject( m_StatBmp );
    mdc.SetBackground( wxBrush( GetGlobalColor( _T("GREY2") ), wxSOLID ) );
    mdc.Clear();

    mdc.SetPen( wxPen( GetGlobalColor( _T("UITX1") ), 1 ) );
    mdc.SetBrush( wxBrush( GetGlobalColor( _T("UITX1") ), wxTRANSPARENT ) );

    mdc.DrawRoundedRectangle( 0, 0, m_StatBmp.GetWidth(), m_StatBmp.GetHeight(),
                              style->GetCompassCornerRadius() );

    wxPoint offset( style->GetCompassLeftMargin(), style->GetCompassTopMargin() );

    //    Build Compass Rose, rotated...
    wxBitmap BMPRose;
    wxPoint after_rotate;

    if( g_bCourseUp ) BMPRose = style->GetIcon( _T("CompassRose") );
    else
        BMPRose = style->GetIcon( _T("CompassRoseBlue") );
    if( ( fabs( cc1->GetVPRotation() ) > .01 ) || ( fabs( cc1->GetVPSkew() ) > .01 )  || (fabs(m_scale-1.0) > 0.1) ) {
        int width = BMPRose.GetWidth() * m_scale;
        int height = BMPRose.GetHeight() * m_scale;
        
        wxImage rose_img = BMPRose.ConvertToImage();
        
        if(fabs(m_scale-1.0) > 0.1)
            rose_img.Rescale(width, height, wxIMAGE_QUALITY_NORMAL);
        
        wxPoint rot_ctr( width / 2, height / 2  );
 
        wxImage rot_image = rose_img.Rotate( rose_angle, rot_ctr, true, &after_rotate );
        BMPRose = wxBitmap( rot_image ).GetSubBitmap( wxRect( -after_rotate.x, -after_rotate.y, width, height ));
    }

    wxBitmap iconBm;

    if( style->HasBackground() ) {
        iconBm = MergeBitmaps( compassBg, BMPRose, wxSize( 0, 0 ) );
    } else {
        iconBm = BMPRose;
    }

    mdc.DrawBitmap( iconBm, offset );
    offset.x += iconBm.GetWidth();

    m_rose_angle = rose_angle;

    //  GPS Icon
    wxBitmap gicon = style->GetIcon( gpsIconName );
    if(fabs(m_scale-1.0) > 0.1){
        int width = gicon.GetWidth() * m_scale;
        int height = gicon.GetHeight() * m_scale;
        
        wxImage gps_img = gicon.ConvertToImage();
        gps_img.Rescale(width, height, wxIMAGE_QUALITY_NORMAL);
        gicon = wxBitmap( gps_img );
    }
    
    if( style->HasBackground() ) {
        iconBm = MergeBitmaps( gpsBg, gicon, wxSize( 0, 0 ) );
    } else {
        iconBm = gicon;
    }
    
    mdc.DrawBitmap( iconBm, offset );
    mdc.SelectObject( wxNullBitmap );
    m_lastgpsIconName = gpsIconName;

    if( style->marginsInvisible ) {
        m_MaskBmp = wxBitmap( m_StatBmp.GetWidth(), m_StatBmp.GetHeight() );
        wxMemoryDC sdc( m_MaskBmp );
        sdc.SetBackground( *wxWHITE_BRUSH );
        sdc.Clear();
        sdc.SetBrush( *wxBLACK_BRUSH );
        sdc.SetPen( *wxBLACK_PEN );
        sdc.DrawRoundedRectangle( wxPoint( leftmargin, topmargin ), toolsize, radius );
        sdc.SelectObject( wxNullBitmap );
    } else if(radius) {
        m_MaskBmp = wxBitmap( m_StatBmp.GetWidth(), m_StatBmp.GetHeight() );
        wxMemoryDC sdc( m_MaskBmp );
        sdc.SetBackground( *wxWHITE_BRUSH );
        sdc.Clear();
        sdc.SetBrush( *wxBLACK_BRUSH );
        sdc.SetPen( *wxBLACK_PEN );
        sdc.DrawRoundedRectangle( 0, 0, m_MaskBmp.GetWidth(), m_MaskBmp.GetHeight(), radius );
        sdc.SelectObject( wxNullBitmap );
    }
   
# ifdef ocpnUSE_GLES  // GLES does not do ocpnDC::DrawBitmap(), so use texture
   if(g_bopengl){
        wxImage image = m_StatBmp.ConvertToImage(); 
        unsigned char *imgdata = image.GetData();
        int tex_w = image.GetWidth();
        int tex_h = image.GetHeight();
        
        GLuint format = GL_RGBA;
        GLuint internalformat = format;
        int stride = 4;
        
        if(imgdata){
            unsigned char *teximage = (unsigned char *) malloc( stride * tex_w * tex_h );
        
            for( int j = 0; j < tex_w*tex_h; j++ ){
                for( int k = 0; k < 3; k++ )
                    teximage[j * stride + k] = imgdata[3*j + k];
                teximage[j * stride + 3] = (unsigned char)0x255;           // alpha
            }
                
            if(texobj){
                glDeleteTextures(1, &texobj);
                texobj = 0;
            }
                
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
   }
#endif
       
}
