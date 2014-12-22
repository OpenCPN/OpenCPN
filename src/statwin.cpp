/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Status Window
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

#include "statwin.h"
#include "chartdb.h"
#include "chart1.h"
#include "chartbase.h"
#include "styles.h"

#include <wx/arrimpl.cpp> 
WX_DEFINE_OBJARRAY(RegionArray);

//------------------------------------------------------------------------------
//    External Static Storage
//------------------------------------------------------------------------------
extern ChartDB *ChartData;
extern ocpnStyle::StyleManager* g_StyleManager;
extern MyFrame *gFrame;
//------------------------------------------------------------------------------
//    StatWin Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(StatWin, wxDialog)
    EVT_PAINT(StatWin::OnPaint)
    EVT_SIZE(StatWin::OnSize)
    EVT_MOUSE_EVENTS(StatWin::MouseEvent)
END_EVENT_TABLE()

StatWin::StatWin( wxWindow *win )
{

    long wstyle = wxSIMPLE_BORDER | wxFRAME_NO_TASKBAR;
#ifndef __WXMAC__
    wstyle |= wxFRAME_SHAPED;
#endif
#ifdef __WXMAC__
    wstyle |= wxSTAY_ON_TOP;
#endif

    wxDialog::Create( win, wxID_ANY, _T(""), wxPoint( 20, 20 ), wxSize( 5, 5 ), wstyle );

    m_backBrush = wxBrush( GetGlobalColor( _T("UIBDR") ), wxSOLID );

    SetBackgroundColour( GetGlobalColor( _T("UIBDR") ) );

    SetBackgroundStyle( wxBG_STYLE_CUSTOM ); // on WXMSW, this prevents flashing on color scheme change

    m_rows = 1;

    //   Create the Children

    pPiano = new PianoWin( (wxFrame *) this );
    Raise();

}

StatWin::~StatWin()
{
    pPiano->Close();
}

void StatWin::RePosition()
{
    wxSize cs = GetParent()->GetClientSize();
    wxPoint position;
    position.x = 0;
    position.y = cs.y - GetSize().y;

    wxPoint screen_pos = GetParent()->ClientToScreen( position );
    Move( screen_pos );
}

void StatWin::ReSize()
{
    wxSize cs = GetParent()->GetClientSize();
    wxSize new_size;
    new_size.x = cs.x;
    new_size.y = 22 * GetRows();
    SetSize(new_size);
    
}

void StatWin::OnPaint( wxPaintEvent& event )
{
    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

    wxPaintDC dc( this );
    if( style->chartStatusWindowTransparent ) return;

    dc.SetBackground( m_backBrush );
    dc.Clear();
}

void StatWin::OnSize( wxSizeEvent& event )
{
    int width, height;
    GetClientSize( &width, &height );
    int x, y;
    GetPosition( &x, &y );

    if( width ) {
        pPiano->SetSize( 0, 0, width * 6 / 10, height * 1 / m_rows );
        pPiano->FormatKeys();
    }
}

void StatWin::FormatStat( void )
{
    pPiano->FormatKeys();
}

void StatWin::MouseEvent( wxMouseEvent& event )
{
    int x, y;
    event.GetPosition( &x, &y );
}

int StatWin::GetFontHeight()
{
    wxClientDC dc( this );

    wxCoord w, h;
    GetTextExtent( _T("TEST"), &w, &h );

    return ( h );
}

void StatWin::SetColorScheme( ColorScheme cs )
{

    m_backBrush = wxBrush( GetGlobalColor( _T("UIBDR") ), wxSOLID );

    //  Also apply color scheme to all known children
    pPiano->SetColorScheme( cs );

    Refresh();
}
//------------------------------------------------------------------------------
//          TextStat Window Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(TStatWin, wxWindow)
    EVT_PAINT(TStatWin::OnPaint)
    EVT_SIZE(TStatWin::OnSize)
END_EVENT_TABLE()

TStatWin::TStatWin( wxFrame *frame ) :
        wxWindow( frame, wxID_ANY, wxPoint( 20, 20 ), wxSize( 5, 5 ), wxSIMPLE_BORDER )
{
    SetBackgroundColour( GetGlobalColor( _T("UIBDR") ) );
    pText = new wxString();
    bTextSet = false;
}

TStatWin::~TStatWin( void )
{
    delete pText;
}

void TStatWin::OnSize( wxSizeEvent& event )
{
}

void TStatWin::OnPaint( wxPaintEvent& event )
{
    wxPaintDC dc( this );
    dc.DrawText( *pText, 0, 0 );
}

void TStatWin::TextDraw( const wxString& text )
{
    *pText = text;
    bTextSet = true;
    Refresh( true );
}
//------------------------------------------------------------------------------
//          Piano Window Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(PianoWin, wxWindow)
    EVT_PAINT(PianoWin::OnPaint)
    EVT_SIZE(PianoWin::OnSize)
    EVT_MOUSE_EVENTS(PianoWin::MouseEvent)
END_EVENT_TABLE()

// Define a constructor
PianoWin::PianoWin( wxFrame *frame ) :
        wxWindow( frame, wxID_ANY, wxPoint( 20, 20 ), wxSize( 5, 5 ), wxNO_BORDER )
{
    m_index_last = -1;
    m_iactive = -1;

    m_hover_icon_last = -1;
    m_hover_last = -1;
    m_brounded = false;

    m_nRegions = 0;

    SetBackgroundStyle( wxBG_STYLE_CUSTOM ); // on WXMSW, this prevents flashing on color scheme change

    m_pVizIconBmp = NULL;
    m_pInVizIconBmp = NULL;
    m_pPolyIconBmp = NULL;
    m_pSkewIconBmp = NULL;
    m_pTmercIconBmp = NULL;
}

PianoWin::~PianoWin()
{
    if( m_pInVizIconBmp ) delete m_pInVizIconBmp;
    if( m_pPolyIconBmp ) delete m_pPolyIconBmp;
    if( m_pSkewIconBmp ) delete m_pSkewIconBmp;
    if( m_pTmercIconBmp ) delete m_pTmercIconBmp;
    if( m_pVizIconBmp ) delete m_pVizIconBmp;
}

void PianoWin::OnSize( wxSizeEvent& event )
{
    m_hash.Clear();
}

void PianoWin::SetColorScheme( ColorScheme cs )
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

}

void PianoWin::OnPaint( wxPaintEvent& event )
{
    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
    int width, height;
    GetClientSize( &width, &height );
    wxPaintDC dc( this );

    wxBitmap shape = wxBitmap( width, height );
    wxMemoryDC shapeDc( shape );
    shapeDc.SetBackground( *wxBLACK_BRUSH);
    shapeDc.SetBrush( *wxWHITE_BRUSH);
    shapeDc.SetPen( *wxWHITE_PEN);
    shapeDc.Clear();

    dc.SetBackground( m_backBrush );
    dc.Clear();

//    Create the Piano Keys

    int nKeys = m_key_array.GetCount();


    if( nKeys ) {
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
            
            if( ChartData->GetDBChartType( m_key_array.Item( i ) ) == CHART_TYPE_CM93 ) {
                    dc.SetBrush( m_cBrush );

                    for( unsigned int ino = 0; ino < m_active_index_array.GetCount(); ino++ ) {
                        if( m_active_index_array.Item( ino ) == key_db_index ) // chart is in the active list
                        dc.SetBrush( m_scBrush );
                    }
            }

            if( ChartData->GetDBChartType( m_key_array.Item( i ) ) == CHART_TYPE_CM93COMP ) {
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

            wxRect box = KeyRegion.Item( i ).GetBox();

            if( m_brounded ) {
                dc.DrawRoundedRectangle( box.x, box.y, box.width, box.height, 4 );
                shapeDc.DrawRoundedRectangle( box.x, box.y, box.width, box.height, 4 );
            } else {
                dc.DrawRectangle( box );
                shapeDc.DrawRectangle( box );
            }

            for( unsigned int ino = 0; ino < m_sublite_index_array.GetCount(); ino++ ) {
                if( m_sublite_index_array.Item( ino ) == key_db_index ) { // chart is in the sublite list
                    dc.SetBrush( dc.GetBackground() );
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
#ifndef __WXMAC__
        if( style->chartStatusWindowTransparent )
            ((wxDialog*) GetParent())->SetShape( wxRegion( shape, *wxBLACK, 0 ) );
    }
    else {
        // SetShape() with a completely empty shape doesn't work, and leaving the shape
        // but hiding the window causes artifacts when dragging in GL mode on MSW.
        // The best solution found so far is to show just a single pixel, this is less
        // disturbing than flashing piano keys when dragging. (wxWidgets 2.8)
        if( style->chartStatusWindowTransparent )
            ((wxDialog*) GetParent())->SetShape( wxRegion( wxRect(0,0,1,1) ) );
    }
#else
    }
#endif
}

void PianoWin::SetKeyArray( ArrayOfInts array )
{
    m_key_array = array;
    FormatKeys();
}

void PianoWin::SetNoshowIndexArray( ArrayOfInts array )
{
    m_noshow_index_array = array;
}

void PianoWin::SetActiveKeyArray( ArrayOfInts array )
{
    m_active_index_array = array;
}

void PianoWin::SetSubliteIndexArray( ArrayOfInts array )
{
    m_sublite_index_array = array;
}

void PianoWin::SetSkewIndexArray( ArrayOfInts array )
{
    m_skew_index_array = array;
}

void PianoWin::SetTmercIndexArray( ArrayOfInts array )
{
    m_tmerc_index_array = array;
}

void PianoWin::SetPolyIndexArray( ArrayOfInts array )
{
    m_poly_index_array = array;
}

wxString PianoWin::GetStateHash()
{
    wxString hash;
    
    for(unsigned int i=0 ; i < m_key_array.GetCount() ; i++){
        wxString a;
        a.Printf(_T("%d|"), m_key_array.Item(i));
        hash += a;
    }
    
    for(unsigned int i=0 ; i < m_noshow_index_array.GetCount() ; i++){
        wxString a;
        a.Printf(_T("%d|"), m_noshow_index_array.Item(i));
        hash += a;
    }
    for(unsigned int i=0 ; i < m_active_index_array.GetCount() ; i++){
        wxString a;
        a.Printf(_T("%d|"), m_active_index_array.Item(i));
        hash += a;
    }
    for(unsigned int i=0 ; i < m_sublite_index_array.GetCount() ; i++){
        wxString a;
        a.Printf(_T("%d|"), m_sublite_index_array.Item(i));
        hash += a;
    }
    for(unsigned int i=0 ; i < m_skew_index_array.GetCount() ; i++){
        wxString a;
        a.Printf(_T("%d|"), m_skew_index_array.Item(i));
        hash += a;
    }
    for(unsigned int i=0 ; i < m_tmerc_index_array.GetCount() ; i++){
        wxString a;
        a.Printf(_T("%d|"), m_tmerc_index_array.Item(i));
        hash += a;
    }
    for(unsigned int i=0 ; i < m_poly_index_array.GetCount() ; i++){
        wxString a;
        a.Printf(_T("%d|"), m_poly_index_array.Item(i));
        hash += a;
    }
    
    return hash;
}
    
wxString &PianoWin::GenerateAndStoreNewHash()
{
    m_hash = GetStateHash();
    return m_hash;
}

wxString &PianoWin::GetStoredHash()
{
    return m_hash;
}


void PianoWin::FormatKeys( void )
{
    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
    int width, height;
    GetClientSize( &width, &height );

    int nKeys = m_key_array.GetCount();
    if( nKeys ) {
        int kw = style->chartStatusIconWidth;
        if( !kw ) kw = width / nKeys;

//    Build the Key Regions

        KeyRegion.Empty();
        KeyRegion.Alloc( nKeys );
        for( int i = 0; i < nKeys; i++ ) {
            wxRegion r( ( i * kw ) + 3, 2, kw - 6, height - 4 );
            KeyRegion.Add( r );
        }
    }
    m_nRegions = nKeys;

}
wxPoint PianoWin::GetKeyOrigin( int key_index )
{
    if( ( key_index >= 0 ) && ( key_index <= (int) m_key_array.GetCount() - 1 ) ) {
        wxRect box = KeyRegion.Item( key_index ).GetBox();
        return wxPoint( box.x, box.y );
    } else
        return wxPoint( -1, -1 );
}

void PianoWin::MouseEvent( wxMouseEvent& event )
{

    int x, y;
    event.GetPosition( &x, &y );

//    Check the regions

    int sel_index = -1;
    int sel_dbindex = -1;

    for( int i = 0; i < m_nRegions; i++ ) {
        if( KeyRegion.Item( i ).Contains( x, y ) == wxInRegion ) {
            sel_index = i;
            sel_dbindex = m_key_array.Item( i );
            break;
        }
    }

    if( event.LeftDown() ) {

        if( -1 != sel_index ) {
            gFrame->HandlePianoClick( sel_index, sel_dbindex );
            gFrame->Raise();
        }
    }

    else if( event.RightDown() ) {
            if( -1 != sel_index ) {
                gFrame->HandlePianoRClick( x, y, sel_index, sel_dbindex );
                gFrame->Raise();
            }
        }

    else if(!event.ButtonUp()){
         if( sel_index != m_hover_last ) {
                gFrame->HandlePianoRollover( sel_index, sel_dbindex );
                m_hover_last = sel_index;
          }
    }

    if( event.Leaving() ) {
        gFrame->HandlePianoRollover( -1, -1 );
        gFrame->HandlePianoRolloverIcon( -1, -1 );

        m_index_last = -1;
        m_hover_icon_last = -1;
        m_hover_last = -1;
    }

    /*
     Todo:
     Could do something like this to better encapsulate the pianowin
     Allows us to get rid of global statics...

     wxCommandEvent ev(MyPianoEvent);    // Private event
     ..set up event to specify action...SelectChart, SetChartThumbnail, etc
     ::PostEvent(pEventReceiver, ev);    // event receiver passed to ctor

     */

}

void PianoWin::ResetRollover( void )
{
    m_index_last = -1;
    m_hover_icon_last = -1;
    m_hover_last = -1;
}


