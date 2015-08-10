/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Toolbar
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
 **************************************************************************/

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif

#include <vector>

#include "ocpn_types.h"
#include "navutil.h"
#include "styles.h"
#include "toolbar.h"
#include "chart1.h"
#include "pluginmanager.h"
#include "FontMgr.h"

extern ocpnFloatingToolbarDialog* g_FloatingToolbarDialog;
extern bool                       g_bTransparentToolbar;
extern bool                       g_bTransparentToolbarInOpenGLOK;
extern ChartCanvas*               cc1;
extern bool                       g_bopengl;
extern ocpnToolBarSimple*         g_toolbar;
extern ocpnStyle::StyleManager*   g_StyleManager;
extern MyFrame*                   gFrame;
extern PlugInManager*             g_pi_manager;
extern wxMenu*                    g_FloatingToolbarConfigMenu;
extern wxString                   g_toolbarConfig;
extern bool                       g_bPermanentMOBIcon;
extern bool                       g_btouch;
extern bool                       g_bsmoothpanzoom;

//----------------------------------------------------------------------------
// GrabberWindow Implementation
//----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(GrabberWin, wxPanel) EVT_MOUSE_EVENTS ( GrabberWin::MouseEvent )
EVT_PAINT ( GrabberWin::OnPaint )
END_EVENT_TABLE()

GrabberWin::GrabberWin( wxWindow *parent, ocpnFloatingToolbarDialog *toolbar, float scale_factor, wxString icon_name, wxPoint position ):
    wxPanel( parent, wxID_ANY, position, wxDefaultSize, wxNO_BORDER )
{
    m_icon_name = icon_name;
    m_style = g_StyleManager->GetCurrentStyle();
    wxBitmap bitmap = m_style->GetIcon( icon_name );
    if(scale_factor > 1.0f){
        int new_width = bitmap.GetWidth() * scale_factor;
        int new_height = bitmap.GetHeight() * scale_factor;
        wxImage scaled_image = bitmap.ConvertToImage();
        m_bitmap = wxBitmap(scaled_image.Scale(new_width, new_height, wxIMAGE_QUALITY_HIGH));
    }
    else
        m_bitmap = bitmap;

    SetSize( wxSize( m_bitmap.GetWidth(), m_bitmap.GetHeight() ) );
    SetMinSize( wxSize( m_bitmap.GetWidth(), m_bitmap.GetHeight() ) );

    m_bLeftDown = false;
    m_bRightDown = false;
    m_scale_factor = scale_factor;
    m_ptoolbar = toolbar;
    m_dragging = false;
    Hide();
    
}


void GrabberWin::OnPaint( wxPaintEvent& event )
{
    wxPaintDC dc( this );
    
    wxColour back_color = GetGlobalColor( _T("GREY2") );
    SetBackgroundColour( back_color );
    ClearBackground();
    
    dc.DrawBitmap( m_bitmap, 0, 0, true );
}

void GrabberWin::SetColorScheme( ColorScheme cs )
{
    wxColour back_color = GetGlobalColor( _T("GREY2") );

    SetBackgroundColour( back_color );
    ClearBackground();

    wxBitmap bitmap = m_style->GetIcon( m_icon_name) ;
    if(m_scale_factor > 1.0f){
        int new_width = bitmap.GetWidth() * m_scale_factor;
        int new_height = bitmap.GetHeight() * m_scale_factor;
        wxImage scaled_image = bitmap.ConvertToImage();
        m_bitmap = wxBitmap(scaled_image.Scale(new_width, new_height, wxIMAGE_QUALITY_HIGH));
    }
    else
        m_bitmap = bitmap;
}

void GrabberWin::MouseEvent( wxMouseEvent& event )
{
    static wxPoint s_gspt;
    int x, y;

    event.GetPosition( &x, &y );

    wxPoint spt = ClientToScreen( wxPoint( x, y ) );
    if( event.LeftDown() ) {
        s_gspt = spt;
    }
    
    
#ifndef __WXQT__

    if( event.LeftDown() ) {
        CaptureMouse();
    }

    if( event.LeftUp() ) {
        if( HasCapture() ) ReleaseMouse();
    }

#endif


    if( event.RightDown() ){
        if(m_ptoolbar){
            m_dragging = true;
            
            if( !m_ptoolbar->m_bnavgrabber ){
                m_ptoolbar->m_bnavgrabber = true;
                m_ptoolbar->SetGrabber(_T("4WayMove") );
            }
            else{
                m_ptoolbar->m_bnavgrabber = false;
                m_ptoolbar->SetGrabber(_T("grabber_hi") );
            }
                
        }
    }
    


    if( event.Dragging() ) {
        if(m_ptoolbar && m_ptoolbar->IsShown() /*&& m_ptoolbar->m_bnavgrabber*/){
            wxPoint par_pos_old = m_ptoolbar->GetPosition();

            wxPoint par_pos = par_pos_old;
            par_pos.x += spt.x - s_gspt.x;
            par_pos.y += spt.y - s_gspt.y;

            m_ptoolbar->MoveDialogInScreenCoords( par_pos, par_pos_old );

            s_gspt = spt;
            m_dragging = true;
        }

    }

    if( event.LeftUp() ) {
        if(m_ptoolbar){
            if(m_ptoolbar->m_bnavgrabber){
                if(!m_dragging)
                    m_ptoolbar->ToggleOrientation();
            }
            else if(!m_dragging){
                if(m_ptoolbar->m_bsubmerged){
                    m_ptoolbar->SurfaceFromGrabber();
                }
                else{
                    m_ptoolbar->SubmergeToGrabber();
                 }
            }
        }
        m_dragging = false;
    }
    
    
#ifndef __OCPN__ANDROID__
    gFrame->Raise();
#endif
    
}

class ocpnToolBarTool: public wxToolBarToolBase {
public:
    ocpnToolBarTool( ocpnToolBarSimple *tbar, int id, const wxString& label,
            const wxBitmap& bmpNormal, const wxBitmap& bmpRollover, wxItemKind kind,
            wxObject *clientData, const wxString& shortHelp, const wxString& longHelp ) :
            wxToolBarToolBase( (wxToolBarBase*) tbar, id, label, bmpNormal, bmpRollover, kind,
                    clientData, shortHelp, longHelp )
    {
        m_enabled = true;
        m_toggled = false;
        rollover = false;
        bitmapOK = false;
        m_btooltip_hiviz = false;

        toolname = g_pi_manager->GetToolOwnerCommonName( id );
        if( toolname == _T("") ) {
            isPluginTool = false;
            toolname = label;
            iconName = label;
        } else {
            isPluginTool = true;
            pluginNormalIcon = &bmpNormal;
            pluginRolloverIcon = &bmpRollover;
        }
    }

    void SetSize( const wxSize& size )
    {
        m_width = size.x;
        m_height = size.y;
    }

    wxCoord GetWidth() const
    {
        return m_width;
    }

    wxCoord GetHeight() const
    {
        return m_height;
    }

    wxString GetToolname()
    {
        return toolname;
    }

    void SetIconName(wxString name)
    {
        iconName = name;
    }
    wxString GetIconName()
    {
        return iconName;
    }

    void SetTooltipHiviz( bool enable){ m_btooltip_hiviz = enable; }

    wxCoord m_x;
    wxCoord m_y;
    wxCoord m_width;
    wxCoord m_height;
    wxRect trect;
    wxString toolname;
    wxString iconName;
    const wxBitmap* pluginNormalIcon;
    const wxBitmap* pluginRolloverIcon;
    bool firstInLine;
    bool lastInLine;
    bool rollover;
    bool bitmapOK;
    bool isPluginTool;
    bool b_hilite;
    bool m_btooltip_hiviz;
    wxRect last_rect;
};

//---------------------------------------------------------------------------------------
//          ocpnFloatingToolbarDialog Implementation
//---------------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(ocpnFloatingToolbarDialog, wxDialog)
    EVT_MOUSE_EVENTS ( ocpnFloatingToolbarDialog::MouseEvent )
    EVT_MENU(wxID_ANY, ocpnFloatingToolbarDialog::OnToolLeftClick)
    EVT_TIMER ( FADE_TIMER, ocpnFloatingToolbarDialog::FadeTimerEvent )
    EVT_TIMER ( DESTROY_TIMER, ocpnFloatingToolbarDialog::DestroyTimerEvent )
    EVT_WINDOW_CREATE(ocpnFloatingToolbarDialog::OnWindowCreate)
END_EVENT_TABLE()

ocpnFloatingToolbarDialog::ocpnFloatingToolbarDialog( wxWindow *parent, wxPoint position,
                                                      long orient, float size_factor )
{
    m_pparent = parent;
    long wstyle = wxNO_BORDER | wxFRAME_NO_TASKBAR;
#ifndef __WXMAC__
    wstyle |= wxFRAME_SHAPED;
#endif

    m_ptoolbar = NULL;

#ifdef __WXOSX__
    wstyle |= wxSTAY_ON_TOP;
#endif
    wxDialog::Create( parent, -1, _T("ocpnToolbarDialog"), wxPoint( -1, -1 ), wxSize( -1, -1 ),
            wstyle );

    m_opacity = 255;

    m_pGrabberwin = new GrabberWin( this, this, size_factor, _T("grabber_hi") );
    m_pGrabberwin->Show();
    
    m_pRecoverwin = NULL;
    m_position = position;
    m_orient = orient;
    m_sizefactor = size_factor;
    
    m_bAutoHideToolbar = false;
    m_nAutoHideToolbar = 5;
    

    m_style = g_StyleManager->GetCurrentStyle();

// A top-level sizer
    m_topSizer = new wxBoxSizer( wxHORIZONTAL );
    SetSizer( m_topSizer );

    //    Set initial "Dock" parameters
    m_dock_x = 0;
    m_dock_y = 0;
    m_block = false;

    m_marginsInvisible = m_style->marginsInvisible;

//    if(m_sizefactor > 1.0 )
 //       m_marginsInvisible = true;

    m_bnavgrabber = false;    
    
    Hide();

    m_bsubmerged = false;
    
    m_fade_timer.SetOwner( this, FADE_TIMER );
    if( g_bTransparentToolbar )
        m_fade_timer.Start( 5000 );
    
    if( m_bAutoHideToolbar && (m_nAutoHideToolbar > 0))
        m_fade_timer.Start( m_nAutoHideToolbar * 1000 );
    
    m_destroyTimer.SetOwner( this, DESTROY_TIMER );
}

ocpnFloatingToolbarDialog::~ocpnFloatingToolbarDialog()
{
    DestroyToolBar();
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


    
void ocpnFloatingToolbarDialog::SetColorScheme( ColorScheme cs )
{
    m_cs = cs;

    wxColour back_color = GetGlobalColor( _T("GREY2") );

    //  Set background
    SetBackgroundColour( back_color );
    ClearBackground();

    if( m_ptoolbar ) {
        wxColour back_color = GetGlobalColor( _T("GREY2") );

        //  Set background
        m_ptoolbar->SetBackgroundColour( back_color );
        m_ptoolbar->ClearBackground();

        m_ptoolbar->SetToggledBackgroundColour( GetGlobalColor( _T("GREY1") ) );

        m_ptoolbar->SetColorScheme( cs );
        m_ptoolbar->Refresh( true );
    }

    if( m_pGrabberwin ) m_pGrabberwin->SetColorScheme( cs );

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
        m_pRecoverwin->Show();
        m_pRecoverwin->Raise();
    }
    else {
        m_bsubmerged = false;
        #ifndef __WXOSX__
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
    
    m_destroyGrabber = m_pRecoverwin;
    m_destroyTimer.Start( 5, wxTIMER_ONE_SHOT );           //  Destor the unneeded recovery grabber
    
}

void ocpnFloatingToolbarDialog::DestroyTimerEvent( wxTimerEvent& event )
{
    delete m_destroyGrabber;
    m_destroyGrabber = NULL;
    m_pRecoverwin = NULL;
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

#ifndef __OCPN__ANDROID__
            if(shape.GetWidth() && shape.GetHeight())
                SetShape( wxRegion( shape, *wxWHITE, 10 ) );
#endif
        }
    }
}

void ocpnFloatingToolbarDialog::OnToolLeftClick( wxCommandEvent& event )
{
    // First see if it was actually the context menu that was clicked.

    if( event.GetId() >= ID_PLUGIN_BASE + 100 ) {

        int itemId = event.GetId() - ID_PLUGIN_BASE - 100;
        wxMenuItem *item = g_FloatingToolbarConfigMenu->FindItem( event.GetId() );

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
                        g_FloatingToolbarConfigMenu->FindItem( event.GetId() )->Check( true );
                        if( answer == 1 && dialog_ret == wxID_OK ) {
                            g_bPermanentMOBIcon = true;
                            delete g_FloatingToolbarConfigMenu;
                            g_FloatingToolbarConfigMenu = new wxMenu();
                            toolbarConfigChanged = true;
                        }
                        return;
                    }
                }

                if( m_ptoolbar->GetVisibleToolCount() == 1 ) {
                    OCPNMessageBox( this,
                            _("You can't hide the last tool from the toolbar\nas this would make it inaccessible."),
                            _("OpenCPN Alert"), wxOK );
                    g_FloatingToolbarConfigMenu->FindItem( event.GetId() )->Check( true );
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
        long winstyle = wxNO_BORDER | wxTB_FLAT;
        winstyle |= m_orient;

        m_ptoolbar = new ocpnToolBarSimple( this, -1, wxPoint( -1, -1 ), wxSize( -1, -1 ),
                winstyle );

        m_ptoolbar->SetBackgroundColour( GetGlobalColor( _T("GREY2") ) );
        m_ptoolbar->ClearBackground();
        m_ptoolbar->SetToggledBackgroundColour( GetGlobalColor( _T("GREY1") ) );
        m_ptoolbar->SetColorScheme( m_cs );

        SetGeometry(false, wxRect());
    }

    return m_ptoolbar;
}

void ocpnFloatingToolbarDialog::DestroyToolBar()
{
    if( m_ptoolbar ) {
        m_ptoolbar->ClearTools();
        delete m_ptoolbar;                  //->Destroy();
        m_ptoolbar = NULL;
    }
 
    m_destroyGrabber = m_pRecoverwin;
    m_destroyTimer.Start( 5, wxTIMER_ONE_SHOT );           //  Destor the unneeded recovery grabber
    
}

//----------------------------------------------------------------------------
// Toolbar Tooltip Popup Window Definition
//----------------------------------------------------------------------------
class ToolTipWin: public wxDialog {
public:
    ToolTipWin( wxWindow *parent );
    ~ToolTipWin();

    void OnPaint( wxPaintEvent& event );

    void SetColorScheme( ColorScheme cs );
    void SetString( wxString &s )
    {
        m_string = s;
    }
    void SetPosition( wxPoint pt )
    {
        m_position = pt;
    }
    void SetBitmap( void );

    void SetHiviz( bool hiviz){ m_hiviz = hiviz; }
    
    wxSize GetRenderedSize( void );
    
private:

    wxString m_string;
    wxSize m_size;
    wxPoint m_position;
    wxBitmap *m_pbm;
    wxColour m_back_color;
    wxColour m_text_color;
    ColorScheme m_cs ;
    bool m_hiviz;

DECLARE_EVENT_TABLE()
};
//-----------------------------------------------------------------------
//
//    Toolbar Tooltip window implementation
//
//-----------------------------------------------------------------------
BEGIN_EVENT_TABLE(ToolTipWin, wxDialog) EVT_PAINT(ToolTipWin::OnPaint)

END_EVENT_TABLE()

// Define a constructor
ToolTipWin::ToolTipWin( wxWindow *parent ) :
        wxDialog( parent, wxID_ANY, _T(""), wxPoint( 0, 0 ), wxSize( 1, 1 ),
                wxNO_BORDER | wxSTAY_ON_TOP )
{
    m_pbm = NULL;

    m_back_color = GetGlobalColor( _T ( "UIBCK" ) );
    m_text_color = GetGlobalColor( _T ( "UITX1" ) );

    SetBackgroundStyle( wxBG_STYLE_CUSTOM );
    SetBackgroundColour( m_back_color );
    m_cs = GLOBAL_COLOR_SCHEME_RGB;

    Hide();
}

ToolTipWin::~ToolTipWin()
{
    delete m_pbm;
}

void ToolTipWin::SetColorScheme( ColorScheme cs )
{
    m_back_color = GetGlobalColor( _T ( "UIBCK" ) );
    m_text_color = FontMgr::Get().GetFontColor( _("ToolTips") );

    m_cs = cs;
}

wxSize ToolTipWin::GetRenderedSize( void )
{
    int h, w;
    wxSize sz;

    wxClientDC cdc( GetParent() );

    wxFont *plabelFont = FontMgr::Get().GetFont( _("ToolTips") );
    cdc.GetTextExtent( m_string, &w, &h, NULL, NULL, plabelFont );

    sz.x = w + 8;
    sz.y = h + 4;
    
    return sz;

}

void ToolTipWin::SetBitmap()
{
    int h, w;

    wxClientDC cdc( GetParent() );

    wxFont *plabelFont = FontMgr::Get().GetFont( _("ToolTips") );
    cdc.GetTextExtent( m_string, &w, &h, NULL, NULL, plabelFont );

    m_size.x = w + 8;
    m_size.y = h + 4;

    wxMemoryDC mdc;

    delete m_pbm;
    m_pbm = new wxBitmap( m_size.x, m_size.y, -1 );
    mdc.SelectObject( *m_pbm );

    wxPen pborder( m_text_color );
    wxBrush bback( m_back_color );
    mdc.SetPen( pborder );
    mdc.SetBrush( bback );

    if(m_hiviz){
        if((m_cs == GLOBAL_COLOR_SCHEME_DUSK) || (m_cs == GLOBAL_COLOR_SCHEME_NIGHT)){
            wxBrush hv_back( wxColour(200,200,200));
            mdc.SetBrush( hv_back );
        }
    }
    mdc.DrawRectangle( 0, 0, m_size.x, m_size.y );

    //    Draw the text
    mdc.SetFont( *plabelFont );
    mdc.SetTextForeground( m_text_color );
    mdc.SetTextBackground( m_back_color );

    mdc.DrawText( m_string, 4, 2 );

    SetSize( m_position.x, m_position.y, m_size.x, m_size.y );

}

void ToolTipWin::OnPaint( wxPaintEvent& event )
{
    int width, height;
    GetClientSize( &width, &height );
    wxPaintDC dc( this );

    if( m_string.Len() ) {
        wxMemoryDC mdc;
        mdc.SelectObject( *m_pbm );
        dc.Blit( 0, 0, width, height, &mdc, 0, 0 );
    }
}


// ----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(ocpnToolBarSimple, wxControl) EVT_SIZE(ocpnToolBarSimple::OnSize)
EVT_PAINT(ocpnToolBarSimple::OnPaint)
EVT_KILL_FOCUS(ocpnToolBarSimple::OnKillFocus)
EVT_MOUSE_EVENTS(ocpnToolBarSimple::OnMouseEvent)
EVT_TIMER(TOOLTIPON_TIMER, ocpnToolBarSimple::OnToolTipTimerEvent)
EVT_TIMER(TOOLTIPOFF_TIMER, ocpnToolBarSimple::OnToolTipOffTimerEvent)

END_EVENT_TABLE()

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// tool bar tools creation
// ----------------------------------------------------------------------------

wxToolBarToolBase *ocpnToolBarSimple::CreateTool( int id, const wxString& label,
        const wxBitmap& bmpNormal, const wxBitmap& bmpDisabled, wxItemKind kind,
        wxObject *clientData, const wxString& shortHelp, const wxString& longHelp )
{
    return new ocpnToolBarTool( this, id, label, bmpNormal, bmpDisabled, kind, clientData,
            shortHelp, longHelp );
}

// ----------------------------------------------------------------------------
// ocpnToolBarSimple creation
// ----------------------------------------------------------------------------

void ocpnToolBarSimple::Init()
{
    m_currentRowsOrColumns = 0;

    m_lastX = m_lastY = 0;

    m_maxWidth = m_maxHeight = 0;

    m_pressedTool = m_currentTool = -1;

    m_xPos = m_yPos = wxDefaultCoord;

    m_style = g_StyleManager->GetCurrentStyle();

    m_defaultWidth = 16;
    m_defaultHeight = 15;

    m_toggle_bg_color = wxSystemSettings::GetColour( wxSYS_COLOUR_3DFACE );
    m_toolOutlineColour.Set( _T("BLACK") );
    m_pToolTipWin = NULL;
    m_last_ro_tool = NULL;

    m_btoolbar_is_zooming = false;
    m_sizefactor = 1.0f;

    m_last_plugin_down_id = -1;
    
    EnableTooltips();
}

wxToolBarToolBase *ocpnToolBarSimple::DoAddTool( int id, const wxString& label,
        const wxBitmap& bitmap, const wxBitmap& bmpDisabled, wxItemKind kind,
        const wxString& shortHelp, const wxString& longHelp, wxObject *clientData, wxCoord xPos,
        wxCoord yPos )
{
    // rememeber the position for DoInsertTool()
    m_xPos = xPos;
    m_yPos = yPos;

    InvalidateBestSize();
    return InsertTool( GetToolsCount(), id, label, bitmap, bmpDisabled, kind, shortHelp, longHelp,
            clientData );

}

///

wxToolBarToolBase *ocpnToolBarSimple::AddTool( int toolid, const wxString& label,
        const wxBitmap& bitmap, const wxBitmap& bmpDisabled, wxItemKind kind,
        const wxString& shortHelp, const wxString& longHelp, wxObject *data )
{
    InvalidateBestSize();
    ocpnToolBarTool* tool = (ocpnToolBarTool*)InsertTool( GetToolsCount(), toolid, label, bitmap, bmpDisabled, kind,
            shortHelp, longHelp, data );
    return tool;
}

wxToolBarToolBase *ocpnToolBarSimple::InsertTool( size_t pos, int id, const wxString& label,
        const wxBitmap& bitmap, const wxBitmap& bmpDisabled, wxItemKind kind,
        const wxString& shortHelp, const wxString& longHelp, wxObject *clientData )
{
    wxCHECK_MSG( pos <= GetToolsCount(), (wxToolBarToolBase *)NULL,
            _T("invalid position in wxToolBar::InsertTool()") );

    wxToolBarToolBase *tool = CreateTool( id, label, bitmap, bmpDisabled, kind, clientData,
            shortHelp, longHelp );

    if( !InsertTool( pos, tool ) ) {
        delete tool;

        return NULL;
    }

    return tool;
}

wxToolBarToolBase *ocpnToolBarSimple::InsertTool( size_t pos, wxToolBarToolBase *tool )
{
    wxCHECK_MSG( pos <= GetToolsCount(), (wxToolBarToolBase *)NULL,
            _T("invalid position in wxToolBar::InsertTool()") );

    if( !tool || !DoInsertTool( pos, tool ) ) {
        return NULL;
    }

    m_tools.Insert( pos, tool );

    return tool;
}

bool ocpnToolBarSimple::DoInsertTool( size_t WXUNUSED(pos), wxToolBarToolBase *toolBase )
{
    ocpnToolBarTool *tool = (ocpnToolBarTool *) toolBase;

    // Check if the plugin is inserting same-named tools. Make sure they have different names,
    // otherwise the style manager cannot differentiate between them.
    if( tool->isPluginTool ) {
        for( unsigned int i=0; i<GetToolsCount(); i++ ) {
            if( tool->GetToolname() == ((ocpnToolBarTool *)m_tools.Item(i)->GetData())->GetToolname()) {
                tool->toolname << _T("1");
            }
        }
    }

    tool->m_x = m_xPos;
    if( tool->m_x == wxDefaultCoord ) tool->m_x = m_style->GetLeftMargin();

    tool->m_y = m_yPos;
    if( tool->m_y == wxDefaultCoord ) tool->m_y = m_style->GetTopMargin();

    if( tool->IsButton() ) {
        tool->SetSize( GetToolSize() );

        // Calculate reasonable max size in case Layout() not called
        if( ( tool->m_x + tool->GetNormalBitmap().GetWidth() + m_style->GetLeftMargin() )
                > m_maxWidth ) m_maxWidth = (wxCoord) ( ( tool->m_x + tool->GetWidth()
                + m_style->GetLeftMargin() ) );

        if( ( tool->m_y + tool->GetNormalBitmap().GetHeight() + m_style->GetTopMargin() )
                > m_maxHeight ) m_maxHeight = (wxCoord) ( ( tool->m_y + tool->GetHeight()
                + m_style->GetTopMargin() ) );
    }

    else
        if( tool->IsControl() ) {
            tool->SetSize( tool->GetControl()->GetSize() );
        }

    tool->b_hilite = false;

    return true;
}

bool ocpnToolBarSimple::DoDeleteTool( size_t WXUNUSED(pos), wxToolBarToolBase *tool )
{
    // VZ: didn't test whether it works, but why not...
    tool->Detach();

    if( m_last_ro_tool == tool ) m_last_ro_tool = NULL;

    Refresh( false );

    return true;
}

bool ocpnToolBarSimple::Create( wxWindow *parent, wxWindowID id, const wxPoint& pos,
        const wxSize& size, long style, const wxString& name )
{
    if( !wxWindow::Create( parent, id, pos, size, style, name ) ) return false;

    // Set it to grey (or other 3D face colour)
    SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DFACE ) );

    if( GetWindowStyleFlag() & wxTB_VERTICAL ) {
        m_lastX = 7;
        m_lastY = 3;

        m_maxRows = 32000;      // a lot
        m_maxCols = 1;
    } else {
        m_lastX = 3;
        m_lastY = 7;

        m_maxRows = 1;
        m_maxCols = 32000;      // a lot
    }

    SetCursor( *wxSTANDARD_CURSOR );

    m_tooltip_timer.SetOwner( this, TOOLTIPON_TIMER );
    m_tooltipoff_timer.SetOwner( this, TOOLTIPOFF_TIMER );
    m_tooltip_off = 3000;

    return true;
}

ocpnToolBarSimple::~ocpnToolBarSimple()
{
    if( m_pToolTipWin ) {
        m_pToolTipWin->Destroy();
        m_pToolTipWin = NULL;
    }

}

void ocpnToolBarSimple::KillTooltip()
{
    m_btooltip_show = false;

    if( m_pToolTipWin ) {
        m_pToolTipWin->Hide();
        m_pToolTipWin->Destroy();
        m_pToolTipWin = NULL;
    }
    m_tooltip_timer.Stop();

    if( m_last_ro_tool ) {
        if( m_last_ro_tool->IsEnabled() ) {
            if( m_last_ro_tool->IsToggled() ) {
                m_last_ro_tool->SetNormalBitmap( m_style->GetToolIcon( m_last_ro_tool->GetToolname(), TOOLICON_TOGGLED ) );
            }
            else {
                m_last_ro_tool->SetNormalBitmap( m_style->GetToolIcon( m_last_ro_tool->GetToolname(), TOOLICON_NORMAL ) );
            }
        }
    }
}

void ocpnToolBarSimple::HideTooltip()
{
    if( m_pToolTipWin ) {
        m_pToolTipWin->Hide();
    }
}

void ocpnToolBarSimple::SetColorScheme( ColorScheme cs )
{
    if( m_pToolTipWin ) {
        m_pToolTipWin->Destroy();
        m_pToolTipWin = NULL;
    }

    m_toolOutlineColour = GetGlobalColor( _T("UIBDR") );

    m_currentColorScheme = cs;
}

bool ocpnToolBarSimple::Realize()
{
    m_currentRowsOrColumns = 0;
    m_LineCount = 1;
    m_lastX = m_style->GetLeftMargin();
    m_lastY = m_style->GetTopMargin();
    m_maxWidth = 0;
    m_maxHeight = 0;

    if( IsVertical() ) m_style->SetOrientation( wxTB_VERTICAL );
    else
        m_style->SetOrientation( wxTB_HORIZONTAL );

    wxSize toolSize = wxSize(-1, -1);
    int separatorSize = m_style->GetToolSeparation();

    ocpnToolBarTool *lastTool = NULL;
    bool firstNode = true;
    wxToolBarToolsList::compatibility_iterator node = m_tools.GetFirst();

    while( node ) {
        ocpnToolBarTool *tool = (ocpnToolBarTool *) node->GetData();

        // Set the tool size to be the size of the first non-separator tool, usually the first one
        if(toolSize.x == -1){
            if( !tool->IsSeparator() ){
                toolSize.x = tool->m_width;
                toolSize.y = tool->m_height;
            }
        }

        tool->firstInLine = firstNode;
        tool->lastInLine = false;
        firstNode = false;

        tool->last_rect.width = 0;              // mark it invalid
        
        if( tool->IsSeparator() ) {
            if( GetWindowStyleFlag() & wxTB_HORIZONTAL ) {
                if( m_currentRowsOrColumns >= m_maxCols ) m_lastY += separatorSize;
                else
                    m_lastX += separatorSize;
            } else {
                if( m_currentRowsOrColumns >= m_maxRows ) m_lastX += separatorSize;
                else
                    m_lastY += separatorSize;
            }
        } else
            if( tool->IsButton() ) {
                if( !IsVertical() ) {
                    if( m_currentRowsOrColumns >= m_maxCols ) {
                        tool->firstInLine = true;
                        if( lastTool && m_LineCount > 1 ) lastTool->lastInLine = true;
                        m_LineCount++;
                        m_currentRowsOrColumns = 0;
                        m_lastX = m_style->GetLeftMargin();
                        m_lastY += toolSize.y + m_style->GetTopMargin();
                    }
                    tool->m_x = (wxCoord) m_lastX;
                    tool->m_y = (wxCoord) m_lastY;

                    tool->trect = wxRect( tool->m_x, tool->m_y, toolSize.x, toolSize.y );
                    tool->trect.Inflate( m_style->GetToolSeparation() / 2,
                            m_style->GetTopMargin() );

                    m_lastX += toolSize.x + m_style->GetToolSeparation();
                } else {
                    if( m_currentRowsOrColumns >= m_maxRows ) {
                        tool->firstInLine = true;
                        if( lastTool ) lastTool->lastInLine = true;
                        m_LineCount++;
                        m_currentRowsOrColumns = 0;
                        m_lastX += toolSize.x + m_style->GetTopMargin();
                        m_lastY = m_style->GetTopMargin();
                    }
                    tool->m_x = (wxCoord) m_lastX;
                    tool->m_y = (wxCoord) m_lastY;

                    tool->trect = wxRect( tool->m_x, tool->m_y, toolSize.x, toolSize.y );
                    tool->trect.Inflate( m_style->GetToolSeparation() / 2,
                            m_style->GetTopMargin() );

                    m_lastY += toolSize.y + m_style->GetToolSeparation();
                }
                m_currentRowsOrColumns++;
            } else
                if( tool->IsControl() ) {
                    tool->m_x = (wxCoord) ( m_lastX );
                    tool->m_y = (wxCoord) ( m_lastY - ( m_style->GetTopMargin() / 2 ) );

                    tool->trect = wxRect( tool->m_x, tool->m_y, tool->GetWidth(),
                            tool->GetHeight() );
                    tool->trect.Inflate( m_style->GetToolSeparation() / 2,
                            m_style->GetTopMargin() );
                    ;

                    wxSize s = tool->GetControl()->GetSize();
                    m_lastX += s.x + m_style->GetToolSeparation();

                }

        if( m_lastX > m_maxWidth ) m_maxWidth = m_lastX;
        if( m_lastY > m_maxHeight ) m_maxHeight = m_lastY;

        lastTool = tool;
        node = node->GetNext();
    }
    if( lastTool && (m_LineCount > 1 || IsVertical()) )
        lastTool->lastInLine = true;

    if( GetWindowStyleFlag() & wxTB_HORIZONTAL ) m_maxHeight += toolSize.y;
    else
        m_maxWidth += toolSize.x;

    m_maxWidth += m_style->GetRightMargin();
    m_maxHeight += m_style->GetBottomMargin();

    SetSize( m_maxWidth, m_maxHeight );
    SetMinSize( wxSize( m_maxWidth, m_maxHeight ) );

    return true;
}

// ----------------------------------------------------------------------------
// event handlers
// ----------------------------------------------------------------------------

void ocpnToolBarSimple::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
    wxPaintDC dc( this );
    PrepareDC( dc );

    wxRegion ru = GetUpdateRegion();
    wxRect upRect = ru.GetBox();

    static int count = 0;
    // Prevent reentry of OnPaint which would cause wxMemoryDC errors.
    if( count > 0 ) return;
    count++;

    for( wxToolBarToolsList::compatibility_iterator node = m_tools.GetFirst(); node;
            node = node->GetNext() ) {
        wxToolBarToolBase *tool = node->GetData();
        ocpnToolBarTool *tools = (ocpnToolBarTool *) tool;
        wxRect toolRect = tools->trect;

        if( toolRect.Intersects( upRect ) ) {

            if( tool->IsButton() ) {
                DrawTool( dc, tool );
            } else
                if( tool->IsControl() ) {
                    if( tool->GetControl()->IsKindOf( CLASSINFO(wxStaticBitmap) ) ) {
                        wxStaticBitmap *psbm = (wxStaticBitmap *) tool->GetControl();
                        ocpnToolBarTool *toolsimp = (ocpnToolBarTool *) tool;
                        dc.DrawBitmap( psbm->GetBitmap(), toolsimp->m_x, toolsimp->m_y, false );
                    }
                }
        }
    }

    count--;
}

void ocpnToolBarSimple::OnSize( wxSizeEvent& WXUNUSED(event) )
{
    if( GetAutoLayout() ) Layout();
}

void ocpnToolBarSimple::OnKillFocus( wxFocusEvent& WXUNUSED(event) )
{
    OnMouseEnter( m_pressedTool = m_currentTool = -1 );
}

void ocpnToolBarSimple::OnToolTipTimerEvent( wxTimerEvent& event )
{
    if( !gFrame->IsActive() ) return;

    if( m_btooltip_show && IsShown() && m_pToolTipWin && ( !m_pToolTipWin->IsShown() ) ) {
        if( m_last_ro_tool ) {
            wxString s = m_last_ro_tool->GetShortHelp();

            if( s.Len() ) {
                m_pToolTipWin->SetString( s );
                m_pToolTipWin->SetHiviz(m_last_ro_tool->m_btooltip_hiviz);

                wxPoint pos_in_toolbar( m_last_ro_tool->m_x, m_last_ro_tool->m_y );
                pos_in_toolbar.x += m_last_ro_tool->m_width + 2;

                m_pToolTipWin->Move(0,0);       // workaround for gtk autocentre dialog behavior

                wxPoint screenPosition = ClientToScreen( pos_in_toolbar );
                wxPoint framePosition = gFrame->ScreenToClient(screenPosition);
                wxSize tipSize = m_pToolTipWin->GetRenderedSize();
                
                if( (framePosition.x + tipSize.x) > gFrame->GetSize().x)
                    screenPosition.x -= (tipSize.x + m_last_ro_tool->m_width + 4);
                
                m_pToolTipWin->SetPosition( screenPosition );
                m_pToolTipWin->SetBitmap();
                m_pToolTipWin->Show();
                gFrame->Raise();
                if( g_btouch )
                    m_tooltipoff_timer.Start(m_tooltip_off, wxTIMER_ONE_SHOT);
            }
        }
    }
}

void ocpnToolBarSimple::OnToolTipOffTimerEvent( wxTimerEvent& event )
{
    HideTooltip();
}


int s_dragx, s_dragy;

void ocpnToolBarSimple::OnMouseEvent( wxMouseEvent & event )
{
#ifdef __OCPN__ANDROID__
    if(!event.IsButton())
        return;
#endif

    wxCoord x, y;
    event.GetPosition( &x, &y );
    ocpnToolBarTool *tool = (ocpnToolBarTool *) FindToolForPosition( x, y );

#ifndef __OCPN__ANDROID__
    if( event.LeftDown() ) {
        CaptureMouse();
        s_dragx = x;
        s_dragy = y;
    }
    if( event.LeftUp() ) {
        if( HasCapture() ) ReleaseMouse();
    }
#endif

    if( tool && tool->IsButton() && IsShown() ) {

        //    ToolTips
        if( NULL == m_pToolTipWin ) {
            m_pToolTipWin = new ToolTipWin( GetParent() );
            m_pToolTipWin->SetColorScheme( m_currentColorScheme );
            m_pToolTipWin->Hide();
        }

        if( tool != m_last_ro_tool ) m_pToolTipWin->Hide();

#ifndef __OCPN__ANDROID__
        if( !m_pToolTipWin->IsShown() ) {
            m_tooltip_timer.Start( m_one_shot, wxTIMER_ONE_SHOT );
        }
#endif

        //    Tool Rollover highlighting
        if(!g_btouch){
            if( tool != m_last_ro_tool ) {
                if( tool->IsEnabled() ) {
                    tool->rollover = true;
                    tool->bitmapOK = false;
                }
                if( m_last_ro_tool ) {
                    if( m_last_ro_tool->IsEnabled() ) {
                        m_last_ro_tool->rollover = false;
                        m_last_ro_tool->bitmapOK = false;
                    }
                }
                m_last_ro_tool = tool;
                if(g_toolbar)
                    g_toolbar->Refresh( false );
            }
        }
    } else {
        //    Tooltips
        if( m_pToolTipWin && m_pToolTipWin->IsShown() ) m_pToolTipWin->Hide();

        //    Remove Highlighting
        if( m_last_ro_tool ) {
            if( m_last_ro_tool->IsEnabled() ) {
                m_last_ro_tool->rollover = false;
                m_last_ro_tool->bitmapOK = false;
            }
            g_toolbar->Refresh( false );
        }
    }

    m_last_ro_tool = tool;

    // allow smooth zooming while toolbutton is held down
    if(g_bsmoothpanzoom && !g_btouch) {
        if(event.LeftUp() && m_btoolbar_is_zooming) {
            cc1->StopMovement();
            m_btoolbar_is_zooming = false;
            return;
        }

        if( event.LeftDown() && tool &&
            (tool->GetId() == ID_ZOOMIN || tool->GetId() == ID_ZOOMOUT) ) {
            cc1->ZoomCanvas( tool->GetId() == ID_ZOOMIN ? 2.0 : .5, false, false );
            m_btoolbar_is_zooming = true;
            return;
        }
    }

    if( !tool ) {
        if( m_currentTool > -1 ) {
            if( event.LeftIsDown() ) SpringUpButton( m_currentTool );
            m_currentTool = -1;
            OnMouseEnter( -1 );
        }

        wxMouseEvent *pev = (wxMouseEvent *) event.Clone();
        GetParent()->GetEventHandler()->AddPendingEvent( *pev );
        wxDELETE( pev );

        return;
    }

    if( !event.IsButton() ) {
        if( tool->GetId() != m_currentTool ) {
            // If the left button is kept down and moved over buttons,
            // press those buttons.
            if( event.LeftIsDown() && tool->IsEnabled() ) {
                SpringUpButton( m_currentTool );

                if( tool->CanBeToggled() ) {
                    tool->Toggle();
                }

                DrawTool( tool );
            }

            m_currentTool = tool->GetId();
            OnMouseEnter( m_currentTool );
        }

        wxMouseEvent *pev = (wxMouseEvent *) event.Clone();
        GetParent()->GetEventHandler()->AddPendingEvent( *pev );
        wxDELETE( pev );

        return;
    }

    // Left button pressed.
    if( event.LeftDown() && tool->IsEnabled() ) {
        if( tool->CanBeToggled() ) {
            tool->Toggle();
            tool->bitmapOK = false;
            
        }

        DrawTool( tool );

        //        Look for PlugIn tools
        //        If found, make the callback.
        if( g_pi_manager ) {
            ArrayOfPlugInToolbarTools tool_array = g_pi_manager->GetPluginToolbarToolArray();
            for( unsigned int i = 0; i < tool_array.GetCount(); i++ ) {
                PlugInToolbarToolContainer *pttc = tool_array.Item( i );
                if( tool->GetId() == pttc->id ) {
                    opencpn_plugin_113 *ppi = dynamic_cast<opencpn_plugin_113 *>(pttc->m_pplugin);
                    if( ppi ) {
                        ppi->OnToolbarToolDownCallback( pttc->id );
                        m_last_plugin_down_id = pttc->id;
                    }
                }
            }
        }
    } else
        if( event.RightDown() ) {
            OnRightClick( tool->GetId(), x, y );
        }

    // Left Button Released.  Only this action confirms selection.
    // If the button is enabled and it is not a toggle tool and it is
    // in the pressed state, then raise the button and call OnLeftClick.
    //
    if( event.LeftUp() && tool->IsEnabled() ) {
        // Pass the OnLeftClick event to tool
        if( !OnLeftClick( tool->GetId(), tool->IsToggled() ) && tool->CanBeToggled() ) {
            // If it was a toggle, and OnLeftClick says No Toggle allowed,
            // then change it back
            tool->Toggle();
            tool->bitmapOK = false;
        }

        DoPluginToolUp();
    }

    wxMouseEvent *pev = (wxMouseEvent *) event.Clone();
    GetParent()->GetEventHandler()->AddPendingEvent( *pev );
    wxDELETE( pev );
    event.Skip();
}

// ----------------------------------------------------------------------------
// drawing
// ----------------------------------------------------------------------------

void ocpnToolBarSimple::DrawTool( wxToolBarToolBase *tool )
{
    wxClientDC dc( this );
    DrawTool( dc, tool );
}

// NB! The current DrawTool code assumes that plugin tools are never disabled
// when they are present on the toolbar, since disabled plugins are removed.

void ocpnToolBarSimple::DrawTool( wxDC& dc, wxToolBarToolBase *toolBase )
{
    ocpnToolBarTool *tool = (ocpnToolBarTool *) toolBase;
    PrepareDC( dc );

    wxPoint drawAt( tool->m_x, tool->m_y );
    wxBitmap bmp;

    if( tool->bitmapOK ) {
        if( tool->IsEnabled() ) {
            bmp = tool->GetNormalBitmap();
            if( !bmp.IsOk() ){
                bmp = m_style->GetToolIcon( tool->GetToolname(), TOOLICON_NORMAL, tool->rollover );
                if(m_sizefactor > 1.0 ){
                    wxImage scaled_image = bmp.ConvertToImage();
                    bmp = wxBitmap(scaled_image.Scale(tool->m_width, tool->m_height, wxIMAGE_QUALITY_HIGH));
                }
                tool->SetNormalBitmap( bmp );
                tool->bitmapOK = true;
            }
        } else {
            bmp = tool->GetDisabledBitmap();
            if( !bmp.IsOk() ){
                bmp = m_style->GetToolIcon( tool->GetToolname(), TOOLICON_DISABLED );
                if(m_sizefactor > 1.0 ){
                    wxImage scaled_image = bmp.ConvertToImage();
                    bmp = wxBitmap(scaled_image.Scale(tool->m_width, tool->m_height, wxIMAGE_QUALITY_HIGH));
                }
                tool->SetDisabledBitmap( bmp );
                tool->bitmapOK = true;
            }
        }
    } else {
        if ( tool->isPluginTool ) {

            // First try getting the icon from the Style.
            // If it is not in the style we build a new icon from the style BG and the plugin icon.

            if( tool->IsToggled() ) {
                bmp = m_style->GetToolIcon( tool->GetToolname(), TOOLICON_TOGGLED, tool->rollover );
                if( bmp.GetDepth() == 1 ) {
                    if( tool->rollover ) {
                        bmp = m_style->BuildPluginIcon( tool->pluginRolloverIcon, TOOLICON_TOGGLED );
                        if( ! bmp.IsOk() )
                            bmp = m_style->BuildPluginIcon( tool->pluginNormalIcon, TOOLICON_TOGGLED );
                    }
                    else
                        bmp = m_style->BuildPluginIcon( tool->pluginNormalIcon, TOOLICON_TOGGLED );
                }
            } else {
                bmp = m_style->GetToolIcon( tool->GetToolname(), TOOLICON_NORMAL, tool->rollover );
                if( bmp.GetDepth() == 1 ) {
                    if( tool->rollover ) {
                        bmp = m_style->BuildPluginIcon( tool->pluginRolloverIcon, TOOLICON_NORMAL );
                        if( ! bmp.IsOk() )
                            bmp = m_style->BuildPluginIcon( tool->pluginNormalIcon, TOOLICON_NORMAL );
                    }
                    else
                        bmp = m_style->BuildPluginIcon( tool->pluginNormalIcon, TOOLICON_NORMAL );
                }
            }
            if(m_sizefactor > 1.0 ){
                wxImage scaled_image = bmp.ConvertToImage();
                bmp = wxBitmap(scaled_image.Scale(tool->m_width, tool->m_height, wxIMAGE_QUALITY_HIGH));
            }
            tool->SetNormalBitmap( bmp );
            tool->bitmapOK = true;
        } else {
            if( tool->IsEnabled() ) {
                if( tool->IsToggled() )
                    bmp = m_style->GetToolIcon( tool->GetToolname(), TOOLICON_TOGGLED, tool->rollover );
                else
                    bmp = m_style->GetToolIcon( tool->GetIconName(), TOOLICON_NORMAL, tool->rollover );

                if(m_sizefactor > 1.0 ){
                    wxImage scaled_image = bmp.ConvertToImage();
                    bmp = wxBitmap(scaled_image.Scale(tool->m_width, tool->m_height, wxIMAGE_QUALITY_HIGH));
                }
                tool->SetNormalBitmap( bmp );
                tool->bitmapOK = true;
            } else {
                bmp = m_style->GetToolIcon( tool->GetToolname(), TOOLICON_DISABLED );
                if(m_sizefactor > 1.0 ){
                    wxImage scaled_image = bmp.ConvertToImage();
                    bmp = wxBitmap(scaled_image.Scale(tool->m_width, tool->m_height, wxIMAGE_QUALITY_HIGH));
                }
                tool->SetDisabledBitmap( bmp );
                tool->bitmapOK = true;
            }
        }
    }

    if( tool->firstInLine ) {
        m_style->DrawToolbarLineStart( bmp );
    }
    if( tool->lastInLine ) {
        m_style->DrawToolbarLineEnd( bmp );
    }

    if( bmp.GetWidth() != m_style->GetToolSize().x
            || bmp.GetHeight() != m_style->GetToolSize().y ) {
//        drawAt.x -= ( bmp.GetWidth() - m_style->GetToolSize().x ) / 2;
//        drawAt.y -= ( bmp.GetHeight() - m_style->GetToolSize().y ) / 2;
    }

    //      Clear the last drawn tool if necessary
    if( tool->last_rect.width && ((tool->last_rect.x != drawAt.x) || (tool->last_rect.y != drawAt.y)) ){
        wxBrush bb(GetGlobalColor( _T("GREY2") ));
        dc.SetBrush(bb);
        dc.SetPen( *wxTRANSPARENT_PEN );
        dc.DrawRectangle(tool->last_rect.x, tool->last_rect.y, tool->last_rect.width, tool->last_rect.height);
    }

    //  could cache this in the tool...
    //  A bit of a hack here.  We only scale tools if they are to be magnified globally
    if(0/*m_sizefactor > 1.0*/ )
    {
        wxImage scaled_image = bmp.ConvertToImage();
        wxBitmap sbmp = wxBitmap(scaled_image.Scale(tool->m_width, tool->m_height, wxIMAGE_QUALITY_HIGH));
        dc.DrawBitmap( sbmp, drawAt );
        tool->last_rect = wxRect(drawAt.x, drawAt.y, sbmp.GetWidth(), sbmp.GetHeight());

    }
    else
    {
        dc.DrawBitmap( bmp, drawAt );
        tool->last_rect = wxRect(drawAt.x, drawAt.y, bmp.GetWidth(), bmp.GetHeight());
    }

}

// ----------------------------------------------------------------------------
// toolbar geometry
// ----------------------------------------------------------------------------

wxToolBarToolBase *ocpnToolBarSimple::FindToolForPosition( wxCoord x, wxCoord y )
{
    wxToolBarToolsList::compatibility_iterator node = m_tools.GetFirst();
    while( node ) {
        ocpnToolBarTool *tool = (ocpnToolBarTool *) node->GetData();
        if( ( x >= tool->m_x ) && ( y >= tool->m_y ) && ( x < ( tool->m_x + tool->GetWidth() ) )
                && ( y < ( tool->m_y + tool->GetHeight() ) ) ) {
            return tool;
        }

        node = node->GetNext();
    }

    return (wxToolBarToolBase *) NULL;
}

void ocpnToolBarSimple::InvalidateBitmaps()
{
    wxToolBarToolsList::compatibility_iterator node = m_tools.GetFirst();
    while( node ) {
        ocpnToolBarTool *tool = (ocpnToolBarTool *) node->GetData();
        tool->bitmapOK = false;
        node = node->GetNext();
    }
}

wxRect ocpnToolBarSimple::GetToolRect( int tool_id )
{
    wxRect rect;
    wxToolBarToolBase *tool = FindById( tool_id );
    if( tool ) {
        ocpnToolBarTool *otool = (ocpnToolBarTool *) tool;
        if( otool ) rect = otool->trect;
    }

    return rect;
}

// ----------------------------------------------------------------------------
// tool state change handlers
// ----------------------------------------------------------------------------

void ocpnToolBarSimple::DoEnableTool( wxToolBarToolBase *tool, bool WXUNUSED(enable) )
{
    DrawTool( tool );
}

void ocpnToolBarSimple::DoToggleTool( wxToolBarToolBase *tool, bool WXUNUSED(toggle) )
{
    ocpnToolBarTool *t = (ocpnToolBarTool *) tool;
    t->bitmapOK = false;
    DrawTool( tool );
}

// Okay, so we've left the tool we're in ... we must check if the tool we're
// leaving was a 'sprung push button' and if so, spring it back to the up
// state.
void ocpnToolBarSimple::SpringUpButton( int id )
{
    wxToolBarToolBase *tool = FindById( id );

    if( tool && tool->CanBeToggled() ) {
        if( tool->IsToggled() ) tool->Toggle();

        DrawTool( tool );
    }
}

// ----------------------------------------------------------------------------
// scrolling implementation
// ----------------------------------------------------------------------------

wxString ocpnToolBarSimple::GetToolShortHelp( int id ) const
{
    wxToolBarToolBase *tool = FindById( id );
    wxCHECK_MSG( tool, wxEmptyString, _T("no such tool") );

    return tool->GetShortHelp();
}

wxString ocpnToolBarSimple::GetToolLongHelp( int id ) const
{
    wxToolBarToolBase *tool = FindById( id );
    wxCHECK_MSG( tool, wxEmptyString, _T("no such tool") );

    return tool->GetLongHelp();
}

void ocpnToolBarSimple::SetToolShortHelp( int id, const wxString& help )
{
    wxToolBarToolBase *tool = FindById( id );
    if( tool ) {
        (void) tool->SetShortHelp( help );
    }
}

void ocpnToolBarSimple::SetToolLongHelp( int id, const wxString& help )
{
    wxToolBarToolBase *tool = FindById( id );
    if( tool ) {
        (void) tool->SetLongHelp( help );
    }
}

int ocpnToolBarSimple::GetToolPos( int id ) const
{
    size_t pos = 0;
    wxToolBarToolsList::compatibility_iterator node;

    for( node = m_tools.GetFirst(); node; node = node->GetNext() ) {
        if( node->GetData()->GetId() == id ) return pos;

        pos++;
    }

    return wxNOT_FOUND;
}
bool ocpnToolBarSimple::GetToolState( int id ) const
{
    wxToolBarToolBase *tool = FindById( id );
    wxCHECK_MSG( tool, false, _T("no such tool") );

    return tool->IsToggled();
}

bool ocpnToolBarSimple::GetToolEnabled( int id ) const
{
    wxToolBarToolBase *tool = FindById( id );
    wxCHECK_MSG( tool, false, _T("no such tool") );

    return tool->IsEnabled();
}

void ocpnToolBarSimple::ToggleTool( int id, bool toggle )
{
    wxToolBarToolBase *tool = FindById( id );
        
    if( tool && tool->CanBeToggled() && tool->Toggle( toggle ) ) {
        DoToggleTool( tool, toggle );
        if( g_toolbar ) g_toolbar->Refresh();
    }
}

wxObject *ocpnToolBarSimple::GetToolClientData( int id ) const
{
    wxToolBarToolBase *tool = FindById( id );
    return tool ? tool->GetClientData() : (wxObject *) NULL;
}

void ocpnToolBarSimple::SetToolClientData( int id, wxObject *clientData )
{
    wxToolBarToolBase *tool = FindById( id );

    wxCHECK_RET( tool, _T("no such tool in wxToolBar::SetToolClientData") );

    tool->SetClientData( clientData );
}

void ocpnToolBarSimple::EnableTool( int id, bool enable )
{
    wxToolBarToolBase *tool = FindById( id );
    if( tool ) {
        if( tool->Enable( enable ) ) {
            DoEnableTool( tool, enable );
        }
    }
    wxMenuItem* configItem = g_FloatingToolbarConfigMenu->FindItem( id );
    configItem->Check( true );
}

void ocpnToolBarSimple::SetToolBitmaps( int id, wxBitmap *bmp, wxBitmap *bmpRollover )
{
    ocpnToolBarTool *tool = (ocpnToolBarTool*)FindById( id );
    if( tool ) {
        tool->pluginNormalIcon = bmp;
        tool->pluginRolloverIcon = bmpRollover;
        tool->bitmapOK = false;
    }
}

void ocpnToolBarSimple::SetToolTooltipHiViz( int id, bool b_hiviz )
{
    ocpnToolBarTool *tool = (ocpnToolBarTool*)FindById( id );
    if( tool ) {
        tool->SetTooltipHiviz( b_hiviz );
    }
}


void ocpnToolBarSimple::ClearTools()
{
    while( GetToolsCount() ) {
        DeleteToolByPos( 0 );
    }
}

int ocpnToolBarSimple::GetVisibleToolCount()
{
    int counter = 0;
    wxToolBarToolsList::compatibility_iterator node = m_tools.GetFirst();
    while( node ) {
        ocpnToolBarTool *tool = (ocpnToolBarTool *) node->GetData();
        counter++;
        node = node->GetNext();
    }
    return counter;
}

bool ocpnToolBarSimple::DeleteToolByPos( size_t pos )
{
    wxCHECK_MSG( pos < GetToolsCount(), false,
            _T("invalid position in wxToolBar::DeleteToolByPos()") );

    wxToolBarToolsList::compatibility_iterator node = m_tools.Item( pos );

    if( !DoDeleteTool( pos, node->GetData() ) ) {
        return false;
    }

    delete node->GetData();
    m_tools.Erase( node );

    return true;
}

bool ocpnToolBarSimple::DeleteTool( int id )
{
    size_t pos = 0;
    wxToolBarToolsList::compatibility_iterator node;
    for( node = m_tools.GetFirst(); node; node = node->GetNext() ) {
        if( node->GetData()->GetId() == id ) break;

        pos++;
    }

    if( !node || !DoDeleteTool( pos, node->GetData() ) ) {
        return false;
    }

    delete node->GetData();
    m_tools.Erase( node );

    return true;
}

wxToolBarToolBase *ocpnToolBarSimple::AddSeparator()
{
    return InsertSeparator( GetToolsCount() );
}

wxToolBarToolBase *ocpnToolBarSimple::InsertSeparator( size_t pos )
{
    wxCHECK_MSG( pos <= GetToolsCount(), (wxToolBarToolBase *)NULL,
            _T("invalid position in wxToolBar::InsertSeparator()") );

    wxToolBarToolBase *tool = CreateTool( wxID_SEPARATOR, wxEmptyString, wxNullBitmap, wxNullBitmap,
            wxITEM_SEPARATOR, (wxObject *) NULL, wxEmptyString, wxEmptyString );

    if( !tool || !DoInsertTool( pos, tool ) ) {
        delete tool;

        return NULL;
    }

    m_tools.Insert( pos, tool );

    return tool;
}

wxToolBarToolBase *ocpnToolBarSimple::RemoveTool( int id )
{
    size_t pos = 0;
    wxToolBarToolsList::compatibility_iterator node;
    for( node = m_tools.GetFirst(); node; node = node->GetNext() ) {
        if( node->GetData()->GetId() == id ) break;

        pos++;
    }

    if( !node ) {
        // don't give any error messages - sometimes we might call RemoveTool()
        // without knowing whether the tool is or not in the toolbar
        return (wxToolBarToolBase *) NULL;
    }

    wxToolBarToolBase *tool = node->GetData();
    if( !DoDeleteTool( pos, tool ) ) {
        return (wxToolBarToolBase *) NULL;
    }

    m_tools.Erase( node );

    return tool;
}


wxControl *ocpnToolBarSimple::FindControl( int id )
{
    for( wxToolBarToolsList::compatibility_iterator node = m_tools.GetFirst(); node;
            node = node->GetNext() ) {
        const wxToolBarToolBase * const tool = node->GetData();
        if( tool->IsControl() ) {
            wxControl * const control = tool->GetControl();

            if( !control ) {
                wxFAIL_MSG( _T("NULL control in toolbar?") );
            } else
                if( control->GetId() == id ) {
                    // found
                    return control;
                }
        }
    }

    return NULL;
}

wxToolBarToolBase *ocpnToolBarSimple::FindById( int id ) const
{
    wxToolBarToolBase *tool = (wxToolBarToolBase *) NULL;

    for( wxToolBarToolsList::compatibility_iterator node = m_tools.GetFirst(); node;
            node = node->GetNext() ) {
        tool = node->GetData();
        if( tool->GetId() == id ) {
            // found
            break;
        }

        tool = NULL;
    }

    return tool;
}

// ----------------------------------------------------------------------------
// event processing
// ----------------------------------------------------------------------------

// Only allow toggle if returns true
bool ocpnToolBarSimple::OnLeftClick( int id, bool toggleDown )
{
    wxCommandEvent event( wxEVT_COMMAND_TOOL_CLICKED, id );
    event.SetEventObject( this );

    // we use SetInt() to make wxCommandEvent::IsChecked() return toggleDown
    event.SetInt( (int) toggleDown );

    // and SetExtraLong() for backwards compatibility
    event.SetExtraLong( (long) toggleDown );

    // Send events to this toolbar instead (and thence up the window hierarchy)
    GetEventHandler()->ProcessEvent( event );

    return true;
}

// Call when right button down.
void ocpnToolBarSimple::OnRightClick( int id, long WXUNUSED(x), long WXUNUSED(y) )
{
    wxCommandEvent event( wxEVT_COMMAND_TOOL_RCLICKED, id );
    event.SetEventObject( this );
    event.SetInt( id );

    HideTooltip();
    ((ocpnFloatingToolbarDialog*)GetParent())->toolbarConfigChanged = false;
    wxMenu* contextMenu = new wxMenu();
    wxMenuItem* submenu = contextMenu->AppendSubMenu( g_FloatingToolbarConfigMenu, _("Visible buttons") );

    PopupMenu( contextMenu );

    contextMenu->Remove( submenu );
    delete contextMenu;

    if( ((ocpnFloatingToolbarDialog*)GetParent())->toolbarConfigChanged )
        gFrame->GetEventHandler()->AddPendingEvent( event );
}

// Called when the mouse cursor enters a tool bitmap (no button pressed).
// Argument is wxID_ANY if mouse is exiting the toolbar.
// Note that for this event, the id of the window is used,
// and the integer parameter of wxCommandEvent is used to retrieve
// the tool id.
void ocpnToolBarSimple::OnMouseEnter( int id )
{
    wxCommandEvent event( wxEVT_COMMAND_TOOL_ENTER, GetId() );
    event.SetEventObject( this );
    event.SetInt( id );

    wxFrame *frame = wxDynamicCast(GetParent(), wxFrame);
    if( frame ) {
        wxString help;
        wxToolBarToolBase* tool = id == wxID_ANY ? (wxToolBarToolBase*) NULL : FindById( id );
        if( tool ) help = tool->GetLongHelp();
        frame->DoGiveHelp( help, id != wxID_ANY );
    }

    (void) GetEventHandler()->ProcessEvent( event );

    DoPluginToolUp();
}

void ocpnToolBarSimple::DoPluginToolUp()
{
    //        Look for PlugIn tools
    //        If found, make the callback.
    if( !g_pi_manager)
        return;

    ArrayOfPlugInToolbarTools tool_array = g_pi_manager->GetPluginToolbarToolArray();
    for( unsigned int i = 0; i < tool_array.GetCount(); i++ ) {
        PlugInToolbarToolContainer *pttc = tool_array.Item( i );
        if( m_last_plugin_down_id == pttc->id ) {
            opencpn_plugin_113 *ppi = dynamic_cast<opencpn_plugin_113 *>(pttc->m_pplugin);
            if( ppi )
                ppi->OnToolbarToolUpCallback( pttc->id );
        }
    }

    m_last_plugin_down_id = -1;
}

void ocpnToolBarSimple::SetToolNormalBitmapEx(wxToolBarToolBase *tool, const wxString & iconName)
{
    if( tool ) {
        ocpnToolBarTool *otool = (ocpnToolBarTool *) tool;
        if(otool){
            ocpnStyle::Style *style = g_StyleManager->GetCurrentStyle();

            wxBitmap bmp = style->GetToolIcon( iconName, TOOLICON_NORMAL );
            if(m_sizefactor > 1.0 ){
                wxImage scaled_image = bmp.ConvertToImage();
                bmp = wxBitmap(scaled_image.Scale(otool->m_width, otool->m_height, wxIMAGE_QUALITY_HIGH));
            }
        
            tool->SetNormalBitmap( bmp );
            otool->SetIconName( iconName );
        }
    }
}


//-------------------------------------------------------------------------------------

ToolbarMOBDialog::ToolbarMOBDialog( wxWindow* parent )
       : wxDialog( parent, wxID_ANY, _("OpenCPN Alert"), wxDefaultPosition, wxSize(250, 230) )
{
    wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer* sizer = new wxBoxSizer( wxVERTICAL );
    topSizer->Add( sizer, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

    choices.push_back( new wxRadioButton( this, 0,  _("No, I don't want to hide it."),
            wxDefaultPosition, wxDefaultSize, wxRB_GROUP ) );

    choices.push_back( new wxRadioButton( this, 1, _("No, and permanently remove the option to hide it."),
            wxDefaultPosition) );

    choices.push_back( new wxRadioButton( this, 2, _("Yes, hide it."),
            wxDefaultPosition) );

    wxStdDialogButtonSizer* buttonSizer = CreateStdDialogButtonSizer( wxOK | wxCANCEL );


    wxStaticText* textCtrl = new wxStaticText( this, wxID_ANY, _("The Man Over Board button could be an important safety feature.\nAre you sure you want to hide it?") );

    sizer->Add( textCtrl, 0, wxEXPAND | wxALL, 5 );
    sizer->Add( choices[0], 0, wxEXPAND | wxALL, 5 );
    sizer->Add( choices[1], 0, wxEXPAND | wxALL, 5 );
    sizer->Add( choices[2], 0, wxEXPAND | wxALL, 5 );
    sizer->Add( buttonSizer, 0, wxEXPAND | wxTOP, 5 );

    topSizer->SetSizeHints(this);
    SetSizer( topSizer );
}

int ToolbarMOBDialog::GetSelection() {
    for( unsigned int i=0; i<choices.size(); i++ ) {
        if( choices[i]->GetValue() ) return choices[i]->GetId();
    }
    return 0;
}
