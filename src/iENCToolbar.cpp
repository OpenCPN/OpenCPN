/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN iENCToolbar
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2017 by David S. Register                               *
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

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif

#include "toolbar.h"
#include "iENCToolbar.h"

#include "chart1.h"
#include "chcanv.h"
#include "s52s57.h"
#include "s52plib.h"
#include "pluginmanager.h"

extern s52plib *ps52plib;
extern MyFrame * gFrame;
extern ChartCanvas *cc1;
extern PlugInManager *g_pi_manager;
 

#if 0
#include <vector>

#include "ocpn_types.h"
#include "navutil.h"
#include "styles.h"
#include "toolbar.h"
#include "chart1.h"
#include "pluginmanager.h"
#include "OCPNPlatform.h"


extern bool                       g_bTransparentToolbar;
extern bool                       g_bTransparentToolbarInOpenGLOK;
extern ChartCanvas*               cc1;
extern bool                       g_bopengl;
extern ocpnStyle::StyleManager*   g_StyleManager;
extern MyFrame*                   gFrame;
extern PlugInManager*             g_pi_manager;
extern wxString                   g_toolbarConfig;
extern bool                       g_bPermanentMOBIcon;
extern bool                       g_btouch;
extern bool                       g_bsmoothpanzoom;
extern OCPNPlatform               *g_Platform;
#endif


//---------------------------------------------------------------------------------------
//          iENCToolbar Implementation
//---------------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(iENCToolbar, ocpnFloatingToolbarDialog)
    EVT_MENU(wxID_ANY, iENCToolbar::OnToolLeftClick)
    EVT_TIMER ( STATE_TIMER, iENCToolbar::StateTimerEvent )
END_EVENT_TABLE()

iENCToolbar::iENCToolbar( wxWindow *parent, wxPoint position, long orient, float size_factor ):
        ocpnFloatingToolbarDialog( parent, position, orient, size_factor)
{
 
    LoadToolBitmaps();
 
    m_rangeFont = wxTheFontList->FindOrCreateFont( 12, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );
    
    
    wxSize a = m_bmMinimum.GetSize();
    m_ptoolbar->SetToolBitmapSize( a );

    m_pbmScratch = new wxBitmap(a.x, a.y);
    
    m_bmTemplate = m_bmRMinus;
    
    m_toolDensity = m_ptoolbar->AddTool( ID_DENSITY, _T("Density"), m_bmMinimum, m_bmMinimum );
    m_ptoolbar->AddTool( ID_RPLUS, _T("RangePlus"), m_bmRPlus, m_bmRPlus );
    m_ptoolbar->AddTool( ID_RMINUS, _T("RangeMinus"), m_bmRMinus, m_bmRMinus );
    
    
    m_nDensity = 0;
    SetDensityToolBitmap(m_nDensity);
    
    // Realize() the toolbar
    Realize();
    Show(true);
    
    RePosition();
    
    m_state_timer.SetOwner( this, STATE_TIMER );
    m_state_timer.Start( 500, wxTIMER_CONTINUOUS );
    
    
    
}

iENCToolbar::~iENCToolbar()
{

}


void iENCToolbar::LoadToolBitmaps()
{
    wxString dataDir = _T("/home/dsr/");
    
    m_bmMinimum = wxBitmap( dataDir + _T("iconMinimum.png"), wxBITMAP_TYPE_PNG);
    m_bmStandard = wxBitmap( dataDir + _T("iconStandard.png"), wxBITMAP_TYPE_PNG);
    m_bmAll = wxBitmap( dataDir + _T("iconAll.png"), wxBITMAP_TYPE_PNG);

    m_bmRPlus = wxBitmap( dataDir + _T("iconRPlus.png"), wxBITMAP_TYPE_PNG);
    m_bmRMinus = wxBitmap( dataDir + _T("iconRMinus.png"), wxBITMAP_TYPE_PNG);
    
}
    
    

void iENCToolbar::OnToolLeftClick( wxCommandEvent& event )
{
    int itemId = event.GetId();

    enum _DisCat nset = STANDARD;
    double range;
    
    switch(itemId){
        case ID_DENSITY:

            if(++m_nDensity > 2)
                m_nDensity = 0;
            
            SetDensityToolBitmap(m_nDensity);
            m_ptoolbar->Refresh();
            
            
            switch(m_nDensity){
                case 0:
                    nset = DISPLAYBASE;
                    break;
                case 1:
                    nset = STANDARD;
                    break;
                case 2:
                    nset = OTHER;
                    break;
                case 3:
                    nset = MARINERS_STANDARD;
                    break;
                default:
                    nset = STANDARD;
                    break;
            }
            
            gFrame->SetENCDisplayCategory( nset );
            
            break;

        case ID_RMINUS:
            range = cc1->GetCanvasRangeMeters();
            range = wxRound(range * 10) / 10.;
            
            if(range > 4000.)
                cc1->SetCanvasRangeMeters(4000.);
            else if(range > 2000.)
                cc1->SetCanvasRangeMeters(2000.);
            else if(range > 1600.)
                cc1->SetCanvasRangeMeters(1600.);
            else if(range > 1200.)
                cc1->SetCanvasRangeMeters(1200.);
            else if(range > 800.)
                cc1->SetCanvasRangeMeters(800.);
            else if(range > 500.)
                cc1->SetCanvasRangeMeters(500.);
                
            break;
                    
        case ID_RPLUS:
            range = cc1->GetCanvasRangeMeters();
            range = wxRound(range * 10) / 10.;
            
            if(range < 500.)
                cc1->SetCanvasRangeMeters(500.);
            else if(range < 800.)
                cc1->SetCanvasRangeMeters(800.);
            else if(range < 1200.)
                cc1->SetCanvasRangeMeters(1200.);
            else if(range < 1600.)
                cc1->SetCanvasRangeMeters(1600.);
            else if(range < 2000.)
                cc1->SetCanvasRangeMeters(2000.);
            else if(range < 4000.)
                cc1->SetCanvasRangeMeters(4000.);
            
            break;
        
        default:
            break;
    }
    
}



void iENCToolbar::SetDensityToolBitmap( int nDensity)
{
    int toolID = m_toolDensity->GetId();
    
    if(nDensity == 0)
        m_ptoolbar->SetToolBitmaps( ID_DENSITY, &m_bmMinimum, &m_bmMinimum );
    else if(nDensity == 1)
        m_ptoolbar->SetToolBitmaps( ID_DENSITY, &m_bmStandard, &m_bmStandard );
    else if(nDensity == 2)
        m_ptoolbar->SetToolBitmaps( ID_DENSITY, &m_bmAll, &m_bmAll );
    
}    


void iENCToolbar::StateTimerEvent( wxTimerEvent& event )
{
    //  Keep the Density tool in sync
    if(ps52plib){
        int nset = 1;

        switch (ps52plib->GetDisplayCategory()) {
            case (DISPLAYBASE):
                nset = 0;
                break;
            case (STANDARD):
                nset = 1;
                break;
            case (OTHER):
                nset = 2;
                break;
            case (MARINERS_STANDARD):
                nset = 3;
                break;
            default:
                nset = 1;
                break;
        }
        
        if( nset != m_nDensity){
            if(nset < 3){
                m_nDensity = nset;
                SetDensityToolBitmap( m_nDensity );
                
                m_ptoolbar->Refresh();
            }
        }
    }
    
    // Keep the Range annunciator updated
    if(cc1){
        double range = cc1->GetCanvasRangeMeters();
     
        if(1/*range != m_range*/){
            
            m_range = range;
            
            // Get the template bitmap
//             wxImage image = m_bmRMinus.ConvertToImage();
//             wxBitmap bmTemplate(image);

            wxBitmap bmTemplate = m_bmRMinus;
            
            wxMask *mask = new wxMask(bmTemplate, wxColour(wxTRANSPARENT));
            bmTemplate.SetMask(mask);
            
            wxMemoryDC dct(bmTemplate);
            
            //  Make a deep copy by Blit
            wxMemoryDC dc;
            dc.SelectObject(*m_pbmScratch);
            dc.SetBackground(wxBrush(GetGlobalColor(_T("GREY2"))));
            dc.Clear();
            
            dc.Blit(0, 0, m_pbmScratch->GetWidth(), m_pbmScratch->GetHeight(), &dct, 0, 0, wxCOPY, true);
            
            dct.SelectObject(wxNullBitmap);
            
            //  Render the range as text onto the template
            
            dc.SetFont( *m_rangeFont );
            dc.SetTextForeground(*wxBLACK);
            dc.SetBackgroundMode(wxTRANSPARENT);
            
            wxString range_string;
            range_string.Printf(_T("%4.0fm"), range);
            
            dc.DrawText(range_string, 42, 8);
            
            dc.SelectObject(wxNullBitmap);

            m_ptoolbar->SetToolBitmaps( ID_RMINUS, m_pbmScratch, m_pbmScratch );
            
            m_ptoolbar->Refresh();
        }
    }
}














#if 0
void ocpnFloatingToolbarDialog::CreateConfigMenu()
{
    if(m_FloatingToolbarConfigMenu)
        delete m_FloatingToolbarConfigMenu;
    m_FloatingToolbarConfigMenu = new wxMenu();
}

bool ocpnFloatingToolbarDialog::_toolbarConfigMenuUtil( int toolid, wxString tipString )
{
    if(m_FloatingToolbarConfigMenu){
        wxMenuItem* menuitem;

        if( toolid == ID_MOB && g_bPermanentMOBIcon ) return true;

        // Item ID trickery is needed because the wxCommandEvents for menu item clicked and toolbar button
        // clicked are 100% identical, so if we use same id's we can't tell the events apart.

        int idOffset = ID_PLUGIN_BASE - ID_ZOOMIN + 100;  // Hopefully no more than 100 plugins loaded...
        int menuItemId = toolid + idOffset;

        menuitem = m_FloatingToolbarConfigMenu->FindItem( menuItemId );

        if( menuitem ) {
            return menuitem->IsChecked();
        }

        menuitem = m_FloatingToolbarConfigMenu->AppendCheckItem( menuItemId, tipString );
        menuitem->Check( g_toolbarConfig.GetChar( toolid - ID_ZOOMIN ) == _T('X') );
        return menuitem->IsChecked();
    }
    else
        return true;
}


void ocpnFloatingToolbarDialog::OnWindowCreate( wxWindowCreateEvent& event )
{
    Realize();
}

void ocpnFloatingToolbarDialog::SetGrabber( wxString icon_name )
{
//    m_pGrabberwin->Destroy();
    m_pGrabberwin = new GrabberWin( this, this, m_sizefactor, icon_name );
    m_pGrabberwin->Show();

    Realize();

#ifdef __WXOSX__
    m_pGrabberwin->Refresh();
#endif

}


void ocpnFloatingToolbarDialog::UpdateRecoveryWindow(bool b_toolbarEnable)
{
    if(m_pRecoverwin ){
        if(b_toolbarEnable){
            m_pRecoverwin->Raise();
            m_pRecoverwin->Refresh( false );
        }
        else
            m_pRecoverwin->Hide();
    }
 }

void ocpnFloatingToolbarDialog::EnableTool( int toolid, bool enable )
{
    if(m_ptoolbar)
        m_ptoolbar->EnableTool( toolid, enable);
}


void ocpnFloatingToolbarDialog::SetColorScheme( ColorScheme cs )
{
    m_cs = cs;

    wxColour back_color = GetGlobalColor( _T("GREY2") );

    //  Set background
    SetBackgroundColour( back_color );
    ClearBackground();

    if( m_ptoolbar ) {
        //  Set background
        m_ptoolbar->SetBackgroundColour( back_color );
        m_ptoolbar->ClearBackground();

        m_ptoolbar->SetToggledBackgroundColour( GetGlobalColor( _T("GREY1") ) );

        m_ptoolbar->SetColorScheme( cs );
        m_ptoolbar->Refresh( true );
    }

    if( m_pGrabberwin ){
        m_pGrabberwin->SetColorScheme( cs );
        m_pGrabberwin->Refresh();
    }

    Refresh(true);

}

void ocpnFloatingToolbarDialog::SetGeometry(bool bAvoid, wxRect rectAvoid)
{

    if( m_ptoolbar ) {
        wxSize style_tool_size = m_style->GetToolSize();

        style_tool_size.x *= m_sizefactor;
        style_tool_size.y *= m_sizefactor;

        m_ptoolbar->SetToolBitmapSize( style_tool_size );

        wxSize tool_size = m_ptoolbar->GetToolBitmapSize();
        int grabber_width =  m_style->GetIcon( _T("grabber") ).GetWidth();

        int max_rows = 10;
        int max_cols = 100;
        if(cc1){

            int avoid_start = cc1->GetClientSize().x - (tool_size.x + m_style->GetToolSeparation()) * 2;  // default
            if(bAvoid && !rectAvoid.IsEmpty()){
                avoid_start = cc1->GetClientSize().x - rectAvoid.width - 10;  // this is compass window, if shown
            }


            max_rows = (cc1->GetClientSize().y / ( tool_size.y + m_style->GetToolSeparation())) - 1;

            max_cols = (avoid_start - grabber_width) / ( tool_size.x + m_style->GetToolSeparation());
            max_cols -= 1;

            if(m_orient == wxTB_VERTICAL)
                max_rows = wxMax( max_rows, 2);             // at least two rows
            else
                max_cols = wxMax( max_cols, 2);             // at least two columns
        }

        if( m_orient == wxTB_VERTICAL )
            m_ptoolbar->SetMaxRowsCols(max_rows, 100);
        else
            m_ptoolbar->SetMaxRowsCols( 100, max_cols);
        m_ptoolbar->SetSizeFactor(m_sizefactor);

    }
 }

void ocpnFloatingToolbarDialog::RePosition()
{
    if(m_block) return;

    if( m_pparent && m_ptoolbar ) {
        wxSize cs = m_pparent->GetClientSize();
        if( -1 == m_dock_x ) m_position.x = 0;
        else
            if( 1 == m_dock_x ) m_position.x = cs.x - GetSize().x;

        if( -1 == m_dock_y ) m_position.y = 0;
        else
            if( 1 == m_dock_y ) m_position.y = cs.y - GetSize().y;

        m_position.x = wxMin(cs.x - GetSize().x, m_position.x);
        m_position.y = wxMin(cs.y - GetSize().y, m_position.y);

        m_position.x = wxMax(0, m_position.x);
        m_position.y = wxMax(0, m_position.y);

        wxPoint screen_pos = m_pparent->ClientToScreen( m_position );

        //  GTK sometimes has trouble with ClientToScreen() if executed in the context of an event handler
        //  The position of the window is calculated incorrectly if a deferred Move() has not been processed yet.
        //  So work around this here...
        //  Discovered with a Dashboard window left-docked, toggled on and off by toolbar tool.
#ifdef __WXGTK__
        wxPoint pp = m_pparent->GetPosition();
        wxPoint ppg = m_pparent->GetParent()->GetScreenPosition();
        wxPoint screen_pos_fix = ppg + pp + m_position;
        screen_pos.x = screen_pos_fix.x;
#endif

        Move( screen_pos );

#ifdef __WXQT__
        Raise();
#endif

    }
}

void ocpnFloatingToolbarDialog::Submerge()
{
    m_bsubmerged = true;
    Hide();
    if( m_ptoolbar ) m_ptoolbar->KillTooltip();
}

void ocpnFloatingToolbarDialog::SubmergeToGrabber()
{
//Submerge();
    m_bsubmerged = true;
    m_bsubmergedToGrabber = true;
    Hide();
    if( m_ptoolbar ) m_ptoolbar->KillTooltip();

    m_pRecoverwin = new GrabberWin( m_pparent, this, m_sizefactor, _T("grabber_ext" ), wxPoint(10,10) );

    m_pRecoverwin->Show();
    m_pRecoverwin->Raise();
#ifdef __WXQT__
    wxSize s = gFrame->GetSize();
    m_recoversize = s;
    s.y--;
    gFrame->TriggerResize(s);
    Raise();
#endif

    gFrame->Refresh();          // Needed for MSW OpenGL
}

void ocpnFloatingToolbarDialog::Surface()
{

    if(m_pRecoverwin){
        //SurfaceFromGrabber();
        m_pRecoverwin->Show();
        m_pRecoverwin->Raise();
    }
    else {
        m_bsubmerged = false;
        #ifdef __WXMSW__
        Hide();
        Move( 0, 0 );
        #endif

        RePosition();
        Show();
        if( m_ptoolbar )
            m_ptoolbar->EnableTooltips();

        #ifdef __WXQT__
        Raise();
        #endif
    }

    if( m_bAutoHideToolbar && (m_nAutoHideToolbar > 0) ){
        m_fade_timer.Start( m_nAutoHideToolbar * 1000 );
    }
}

bool ocpnFloatingToolbarDialog::CheckSurfaceRequest( wxMouseEvent &event )
{
    if( m_bsubmerged ){
        if( event.LeftUp() ){
            int x,y;
            event.GetPosition( &x, &y );
            if( m_pRecoverwin ){
                wxRect winRect = m_pRecoverwin->GetRect();
                if( winRect.Contains( x, y ) ){
                    SurfaceFromGrabber();
                    return true;
                }
            }
        }
    }

    return false;
}

void ocpnFloatingToolbarDialog::SurfaceFromGrabber()
{
    m_bsubmerged = false;
    m_bsubmergedToGrabber = false;

#ifndef __WXOSX__
    Hide();
    Move( 0, 0 );
#endif

    if( m_ptoolbar )
        m_ptoolbar->InvalidateBitmaps();

    RePosition();
    Show();
    if( m_ptoolbar )
        m_ptoolbar->EnableTooltips();

    if( m_bAutoHideToolbar && (m_nAutoHideToolbar > 0) ){
        m_fade_timer.Start( m_nAutoHideToolbar * 1000 );
    }

#ifdef __WXQT__
    wxSize s = gFrame->GetSize();               // check for rotation
    if(m_recoversize.x == s.x)
        gFrame->TriggerResize(m_recoversize);
    Raise();
#endif

    if(!m_destroyTimer.IsRunning()){
        m_destroyGrabber = m_pRecoverwin;
        m_pRecoverwin = NULL;
        m_destroyTimer.Start( 5, wxTIMER_ONE_SHOT );           //  Destor the unneeded recovery grabber
    }

}

void ocpnFloatingToolbarDialog::DestroyTimerEvent( wxTimerEvent& event )
{
    delete m_destroyGrabber;
    m_destroyGrabber = NULL;

}

bool ocpnFloatingToolbarDialog::isSubmergedToGrabber()
{
    return (m_bsubmergedToGrabber);
}

void ocpnFloatingToolbarDialog::HideTooltip()
{
    if( m_ptoolbar ) m_ptoolbar->HideTooltip();
}

void ocpnFloatingToolbarDialog::ShowTooltips()
{
    if( m_ptoolbar ) m_ptoolbar->EnableTooltips();
}

void ocpnFloatingToolbarDialog::ToggleOrientation()
{
    if( m_orient == wxTB_HORIZONTAL )
        m_orient = wxTB_VERTICAL;
    else
        m_orient = wxTB_HORIZONTAL;

    m_style->SetOrientation( m_orient );

    wxPoint old_screen_pos = m_pparent->ClientToScreen( m_position );
    wxPoint grabber_point_abs = ClientToScreen( m_pGrabberwin->GetPosition() );

    gFrame->RequestNewToolbar();
    wxPoint pos_abs = grabber_point_abs;
    pos_abs.x -= m_pGrabberwin->GetPosition().x;
    MoveDialogInScreenCoords( pos_abs, old_screen_pos );


    Show();   // this seems to be necessary on GTK to kick the sizer into gear...(FS#553)
    Refresh(true);
    Raise();
}

void ocpnFloatingToolbarDialog::MouseEvent( wxMouseEvent& event )
{
    if( g_bTransparentToolbar ) {
        if( event.Entering() && ( m_opacity < 255 ) ) {
            SetTransparent( 255 );
            m_opacity = 255;
        }

        m_fade_timer.Start( 5000 );           // retrigger the continuous timer
    }

    if(m_bAutoHideToolbar && (m_nAutoHideToolbar > 0) ){
        m_fade_timer.Start( m_nAutoHideToolbar * 1000 );
    }
}

void ocpnFloatingToolbarDialog::FadeTimerEvent( wxTimerEvent& event )
{
    if(m_bnavgrabber){
        m_fade_timer.Start( 5000 );           // do nothing if nav grabber is shown
    }
    else{
        if( g_bTransparentToolbar && (!g_bopengl || g_bTransparentToolbarInOpenGLOK) ){
            DoFade( 128 );
            m_fade_timer.Start( 5000 );           // retrigger the continuous timer
        }

        if(m_bAutoHideToolbar && (m_nAutoHideToolbar > 0) && !m_bsubmerged){
            SubmergeToGrabber();
//            m_fade_timer.Stop();
        }
    }
}

void ocpnFloatingToolbarDialog::SetAutoHideTimer(int time)
{
    m_nAutoHideToolbar = time;
    if(m_bAutoHideToolbar){
        m_fade_timer.Stop();
        m_fade_timer.Start( m_nAutoHideToolbar * 1000 );
    }
}

void ocpnFloatingToolbarDialog::DoFade( int value )
{
    if( value != m_opacity ) SetTransparent( value );
    m_opacity = value;
}

void ocpnFloatingToolbarDialog::RefreshFadeTimer()
{
    SetTransparent( 255 );
    m_opacity = 255;
    m_fade_timer.Start( 500 );           // retrigger the continuous timer

    if(m_bAutoHideToolbar && (m_nAutoHideToolbar > 0) ){
        m_fade_timer.Start( m_nAutoHideToolbar * 1000 );
    }

}

void ocpnFloatingToolbarDialog::SetToolShortHelp( int id, const wxString& help )
{
    if(m_ptoolbar)
        m_ptoolbar->SetToolShortHelp( id, help);
}


void ocpnFloatingToolbarDialog::MoveDialogInScreenCoords( wxPoint posn, wxPoint posn_old )
{
    wxPoint pos_in_parent = m_pparent->ScreenToClient( posn );
    wxPoint pos_in_parent_old = m_pparent->ScreenToClient( posn_old );

    //    "Docking" support
#define DOCK_MARGIN 40

    // X
    m_dock_x = 0;
    if( pos_in_parent.x < pos_in_parent_old.x )            // moving left
            {
        if( pos_in_parent.x < DOCK_MARGIN ) {
            pos_in_parent.x = 0;
            m_dock_x = -1;
        }
    } else
        if( pos_in_parent.x > pos_in_parent_old.x )            // moving right
                {
            int max_right = m_pparent->GetClientSize().x - GetSize().x;
            if( pos_in_parent.x > ( max_right - DOCK_MARGIN ) ) {
                pos_in_parent.x = max_right;
                m_dock_x = 1;
            }
        }

    // Y
    m_dock_y = 0;
    if( pos_in_parent.y < pos_in_parent_old.y )            // moving up
            {
        if( pos_in_parent.y < DOCK_MARGIN ) {
            pos_in_parent.y = 0;
            m_dock_y = -1;
        }
    } else
        if( pos_in_parent.y > pos_in_parent_old.y )            // moving down
                {
            int max_down = m_pparent->GetClientSize().y - GetSize().y;
            if( pos_in_parent.y > ( max_down - DOCK_MARGIN ) ) {
                pos_in_parent.y = max_down;
                m_dock_y = 1;
            }
        }

    m_position = pos_in_parent;

    wxPoint final_pos = m_pparent->ClientToScreen( pos_in_parent );

    Move( final_pos );
}

void ocpnFloatingToolbarDialog::Realize()
{
    if( m_ptoolbar ) {
        m_ptoolbar->Realize();

        m_topSizer->Clear();
        m_topSizer->Add( m_ptoolbar );
        m_topSizer->Add( m_pGrabberwin, 0, wxTOP, m_style->GetTopMargin() );

        m_topSizer->Layout();
        Fit();

        //    Update "Dock" parameters
        if( m_position.x == 0 ) m_dock_x = -1;
        else
            if( m_position.x == m_pparent->GetClientSize().x - GetSize().x ) m_dock_x = 1;

        if( m_position.y == 0 ) m_dock_y = -1;
        else
            if( m_position.y == m_pparent->GetClientSize().y - GetSize().y ) m_dock_y = 1;

        // Now create a bitmap mask forthe frame shape.

        if( m_marginsInvisible ) {

            wxSize tool_size = m_ptoolbar->GetToolBitmapSize();

            //  Determine whether the tool icons are meant (by style) to join without speces between
            //  This will determine what type of region to draw.
            bool b_overlap = false;

            wxToolBarToolsList::compatibility_iterator node1 = m_ptoolbar->m_tools.GetFirst();
            wxToolBarToolsList::compatibility_iterator node2 = node1->GetNext() ;

            wxToolBarToolBase *tool1 = node1->GetData();
            ocpnToolBarTool *tools1 = (ocpnToolBarTool *) tool1;

            wxToolBarToolBase *tool2 = node2->GetData();
            ocpnToolBarTool *tools2 = (ocpnToolBarTool *) tool2;

            if( (tools1->m_x + tools1->m_width) >= tools2->m_x)
                b_overlap = true;




            int toolCount = m_ptoolbar->GetVisibleToolCount();

            wxPoint upperLeft( m_style->GetLeftMargin(), m_style->GetTopMargin() );
            wxSize visibleSize;
            if( m_ptoolbar->IsVertical() ) {
                int noTools = m_ptoolbar->GetMaxRows();
                if( noTools > toolCount )
                    noTools = toolCount;
                visibleSize.x = m_ptoolbar->GetLineCount() * ( tool_size.x + m_style->GetTopMargin() );
                visibleSize.y = noTools * ( tool_size.y + m_style->GetToolSeparation() );
                visibleSize.x -= m_style->GetTopMargin();
                visibleSize.y -= m_style->GetToolSeparation();
            } else {
                    int noTools = m_ptoolbar->GetMaxCols();
                    if( noTools > toolCount )
                        noTools = toolCount;
                visibleSize.x = noTools * ( tool_size.x + m_style->GetToolSeparation() );
                visibleSize.y = m_ptoolbar->GetLineCount() * ( tool_size.y + m_style->GetTopMargin() );
                visibleSize.x -= m_style->GetToolSeparation();
                visibleSize.y -= m_style->GetTopMargin();
            }

            wxBitmap shape( visibleSize.x + tool_size.x, visibleSize.y + tool_size.y);          // + fluff
            wxMemoryDC sdc( shape );
            sdc.SetBackground( *wxWHITE_BRUSH );
            sdc.SetBrush( *wxBLACK_BRUSH );
            sdc.SetPen( *wxBLACK_PEN );
            sdc.Clear();

            if(b_overlap) {
                int lines = m_ptoolbar->GetLineCount();
                for( int i = 1; i <= lines; i++ ) {
                    if( m_ptoolbar->IsVertical() ) {
                        wxSize barsize( tool_size.x, visibleSize.y );
                        if( i == lines && i > 1 ) {
                            int toolsInLastLine = toolCount % m_ptoolbar->GetMaxRows();
                            if( toolsInLastLine == 0 ) toolsInLastLine = m_ptoolbar->GetMaxRows();
                            int emptySpace = ( m_ptoolbar->GetMaxRows() - toolsInLastLine );
                            barsize.y -= emptySpace
                            * ( tool_size.y + m_style->GetToolSeparation() );
                        }
                        if( i == lines ) {
                            // Also do grabber here, since it is to the right of the last line.
                            wxRect grabMask( upperLeft, barsize );
                            grabMask.width += m_style->GetIcon( _T("grabber") ).GetWidth();
                            grabMask.height = m_style->GetIcon( _T("grabber") ).GetHeight();
                            sdc.DrawRoundedRectangle( grabMask, m_style->GetToolbarCornerRadius() );
                        }
                        sdc.DrawRoundedRectangle( upperLeft, barsize,
                                m_style->GetToolbarCornerRadius() );
                        upperLeft.x += m_style->GetTopMargin() + tool_size.x;
                    } else {
                        wxSize barsize( visibleSize.x, tool_size.y );

                        if( i == 1 ) {
                            barsize.x += m_style->GetIcon( _T("grabber") ).GetWidth();
                        }
                        if( i == lines && i > 1 ) {
                            int toolsInLastLine = toolCount % m_ptoolbar->GetMaxCols();
                            if( toolsInLastLine == 0 ) toolsInLastLine = m_ptoolbar->GetMaxCols();
                            int emptySpace = ( m_ptoolbar->GetMaxCols() - toolsInLastLine );
                            barsize.x -= emptySpace * ( tool_size.x + m_style->GetToolSeparation() );
                        }

                        sdc.DrawRoundedRectangle( upperLeft, barsize,
                                m_style->GetToolbarCornerRadius() );
                        upperLeft.y += m_style->GetTopMargin() + tool_size.y;
                    }
                }
            } //b_overlap
            else {
                for( wxToolBarToolsList::compatibility_iterator node = m_ptoolbar->m_tools.GetFirst(); node; node = node->GetNext() ) {
                    wxToolBarToolBase *tool = node->GetData();
                    ocpnToolBarTool *tools = (ocpnToolBarTool *) tool;
                    wxRect toolRect = tools->trect;

                    sdc.DrawRoundedRectangle( tools->m_x, tools->m_y, tool_size.x, tool_size.y,
                                              m_style->GetToolbarCornerRadius() );
                }
            }

#ifndef __WXQT__
            if(shape.GetWidth() && shape.GetHeight())
                SetShape( wxRegion( shape, *wxWHITE, 10 ) );
#endif
        }
    }
}

void ocpnFloatingToolbarDialog::OnToolLeftClick( wxCommandEvent& event )
{
    // First see if it was actually the context menu that was clicked.


    if( m_FloatingToolbarConfigMenu &&( event.GetId() >= ID_PLUGIN_BASE + 100 )) {

        int itemId = event.GetId() - ID_PLUGIN_BASE - 100;
        wxMenuItem *item = m_FloatingToolbarConfigMenu->FindItem( event.GetId() );

        if(item){
            bool toolIsChecked = item->IsChecked();

            if( toolIsChecked ) {
                g_toolbarConfig.SetChar( itemId, _T('X') );
            } else {

                if( itemId + ID_ZOOMIN == ID_MOB ) {
                    ToolbarMOBDialog mdlg( this );
                    int dialog_ret = mdlg.ShowModal();
                    int answer = mdlg.GetSelection();

                    if( answer == 0 || answer == 1 || dialog_ret == wxID_CANCEL ) {
                        m_FloatingToolbarConfigMenu->FindItem( event.GetId() )->Check( true );
                        if( answer == 1 && dialog_ret == wxID_OK ) {
                            g_bPermanentMOBIcon = true;
                            delete m_FloatingToolbarConfigMenu;
                            m_FloatingToolbarConfigMenu = new wxMenu();
                            toolbarConfigChanged = true;
                        }
                        return;
                    }
                }

                if( m_ptoolbar->GetVisibleToolCount() == 1 ) {
                    OCPNMessageBox( this,
                            _("You can't hide the last tool from the toolbar\nas this would make it inaccessible."),
                            _("OpenCPN Alert"), wxOK );
                    m_FloatingToolbarConfigMenu->FindItem( event.GetId() )->Check( true );
                    return;
                }

                g_toolbarConfig.SetChar( itemId, _T('.') );
            }
        }

        toolbarConfigChanged = true;
        return;
    }

    // No it was a button that was clicked.
    // Since Dialog events don't propagate automatically, we send it explicitly
    // (instead of relying on event.Skip()). Send events up the window hierarchy

    m_pparent->GetEventHandler()->AddPendingEvent( event );
#ifndef __WXQT__
    gFrame->Raise();
#endif
}

ocpnToolBarSimple *ocpnFloatingToolbarDialog::GetToolbar()
{
    if( !m_ptoolbar ) {
        m_ptoolbar = CreateNewToolbar();
    }

    return m_ptoolbar;
}

ocpnToolBarSimple *ocpnFloatingToolbarDialog::CreateNewToolbar()
{
    long winstyle = wxNO_BORDER | wxTB_FLAT;
    winstyle |= m_orient;

    m_ptoolbar = new ocpnToolBarSimple( this, -1, wxPoint( -1, -1 ), wxSize( -1, -1 ), winstyle );

    m_ptoolbar->SetBackgroundColour( GetGlobalColor( _T("GREY2") ) );
    m_ptoolbar->ClearBackground();
    m_ptoolbar->SetToggledBackgroundColour( GetGlobalColor( _T("GREY1") ) );
    m_ptoolbar->SetColorScheme( m_cs );

    return m_ptoolbar;
}

void ocpnFloatingToolbarDialog::DestroyToolBar()
{
    if( m_ptoolbar ) {
        m_ptoolbar->ClearTools();
        delete m_ptoolbar;                  //->Destroy();
        m_ptoolbar = NULL;
    }

    if(!m_destroyTimer.IsRunning()){
        m_destroyGrabber = m_pRecoverwin;
        m_pRecoverwin = NULL;
        m_destroyTimer.Start( 5, wxTIMER_ONE_SHOT );           //  Destor the unneeded recovery grabber
    }

}

#endif



