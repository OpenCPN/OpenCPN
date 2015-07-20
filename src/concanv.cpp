/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Console Canvas
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
 *
 */


#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include "dychart.h"

#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "wx/datetime.h"

#include "chart1.h"
#include "concanv.h"
#include "styles.h"
#include "routeman.h"
#include "navutil.h"
#include "FontMgr.h"
#include "wx28compat.h"

extern Routeman         *g_pRouteMan;
extern MyFrame          *gFrame;
extern bool             g_bShowActiveRouteHighway;
extern double           gCog;
extern double           gSog;
extern bool             g_bShowMag;

extern ocpnStyle::StyleManager* g_StyleManager;

enum eMenuItems {
    ID_NAVLEG,
    ID_NAVROUTE,
    ID_NAVHIGHWAY
} menuItems;

//------------------------------------------------------------------------------
//    ConsoleCanvas Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(ConsoleCanvas, wxWindow)
    EVT_PAINT(ConsoleCanvas::OnPaint)
    EVT_SHOW(ConsoleCanvas::OnShow)
    EVT_CONTEXT_MENU(ConsoleCanvas::OnContextMenu)
    EVT_MENU(ID_NAVLEG, ConsoleCanvas::OnContextMenuSelection)
    EVT_MENU(ID_NAVROUTE, ConsoleCanvas::OnContextMenuSelection)
    EVT_MENU(ID_NAVHIGHWAY, ConsoleCanvas::OnContextMenuSelection)
    
END_EVENT_TABLE()

// Define a constructor for my canvas
ConsoleCanvas::ConsoleCanvas( wxWindow *frame )
{
    pbackBrush = NULL;
    m_bNeedClear = false;

long style = wxSIMPLE_BORDER | wxCLIP_CHILDREN;
#ifdef __WXOSX__
    style |= wxSTAY_ON_TOP;
#endif

    wxDialog::Create( frame, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, style );

    m_pParent = frame;

    m_pitemBoxSizerLeg = new wxBoxSizer( wxVERTICAL );

    pThisLegText = new wxStaticText( this, -1, _("This Leg") );
    pThisLegText->Fit();
    m_pitemBoxSizerLeg->Add( pThisLegText, 0, wxALIGN_CENTER_HORIZONTAL, 2 );

    //     pSBoxRgn = new wxRegion(pThisLegBox->GetRect() );

    pThisLegFont = wxTheFontList->FindOrCreateFont( 10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD );

    pThisLegText->SetFont( *pThisLegFont );

    pXTE = new AnnunText( this, -1, _("Console Legend"), _("Console Value") );
    pXTE->SetALabel( _T("XTE") );
    m_pitemBoxSizerLeg->Add( pXTE, 1, wxALIGN_LEFT | wxALL, 2 );

    pBRG = new AnnunText( this, -1, _("Console Legend"), _("Console Value") );
    pBRG->SetALabel( _T("BRG") );
    m_pitemBoxSizerLeg->Add( pBRG, 1, wxALIGN_LEFT | wxALL, 2 );

    pVMG = new AnnunText( this, -1, _("Console Legend"), _("Console Value") );
    pVMG->SetALabel( _T("VMG") );
    m_pitemBoxSizerLeg->Add( pVMG, 1, wxALIGN_LEFT | wxALL, 2 );

    pRNG = new AnnunText( this, -1, _("Console Legend"), _("Console Value") );
    pRNG->SetALabel( _T("RNG") );
    m_pitemBoxSizerLeg->Add( pRNG, 1, wxALIGN_LEFT | wxALL, 2 );

    pTTG = new AnnunText( this, -1, _("Console Legend"), _("Console Value") );
    pTTG->SetALabel( _T("TTG") );
    m_pitemBoxSizerLeg->Add( pTTG, 1, wxALIGN_LEFT | wxALL, 2 );

//    Create CDI Display Window

    pCDI = new CDI( this, -1, wxSIMPLE_BORDER, _T("CDI") );
    m_pitemBoxSizerLeg->AddSpacer( 5 );
    m_pitemBoxSizerLeg->Add( pCDI, 0, wxALL | wxEXPAND, 2 );

    m_bShowRouteTotal = false;

    SetSizer( m_pitemBoxSizerLeg );      // use the sizer for layout
    m_pitemBoxSizerLeg->SetSizeHints( this );
    Layout();
    Fit();

    Hide();
}

ConsoleCanvas::~ConsoleCanvas()
{
    delete pCDI;
}
    
void ConsoleCanvas::SetColorScheme( ColorScheme cs )
{
    pbackBrush = wxTheBrushList->FindOrCreateBrush( GetGlobalColor( _T("DILG1"/*UIBDR*/) ),
            wxBRUSHSTYLE_SOLID );
    SetBackgroundColour( GetGlobalColor( _T("DILG1"/*"UIBDR"*/) ) );

    //  Also apply color scheme to all known children

    pThisLegText->SetBackgroundColour( GetGlobalColor( _T("DILG1"/*"UIBDR"*/) ) );

    pXTE->SetColorScheme( cs );
    pBRG->SetColorScheme( cs );
    pRNG->SetColorScheme( cs );
    pTTG->SetColorScheme( cs );
    pVMG->SetColorScheme( cs );

    pCDI->SetColorScheme( cs );
}

void ConsoleCanvas::OnPaint( wxPaintEvent& event )
{
    wxPaintDC dc( this );

    if( g_pRouteMan->GetpActiveRoute() ) {
        if( m_bNeedClear ) {
            pThisLegText->Refresh();
            m_bNeedClear = false;
        }

        UpdateRouteData();
    }

    if( ! g_bShowActiveRouteHighway ) pCDI->Hide();
}

void ConsoleCanvas::OnShow( wxShowEvent& event )
{
    pCDI->Show( g_bShowActiveRouteHighway );
    m_pitemBoxSizerLeg->SetSizeHints( this );
}

void ConsoleCanvas::LegRoute()
{
    if( m_bShowRouteTotal )
        pThisLegText->SetLabel( _("Route") );
    else
        pThisLegText->SetLabel( _("This Leg") );

    pThisLegText->Refresh( true );
    RefreshConsoleData();
}

void ConsoleCanvas::OnContextMenu( wxContextMenuEvent& event ) {
    wxMenu* contextMenu = new wxMenu();
    wxMenuItem* btnLeg = new wxMenuItem(contextMenu, ID_NAVLEG, _("This Leg"), _T(""), wxITEM_RADIO );
    wxMenuItem* btnRoute = new wxMenuItem(contextMenu, ID_NAVROUTE, _("Full Route"), _T(""), wxITEM_RADIO );
    wxMenuItem* btnHighw = new wxMenuItem(contextMenu, ID_NAVHIGHWAY, _("Show Highway"), _T(""), wxITEM_CHECK );
    contextMenu->Append( btnLeg );
    contextMenu->Append( btnRoute );
    contextMenu->AppendSeparator();
    contextMenu->Append( btnHighw );

    btnLeg->Check( ! m_bShowRouteTotal );
    btnRoute->Check( m_bShowRouteTotal );
    btnHighw->Check( g_bShowActiveRouteHighway );

    PopupMenu( contextMenu );

    delete contextMenu;
}

void ConsoleCanvas::OnContextMenuSelection( wxCommandEvent& event ) {
    switch( event.GetId() ) {
        case ID_NAVLEG: {
            m_bShowRouteTotal = false;
            LegRoute();
            break;
        }
        case ID_NAVROUTE: {
            m_bShowRouteTotal = true;
            LegRoute();
            break;
        }
        case ID_NAVHIGHWAY: {
            g_bShowActiveRouteHighway = !g_bShowActiveRouteHighway;
            if( g_bShowActiveRouteHighway ) {
                pCDI->Show();
            } else {
                pCDI->Hide();
            }
            m_pitemBoxSizerLeg->SetSizeHints( this );
            break;
        }
    }
}

void ConsoleCanvas::UpdateRouteData()
{
    wxString str_buf;

    if( g_pRouteMan->GetpActiveRoute() )
    {
        if( g_pRouteMan->m_bDataValid )
        {
            // Range to the next waypoint is needed always
            float rng = g_pRouteMan->GetCurrentRngToActivePoint();

            // Brg to the next waypoint
            float dcog = g_pRouteMan->GetCurrentBrgToActivePoint();
            if( dcog >= 359.5 )
                dcog = 0;
            
            wxString cogstr;
            if( g_bShowMag )
                cogstr << wxString::Format( wxString("%6.0f(M)", wxConvUTF8 ), gFrame->GetTrueOrMag( dcog ) );
            else
                cogstr << wxString::Format( wxString("%6.0f", wxConvUTF8 ), gFrame->GetTrueOrMag( dcog ) );
            
            pBRG->SetAValue( cogstr );

            // VMG
            // VMG is always to next waypoint, not to end of route
            // VMG is SOG x cosine (difference between COG and BRG to Waypoint)
            double VMG = 0.;
            if( !wxIsNaN(gCog) && !wxIsNaN(gSog) )
            {
                double BRG;
                BRG = g_pRouteMan->GetCurrentBrgToActivePoint();
                VMG = gSog * cos( ( BRG - gCog ) * PI / 180. ) ;
                str_buf.Printf( _T("%6.2f"), toUsrSpeed( VMG ) );
            }
            else
                str_buf = _T("---");

            pVMG->SetAValue( str_buf );

            if( !m_bShowRouteTotal )
            {
                float nrng = g_pRouteMan->GetCurrentRngToActiveNormalArrival();
                wxString srng;
                double deltarng = fabs( rng - nrng );
                if( ( deltarng > .01 ) && ( ( deltarng / rng ) > .10 ) && ( rng < 10.0 ) ) // show if there is more than 10% difference in ranges, etc...
                {
                    if( nrng < 10.0 )
                        srng.Printf( _T("%5.2f/%5.2f"), toUsrDistance( rng ), toUsrDistance( nrng ) );
                    else
                        srng.Printf( _T("%5.1f/%5.1f"), toUsrDistance( rng ), toUsrDistance( nrng ) );
                }
                else
                {
                    if( rng < 10.0 )
                        srng.Printf( _T("%6.2f"), toUsrDistance( rng ) );
                    else
                        srng.Printf( _T("%6.1f"), toUsrDistance( rng ) );
                }

                //RNG to the next WPT
                pRNG->SetAValue( srng );
                // XTE
                str_buf.Printf( _T("%6.2f"), toUsrDistance( g_pRouteMan->GetCurrentXTEToActivePoint() ) );
                pXTE->SetAValue( str_buf );
                if( g_pRouteMan->GetXTEDir() < 0 )
                    pXTE->SetALabel( wxString( _("XTE         L") ) );
                else
                    pXTE->SetALabel( wxString( _("XTE         R") ) );
                // TTG
                // In all cases, ttg/eta are declared invalid if VMG <= 0.
                // If showing only "this leg", use VMG for calculation of ttg
                wxString ttg_s;
                if( ( VMG > 0. ) && !wxIsNaN(gCog) && !wxIsNaN(gSog) )
                {
                    float ttg_sec = ( rng / VMG ) * 3600.;
                    wxTimeSpan ttg_span( 0, 0, long( ttg_sec ), 0 );
                    ttg_s = ttg_span.Format();
                }
                else
                    ttg_s = _T("---");

                pTTG->SetAValue( ttg_s );
            }
            else
            {
                //    Remainder of route
                float trng = rng;

                Route *prt = g_pRouteMan->GetpActiveRoute();
                wxRoutePointListNode *node = ( prt->pRoutePointList )->GetFirst();
                RoutePoint *prp;

                int n_addflag = 0;
                while( node )
                {
                    prp = node->GetData();
                    if( n_addflag )
                        trng += prp->m_seg_len;

                    if( prp == prt->m_pRouteActivePoint )
                        n_addflag++;

                    node = node->GetNext();
                }

    //                total rng
                wxString strng;
                if( trng < 10.0 )
                    strng.Printf( _T("%6.2f"), toUsrDistance( trng ) );
                else
                    strng.Printf( _T("%6.1f"), toUsrDistance( trng ) );

                pRNG->SetAValue( strng );

                // total TTG
                // If showing total route TTG/ETA, use gSog for calculation

                wxString tttg_s;
                wxTimeSpan tttg_span;
                if( VMG > 0. )
                {
                    float tttg_sec = ( trng / gSog ) * 3600.;
                    tttg_span = wxTimeSpan::Seconds( (long) tttg_sec );
                    tttg_s = tttg_span.Format();
                }
                else
                {
                    tttg_span = wxTimeSpan::Seconds( 0 );
                    tttg_s = _T("---");
                }

                pTTG->SetAValue( tttg_s );

    //                total ETA to be shown on XTE panel
                wxDateTime dtnow, eta;
                dtnow.SetToCurrent();
                eta = dtnow.Add( tttg_span );
                wxString seta;

                if( VMG > 0. )
                    seta = eta.Format( _T("%H:%M") );
                else
                    seta = _T("---");

                pXTE->SetAValue( seta );
                pXTE->SetALabel( wxString( _("ETA          ") ) );
            }

            pRNG->Refresh();
            pBRG->Refresh();
            pVMG->Refresh();
            pTTG->Refresh();
            pXTE->Refresh();
        }
    }
}

void ConsoleCanvas::RefreshConsoleData( void )
{
    UpdateRouteData();

    pRNG->Refresh();
    pBRG->Refresh();
    pVMG->Refresh();
    pTTG->Refresh();
    pXTE->Refresh();
    pCDI->Refresh();
}

void ConsoleCanvas::ShowWithFreshFonts( void )
{
    Hide();
    Move( 0, 0 );

    UpdateFonts();
    gFrame->PositionConsole();
    Show();

}

void ConsoleCanvas::UpdateFonts( void )
{
    pBRG->RefreshFonts();
    pXTE->RefreshFonts();
    pTTG->RefreshFonts();
    pRNG->RefreshFonts();
    pVMG->RefreshFonts();

    m_pitemBoxSizerLeg->SetSizeHints( this );
    Layout();
    Fit();

    Refresh();
}

//------------------------------------------------------------------------------
//    AnnunText Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(AnnunText, wxWindow)
EVT_PAINT(AnnunText::OnPaint)
EVT_MOUSE_EVENTS ( AnnunText::MouseEvent )
END_EVENT_TABLE()

AnnunText::AnnunText( wxWindow *parent, wxWindowID id, const wxString& LegendElement,
        const wxString& ValueElement ) :
        wxWindow( parent, id, wxDefaultPosition, wxDefaultSize, wxNO_BORDER )
{
    m_label = _T("Label");
    m_value = _T("-----");

    m_plabelFont = wxTheFontList->FindOrCreateFont( 14, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, FALSE,
            wxString( _T("Arial Bold") ) );
    m_pvalueFont = wxTheFontList->FindOrCreateFont( 24, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD,
            FALSE, wxString( _T("helvetica") ), wxFONTENCODING_ISO8859_1 );

    m_LegendTextElement = LegendElement;
    m_ValueTextElement = ValueElement;

    RefreshFonts();
}

AnnunText::~AnnunText()
{
}
void AnnunText::MouseEvent( wxMouseEvent& event )
{
#ifdef __OCPN__ANDROID__    
    if( event.RightDown() ) {
        qDebug() << "right down";
        
        wxContextMenuEvent cevt;
        cevt.SetPosition( event.GetPosition());
        
        ConsoleCanvas *ccp = dynamic_cast<ConsoleCanvas*>(GetParent());
        if(ccp)
            ccp->OnContextMenu( cevt );
        
    }
#endif    
}

void AnnunText::CalculateMinSize( void )
{
    //    Calculate the minimum required size of the window based on text size

    int wl = 50;            // reasonable defaults?
    int hl = 20;
    int wv = 50;
    int hv = 20;

    if( m_plabelFont ) GetTextExtent( _T("1234"), &wl, &hl, NULL, NULL, m_plabelFont );

    if( m_pvalueFont ) GetTextExtent( _T("123.456"), &wv, &hv, NULL, NULL, m_pvalueFont );

    wxSize min;
    min.x = wl + wv;
    
    // Space is tight on Android....
#ifdef __OCPN__ANDROID__
    min.x = wv * 1.2; 
#endif    
    
    min.y = (int) ( ( hl + hv ) * 1.2 );

    SetMinSize( min );
}

void AnnunText::SetColorScheme( ColorScheme cs )
{
    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
    m_backBrush = *wxTheBrushList->FindOrCreateBrush( GetGlobalColor( _T("UBLCK") ), wxBRUSHSTYLE_SOLID );

    m_default_text_color = style->consoleFontColor;
    RefreshFonts();
}

void AnnunText::RefreshFonts()
{
    m_plabelFont = FontMgr::Get().GetFont( m_LegendTextElement );
    m_pvalueFont = FontMgr::Get().GetFont( m_ValueTextElement );

    CalculateMinSize();
    
    // Make sure that the background color and the text colors are not too close, for contrast
    if(m_backBrush.IsOk()){
        wxColour back_color = m_backBrush.GetColour();
    
        wxColour legend_color = FontMgr::Get().GetFontColor( _("Console Legend") );
        if( (abs(legend_color.Red() - back_color.Red()) < 5) &&
                (abs(legend_color.Green() - back_color.Blue()) < 5) &&
                (abs(legend_color.Blue() - back_color.Blue()) < 5))
            m_legend_color = m_default_text_color;
            
        wxColour value_color = FontMgr::Get().GetFontColor( _("Console Value") );
        if( (abs(value_color.Red() - back_color.Red()) < 5) &&
            (abs(value_color.Green() - back_color.Blue()) < 5) &&
            (abs(value_color.Blue() - back_color.Blue()) < 5))
            m_val_color = m_default_text_color;
            
    }
    
        

}

void AnnunText::SetLegendElement( const wxString &element )
{
    m_LegendTextElement = element;
}

void AnnunText::SetValueElement( const wxString &element )
{
    m_ValueTextElement = element;
}

void AnnunText::SetALabel( const wxString &l )
{
    m_label = l;
}

void AnnunText::SetAValue( const wxString &v )
{
    m_value = v;
}

void AnnunText::OnPaint( wxPaintEvent& event )
{
    int sx, sy;
    GetClientSize( &sx, &sy );
    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

    //    Do the drawing on an off-screen memory DC, and blit into place
    //    to avoid objectionable flashing
    wxMemoryDC mdc;

    wxBitmap m_bitmap( sx, sy, -1 );
    mdc.SelectObject( m_bitmap );
    mdc.SetBackground( m_backBrush );
    mdc.Clear();

    if( style->consoleTextBackground.IsOk() ) mdc.DrawBitmap( style->consoleTextBackground, 0, 0 );

    mdc.SetTextForeground( m_default_text_color );

    if( m_plabelFont ) {
        mdc.SetFont( *m_plabelFont );
        mdc.SetTextForeground( m_legend_color );
        mdc.DrawText( m_label, 5, 2 );
    }

    if( m_pvalueFont ) {
        mdc.SetFont( *m_pvalueFont );
        mdc.SetTextForeground( m_val_color );

        int w, h;
        mdc.GetTextExtent( m_value, &w, &h );
        int cw, ch;
        mdc.GetSize( &cw, &ch );

        mdc.DrawText( m_value, cw - w - 2, ch - h - 2 );
    }

    wxPaintDC dc( this );
    dc.Blit( 0, 0, sx, sy, &mdc, 0, 0 );
    
}
//------------------------------------------------------------------------------
//    CDI Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(CDI, wxWindow)
EVT_PAINT(CDI::OnPaint)
EVT_MOUSE_EVENTS ( CDI::MouseEvent )
END_EVENT_TABLE()

CDI::CDI( wxWindow *parent, wxWindowID id, long style, const wxString& name ) :
        wxWindow( parent, id, wxDefaultPosition, wxDefaultSize, style, name )

{
    SetMinSize( wxSize( 10, 150 ) );
}

void CDI::MouseEvent( wxMouseEvent& event )
{
#ifdef    __OCPN__ANDROID__
    if( event.RightDown() ) {
        qDebug() << "right down";
         
        wxContextMenuEvent cevt;
        cevt.SetPosition( event.GetPosition());
        
        ConsoleCanvas *ccp = dynamic_cast<ConsoleCanvas*>(GetParent());
        if(ccp)
            ccp->OnContextMenu( cevt );
        
    }
#endif    
}

void CDI::SetColorScheme( ColorScheme cs )
{
    m_pbackBrush = wxTheBrushList->FindOrCreateBrush( GetGlobalColor( _T("DILG2") ), wxBRUSHSTYLE_SOLID );
    m_proadBrush = wxTheBrushList->FindOrCreateBrush( GetGlobalColor( _T("DILG1") ), wxBRUSHSTYLE_SOLID );
    m_proadPen = wxThePenList->FindOrCreatePen( GetGlobalColor( _T("CHBLK") ), 1, wxPENSTYLE_SOLID );
}

void CDI::OnPaint( wxPaintEvent& event )
{
    int sx, sy;
    GetClientSize( &sx, &sy );

    //    Do the drawing on an off-screen memory DC, and blit into place
    //    to avoid objectionable flashing
    wxMemoryDC mdc;

    wxBitmap m_bitmap( sx, sy, -1 );
    mdc.SelectObject( m_bitmap );
    mdc.SetBackground( *m_pbackBrush );
    mdc.Clear();

    int xp = sx / 2;
    int yp = sy * 9 / 10;

    int path_length = sy * 3;
    int pix_per_xte = 120;

    if( g_pRouteMan->GetpActiveRoute() ) {
        double angle = 90 - ( g_pRouteMan->GetCurrentSegmentCourse() - gCog );

        double dy = path_length * sin( angle * PI / 180. );
        double dx = path_length * cos( angle * PI / 180. );

        int xtedir;
        xtedir = g_pRouteMan->GetXTEDir();
        double xte = g_pRouteMan->GetCurrentXTEToActivePoint();

        double ddy = xtedir * pix_per_xte * xte * sin( ( 90 - angle ) * PI / 180. );
        double ddx = xtedir * pix_per_xte * xte * cos( ( 90 - angle ) * PI / 180. );

        int ddxi = (int) ddx;
        int ddyi = (int) ddy;

        int xc1 = xp - (int) ( dx / 2 ) + ddxi;
        int yc1 = yp + (int) ( dy / 2 ) + ddyi;
        int xc2 = xp + (int) ( dx / 2 ) + ddxi;
        int yc2 = yp - (int) ( dy / 2 ) + ddyi;

        wxPoint road[4];

        int road_top_width = 10;
        int road_bot_width = 40;

        road[0].x = xc1 - (int) ( road_bot_width * cos( ( 90 - angle ) * PI / 180. ) );
        road[0].y = yc1 - (int) ( road_bot_width * sin( ( 90 - angle ) * PI / 180. ) );

        road[1].x = xc2 - (int) ( road_top_width * cos( ( 90 - angle ) * PI / 180. ) );
        road[1].y = yc2 - (int) ( road_top_width * sin( ( 90 - angle ) * PI / 180. ) );

        road[2].x = xc2 + (int) ( road_top_width * cos( ( 90 - angle ) * PI / 180. ) );
        road[2].y = yc2 + (int) ( road_top_width * sin( ( 90 - angle ) * PI / 180. ) );

        road[3].x = xc1 + (int) ( road_bot_width * cos( ( 90 - angle ) * PI / 180. ) );
        road[3].y = yc1 + (int) ( road_bot_width * sin( ( 90 - angle ) * PI / 180. ) );

        mdc.SetBrush( *m_proadBrush );
        mdc.SetPen( *m_proadPen );
        mdc.DrawPolygon( 4, road, 0, 0, wxODDEVEN_RULE );

///        mdc.DrawLine( xc1, yc1, xc2, yc2 );

        mdc.DrawLine( 0, yp, sx, yp );
        mdc.DrawCircle( xp, yp, 6 );
        mdc.DrawLine( xp, yp + 5, xp, yp - 5 );
    }

    wxPaintDC dc( this );
    dc.Blit( 0, 0, sx, sy, &mdc, 0, 0 );
}

