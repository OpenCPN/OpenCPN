/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Toolbar
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
 *
 */

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

extern FontMgr*                   pFontMgr;
extern ocpnFloatingToolbarDialog* g_FloatingToolbarDialog;
extern bool                       g_bTransparentToolbar;
extern ChartCanvas*               cc1;
extern bool                       g_bopengl;
extern ocpnToolBarSimple*         g_toolbar;
extern ocpnStyle::StyleManager*   g_StyleManager;
extern MyFrame*                   gFrame;
extern PlugInManager*             g_pi_manager;
extern wxMenu*                    g_FloatingToolbarConfigMenu;
extern wxString                   g_toolbarConfig;
extern bool                       g_bPermanentMOBIcon;

//----------------------------------------------------------------------------
// GrabberWindow Implementation
//----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(GrabberWin, wxPanel) EVT_MOUSE_EVENTS ( GrabberWin::MouseEvent )
EVT_PAINT ( GrabberWin::OnPaint )
END_EVENT_TABLE()

GrabberWin::GrabberWin( wxWindow *parent )
{
    m_style = g_StyleManager->GetCurrentStyle();
    m_pbitmap = m_style->GetIcon( _T("grabber") );

    Create( parent, -1 );

    SetSize( wxSize( m_pbitmap.GetWidth(), m_pbitmap.GetHeight() ) );

    m_bLeftDown = false;
    m_bRightDown = false;
}

void GrabberWin::OnPaint( wxPaintEvent& event )
{
    wxPaintDC dc( this );

    dc.DrawBitmap( m_pbitmap, 0, 0, true );
}

void GrabberWin::SetColorScheme( ColorScheme cs )
{
    wxColour back_color = GetGlobalColor( _T("GREY2") );

    SetBackgroundColour( back_color );
    ClearBackground();
    m_pbitmap = m_style->GetIcon( _T("grabber") );
}

void GrabberWin::MouseEvent( wxMouseEvent& event )
{
    static wxPoint s_gspt;
    int x, y;

    event.GetPosition( &x, &y );

    wxPoint spt = ClientToScreen( wxPoint( x, y ) );

#ifdef __WXOSX__
    if (!m_bLeftDown && event.LeftIsDown())
    {
        m_bLeftDown = true;
        s_gspt = spt;
        if (!HasCapture())
        CaptureMouse();
    }
    else if (m_bLeftDown && !event.LeftIsDown())
    {
        m_bLeftDown = false;
        if (HasCapture())
        ReleaseMouse();
    }

    if (!m_bRightDown && event.RightIsDown()) {
        m_bRightDown = true;
        if (!HasCapture()) {
            CaptureMouse();
            ocpnFloatingToolbarDialog *pp = wxDynamicCast(GetParent(), ocpnFloatingToolbarDialog);
            pp->ToggleOrientation();
        }
    }
    else if (m_bRightDown && !event.RightIsDown()) {
        m_bRightDown = false;
        if (HasCapture())
        ReleaseMouse();
    }
#else
    if( event.LeftDown() ) {
        s_gspt = spt;
        CaptureMouse();
    }

    if( event.LeftUp() ) {
        if( HasCapture() ) ReleaseMouse();
    }

    if( event.RightDown() ) {
        ocpnFloatingToolbarDialog *pp = wxDynamicCast(GetParent(), ocpnFloatingToolbarDialog);
        pp->ToggleOrientation();
    }
#endif

    if( event.Dragging() ) {
        wxPoint par_pos_old = GetParent()->GetPosition();

        wxPoint par_pos = par_pos_old;
        par_pos.x += spt.x - s_gspt.x;
        par_pos.y += spt.y - s_gspt.y;

        ocpnFloatingToolbarDialog *dp = wxDynamicCast(GetParent(), ocpnFloatingToolbarDialog);
        if( dp ) dp->MoveDialogInScreenCoords( par_pos, par_pos_old );

        s_gspt = spt;

    }
    gFrame->Raise();
}
//---------------------------------------------------------------------------------------
//          ocpnFloatingToolbarDialog Implementation
//---------------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(ocpnFloatingToolbarDialog, wxDialog)
    EVT_MOUSE_EVENTS ( ocpnFloatingToolbarDialog::MouseEvent )
    EVT_MENU(wxID_ANY, ocpnFloatingToolbarDialog::OnToolLeftClick)
    EVT_TIMER ( FADE_TIMER, ocpnFloatingToolbarDialog::FadeTimerEvent )
    EVT_WINDOW_CREATE(ocpnFloatingToolbarDialog::OnWindowCreate)
END_EVENT_TABLE()

ocpnFloatingToolbarDialog::ocpnFloatingToolbarDialog( wxWindow *parent, wxPoint position,
        long orient )
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
    m_fade_timer.SetOwner( this, FADE_TIMER );
    if( g_bTransparentToolbar ) {
//            DoFade(128);
        m_fade_timer.Start( 5000 );
    }

    m_pGrabberwin = new GrabberWin( this );

    m_position = position;
    m_orient = orient;

    m_style = g_StyleManager->GetCurrentStyle();

// A top-level sizer
    m_topSizer = new wxBoxSizer( wxHORIZONTAL );
    SetSizer( m_topSizer );

    //    Set initial "Dock" parameters
    m_dock_x = 0;
    m_dock_y = 0;
    m_block = false;

    Hide();
}

ocpnFloatingToolbarDialog::~ocpnFloatingToolbarDialog()
{
    DestroyToolBar();
}

void ocpnFloatingToolbarDialog::OnWindowCreate( wxWindowCreateEvent& event )
{
    Realize();
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

void ocpnFloatingToolbarDialog::SetGeometry()
{

    if( m_ptoolbar ) {
        m_ptoolbar->SetToolBitmapSize( m_style->GetToolSize() );

        wxSize tool_size = m_ptoolbar->GetToolBitmapSize();

        if( m_orient == wxTB_VERTICAL ) m_ptoolbar->SetMaxRowsCols(
                ( cc1->GetSize().y / ( tool_size.y + m_style->GetToolSeparation() ) ) - 1, 100 );
        else
            m_ptoolbar->SetMaxRowsCols( 100,
                    ( cc1->GetSize().x / ( tool_size.x + m_style->GetToolSeparation() ) ) - 1 );
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
    }
}

void ocpnFloatingToolbarDialog::Submerge()
{
    Hide();
    if( m_ptoolbar ) m_ptoolbar->KillTooltip();
}

void ocpnFloatingToolbarDialog::Surface()
{
#ifndef __WXOSX__
    Hide();
    Move( 0, 0 );
#endif

    RePosition();
    Show();
    if( m_ptoolbar ) m_ptoolbar->EnableTooltips();
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
    wxPoint old_screen_pos = m_pparent->ClientToScreen( m_position );

    if( m_orient == wxTB_HORIZONTAL ) {
        m_orient = wxTB_VERTICAL;
        m_ptoolbar->SetWindowStyleFlag( m_ptoolbar->GetWindowStyleFlag() & ~wxTB_HORIZONTAL );
        m_ptoolbar->SetWindowStyleFlag( m_ptoolbar->GetWindowStyleFlag() | wxTB_VERTICAL );
    } else {
        m_orient = wxTB_HORIZONTAL;
        m_ptoolbar->SetWindowStyleFlag( m_ptoolbar->GetWindowStyleFlag() & ~wxTB_VERTICAL );
        m_ptoolbar->SetWindowStyleFlag( m_ptoolbar->GetWindowStyleFlag() | wxTB_HORIZONTAL );
    }

    wxPoint grabber_point_abs = ClientToScreen( m_pGrabberwin->GetPosition() );

    m_style->SetOrientation( m_orient );
    m_ptoolbar->InvalidateBitmaps();

    SetGeometry();
    Realize();

    wxPoint pos_abs = grabber_point_abs;
    pos_abs.x -= m_pGrabberwin->GetPosition().x;
    MoveDialogInScreenCoords( pos_abs, old_screen_pos );

    RePosition();

    Show();   // this seems to be necessary on GTK to kick the sizer into gear...(FS#553)
    Refresh();
    //GetParent()->Refresh( false );
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
}

void ocpnFloatingToolbarDialog::FadeTimerEvent( wxTimerEvent& event )
{
    if( g_bTransparentToolbar && !g_bopengl ) DoFade( 128 );

    m_fade_timer.Start( 5000 );           // retrigger the continuous timer
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
        m_topSizer->Fit( this );

        //    Update "Dock" parameters
        if( m_position.x == 0 ) m_dock_x = -1;
        else
            if( m_position.x == m_pparent->GetClientSize().x - GetSize().x ) m_dock_x = 1;

        if( m_position.y == 0 ) m_dock_y = -1;
        else
            if( m_position.y == m_pparent->GetClientSize().y - GetSize().y ) m_dock_y = 1;

        // Now create a bitmap mask forthe frame shape.

        if( m_style->marginsInvisible ) {
            int toolCount = m_ptoolbar->GetVisibleToolCount();
            wxBitmap shape( GetSize().x, GetSize().y );
            wxMemoryDC sdc( shape );
            sdc.SetBackground( *wxWHITE_BRUSH );
            sdc.SetBrush( *wxBLACK_BRUSH );
            sdc.SetPen( *wxBLACK_PEN );
            sdc.Clear();

            wxPoint upperLeft( m_style->GetLeftMargin(), m_style->GetTopMargin() );
            wxSize visibleSize;
            if( m_ptoolbar->IsVertical() ) {
                int noTools = m_ptoolbar->GetMaxRows();
                if( noTools > toolCount ) noTools = toolCount;
                visibleSize.x = m_ptoolbar->GetLineCount()
                        * ( m_style->GetToolSize().x + m_style->GetTopMargin() );
                visibleSize.y = noTools
                        * ( m_style->GetToolSize().y + m_style->GetToolSeparation() );
                visibleSize.x -= m_style->GetTopMargin();
                visibleSize.y -= m_style->GetToolSeparation();
            } else {
                int noTools = m_ptoolbar->GetMaxCols();
                if( noTools > toolCount ) noTools = toolCount;
                visibleSize.x = noTools
                        * ( m_style->GetToolSize().x + m_style->GetToolSeparation() );
                visibleSize.y = m_ptoolbar->GetLineCount()
                        * ( m_style->GetToolSize().y + m_style->GetTopMargin() );
                visibleSize.x -= m_style->GetToolSeparation();
                visibleSize.y -= m_style->GetTopMargin();
            }

            int lines = m_ptoolbar->GetLineCount();
            for( int i = 1; i <= lines; i++ ) {
                if( m_ptoolbar->IsVertical() ) {
                    wxSize barsize( m_style->GetToolSize().x, visibleSize.y );
                    if( i == lines && i > 1 ) {
                        int toolsInLastLine = toolCount % m_ptoolbar->GetMaxRows();
                        if( toolsInLastLine == 0 ) toolsInLastLine = m_ptoolbar->GetMaxRows();
                        int emptySpace = ( m_ptoolbar->GetMaxRows() - toolsInLastLine );
                        barsize.y -= emptySpace
                                * ( m_style->GetToolSize().y + m_style->GetToolSeparation() );
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
                    upperLeft.x += m_style->GetTopMargin() + m_style->GetToolSize().x;
                } else {
                    wxSize barsize( visibleSize.x, m_style->GetToolSize().y );

                    if( i == 1 ) {
                        barsize.x += m_style->GetIcon( _T("grabber") ).GetWidth();
                    }
                    if( i == lines && i > 1 ) {
                        int toolsInLastLine = toolCount % m_ptoolbar->GetMaxCols();
                        if( toolsInLastLine == 0 ) toolsInLastLine = m_ptoolbar->GetMaxCols();
                        int emptySpace = ( m_ptoolbar->GetMaxCols() - toolsInLastLine );
                        barsize.x -= emptySpace
                                * ( m_style->GetToolSize().x + m_style->GetToolSeparation() );
                    }

                    sdc.DrawRoundedRectangle( upperLeft, barsize,
                            m_style->GetToolbarCornerRadius() );
                    upperLeft.y += m_style->GetTopMargin() + m_style->GetToolSize().y;
                }
            }
#ifndef __WXMAC__
            SetShape( wxRegion( shape, *wxWHITE, 10 ) );
#endif
        }
//        GetParent()->Update();                // not needed, as toolbat is a TopLevelWindow
    }
}

void ocpnFloatingToolbarDialog::OnToolLeftClick( wxCommandEvent& event )
{
    // First see if it was actually the context menu that was clicked.

    if( event.GetId() >= ID_PLUGIN_BASE + 100 ) {

        int itemId = event.GetId() - ID_PLUGIN_BASE - 100;
        bool toolIsChecked = g_FloatingToolbarConfigMenu->FindItem( event.GetId() )->IsChecked();

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
                        _("You can't hide the last tool from the toolbar\nas this would make is inaccessible."),
                        _("OpenCPN Alert"), wxOK );
                g_FloatingToolbarConfigMenu->FindItem( event.GetId() )->Check( true );
                return;
            }

            g_toolbarConfig.SetChar( itemId, _T('.') );
        }
        toolbarConfigChanged = true;
        return;
    }

    // No it was a button that was clicked.
    // Since Dialog events don't propagate automatically, we send it explicitly
    // (instead of relying on event.Skip()). Send events up the window hierarchy

    m_pparent->GetEventHandler()->AddPendingEvent( event );
    gFrame->Raise();
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

        SetGeometry();
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

private:

    wxString m_string;
    wxSize m_size;
    wxPoint m_position;
    wxBitmap *m_pbm;
    wxColour m_back_color;
    wxColour m_text_color;

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
    Hide();
}

ToolTipWin::~ToolTipWin()
{
    delete m_pbm;
}

void ToolTipWin::SetColorScheme( ColorScheme cs )
{
    m_back_color = GetGlobalColor( _T ( "UIBCK" ) );
    m_text_color = GetGlobalColor( _T ( "UITX1" ) );
}

void ToolTipWin::SetBitmap()
{
    int h, w;

    wxClientDC cdc( GetParent() );

    wxFont *plabelFont = pFontMgr->GetFont( _("ToolTips") );
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

    mdc.DrawRectangle( 0, 0, m_size.x, m_size.y );

    //    Draw the text
    mdc.SetFont( *plabelFont );
    mdc.SetTextForeground( m_text_color );
    mdc.SetTextBackground( m_back_color );

    mdc.DrawText( m_string, 4, 2 );

    int parent_width;
    cdc.GetSize( &parent_width, NULL );
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

        toolname = g_pi_manager->GetToolOwnerCommonName( id );
        if( toolname == _T("") ) {
            isPluginTool = false;
            toolname = label;
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

    wxCoord m_x;
    wxCoord m_y;
    wxCoord m_width;
    wxCoord m_height;
    wxRect trect;
    wxString toolname;
    const wxBitmap* pluginNormalIcon;
    const wxBitmap* pluginRolloverIcon;
    bool firstInLine;
    bool lastInLine;
    bool rollover;
    bool bitmapOK;
    bool isPluginTool;
    bool b_hilite;
};

// ----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(ocpnToolBarSimple, wxControl) EVT_SIZE(ocpnToolBarSimple::OnSize)
EVT_PAINT(ocpnToolBarSimple::OnPaint)
EVT_KILL_FOCUS(ocpnToolBarSimple::OnKillFocus)
EVT_MOUSE_EVENTS(ocpnToolBarSimple::OnMouseEvent)
EVT_TIMER(TOOLTIPON_TIMER, ocpnToolBarSimple::OnToolTipTimerEvent)

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

    wxSize toolSize = m_style->GetToolSize();
    int separatorSize = m_style->GetToolSeparation();

    ocpnToolBarTool *lastTool = NULL;
    bool firstNode = true;
    wxToolBarToolsList::compatibility_iterator node = m_tools.GetFirst();

    while( node ) {
        ocpnToolBarTool *tool = (ocpnToolBarTool *) node->GetData();
        tool->firstInLine = firstNode;
        tool->lastInLine = false;
        firstNode = false;

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
    if( m_LineCount > 1 || IsVertical() ) lastTool->lastInLine = true;

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

                wxPoint pos_in_toolbar( m_last_ro_tool->m_x, m_last_ro_tool->m_y );
                pos_in_toolbar.x += m_last_ro_tool->m_width + 2;

                //    Quick hack for right docked toolbar, to avoid tooltip interference
                if( ( g_FloatingToolbarDialog->GetDockX() == 1 )
                        && ( g_FloatingToolbarDialog->GetOrient() == wxTB_VERTICAL ) ) pos_in_toolbar.y =
                        m_last_ro_tool->m_y - 30;

                m_pToolTipWin->Move(0,0);       // workaround for gtk autocentre dialog behavior
                
                m_pToolTipWin->SetPosition( ClientToScreen( pos_in_toolbar ) );
                m_pToolTipWin->SetBitmap();
                m_pToolTipWin->Show();
                gFrame->Raise();
            }
        }

        m_one_shot = 10;
    }
}

int s_dragx, s_dragy;

void ocpnToolBarSimple::OnMouseEvent( wxMouseEvent & event )
{

    if( event.Leaving() ) m_one_shot = 500;                   // inital value
    if( event.Entering() ) m_one_shot = 500;

    wxCoord x, y;
    event.GetPosition( &x, &y );
    ocpnToolBarTool *tool = (ocpnToolBarTool *) FindToolForPosition( x, y );
    if( event.LeftDown() ) {
        CaptureMouse();
        s_dragx = x;
        s_dragy = y;
    }
    if( event.LeftUp() ) {
        if( HasCapture() ) ReleaseMouse();
    }

    if( tool && tool->IsButton() && IsShown() ) {

#ifndef __WXOSX__
        //    ToolTips
        if( NULL == m_pToolTipWin ) {
            m_pToolTipWin = new ToolTipWin( GetParent() );
            m_pToolTipWin->SetColorScheme( m_currentColorScheme );
            m_pToolTipWin->Hide();
        }

        if( tool != m_last_ro_tool ) m_pToolTipWin->Hide();

        if( !m_pToolTipWin->IsShown() ) {
            m_tooltip_timer.Start( m_one_shot, wxTIMER_ONE_SHOT );
        }
#endif

        //    Tool Rollover highlighting
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
            g_toolbar->Refresh( false );
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
        }

        DrawTool( tool );
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
        }
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
            if( !bmp.IsOk() ) bmp = m_style->GetToolIcon( tool->GetToolname(), TOOLICON_NORMAL,
                    tool->rollover );
        } else {
            bmp = tool->GetDisabledBitmap();
            if( !bmp.IsOk() ) bmp = m_style->GetToolIcon( tool->GetToolname(), TOOLICON_DISABLED );
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
						if( ! bmp.IsOk() ) bmp = m_style->BuildPluginIcon( tool->pluginNormalIcon, TOOLICON_TOGGLED );
                    }
                    else
                        bmp = m_style->BuildPluginIcon( tool->pluginNormalIcon, TOOLICON_TOGGLED );
                }
            } else {
                bmp = m_style->GetToolIcon( tool->GetToolname(), TOOLICON_NORMAL, tool->rollover );
                if( bmp.GetDepth() == 1 ) {
                    if( tool->rollover ) {
                        bmp = m_style->BuildPluginIcon( tool->pluginRolloverIcon, TOOLICON_NORMAL );
                        if( ! bmp.IsOk() ) bmp = m_style->BuildPluginIcon( tool->pluginNormalIcon, TOOLICON_NORMAL );
                    }
                    else
                        bmp = m_style->BuildPluginIcon( tool->pluginNormalIcon, TOOLICON_NORMAL );
                }
            }
            tool->SetNormalBitmap( bmp );
            tool->bitmapOK = true;
        } else {
            if( tool->IsEnabled() ) {
                if( tool->IsToggled() ) bmp = m_style->GetToolIcon( tool->GetToolname(), TOOLICON_TOGGLED,
                        tool->rollover );
                else
                    bmp = m_style->GetToolIcon( tool->GetToolname(), TOOLICON_NORMAL, tool->rollover );
                tool->SetNormalBitmap( bmp );
                tool->bitmapOK = true;
            } else {
                bmp = m_style->GetToolIcon( tool->GetToolname(), TOOLICON_DISABLED );
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
        drawAt.x -= ( bmp.GetWidth() - m_style->GetToolSize().x ) / 2;
        drawAt.y -= ( bmp.GetHeight() - m_style->GetToolSize().y ) / 2;
    }

    dc.DrawBitmap( bmp, drawAt );
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
    if( tool ) {
        tool->Toggle( toggle );
        DoToggleTool( tool, toggle );
    }
    if( g_toolbar ) g_toolbar->Refresh();
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
