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
#include "compasswin.h"
#include "chcanv.h"
#include "styles.h"
#include "wx28compat.h"

BEGIN_EVENT_TABLE(ocpnFloatingCompassWindow, wxWindow) EVT_PAINT ( ocpnFloatingCompassWindow::OnPaint )
    EVT_LEFT_DOWN ( ocpnFloatingCompassWindow::MouseEvent )
END_EVENT_TABLE()

extern ocpnStyle::StyleManager* g_StyleManager;
extern ChartCanvas *cc1;
extern bool bGPSValid;
extern bool g_bSatValid;
extern int g_SatsInView;
extern bool g_bCourseUp;
extern bool g_bskew_comp;
extern MyFrame *gFrame;

ocpnFloatingCompassWindow::ocpnFloatingCompassWindow( wxWindow *parent )
{
    m_pparent = parent;
    long wstyle = wxNO_BORDER | wxFRAME_NO_TASKBAR;
#ifndef __WXMAC__
    wstyle |= wxFRAME_SHAPED;
#endif    
#ifdef __WXMAC__
    wstyle |= wxSTAY_ON_TOP;
#endif
    wxDialog::Create( parent, -1, _T(""), wxPoint( 0, 0 ), wxSize( -1, -1 ), wstyle );

    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
    _img_compass = style->GetIcon( _T("CompassRose") );
    _img_gpsRed = style->GetIcon( _T("gpsRed") );

    m_rose_angle = -999;  // force a refresh when first used

    m_pStatBoxToolStaticBmp = NULL;

    m_scale = 1.0;
    SetSize(
        m_scale * ( ( _img_compass.GetWidth() + _img_gpsRed.GetWidth() ) + style->GetCompassLeftMargin() * 2
        + style->GetToolSeparation()),
                   m_scale * (_img_compass.GetHeight() + style->GetCompassTopMargin() + style->GetCompassBottomMargin()) );
    
    m_xoffset = style->GetCompassXOffset();
    m_yoffset = style->GetCompassYOffset();
}

ocpnFloatingCompassWindow::~ocpnFloatingCompassWindow()
{
    delete m_pStatBoxToolStaticBmp;
}

void ocpnFloatingCompassWindow::OnPaint( wxPaintEvent& event )
{
    int width, height;
    GetClientSize( &width, &height );
    wxPaintDC dc( this );

    dc.DrawBitmap( m_StatBmp, 0, 0, false );
}

void ocpnFloatingCompassWindow::MouseEvent( wxMouseEvent& event )
{
    gFrame->ToggleCourseUp();
}

void ocpnFloatingCompassWindow::SetColorScheme( ColorScheme cs )
{
    wxColour back_color = GetGlobalColor( _T("GREY2") );

    //  Set background
    SetBackgroundColour( back_color );
    ClearBackground();

    UpdateStatus( true );

}

void ocpnFloatingCompassWindow::UpdateStatus( bool bnew )
{
    if( bnew ) m_lastgpsIconName.Clear();        // force an update to occur

    wxBitmap statbmp = CreateBmp( bnew );
    if( statbmp.IsOk() ) m_StatBmp = statbmp;

    Show();
    Refresh( false );
}

void ocpnFloatingCompassWindow::SetScaleFactor( float factor)
{
//    qDebug() << m_scale << factor;
    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
    
    if(factor > 0.1)
        m_scale = factor;
    else
        m_scale = 1.0;
    
    SetSize(
        m_scale * ( ( _img_compass.GetWidth() + _img_gpsRed.GetWidth() ) + style->GetCompassLeftMargin() * 2
        + style->GetToolSeparation()),
            m_scale * (_img_compass.GetHeight() + style->GetCompassTopMargin() + style->GetCompassBottomMargin()) );
    
}


wxBitmap ocpnFloatingCompassWindow::CreateBmp( bool newColorScheme )
{
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
            
            if(fabs(m_scale-1.0) > 0.1){
                wxImage bg_img = compassBg.ConvertToImage();
                bg_img.Rescale(compassBg.GetWidth() * m_scale, compassBg.GetHeight() *m_scale, wxIMAGE_QUALITY_NORMAL);
                compassBg = wxBitmap( bg_img );
                
                bg_img = gpsBg.ConvertToImage();
                bg_img.Rescale(gpsBg.GetWidth() * m_scale, gpsBg.GetHeight() *m_scale, wxIMAGE_QUALITY_NORMAL);
                gpsBg = wxBitmap( bg_img );
            }
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

    if( fabs( m_rose_angle - rose_angle ) > .1 )
        b_need_refresh = true;

    if( b_need_refresh ) {
        wxBitmap StatBmp;

        StatBmp.Create(
                m_scale * ( ( _img_compass.GetWidth() + _img_gpsRed.GetWidth() ) + style->GetCompassLeftMargin() * 2
                        + style->GetToolSeparation()),
                m_scale * (_img_compass.GetHeight() + style->GetCompassTopMargin() + style->GetCompassBottomMargin()) );

        if( StatBmp.IsOk() ) {

            wxMemoryDC mdc;
            mdc.SelectObject( StatBmp );
            mdc.SetBackground( wxBrush( GetGlobalColor( _T("GREY2") ), wxBRUSHSTYLE_SOLID ) );
            mdc.Clear();

            mdc.SetPen( wxPen( GetGlobalColor( _T("UITX1") ), 1 ) );
            mdc.SetBrush( wxBrush( GetGlobalColor( _T("UITX1") ), wxBRUSHSTYLE_TRANSPARENT ) );

            mdc.DrawRoundedRectangle( 0, 0, StatBmp.GetWidth(), StatBmp.GetHeight(),
                    m_scale * style->GetCompassCornerRadius() );

            wxPoint offset( style->GetCompassLeftMargin(), style->GetCompassTopMargin() );

            wxBitmap iconBm;
            
            
            //    Build Compass Rose, rotated...
            wxBitmap BMPRose;
            wxPoint after_rotate;

            if( g_bCourseUp )
                BMPRose = style->GetIcon( _T("CompassRose") );
            else
                BMPRose = style->GetIcon( _T("CompassRoseBlue") );
            
            if( ( fabs( cc1->GetVPRotation() ) > .01 ) || ( fabs( cc1->GetVPSkew() ) > .01 )  || (fabs(m_scale-1.0) > 0.1) ) {
                int width = BMPRose.GetWidth() * m_scale;
                int height = BMPRose.GetHeight() * m_scale;
                
                wxImage rose_img = BMPRose.ConvertToImage();
                
                if(fabs(m_scale-1.0) > 0.1)
                    rose_img.Rescale(width, height, wxIMAGE_QUALITY_NORMAL);
                

                if(fabs(rose_angle) > 0.01){
                    wxPoint rot_ctr( width / 2, height / 2 );
                    wxImage rot_image = rose_img.Rotate( rose_angle, rot_ctr, true, &after_rotate );
                    BMPRose = wxBitmap( rot_image ).GetSubBitmap( wxRect( -after_rotate.x, -after_rotate.y, width, height) );
                }
                else
                    BMPRose = wxBitmap( rose_img );
                
            }


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
        }

#if !defined(__WXMAC__) && !defined(__OCPN__ANDROID__)       
        if( style->marginsInvisible ) {
            m_MaskBmp = wxBitmap( StatBmp.GetWidth(), StatBmp.GetHeight() );
            wxMemoryDC sdc( m_MaskBmp );
            sdc.SetBackground( *wxWHITE_BRUSH );
            sdc.Clear();
            sdc.SetBrush( *wxBLACK_BRUSH );
            sdc.SetPen( *wxBLACK_PEN );
            sdc.DrawRoundedRectangle( wxPoint( leftmargin, topmargin ), toolsize, radius );
            sdc.SelectObject( wxNullBitmap );
            SetShape( wxRegion( m_MaskBmp, *wxWHITE, 0 ) );
        }
        else if(radius) {
            m_MaskBmp = wxBitmap( GetSize().x, GetSize().y );
            wxMemoryDC sdc( m_MaskBmp );
            sdc.SetBackground( *wxWHITE_BRUSH );
            sdc.Clear();
            sdc.SetBrush( *wxBLACK_BRUSH );
            sdc.SetPen( *wxBLACK_PEN );
            sdc.DrawRoundedRectangle( 0, 0, m_MaskBmp.GetWidth(), m_MaskBmp.GetHeight(), radius );
            sdc.SelectObject( wxNullBitmap );
            SetShape( wxRegion( m_MaskBmp, *wxWHITE, 0 ) );
        }
#endif        

        return StatBmp;
    }

    else
        return wxNullBitmap;
}

