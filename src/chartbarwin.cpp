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
#include "chartbarwin.h"
#include "chartdb.h"
#include "chart1.h"
#include "chartbase.h"
#include "styles.h"
#include "ocpndc.h"

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
extern ChartBarWin               *g_ChartBarWin;
extern Piano                     *g_Piano;

//------------------------------------------------------------------------------
//    ChartBarWin Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(ChartBarWin, wxDialog)
    EVT_PAINT(ChartBarWin::OnPaint)
    EVT_SIZE(ChartBarWin::OnSize)
    EVT_MOUSE_EVENTS(ChartBarWin::MouseEvent)
END_EVENT_TABLE()

ChartBarWin::ChartBarWin( wxWindow *win )
{

    long wstyle = wxSIMPLE_BORDER | wxFRAME_NO_TASKBAR;
#ifndef __WXMAC__
    wstyle |= wxFRAME_SHAPED;
#endif
#ifdef __WXMAC__
    wstyle |= wxSTAY_ON_TOP;
#endif

//    wstyle = wxCAPTION | wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxRESIZE_BORDER;

    wxDialog::Create( win, wxID_ANY, _T(""), wxPoint( 0, 0 ), wxSize( 200, 20 ), wstyle );

    SetBackgroundColour( GetGlobalColor( _T("UIBDR") ) );

    SetBackgroundStyle( wxBG_STYLE_CUSTOM ); // on WXMSW, this prevents flashing on color scheme change

    //   Create the Children

    Raise();

}

ChartBarWin::~ChartBarWin()
{
}

void ChartBarWin::RePosition()
{
    wxSize cs = GetParent()->GetClientSize();
    wxFrame *frame = dynamic_cast<wxFrame*>(GetParent());
    wxPoint position;
    position.x = 0;
    position.y = cs.y;
    position.y -= GetSize().y;

    wxPoint screen_pos = GetParent()->ClientToScreen( position );
    Move( screen_pos );
#ifdef __OCPN__ANDROID__
    Raise();
#endif
}

void ChartBarWin::ReSize()
{
    wxSize cs = GetParent()->GetClientSize();
    SetSize(wxSize(cs.x, g_Piano->GetHeight()));
}

void ChartBarWin::OnPaint( wxPaintEvent& event )
{
    wxPaintDC dc( this );

    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

    wxBitmap shape = wxBitmap( cc1->GetClientSize().x, g_Piano->GetHeight() );
    wxMemoryDC shapeDC( shape );

    g_Piano->Paint(0, dc, &shapeDC);

#ifndef __WXMAC__
    if( style->chartStatusWindowTransparent ) {
        wxRegion region( shape, *wxBLACK, 0 );
        if(region.Empty())
        // SetShape() with a completely empty shape doesn't work, and leaving the shape
        // but hiding the window causes artifacts when dragging in GL mode on MSW.
        // The best solution found so far is to show just a single pixel, this is less
        // disturbing than flashing piano keys when dragging. (wxWidgets 2.8)
           g_ChartBarWin->SetShape( wxRegion( wxRect(0,0,1,1) ) );
        else
            g_ChartBarWin->SetShape( region );
    }
#endif


#ifdef __WXQT__ // temporary workaround
    g_Piano->Refresh();
#endif
}

void ChartBarWin::OnSize( wxSizeEvent& event )
{
    if (!IsShown())
        return;

    g_Piano->FormatKeys();
}

void ChartBarWin::MouseEvent( wxMouseEvent& event )
{
    g_Piano->MouseEvent( event );
}

int ChartBarWin::GetFontHeight()
{
    wxClientDC dc( this );

    wxCoord w, h;
    GetTextExtent( _T("TEST"), &w, &h );

    return ( h );
}

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

    wxPen ppPen( GetGlobalColor( _T("CHBLK") ), 1, wxSOLID );
    dc.SetPen( ppPen );

    dc.SetBrush( m_tBrush );

    for( int i = 0; i < nKeys; i++ ) {
        int key_db_index = m_key_array.Item( i );

        if( -1 == key_db_index ) continue;

        if( ChartData->GetDBChartFamily( m_key_array.Item( i ) ) == CHART_FAMILY_VECTOR ) {
            dc.SetBrush( m_vBrush );

            for( unsigned int ino = 0; ino < m_active_index_array.GetCount(); ino++ ) {
                if( m_active_index_array.Item( ino ) == key_db_index ) // chart is in the active list
                    dc.SetBrush( m_svBrush );
            }
        }
        else {
            dc.SetBrush( m_tBrush );

            for( unsigned int ino = 0; ino < m_active_index_array.GetCount(); ino++ ) {
                if( m_active_index_array.Item( ino ) == key_db_index ) // chart is in the active list
                    dc.SetBrush( m_slBrush );
            }
        }

        if( ChartData->GetDBChartType( m_key_array.Item( i ) ) == CHART_TYPE_CM93 ||
            ChartData->GetDBChartType( m_key_array.Item( i ) ) == CHART_TYPE_CM93COMP ) {
            dc.SetBrush( m_cBrush );

            for( unsigned int ino = 0; ino < m_active_index_array.GetCount(); ino++ ) {
                if( m_active_index_array.Item( ino ) == key_db_index ) // chart is in the active list
                    dc.SetBrush( m_scBrush );
            }
        }

        // Check to see if this box appears in the sub_light array
        // If so, add a crosshatch pattern to the brush
        for( unsigned int ino = 0; ino < m_sublite_index_array.GetCount(); ino++ ) {
            if( m_sublite_index_array.Item( ino ) == key_db_index ) // chart is in the sublite list
            {
                wxBrush ebrush( dc.GetBrush().GetColour(), wxCROSSDIAG_HATCH );
//                              dc.SetBrush(ebrush);
            }
        }

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

        for( unsigned int ino = 0; ino < m_sublite_index_array.GetCount(); ino++ ) {
            if( m_sublite_index_array.Item( ino ) == key_db_index ) { // chart is in the sublite list
                dc.SetBrush( m_backBrush );
                int w = 3;
                dc.DrawRoundedRectangle( box.x + w, box.y + w, box.width - ( 2 * w ),
                                         box.height - ( 2 * w ), 3 );
            }
        }

        //    Look in the current noshow array for this index
        for( unsigned int ino = 0; ino < m_noshow_index_array.GetCount(); ino++ ) {
            if( m_noshow_index_array.Item( ino ) == key_db_index ) { // chart is in the noshow list
                if( m_pInVizIconBmp && m_pInVizIconBmp->IsOk() ) dc.DrawBitmap(
                    ConvertTo24Bit( dc.GetBrush().GetColour(), *m_pInVizIconBmp ), box.x + 4,
                    box.y + 3, false );
                break;
            }
        }

        //    Look in the current skew array for this index
        for( unsigned int ino = 0; ino < m_skew_index_array.GetCount(); ino++ ) {
            if( m_skew_index_array.Item( ino ) == key_db_index ) {       // chart is in the list
                if( m_pSkewIconBmp && m_pSkewIconBmp->IsOk() ) dc.DrawBitmap(
                    ConvertTo24Bit( dc.GetBrush().GetColour(), *m_pSkewIconBmp ),
                    box.x + box.width - m_pSkewIconBmp->GetWidth() - 4, box.y + 2, false );
                break;
            }
        }

        //    Look in the current tmerc array for this index
        for( unsigned int ino = 0; ino < m_tmerc_index_array.GetCount(); ino++ ) {
            if( m_tmerc_index_array.Item( ino ) == key_db_index ) {      // chart is in the list
                if( m_pTmercIconBmp && m_pTmercIconBmp->IsOk() ) dc.DrawBitmap(
                    ConvertTo24Bit( dc.GetBrush().GetColour(), *m_pTmercIconBmp ),
                    box.x + box.width - m_pTmercIconBmp->GetWidth() - 4, box.y + 2, false );
                break;
            }
        }

        //    Look in the current poly array for this index
        for( unsigned int ino = 0; ino < m_poly_index_array.GetCount(); ino++ ) {
            if( m_poly_index_array.Item( ino ) == key_db_index ) {       // chart is in the list
                if( m_pPolyIconBmp && m_pPolyIconBmp->IsOk() ) dc.DrawBitmap(
                    ConvertTo24Bit( dc.GetBrush().GetColour(), *m_pPolyIconBmp ),
                    box.x + box.width - m_pPolyIconBmp->GetWidth() - 4, box.y + 2, false );
                break;
            }
        }
    }
}

/*
void PianoWin::OnSize( wxSizeEvent& event )
{
    m_hash.Clear();
    }*/

void Piano::SetColorScheme( ColorScheme cs )
{

    //    Recreate the local brushes

    m_backBrush = wxBrush( GetGlobalColor( _T("UIBDR") ), wxSOLID );

    m_tBrush = wxBrush( GetGlobalColor( _T("BLUE2") ), wxSOLID );    // Raster Chart unselected
    m_slBrush = wxBrush( GetGlobalColor( _T("BLUE1") ), wxSOLID );    // and selected

    m_vBrush = wxBrush( GetGlobalColor( _T("GREEN2") ), wxSOLID );    // Vector Chart unselected
    m_svBrush = wxBrush( GetGlobalColor( _T("GREEN1") ), wxSOLID );    // and selected

    m_cBrush = wxBrush( GetGlobalColor( _T("YELO2") ), wxSOLID );     // CM93 Chart unselected
    m_scBrush = wxBrush( GetGlobalColor( _T("YELO1") ), wxSOLID );    // and selected

    m_uvBrush = wxBrush( GetGlobalColor( _T("UINFD") ), wxSOLID );    // and unavailable

    if(g_ChartBarWin)
        g_ChartBarWin->Refresh();

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

void Piano::SetSubliteIndexArray( ArrayOfInts array )
{
    m_sublite_index_array = array;
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
    for(unsigned int i=0 ; i < m_sublite_index_array.GetCount() ; i++){
        wxString a;
        a.Printf(_T("%dS"), m_sublite_index_array.Item(i));
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

    if(g_ChartBarWin)
        m_bleaving = event.Leaving();
    else {
        if(event.Leaving() || y < cc1->GetCanvasHeight() - GetHeight()) {
            if(m_bleaving)
                return false;
            m_bleaving = true;
        } else
            m_bleaving = false;
    }

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
                m_action = DEFERRED_KEY_CLICK_UP;
                m_eventTimer.Start(10, wxTIMER_ONE_SHOT);
            }
        } else if( event.RightDown() ) {
            if( sel_index != m_hover_last ) {
                gFrame->HandlePianoRollover( sel_index, sel_dbindex );
                m_hover_last = sel_index;
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
            gFrame->HandlePianoClick( m_click_sel_index, m_click_sel_dbindex );
            ShowBusy( false );
            break;
        default:
            break;
    }
}
