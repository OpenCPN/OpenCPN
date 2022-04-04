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

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <vector>

#include "config.h"
#include "ocpn_types.h"
#include "navutil.h"
#include "styles.h"
#include "toolbar.h"
#include "chart1.h"
#include "pluginmanager.h"
#include "FontMgr.h"
#include "OCPNPlatform.h"
#include "chcanv.h"
#include "gui_lib.h"
#include "svg_utils.h"

#ifdef __OCPN__ANDROID__
#include "androidUTIL.h"
#endif

extern bool g_bTransparentToolbar;
extern bool g_bTransparentToolbarInOpenGLOK;
extern bool g_bopengl;
extern ocpnStyle::StyleManager *g_StyleManager;
extern MyFrame *gFrame;
extern PlugInManager *g_pi_manager;
extern bool g_bPermanentMOBIcon;
extern bool g_btouch;
extern bool g_bsmoothpanzoom;
extern OCPNPlatform *g_Platform;
extern bool g_bmasterToolbarFull;
extern bool g_useMUI;
extern wxString g_toolbarConfig;
extern double g_plus_minus_zoom_factor;
extern int g_maintoolbar_x;
extern int g_maintoolbar_y;

//----------------------------------------------------------------------------
// GrabberWindow Implementation
//----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(GrabberWin, wxPanel)
EVT_MOUSE_EVENTS(GrabberWin::MouseEvent)
EVT_PAINT(GrabberWin::OnPaint)
END_EVENT_TABLE()

GrabberWin::GrabberWin(wxWindow *parent, ocpnFloatingToolbarDialog *toolbar,
                       float scale_factor, wxString icon_name, wxPoint position)
    : wxPanel(parent, wxID_ANY, position, wxDefaultSize, wxNO_BORDER) {
  m_icon_name = icon_name;
  m_style = g_StyleManager->GetCurrentStyle();
  wxBitmap bitmap = m_style->GetIcon(icon_name);
  if (scale_factor > 1.0f) {
    int new_width = bitmap.GetWidth() * scale_factor;
    int new_height = bitmap.GetHeight() * scale_factor;
    wxImage scaled_image = bitmap.ConvertToImage();
    m_bitmap = wxBitmap(
        scaled_image.Scale(new_width, new_height, wxIMAGE_QUALITY_HIGH));
  } else
    m_bitmap = bitmap;

  SetSize(wxSize(m_bitmap.GetWidth(), m_bitmap.GetHeight()));
  SetMinSize(wxSize(m_bitmap.GetWidth(), m_bitmap.GetHeight()));

  m_bLeftDown = false;
  m_bRightDown = false;
  m_scale_factor = scale_factor;
  m_ptoolbar = toolbar;
  m_dragging = false;
  Hide();
}

void GrabberWin::OnPaint(wxPaintEvent &event) {
  wxPaintDC dc(this);

  wxColour back_color = GetGlobalColor(_T("GREY2"));
  SetBackgroundColour(back_color);
  ClearBackground();

  dc.DrawBitmap(m_bitmap, 0, 0, true);
}

void GrabberWin::SetColorScheme(ColorScheme cs) {
  wxColour back_color = GetGlobalColor(_T("GREY2"));

  SetBackgroundColour(back_color);
  ClearBackground();

  wxBitmap bitmap = m_style->GetIcon(m_icon_name);
  if (m_scale_factor > 1.0f) {
    int new_width = bitmap.GetWidth() * m_scale_factor;
    int new_height = bitmap.GetHeight() * m_scale_factor;
    wxImage scaled_image = bitmap.ConvertToImage();
    m_bitmap = wxBitmap(
        scaled_image.Scale(new_width, new_height, wxIMAGE_QUALITY_HIGH));
  } else
    m_bitmap = bitmap;
}

void GrabberWin::MouseEvent(wxMouseEvent &event) {}
#if 0
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


    if( event.RightDown() && m_ptoolbar->GetCanToggleOrientation()){
        if(m_ptoolbar){
            if(!m_ptoolbar->isSubmergedToGrabber()){
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
                if(m_ptoolbar->isSubmergedToGrabber()){
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
#endif

class ocpnToolBarTool : public wxToolBarToolBase {
public:
  ocpnToolBarTool(ocpnToolBarSimple *tbar, int id, const wxString &label,
                  const wxBitmap &bmpNormal, const wxBitmap &bmpRollover,
                  wxItemKind kind, wxObject *clientData,
                  const wxString &shortHelp, const wxString &longHelp)
      : wxToolBarToolBase((wxToolBarBase *)tbar, id, label, bmpNormal,
                          bmpRollover, kind, clientData, shortHelp, longHelp) {
    m_enabled = true;
    m_toggled = false;
    rollover = false;
    bitmapOK = false;
    m_btooltip_hiviz = false;

    toolname = g_pi_manager->GetToolOwnerCommonName(id);
    if (toolname == _T("")) {
      isPluginTool = false;
      toolname = label;
      iconName = label;

    } else {
      isPluginTool = true;
      pluginNormalIcon = bmpNormal;
      pluginRolloverIcon = bmpRollover;
    }
  }

  ocpnToolBarTool(ocpnToolBarSimple *tbar, int id, const wxBitmap &bmpNormal,
                  const wxBitmap &bmpRollover, wxItemKind kind,
                  wxObject *clientData, const wxString &shortHelp,
                  const wxString &longHelp)
      : wxToolBarToolBase((wxToolBarBase *)tbar, id, _T(""), bmpNormal,
                          bmpRollover, kind, clientData, shortHelp, longHelp) {
    m_enabled = true;
    m_toggled = false;
    rollover = false;
    m_btooltip_hiviz = false;
    isPluginTool = false;

    m_bmpNormal = bmpNormal;
    bitmapOK = true;
  }

  void SetSize(const wxSize &size) {
    m_width = size.x;
    m_height = size.y;
  }

  wxCoord GetWidth() const { return m_width; }

  wxCoord GetHeight() const { return m_height; }

  wxString GetToolname() { return toolname; }

  void SetIconName(wxString name) { iconName = name; }
  wxString GetIconName() { return iconName; }

  void SetTooltipHiviz(bool enable) { m_btooltip_hiviz = enable; }

  wxCoord m_x;
  wxCoord m_y;
  wxCoord m_width;
  wxCoord m_height;
  wxRect trect;
  wxString toolname;
  wxString iconName;
  wxBitmap pluginNormalIcon;
  wxBitmap pluginRolloverIcon;
  const wxBitmap *pluginToggledIcon;
  bool firstInLine;
  bool lastInLine;
  bool rollover;
  bool bitmapOK;
  bool isPluginTool;
  bool b_hilite;
  bool m_btooltip_hiviz;
  wxRect last_rect;
  wxString pluginNormalIconSVG;
  wxString pluginRolloverIconSVG;
  wxString pluginToggledIconSVG;
};

//---------------------------------------------------------------------------------------
//          ocpnFloatingToolbarDialog Implementation
//---------------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(ocpnFloatingToolbarDialog, wxFrame)
EVT_MOUSE_EVENTS(ocpnFloatingToolbarDialog::MouseEvent)
EVT_MENU(wxID_ANY, ocpnFloatingToolbarDialog::OnToolLeftClick)
EVT_TIMER(FADE_TIMER, ocpnFloatingToolbarDialog::FadeTimerEvent)
EVT_TIMER(DESTROY_TIMER, ocpnFloatingToolbarDialog::DestroyTimerEvent)
EVT_KEY_DOWN(ocpnFloatingToolbarDialog::OnKeyDown)
EVT_KEY_UP(ocpnFloatingToolbarDialog::OnKeyUp)
EVT_WINDOW_CREATE(ocpnFloatingToolbarDialog::OnWindowCreate)
END_EVENT_TABLE()

ocpnFloatingToolbarDialog::ocpnFloatingToolbarDialog(wxWindow *parent,
                                                     wxPoint position,
                                                     long orient,
                                                     float size_factor) {
  m_pparent = parent;
  long wstyle = wxNO_BORDER | wxFRAME_NO_TASKBAR | wxFRAME_SHAPED |
                wxFRAME_FLOAT_ON_PARENT | wxFRAME_TOOL_WINDOW;

  m_ptoolbar = NULL;

  wxFrame::Create(parent, -1, _T(""), wxPoint(-1, -1), wxSize(-1, -1), wstyle);

  m_opacity = 255;

  m_pGrabberwin =
      NULL;  // new GrabberWin( this, this, size_factor, _T("grabber_hi") );
  m_bGrabberEnable = true;  // default

  m_pRecoverwin = NULL;
  m_position = position;
  m_orient = orient;
  m_sizefactor = size_factor;
  m_cornerRadius = 0;

  m_bAutoHideToolbar = false;
  m_nAutoHideToolbar = 5;
  m_toolbar_scale_tools_shown = false;
  m_backcolorString = _T("GREY2");
  m_toolShowMask = _T("XXXXXXXXXXXXXXXX");
  n_toolbarHideMethod = TOOLBAR_HIDE_TO_GRABBER;
  b_canToggleOrientation = true;
  m_enableRolloverBitmaps = true;
  m_auxOffsetY = 0;

  m_ptoolbar = CreateNewToolbar();

  m_cs = (ColorScheme)-1;

  m_style = g_StyleManager->GetCurrentStyle();
  SetULDockPosition(wxPoint(4, 4));

  SetGeometry(false, wxRect());

  // A top-level sizer
  m_topSizer = new wxBoxSizer(wxHORIZONTAL);
  SetSizer(m_topSizer);

  //    Set initial "Dock" parameters
  m_dock_x = 0;
  m_dock_y = 0;
  m_block = false;

  m_marginsInvisible = m_style->marginsInvisible;

  //    if(m_sizefactor > 1.0 )
  //       m_marginsInvisible = true;

  m_bnavgrabber = false;

  m_FloatingToolbarConfigMenu = NULL;

  Hide();

  m_bsubmerged = false;
  m_bsubmergedToGrabber = false;

  m_fade_timer.SetOwner(this, FADE_TIMER);
  if (g_bTransparentToolbar) m_fade_timer.Start(5000);

  if (m_bAutoHideToolbar && (m_nAutoHideToolbar > 0))
    m_fade_timer.Start(m_nAutoHideToolbar * 1000);

  m_destroyTimer.SetOwner(this, DESTROY_TIMER);

  m_benableSubmerge = true;
}

ocpnFloatingToolbarDialog::~ocpnFloatingToolbarDialog() {
  delete m_FloatingToolbarConfigMenu;

  DestroyToolBar();
}

void ocpnFloatingToolbarDialog::AddToolItem(ToolbarItemContainer *item) {
  m_Items.push_back(item);
}

int ocpnFloatingToolbarDialog::RebuildToolbar() {
  ocpnToolBarSimple *tb = GetToolbar();
  if (!tb) return 0;

  // Iterate over the array of items added,
  // Creating the toolbar from enabled items.
  int i_count = 0;
  for (auto it = m_Items.cbegin(); it != m_Items.cend(); it++) {
    ToolbarItemContainer *tic = *it;
    if (!tic) continue;

    bool bEnabled = _toolbarConfigMenuUtil(tic);

    if (bEnabled) {
      wxToolBarToolBase *tool =
          tb->AddTool(tic->m_ID, tic->m_label, tic->m_bmpNormal,
                      tic->m_bmpDisabled, tic->m_toolKind, tic->m_tipString);
      tic->m_tool = tool;

      //  Plugin tools may have prescribed their own SVG toolbars as file
      //  locations.
      if (!tic->m_NormalIconSVG.IsEmpty()) {
        tb->SetToolBitmapsSVG(tic->m_ID, tic->m_NormalIconSVG,
                              tic->m_RolloverIconSVG, tic->m_ToggledIconSVG);
      }
    }

    i_count++;
  }

  return i_count;
}

void ocpnFloatingToolbarDialog::SetULDockPosition(wxPoint position) {
  if (position.x >= 0) m_dock_min_x = position.x;
  if (position.y >= 0) m_dock_min_y = position.y;
}

size_t ocpnFloatingToolbarDialog::GetToolCount() {
  if (m_ptoolbar)
    return m_ptoolbar->GetToolsCount();
  else
    return 0;
}

void ocpnFloatingToolbarDialog::SetToolShowMask(wxString mask) {}

void ocpnFloatingToolbarDialog::SetToolShowCount(int count) {
  if (m_ptoolbar) m_ptoolbar->SetToolShowCount(count);
}

int ocpnFloatingToolbarDialog::GetToolShowCount(void) {
  if (m_ptoolbar)
    return m_ptoolbar->GetToolShowCount();
  else
    return 0;
}

void ocpnFloatingToolbarDialog::SetBackGroundColorString(wxString colorRef) {
  m_backcolorString = colorRef;
  SetColorScheme(m_cs);  // Causes a reload of background color
}

void ocpnFloatingToolbarDialog::OnKeyDown(wxKeyEvent &event) { event.Skip(); }

void ocpnFloatingToolbarDialog::OnKeyUp(wxKeyEvent &event) { event.Skip(); }

void ocpnFloatingToolbarDialog::CreateConfigMenu() {
  if (m_FloatingToolbarConfigMenu) delete m_FloatingToolbarConfigMenu;
  m_FloatingToolbarConfigMenu = new wxMenu();
}

bool ocpnFloatingToolbarDialog::_toolbarConfigMenuUtil(
    ToolbarItemContainer *tic) {
  if (m_FloatingToolbarConfigMenu) {
    wxMenuItem *menuitem;

    if (tic->m_ID == ID_MOB && g_bPermanentMOBIcon) return true;

    if (tic->m_bRequired) return true;
    if (tic->m_bPlugin) return true;

    // Item ID trickery is needed because the wxCommandEvents for menu item
    // clicked and toolbar button clicked are 100% identical, so if we use same
    // id's we can't tell the events apart.

    int idOffset = 100;  // Hopefully no more than 100 total icons...
    int menuItemId = tic->m_ID + idOffset;

    menuitem = m_FloatingToolbarConfigMenu->FindItem(menuItemId);

    if (menuitem) {
      return menuitem->IsChecked();
    }

    menuitem = m_FloatingToolbarConfigMenu->AppendCheckItem(menuItemId,
                                                            tic->m_tipString);
    size_t n = m_FloatingToolbarConfigMenu->GetMenuItemCount();
    menuitem->Check(m_configString.Len() >= n
                        ? m_configString.GetChar(n - 1) == _T('X')
                        : true);
    return menuitem->IsChecked();
  } else
    return true;
}

void ocpnFloatingToolbarDialog::OnWindowCreate(wxWindowCreateEvent &event) {
  // At least on MSW, this call leads to recursion and stack overflow.
  //  Probably not needed any longer
  // Realize();
}

void ocpnFloatingToolbarDialog::SetGrabber(wxString icon_name) {
  //    m_pGrabberwin->Destroy();
  m_pGrabberwin = new GrabberWin(this, this, m_sizefactor, icon_name);
  m_pGrabberwin->Hide();

  Realize();

#ifdef __WXOSX__
  m_pGrabberwin->Refresh();
#endif
}

void ocpnFloatingToolbarDialog::UpdateRecoveryWindow(bool b_toolbarEnable) {
  if (m_pRecoverwin) {
    if (b_toolbarEnable) {
      m_pRecoverwin->Raise();
      m_pRecoverwin->Refresh(false);
    } else
      m_pRecoverwin->Hide();
  }
}

void ocpnFloatingToolbarDialog::EnableTool(int toolid, bool enable) {
  if (m_ptoolbar) m_ptoolbar->EnableTool(toolid, enable);
}

void ocpnFloatingToolbarDialog::SetColorScheme(ColorScheme cs) {
  m_cs = cs;

  wxColour back_color = GetGlobalColor(m_backcolorString);

  //  Set background
  SetBackgroundColour(back_color);
  ClearBackground();

  if (m_ptoolbar) {
    //  Set background
    m_ptoolbar->SetBackgroundColour(back_color);
    m_ptoolbar->ClearBackground();

    m_ptoolbar->SetToggledBackgroundColour(GetGlobalColor(_T("GREY1")));

    m_ptoolbar->SetColorScheme(cs);
    m_ptoolbar->Refresh(true);
  }

  if (m_pGrabberwin) {
    m_pGrabberwin->SetColorScheme(cs);
    m_pGrabberwin->Refresh();
  }

  Refresh(true);
}

wxSize ocpnFloatingToolbarDialog::GetToolSize() {
  wxSize style_tool_size;
  if (m_ptoolbar) {
    style_tool_size = m_style->GetToolSize();

    style_tool_size.x *= m_sizefactor;
    style_tool_size.y *= m_sizefactor;
  } else {
    style_tool_size.x = 32;
    style_tool_size.y = 32;
  }

  return style_tool_size;
}

void ocpnFloatingToolbarDialog::SetGeometry(bool bAvoid, wxRect rectAvoid) {
  if (m_ptoolbar) {
    wxSize style_tool_size = m_style->GetToolSize();

    style_tool_size.x *= m_sizefactor;
    style_tool_size.y *= m_sizefactor;

    m_ptoolbar->SetToolBitmapSize(style_tool_size);

    wxSize tool_size = m_ptoolbar->GetToolBitmapSize();
    int grabber_width = m_style->GetIcon(_T("grabber")).GetWidth();

    int max_rows = 10;
    int max_cols = 100;

    if (GetParent()) {
      int avoid_start =
          GetParent()->GetClientSize().x -
          (tool_size.x + m_style->GetToolSeparation()) * 2;  // default
      if (bAvoid && !rectAvoid.IsEmpty()) {
        avoid_start = GetParent()->GetClientSize().x - rectAvoid.width -
                      10;  // this is compass window, if shown
      }

      max_rows = (GetParent()->GetClientSize().y /
                  (tool_size.y + m_style->GetToolSeparation())) -
                 2;

      max_cols = (avoid_start - grabber_width) /
                 (tool_size.x + m_style->GetToolSeparation());
      max_cols -= 1;

      if (m_orient == wxTB_VERTICAL)
        max_rows = wxMax(max_rows, 2);  // at least two rows
      else
        max_cols = wxMax(max_cols, 2);  // at least two columns
    }

    if (m_orient == wxTB_VERTICAL)
      m_ptoolbar->SetMaxRowsCols(max_rows, 100);
    else
      m_ptoolbar->SetMaxRowsCols(100, max_cols);
    m_ptoolbar->SetSizeFactor(m_sizefactor);
  }
}

void ocpnFloatingToolbarDialog::GetFrameRelativePosition(int *x, int *y) {
  wxPoint myPos = GetPosition();

  wxPoint relPos = gFrame->GetPrimaryCanvas()->ScreenToClient(myPos);
  if (x) *x = relPos.x;
  if (y) *y = relPos.y;
}

void ocpnFloatingToolbarDialog::RestoreRelativePosition(int x, int y) {
  if ((x < 0) || (y < 0)) {
    SetDefaultPosition();
    return;
  }

  wxPoint screenPos = gFrame->GetPrimaryCanvas()->ClientToScreen(wxPoint(x, y));
  Move(wxPoint(screenPos));
}

void ocpnFloatingToolbarDialog::SetDefaultPosition() {
  if (m_block) return;

  if (m_pparent && m_ptoolbar) {
    wxSize cs = m_pparent->GetClientSize();
    if (-1 == m_dock_x)
      m_position.x = m_dock_min_x;
    else if (1 == m_dock_x)
      m_position.x = cs.x - GetSize().x;

    if (-1 == m_dock_y)
      m_position.y = m_dock_min_y;
    else if (1 == m_dock_y)
      m_position.y = cs.y - GetSize().y;

    m_position.x = wxMin(cs.x - GetSize().x, m_position.x);
    m_position.y = wxMin(cs.y - GetSize().y, m_position.y);

    m_position.x = wxMax(m_dock_min_x, m_position.x);
    m_position.y = wxMax(m_dock_min_y, m_position.y);

    m_position.y += m_auxOffsetY;

    g_maintoolbar_x = m_position.x;
    g_maintoolbar_y = m_position.y;

    // take care of left docked instrument windows and don't blast the main
    // toolbar on top of them, hinding instruments this positions the main
    // toolbar directly right of the left docked instruments onto the chart
    //        wxPoint screen_pos = m_pparent->ClientToScreen( m_position );
    wxPoint screen_pos = gFrame->GetPrimaryCanvas()->ClientToScreen(m_position);

    //  GTK sometimes has trouble with ClientToScreen() if executed in the
    //  context of an event handler The position of the window is calculated
    //  incorrectly if a deferred Move() has not been processed yet. So work
    //  around this here... Discovered with a Dashboard window left-docked,
    //  toggled on and off by toolbar tool.

    //  But this causes another problem. If a toolbar is NOT left docked, it
    //  will walk left by two pixels on each call to Reposition().
    // TODO
#ifdef __WXGTK__
    if (m_pparent->GetParent()) {
      wxPoint pp = m_pparent->GetPosition();
      wxPoint ppg = m_pparent->GetParent()->GetScreenPosition();
      wxPoint screen_pos_fix = ppg + pp + m_position;
      screen_pos.x = screen_pos_fix.x;
    }
#endif

    Move(screen_pos);

#ifdef __WXQT__
    Raise();
#endif
  }
}

void ocpnFloatingToolbarDialog::Submerge() {
  m_bsubmerged = true;
  Hide();
  if (m_ptoolbar) m_ptoolbar->KillTooltip();
}

void ocpnFloatingToolbarDialog::SubmergeToGrabber() {
  if (!m_benableSubmerge) return;

  // Submerge();
  m_bsubmerged = true;
  m_bsubmergedToGrabber = true;
  Hide();
  if (m_ptoolbar) m_ptoolbar->KillTooltip();

  if (!m_pRecoverwin) {
    wxPoint tbPoint = GetPosition();
    wxPoint tbPosInCanvas = m_pparent->ScreenToClient(tbPoint);
    // tbPosInCanvas.y += 2;           // prettify
    m_pRecoverwin = new GrabberWin(m_pparent, this, m_sizefactor,
                                   _T("grabber_ext" ), tbPosInCanvas);
  }

  m_pRecoverwin->Show();
  m_pRecoverwin->Raise();
#ifdef __WXQT__
  wxSize s = gFrame->GetSize();
  m_recoversize = s;
  s.y--;
  // gFrame->TriggerResize(s);
  Raise();
#endif

  gFrame->Refresh();  // Needed for MSW OpenGL
}

void ocpnFloatingToolbarDialog::Surface() {
  if (m_pRecoverwin) {
    // SurfaceFromGrabber();
    m_pRecoverwin->Show();
    m_pRecoverwin->Raise();
  } else {
    m_bsubmerged = false;
#ifdef __WXMSW__
    Hide();
    Move(0, 0);
#endif

    RestoreRelativePosition(g_maintoolbar_x, g_maintoolbar_y);
    Show();
    if (m_ptoolbar) m_ptoolbar->EnableTooltips();

#ifdef __WXQT__
    Raise();
#endif
  }

  if (m_bAutoHideToolbar && (m_nAutoHideToolbar > 0)) {
    m_fade_timer.Start(m_nAutoHideToolbar * 1000);
  }
}

bool ocpnFloatingToolbarDialog::CheckSurfaceRequest(wxMouseEvent &event) {
  return false;
}

void ocpnFloatingToolbarDialog::SurfaceFromGrabber() {}

void ocpnFloatingToolbarDialog::DestroyTimerEvent(wxTimerEvent &event) {
  delete m_destroyGrabber;
  m_destroyGrabber = NULL;
}

bool ocpnFloatingToolbarDialog::isSubmergedToGrabber() {
  return (m_bsubmergedToGrabber);
}

void ocpnFloatingToolbarDialog::HideTooltip() {
#ifndef __OCPN__ANDROID__
  if (m_ptoolbar) m_ptoolbar->HideTooltip();
#endif
}

void ocpnFloatingToolbarDialog::ShowTooltips() {
#ifndef __OCPN__ANDROID__
  if (m_ptoolbar) m_ptoolbar->EnableTooltips();
#endif
}

void ocpnFloatingToolbarDialog::ToggleOrientation() {}

void ocpnFloatingToolbarDialog::MouseEvent(wxMouseEvent &event) {
  if (g_bTransparentToolbar) {
    if (event.Entering() && (m_opacity < 255)) {
      SetTransparent(255);
      m_opacity = 255;
    }

    m_fade_timer.Start(5000);  // retrigger the continuous timer
  }

  if (m_bAutoHideToolbar && (m_nAutoHideToolbar > 0)) {
    m_fade_timer.Start(m_nAutoHideToolbar * 1000);
  }

  event.Skip();
}

void ocpnFloatingToolbarDialog::FadeTimerEvent(wxTimerEvent &event) {
  if (n_toolbarHideMethod == TOOLBAR_HIDE_TO_FIRST_TOOL) {
    if (g_bmasterToolbarFull) {
      if (m_bAutoHideToolbar && (m_nAutoHideToolbar > 0) && !m_bsubmerged) {
        wxCommandEvent event;
        event.SetId(ID_MASTERTOGGLE);
        gFrame->OnToolLeftClick(event);
      }
    }
  } else {
    if (m_bnavgrabber) {
      m_fade_timer.Start(5000);  // do nothing if nav grabber is shown
    } else {
      if (g_bTransparentToolbar &&
          (!g_bopengl || g_bTransparentToolbarInOpenGLOK)) {
        DoFade(128);
        m_fade_timer.Start(5000);  // retrigger the continuous timer
      }

      if (m_bAutoHideToolbar && (m_nAutoHideToolbar > 0) && !m_bsubmerged) {
        SubmergeToGrabber();
      }
    }
  }
}

void ocpnFloatingToolbarDialog::SetAutoHideTimer(int time) {
  m_nAutoHideToolbar = time;
  if (m_bAutoHideToolbar) {
    m_fade_timer.Stop();
    m_fade_timer.Start(m_nAutoHideToolbar * 1000);
  }
}

void ocpnFloatingToolbarDialog::DoFade(int value) {
  if (value != m_opacity) SetTransparent(value);
  m_opacity = value;
}

void ocpnFloatingToolbarDialog::RefreshFadeTimer() {
  SetTransparent(255);
  m_opacity = 255;
  m_fade_timer.Start(500);  // retrigger the continuous timer

  if (m_bAutoHideToolbar && (m_nAutoHideToolbar > 0)) {
    m_fade_timer.Start(m_nAutoHideToolbar * 1000);
  }
}

void ocpnFloatingToolbarDialog::SetToolShortHelp(int id, const wxString &help) {
  if (m_ptoolbar) m_ptoolbar->SetToolShortHelp(id, help);
}

void ocpnFloatingToolbarDialog::MoveDialogInScreenCoords(wxPoint posn,
                                                         wxPoint posn_old) {
  wxPoint pos_in_parent = m_pparent->ScreenToClient(posn);
  wxPoint pos_in_parent_old = m_pparent->ScreenToClient(posn_old);

  //    "Docking" support
#define DOCK_MARGIN 40

  // X
  m_dock_x = 0;
  if (pos_in_parent.x < pos_in_parent_old.x)  // moving left
  {
    if (pos_in_parent.x < DOCK_MARGIN) {
      pos_in_parent.x = m_dock_min_x;  // but dock position may be offset
      m_dock_x = -1;
    }
  } else if (pos_in_parent.x > pos_in_parent_old.x)  // moving right
  {
    int max_right = m_pparent->GetClientSize().x - GetSize().x;
    if (pos_in_parent.x > (max_right - DOCK_MARGIN)) {
      pos_in_parent.x = max_right;
      m_dock_x = 1;
    }
  }

  // Y
  m_dock_y = 0;
  if (pos_in_parent.y < pos_in_parent_old.y)  // moving up
  {
    if (pos_in_parent.y < DOCK_MARGIN) {
      pos_in_parent.y = 0;
      m_dock_y = -1;
    }
  } else if (pos_in_parent.y > pos_in_parent_old.y)  // moving down
  {
    int max_down = m_pparent->GetClientSize().y - GetSize().y;
    if (pos_in_parent.y > (max_down - DOCK_MARGIN)) {
      pos_in_parent.y = max_down;
      m_dock_y = 1;
    }
  }

  m_position = pos_in_parent;

  wxPoint final_pos = m_pparent->ClientToScreen(pos_in_parent);

  Move(final_pos);
}

void ocpnFloatingToolbarDialog::Realize() {
  if (m_ptoolbar) {
    m_ptoolbar->Realize();

    m_topSizer->Clear();
    m_topSizer->Add(m_ptoolbar);

    if (m_bGrabberEnable) {
      if (!m_pGrabberwin) {
        m_pGrabberwin =
            new GrabberWin(this, this, m_sizefactor, _T("grabber_hi"));
        m_pGrabberwin->Hide();
      }

      m_pGrabberwin->Show();
      m_topSizer->Add(m_pGrabberwin, 0, wxTOP, m_style->GetTopMargin());
    }

    m_topSizer->Layout();
    Fit();

    //    Update "Dock" parameters
    if (m_position.x == 0)
      m_dock_x = -1;
    else if (m_position.x == m_pparent->GetClientSize().x - GetSize().x)
      m_dock_x = 1;

    if (m_position.y == 0)
      m_dock_y = -1;
    else if (m_position.y == m_pparent->GetClientSize().y - GetSize().y)
      m_dock_y = 1;

    // Now create a bitmap mask forthe frame shape.

    if (m_marginsInvisible) {
      wxSize tool_size = m_ptoolbar->GetToolBitmapSize();

      //  Determine whether the tool icons are meant (by style) to join without
      //  speces between This will determine what type of region to draw.
      bool b_overlap = false;

      wxToolBarToolsList::compatibility_iterator node1 =
          m_ptoolbar->m_tools.GetFirst();
      if (node1) {
        wxToolBarToolsList::compatibility_iterator node2 = node1->GetNext();
        if (node2) {
          wxToolBarToolBase *tool1 = node1->GetData();
          ocpnToolBarTool *tools1 = (ocpnToolBarTool *)tool1;

          wxToolBarToolBase *tool2 = node2->GetData();
          ocpnToolBarTool *tools2 = (ocpnToolBarTool *)tool2;

          if ((tools1->m_x + tools1->m_width) >= tools2->m_x) b_overlap = true;
        }
      }

      int toolCount = m_ptoolbar->GetVisibleToolCount();

      wxPoint upperLeft(m_style->GetLeftMargin(), m_style->GetTopMargin());
      wxSize visibleSize;
      if (m_ptoolbar->IsVertical()) {
        int noTools = m_ptoolbar->GetMaxRows();
        if (noTools > toolCount) noTools = toolCount;
        visibleSize.x = m_ptoolbar->GetLineCount() *
                        (tool_size.x + m_style->GetTopMargin());
        visibleSize.y = noTools * (tool_size.y + m_style->GetToolSeparation());
        visibleSize.x -= m_style->GetTopMargin();
        visibleSize.y -= m_style->GetToolSeparation();
      } else {
        int noTools = m_ptoolbar->GetMaxCols();
        if (noTools > toolCount) noTools = toolCount;
        visibleSize.x = noTools * (tool_size.x + m_style->GetToolSeparation());
        visibleSize.y = m_ptoolbar->GetLineCount() *
                        (tool_size.y + m_style->GetTopMargin());
        visibleSize.x -= m_style->GetToolSeparation();
        visibleSize.y -= m_style->GetTopMargin();
      }

      wxBitmap shape(visibleSize.x + tool_size.x,
                     visibleSize.y + tool_size.y);  // + fluff
      wxMemoryDC sdc(shape);
      sdc.SetBackground(*wxWHITE_BRUSH);
      sdc.SetBrush(*wxBLACK_BRUSH);
      sdc.SetPen(*wxBLACK_PEN);
      sdc.Clear();

      if (b_overlap) {
        int lines = m_ptoolbar->GetLineCount();
        for (int i = 1; i <= lines; i++) {
          if (m_ptoolbar->IsVertical()) {
            wxSize barsize(tool_size.x, visibleSize.y);
            if (i == lines && i > 1) {
              int toolsInLastLine = toolCount % m_ptoolbar->GetMaxRows();
              if (toolsInLastLine == 0)
                toolsInLastLine = m_ptoolbar->GetMaxRows();
              int emptySpace = (m_ptoolbar->GetMaxRows() - toolsInLastLine);
              barsize.y -=
                  emptySpace * (tool_size.y + m_style->GetToolSeparation());
            }
            if (i == lines) {
              // Also do grabber here, since it is to the right of the last
              // line.
              wxRect grabMask(upperLeft, barsize);
              grabMask.width += m_style->GetIcon(_T("grabber")).GetWidth();
              grabMask.height = m_style->GetIcon(_T("grabber")).GetHeight();
              sdc.DrawRoundedRectangle(grabMask,
                                       m_style->GetToolbarCornerRadius());
            }
            sdc.DrawRoundedRectangle(upperLeft, barsize,
                                     m_style->GetToolbarCornerRadius());
            upperLeft.x += m_style->GetTopMargin() + tool_size.x;
          } else {
            wxSize barsize(visibleSize.x, tool_size.y);

            if (i == 1) {
              barsize.x += m_style->GetIcon(_T("grabber")).GetWidth();
            }
            if (i == lines && i > 1) {
              int toolsInLastLine = toolCount % m_ptoolbar->GetMaxCols();
              if (toolsInLastLine == 0)
                toolsInLastLine = m_ptoolbar->GetMaxCols();
              int emptySpace = (m_ptoolbar->GetMaxCols() - toolsInLastLine);
              barsize.x -=
                  emptySpace * (tool_size.x + m_style->GetToolSeparation());
            }

            sdc.DrawRoundedRectangle(upperLeft, barsize,
                                     m_style->GetToolbarCornerRadius());
            upperLeft.y += m_style->GetTopMargin() + tool_size.y;
          }
        }
      }  // b_overlap
      else {
        for (wxToolBarToolsList::compatibility_iterator node =
                 m_ptoolbar->m_tools.GetFirst();
             node; node = node->GetNext()) {
          wxToolBarToolBase *tool = node->GetData();
          ocpnToolBarTool *tools = (ocpnToolBarTool *)tool;

          sdc.DrawRoundedRectangle(tools->m_x, tools->m_y, tool_size.x,
                                   tool_size.y,
                                   m_style->GetToolbarCornerRadius());
        }
      }

#ifndef __WXQT__
      if (shape.GetWidth() && shape.GetHeight())
        SetShape(wxRegion(shape, *wxWHITE, 10));
#endif
    } else {
#if !defined(__WXMAC__) && !defined(__OCPN__ANDROID__)
      if (m_cornerRadius) {
        wxBitmap m_MaskBmp = wxBitmap(GetSize().x, GetSize().y);
        wxMemoryDC sdc(m_MaskBmp);
        sdc.SetBackground(*wxWHITE_BRUSH);
        sdc.Clear();
        sdc.SetBrush(*wxBLACK_BRUSH);
        sdc.SetPen(*wxBLACK_PEN);
        sdc.DrawRoundedRectangle(0, 0, m_MaskBmp.GetWidth(),
                                 m_MaskBmp.GetHeight(), m_cornerRadius);
        sdc.SelectObject(wxNullBitmap);
        SetShape(wxRegion(m_MaskBmp, *wxWHITE, 0));
      }
#endif
    }
  }
}

void ocpnFloatingToolbarDialog::OnToolLeftClick(wxCommandEvent &event) {
  // Since Dialog events don't propagate automatically, we send it explicitly
  // (instead of relying on event.Skip()). Send events up the window hierarchy

  m_pparent->GetEventHandler()->AddPendingEvent(event);
#ifndef __WXQT__
  gFrame->Raise();
#endif
}

ocpnToolBarSimple *ocpnFloatingToolbarDialog::GetToolbar() {
  if (!m_ptoolbar) {
    m_ptoolbar = CreateNewToolbar();
  }

  return m_ptoolbar;
}

ocpnToolBarSimple *ocpnFloatingToolbarDialog::CreateNewToolbar() {
  long winstyle = wxNO_BORDER | wxTB_FLAT;
  winstyle |= m_orient;

  m_ptoolbar = new ocpnToolBarSimple(this, -1, wxPoint(-1, -1), wxSize(-1, -1),
                                     winstyle);

  m_ptoolbar->SetBackgroundColour(GetGlobalColor(_T("GREY2")));
  m_ptoolbar->ClearBackground();
  m_ptoolbar->SetToggledBackgroundColour(GetGlobalColor(_T("GREY1")));
  m_ptoolbar->SetColorScheme(m_cs);
  m_ptoolbar->EnableRolloverBitmaps(GetEnableRolloverBitmaps());

  return m_ptoolbar;
}

void ocpnFloatingToolbarDialog::DestroyToolBar() {
  g_toolbarConfig = GetToolConfigString();

  if (m_ptoolbar) {
    m_ptoolbar->ClearTools();
    delete m_ptoolbar;  //->Destroy();
    m_ptoolbar = NULL;
  }

  if (!m_destroyTimer.IsRunning()) {
    m_destroyGrabber = m_pRecoverwin;
    m_pRecoverwin = NULL;
    m_destroyTimer.Start(
        5, wxTIMER_ONE_SHOT);  //  Destor the unneeded recovery grabber
  }

  m_Items.clear();
}

#include "s52plib.h"
#include "compass.h"
#include "chartdb.h"

extern bool g_bAllowShowScaled;
extern bool g_bTrackActive;
extern s52plib *ps52plib;

ocpnToolBarSimple *ocpnFloatingToolbarDialog::CreateMyToolbar() {
  return NULL;
#if 0

    ocpnToolBarSimple *tb = GetToolbar();
    if( !tb )
        return 0;

    ocpnCompass *pCompass = NULL;
    ChartCanvas *parentCanvas = dynamic_cast<ChartCanvas *>( GetParent() );
    if(parentCanvas)
        pCompass = parentCanvas->GetCompass();

    if(pCompass)
        SetGeometry(pCompass->IsShown(), pCompass->GetRect());
    else
        SetGeometry(false, wxRect(0,0,1,1));

    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

    wxString tipString;
    wxToolBarToolBase* newtool;

    CheckAndAddPlugInTool( tb );
    tipString = wxString( _("Zoom In") ) << _T(" (+)");
    if( _toolbarConfigMenuUtil( ID_ZOOMIN, tipString ) )
        tb->AddTool( ID_ZOOMIN, _T("zoomin"),
            style->GetToolIcon( _T("zoomin"), TOOLICON_NORMAL ), tipString, wxITEM_NORMAL );

    CheckAndAddPlugInTool( tb );
    tipString = wxString( _("Zoom Out") ) << _T(" (-)");
    if( _toolbarConfigMenuUtil( ID_ZOOMOUT, tipString ) )
        tb->AddTool( ID_ZOOMOUT, _T("zoomout"),
            style->GetToolIcon( _T("zoomout"), TOOLICON_NORMAL ), tipString, wxITEM_NORMAL );

    CheckAndAddPlugInTool( tb );
    tipString = wxString( _("Shift to Larger Scale Chart") ) << _T(" (F7)");
    if( _toolbarConfigMenuUtil( ID_STKDN, tipString ) ) {
        newtool = tb->AddTool( ID_STKDN, _T("scin"),
                style->GetToolIcon( _T("scin"), TOOLICON_NORMAL ), tipString, wxITEM_NORMAL );
        newtool->Enable( m_toolbar_scale_tools_shown );
    }

    CheckAndAddPlugInTool( tb );
    tipString = wxString( _("Shift to Smaller Scale Chart") ) << _T(" (F8)");
    if( _toolbarConfigMenuUtil( ID_STKUP, tipString ) ) {
        newtool = tb->AddTool( ID_STKUP, _T("scout"),
                style->GetToolIcon( _T("scout"), TOOLICON_NORMAL ), tipString, wxITEM_NORMAL );
        newtool->Enable( m_toolbar_scale_tools_shown );
    }

    CheckAndAddPlugInTool( tb );
    tipString = wxString( _("Create Route") ) << _T(" (Ctrl-R)");
    if( _toolbarConfigMenuUtil( ID_ROUTE, tipString ) )
        tb->AddTool( ID_ROUTE, _T("route"),
            style->GetToolIcon( _T("route"), TOOLICON_NORMAL ),
            style->GetToolIcon( _T("route"), TOOLICON_TOGGLED ), wxITEM_CHECK, tipString );

    CheckAndAddPlugInTool( tb );
    tipString = wxString( _("Auto Follow") ) << _T(" (F2)");
    if( _toolbarConfigMenuUtil( ID_FOLLOW, tipString ) )
        tb->AddTool( ID_FOLLOW, _T("follow"),
            style->GetToolIcon( _T("follow"), TOOLICON_NORMAL ),
            style->GetToolIcon( _T("follow"), TOOLICON_TOGGLED ), wxITEM_CHECK, tipString );

    CheckAndAddPlugInTool( tb );
    tipString = _("Options");
    if( _toolbarConfigMenuUtil( ID_SETTINGS, tipString ) )
        tb->AddTool( ID_SETTINGS, _T("settings"),
            style->GetToolIcon( _T("settings"), TOOLICON_NORMAL ), tipString, wxITEM_NORMAL );

    CheckAndAddPlugInTool( tb );
    bool gs = false;
    if (ps52plib)
        gs = ps52plib->GetShowS57Text();

    if (gs)
        tipString = wxString( _("Hide ENC text") ) << _T(" (T)");
    else
        tipString = wxString( _("Show ENC text") ) << _T(" (T)");

    if( _toolbarConfigMenuUtil( ID_ENC_TEXT, tipString ) )
        tb->AddTool( ID_ENC_TEXT, _T("text"),
            style->GetToolIcon( _T("text"), TOOLICON_NORMAL ),
            style->GetToolIcon( _T("text"), TOOLICON_TOGGLED ), wxITEM_CHECK, tipString );

    m_pTBAISTool = NULL;
    CheckAndAddPlugInTool( tb );
    tipString = _("Hide AIS Targets");          // inital state is on
    if( _toolbarConfigMenuUtil( ID_AIS, tipString ) )
        m_pTBAISTool = tb->AddTool( ID_AIS, _T("AIS"), style->GetToolIcon( _T("AIS"), TOOLICON_NORMAL ),
                                  style->GetToolIcon( _T("AIS"), TOOLICON_DISABLED ),
                                  wxITEM_NORMAL, tipString );

    CheckAndAddPlugInTool( tb );
    tipString = _("Show Currents");
    if( _toolbarConfigMenuUtil( ID_CURRENT, tipString ) )
        tb->AddTool( ID_CURRENT, _T("current"),
            style->GetToolIcon( _T("current"), TOOLICON_NORMAL ), tipString, wxITEM_CHECK );

    CheckAndAddPlugInTool( tb );
    tipString = _("Show Tides");
    if( _toolbarConfigMenuUtil( ID_TIDE, tipString ) )
        tb->AddTool( ID_TIDE, _T("tide"),
            style->GetToolIcon( _T("tide"), TOOLICON_NORMAL ), tipString, wxITEM_CHECK );

    CheckAndAddPlugInTool( tb );
    tipString = _("Print Chart");
    if( _toolbarConfigMenuUtil( ID_PRINT, tipString ) )
        tb->AddTool( ID_PRINT, _T("print"),
            style->GetToolIcon( _T("print"), TOOLICON_NORMAL ), tipString, wxITEM_NORMAL );

    CheckAndAddPlugInTool( tb );
    tipString = _("Route & Mark Manager");
    if( _toolbarConfigMenuUtil( ID_ROUTEMANAGER, tipString ) )
        tb->AddTool( ID_ROUTEMANAGER,
            _T("route_manager"), style->GetToolIcon( _T("route_manager"), TOOLICON_NORMAL ),
            tipString, wxITEM_NORMAL );

    CheckAndAddPlugInTool( tb );
    tipString = _("Enable Tracking");
    if( _toolbarConfigMenuUtil( ID_TRACK, tipString ) )
        tb->AddTool( ID_TRACK, _T("track"),
            style->GetToolIcon( _T("track"), TOOLICON_NORMAL ),
            style->GetToolIcon( _T("track"), TOOLICON_TOGGLED ), wxITEM_CHECK, tipString );

    CheckAndAddPlugInTool( tb );
    tipString = wxString( _("Change Color Scheme") ) << _T(" (F5)");
    if( _toolbarConfigMenuUtil( ID_COLSCHEME, tipString ) ){
        tb->AddTool( ID_COLSCHEME,
            _T("colorscheme"), style->GetToolIcon( _T("colorscheme"), TOOLICON_NORMAL ),
            tipString, wxITEM_NORMAL );
        tb->SetToolTooltipHiViz( ID_COLSCHEME, true );  // cause the Tooltip to always be visible, whatever
                                                        //  the colorscheme
    }

    CheckAndAddPlugInTool( tb );
    tipString = _("About OpenCPN");
    if( _toolbarConfigMenuUtil( ID_ABOUT, tipString ) )
        tb->AddTool( ID_ABOUT, _T("help"),
            style->GetToolIcon( _T("help"), TOOLICON_NORMAL ), tipString, wxITEM_NORMAL );

    //      Add any PlugIn toolbar tools that request default positioning
    AddDefaultPositionPlugInTools( tb );

    //  And finally add the MOB tool
    tipString = wxString( _("Drop MOB Marker") ) << _(" (Ctrl-Space)");
    if( _toolbarConfigMenuUtil( ID_MOB, tipString ) )
        tb->AddTool( ID_MOB, _T("mob_btn"),
                     style->GetToolIcon( _T("mob_btn"), TOOLICON_NORMAL ), tipString, wxITEM_NORMAL );


// Realize() the toolbar
    style->Unload();
    Realize();

//      Set up the toggle states

    if( parentCanvas ) {
        //  Re-establish toggle states
        tb->ToggleTool( ID_CURRENT, parentCanvas->GetbShowCurrent() );
        tb->ToggleTool( ID_TIDE, parentCanvas->GetbShowTide() );
        tb->ToggleTool( ID_FOLLOW, parentCanvas->m_bFollow );
    }

    if( ( ps52plib ) ){
        if( ps52plib->m_bOK )
            tb->ToggleTool( ID_ENC_TEXT, ps52plib->GetShowS57Text() );
    }

    wxString initiconName;
    if( parentCanvas->GetShowAIS() ) {
        if (g_bAllowShowScaled){
            if(!parentCanvas->GetAttenAIS())
                tb->SetToolShortHelp( ID_AIS, _("Attenuate less critical AIS targets") );
            else
                tb->SetToolShortHelp( ID_AIS, _("Hide AIS Targets") );
        }
        else
            tb->SetToolShortHelp( ID_AIS, _("Hide AIS Targets") );
        initiconName = _T("AIS");
    }
    else {
        tb->SetToolShortHelp( ID_AIS, _("Show AIS Targets") );
        initiconName = _T("AIS_Disabled");
    }
    tb->SetToolNormalBitmapEx( m_pTBAISTool, initiconName );
    m_tblastAISiconName = initiconName;

    tb->ToggleTool( ID_TRACK, g_bTrackActive );

    //  Set PlugIn tool toggle states
    ArrayOfPlugInToolbarTools tool_array = g_pi_manager->GetPluginToolbarToolArray();
    for( unsigned int i = 0; i < tool_array.GetCount(); i++ ) {
        PlugInToolbarToolContainer *pttc = tool_array.Item( i );
        if( !pttc->b_viz )
            continue;

        if( pttc->kind == wxITEM_CHECK )
            tb->ToggleTool( pttc->id, pttc->b_toggle );
    }


    // TODO SetStatusBarPane( -1 );                   // don't show help on status bar

    return tb;
#endif
}

bool ocpnFloatingToolbarDialog::CheckAndAddPlugInTool(ocpnToolBarSimple *tb) {
  if (!g_pi_manager) return false;

  // We only add plugin tools on toolbar associated with canvas #0, the primary.
  // Except, if in gMUI mode, we allow no plugins ever on per-canvas toolbars.

  ChartCanvas *parentCanvas = dynamic_cast<ChartCanvas *>(GetParent());
  if (parentCanvas) {
    if (g_useMUI) {
      return false;
    } else if (!parentCanvas->IsPrimaryCanvas())
      return false;
  }

  bool bret = false;
  int n_tools = tb->GetToolsCount();

  //    Walk the PlugIn tool spec array, checking the requested position
  //    If a tool has been requested by a plugin at this position, add it
  ArrayOfPlugInToolbarTools tool_array =
      g_pi_manager->GetPluginToolbarToolArray();

  for (unsigned int i = 0; i < tool_array.GetCount(); i++) {
    PlugInToolbarToolContainer *pttc = tool_array.Item(i);
    if (pttc->position == n_tools) {
      wxBitmap *ptool_bmp;

      switch (m_cs) {
        case GLOBAL_COLOR_SCHEME_DAY:
          ptool_bmp = pttc->bitmap_day;
          ;
          break;
        case GLOBAL_COLOR_SCHEME_DUSK:
          ptool_bmp = pttc->bitmap_dusk;
          break;
        case GLOBAL_COLOR_SCHEME_NIGHT:
          ptool_bmp = pttc->bitmap_night;
          break;
        default:
          ptool_bmp = pttc->bitmap_day;
          ;
          break;
      }

      wxToolBarToolBase *tool =
          tb->AddTool(pttc->id, wxString(pttc->label), *(ptool_bmp),
                      wxString(pttc->shortHelp), pttc->kind);

      tb->SetToolBitmapsSVG(pttc->id, pttc->pluginNormalIconSVG,
                            pttc->pluginRolloverIconSVG,
                            pttc->pluginToggledIconSVG);

      bret = true;
    }
  }

  //    If we added a tool, call again (recursively) to allow for adding
  //    adjacent tools
  if (bret)
    while (CheckAndAddPlugInTool(tb)) { /* nothing to do */
    }

  return bret;
}

bool ocpnFloatingToolbarDialog::AddDefaultPositionPlugInTools(
    ocpnToolBarSimple *tb) {
  // We only add plugin tools on toolbar associated with canvas #0, the primary.
  // Except, if in gMUI mode, we allow no plugins ever on per-canvas toolbars.
  ChartCanvas *parentCanvas = dynamic_cast<ChartCanvas *>(GetParent());
  if (parentCanvas) {
    if (g_useMUI) {
      return false;
    }
    if (!parentCanvas->IsPrimaryCanvas()) return false;
  }

  if (!g_pi_manager) return false;

  bool bret = false;

  //    Walk the PlugIn tool spec array, checking the requested position
  //    If a tool has been requested by a plugin at this position, add it
  ArrayOfPlugInToolbarTools tool_array =
      g_pi_manager->GetPluginToolbarToolArray();

  for (unsigned int i = 0; i < tool_array.GetCount(); i++) {
    PlugInToolbarToolContainer *pttc = tool_array.Item(i);

    //      Tool is currently tagged as invisible
    if (!pttc->b_viz) continue;

    if (pttc->position == -1)  // PlugIn has requested default positioning
    {
      wxBitmap *ptool_bmp;

      switch (m_cs) {
        case GLOBAL_COLOR_SCHEME_DAY:
          ptool_bmp = pttc->bitmap_day;
          break;
        case GLOBAL_COLOR_SCHEME_DUSK:
          ptool_bmp = pttc->bitmap_dusk;
          break;
        case GLOBAL_COLOR_SCHEME_NIGHT:
          ptool_bmp = pttc->bitmap_night;
          break;
        default:
          ptool_bmp = pttc->bitmap_day;
          break;
      }

      wxToolBarToolBase *tool =
          tb->AddTool(pttc->id, wxString(pttc->label), *(ptool_bmp),
                      wxString(pttc->shortHelp), pttc->kind);

      tb->SetToolBitmapsSVG(pttc->id, pttc->pluginNormalIconSVG,
                            pttc->pluginRolloverIconSVG,
                            pttc->pluginToggledIconSVG);

      bret = true;
    }
  }
  return bret;
}

void ocpnFloatingToolbarDialog::EnableRolloverBitmaps(bool bEnable) {
  m_enableRolloverBitmaps = bEnable;
  if (m_ptoolbar) m_ptoolbar->EnableRolloverBitmaps(bEnable);
}

//----------------------------------------------------------------------------
// Toolbar Tooltip Popup Window Definition
//----------------------------------------------------------------------------
class ToolTipWin : public wxFrame {
public:
  ToolTipWin(wxWindow *parent);
  ~ToolTipWin();

  void OnPaint(wxPaintEvent &event);

  void SetColorScheme(ColorScheme cs);
  void SetString(wxString &s) { m_string = s; }
  void SetPosition(wxPoint pt) { m_position = pt; }
  void SetBitmap(void);

  void SetHiviz(bool hiviz) { m_hiviz = hiviz; }

  wxSize GetRenderedSize(void);

private:
  wxString m_string;
  wxSize m_size;
  wxPoint m_position;
  wxBitmap *m_pbm;
  wxColour m_back_color;
  wxColour m_text_color;
  ColorScheme m_cs;
  bool m_hiviz;

  DECLARE_EVENT_TABLE()
};
//-----------------------------------------------------------------------
//
//    Toolbar Tooltip window implementation
//
//-----------------------------------------------------------------------
BEGIN_EVENT_TABLE(ToolTipWin, wxFrame)
EVT_PAINT(ToolTipWin::OnPaint)

END_EVENT_TABLE()

// Define a constructor
ToolTipWin::ToolTipWin(wxWindow *parent)
    : wxFrame(parent, wxID_ANY, _T(""), wxPoint(0, 0), wxSize(1, 1),
              wxNO_BORDER | wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR) {
  m_pbm = NULL;

  m_back_color = GetGlobalColor(_T ( "UIBCK" ));
  m_text_color = GetGlobalColor(_T ( "UITX1" ));

  SetBackgroundStyle(wxBG_STYLE_CUSTOM);
  SetBackgroundColour(m_back_color);
  m_cs = GLOBAL_COLOR_SCHEME_RGB;

  Hide();
}

ToolTipWin::~ToolTipWin() { delete m_pbm; }

void ToolTipWin::SetColorScheme(ColorScheme cs) {
  m_back_color = GetGlobalColor(_T ( "UIBCK" ));
  m_text_color = GetGlobalColor(_T ( "UITX1" ));

#ifndef __WXOSX__
  m_text_color = FontMgr::Get().GetFontColor(_("ToolTips"));
  // assume black is the default
  if (m_text_color == *wxBLACK) m_text_color = GetGlobalColor(_T ( "UITX1" ));
#endif

  m_cs = cs;
}

wxSize ToolTipWin::GetRenderedSize(void) {
  int h, w;
  wxSize sz;

  wxScreenDC cdc;

  wxFont *plabelFont = FontMgr::Get().GetFont(_("ToolTips"));
  cdc.GetTextExtent(m_string, &w, &h, NULL, NULL, plabelFont);

  sz.x = w + 8;
  sz.y = h + 4;

  return sz;
}

void ToolTipWin::SetBitmap() {
  int h, w;

  wxScreenDC cdc;

  wxFont *plabelFont = FontMgr::Get().GetFont(_("ToolTips"));
  cdc.GetTextExtent(m_string, &w, &h, NULL, NULL, plabelFont);

  m_size.x = w + 8;
  m_size.y = h + 4;

  wxMemoryDC mdc;

  delete m_pbm;
  m_pbm = new wxBitmap(m_size.x, m_size.y, -1);
  mdc.SelectObject(*m_pbm);

  wxPen pborder(m_text_color);
  wxBrush bback(m_back_color);
  mdc.SetPen(pborder);
  mdc.SetBrush(bback);

  if (m_hiviz) {
    if ((m_cs == GLOBAL_COLOR_SCHEME_DUSK) ||
        (m_cs == GLOBAL_COLOR_SCHEME_NIGHT)) {
      wxBrush hv_back(wxColour(200, 200, 200));
      mdc.SetBrush(hv_back);
    }
  }
  mdc.DrawRectangle(0, 0, m_size.x, m_size.y);

  //    Draw the text
  mdc.SetFont(*plabelFont);
  mdc.SetTextForeground(m_text_color);
  mdc.SetTextBackground(m_back_color);

  mdc.DrawText(m_string, 4, 2);

  SetSize(m_position.x, m_position.y, m_size.x, m_size.y);
}

void ToolTipWin::OnPaint(wxPaintEvent &event) {
  int width, height;
  GetClientSize(&width, &height);
  wxPaintDC dc(this);

  if (m_string.Len()) {
    wxMemoryDC mdc;
    mdc.SelectObject(*m_pbm);
    dc.Blit(0, 0, width, height, &mdc, 0, 0);
  }
}

// ----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(ocpnToolBarSimple, wxControl)
EVT_SIZE(ocpnToolBarSimple::OnSize)
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

wxToolBarToolBase *ocpnToolBarSimple::CreateTool(
    int id, const wxString &label, const wxBitmap &bmpNormal,
    const wxBitmap &bmpDisabled, wxItemKind kind, wxObject *clientData,
    const wxString &shortHelp, const wxString &longHelp) {
  if (m_style->NativeToolIconExists(label)) {
    return new ocpnToolBarTool(this, id, label, bmpNormal, bmpDisabled, kind,
                               clientData, shortHelp, longHelp);
  } else {
    wxString testToolname = g_pi_manager->GetToolOwnerCommonName(id);

    if (testToolname == _T("")) {  // Not a PlugIn tool...
      return new ocpnToolBarTool(this, id, bmpNormal, bmpDisabled, kind,
                                 clientData, shortHelp, longHelp);
    } else {
      return new ocpnToolBarTool(this, id, label, bmpNormal, bmpDisabled, kind,
                                 clientData, shortHelp, longHelp);
    }
  }
}

// ----------------------------------------------------------------------------
// ocpnToolBarSimple creation
// ----------------------------------------------------------------------------

void ocpnToolBarSimple::Init() {
  m_currentRowsOrColumns = 0;

  m_lastX = m_lastY = 0;

  m_maxWidth = m_maxHeight = 0;

  m_pressedTool = m_currentTool = -1;

  m_xPos = m_yPos = wxDefaultCoord;

  m_style = g_StyleManager->GetCurrentStyle();

  m_defaultWidth = 16;
  m_defaultHeight = 15;

  m_toggle_bg_color = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
  m_toolOutlineColour.Set(_T("BLACK"));
  m_pToolTipWin = NULL;
  m_last_ro_tool = NULL;

  m_btoolbar_is_zooming = false;
  m_sizefactor = 1.0f;

  m_last_plugin_down_id = -1;
  m_leftDown = false;
  m_nShowTools = 0;
  m_btooltip_show = false;
#ifndef __OCPN__ANDROID__
  EnableTooltips();
#endif
  m_tbenableRolloverBitmaps = false;
}

wxToolBarToolBase *ocpnToolBarSimple::DoAddTool(
    int id, const wxString &label, const wxBitmap &bitmap,
    const wxBitmap &bmpDisabled, wxItemKind kind, const wxString &shortHelp,
    const wxString &longHelp, wxObject *clientData, wxCoord xPos,
    wxCoord yPos) {
  // rememeber the position for DoInsertTool()
  m_xPos = xPos;
  m_yPos = yPos;

  InvalidateBestSize();
  return InsertTool(GetToolsCount(), id, label, bitmap, bmpDisabled, kind,
                    shortHelp, longHelp, clientData);
}

///

wxToolBarToolBase *ocpnToolBarSimple::AddTool(
    int toolid, const wxString &label, const wxBitmap &bitmap,
    const wxBitmap &bmpDisabled, wxItemKind kind, const wxString &shortHelp,
    const wxString &longHelp, wxObject *data) {
  InvalidateBestSize();
  ocpnToolBarTool *tool = (ocpnToolBarTool *)InsertTool(
      GetToolsCount(), toolid, label, bitmap, bmpDisabled, kind, shortHelp,
      longHelp, data);
  return tool;
}

wxToolBarToolBase *ocpnToolBarSimple::InsertTool(
    size_t pos, int id, const wxString &label, const wxBitmap &bitmap,
    const wxBitmap &bmpDisabled, wxItemKind kind, const wxString &shortHelp,
    const wxString &longHelp, wxObject *clientData) {
  wxCHECK_MSG(pos <= GetToolsCount(), (wxToolBarToolBase *)NULL,
              _T("invalid position in wxToolBar::InsertTool()"));

  wxToolBarToolBase *tool = CreateTool(id, label, bitmap, bmpDisabled, kind,
                                       clientData, shortHelp, longHelp);

  if (!InsertTool(pos, tool)) {
    delete tool;

    return NULL;
  }

  return tool;
}

wxToolBarToolBase *ocpnToolBarSimple::InsertTool(size_t pos,
                                                 wxToolBarToolBase *tool) {
  wxCHECK_MSG(pos <= GetToolsCount(), (wxToolBarToolBase *)NULL,
              _T("invalid position in wxToolBar::InsertTool()"));

  if (!tool || !DoInsertTool(pos, tool)) {
    return NULL;
  }

  m_tools.Insert(pos, tool);
  m_nShowTools++;

  return tool;
}

bool ocpnToolBarSimple::DoInsertTool(size_t WXUNUSED(pos),
                                     wxToolBarToolBase *toolBase) {
  ocpnToolBarTool *tool = (ocpnToolBarTool *)toolBase;

  // Check if the plugin is inserting same-named tools. Make sure they have
  // different names, otherwise the style manager cannot differentiate between
  // them.
  if (tool->isPluginTool) {
    for (unsigned int i = 0; i < GetToolsCount(); i++) {
      if (tool->GetToolname() ==
          ((ocpnToolBarTool *)m_tools.Item(i)->GetData())->GetToolname()) {
        tool->toolname << _T("1");
      }
    }
  }

  tool->m_x = m_xPos;
  if (tool->m_x == wxDefaultCoord) tool->m_x = m_style->GetLeftMargin();

  tool->m_y = m_yPos;
  if (tool->m_y == wxDefaultCoord) tool->m_y = m_style->GetTopMargin();

  if (tool->IsButton()) {
    tool->SetSize(GetToolSize());

    // Calculate reasonable max size in case Layout() not called
    if ((tool->m_x + tool->GetNormalBitmap().GetWidth() +
         m_style->GetLeftMargin()) > m_maxWidth)
      m_maxWidth =
          (wxCoord)((tool->m_x + tool->GetWidth() + m_style->GetLeftMargin()));

    if ((tool->m_y + tool->GetNormalBitmap().GetHeight() +
         m_style->GetTopMargin()) > m_maxHeight)
      m_maxHeight =
          (wxCoord)((tool->m_y + tool->GetHeight() + m_style->GetTopMargin()));
  }

  else if (tool->IsControl()) {
    tool->SetSize(tool->GetControl()->GetSize());
  }

  tool->b_hilite = false;

  return true;
}

bool ocpnToolBarSimple::DoDeleteTool(size_t WXUNUSED(pos),
                                     wxToolBarToolBase *tool) {
  // VZ: didn't test whether it works, but why not...
  tool->Detach();

  if (m_last_ro_tool == tool) m_last_ro_tool = NULL;

  Refresh(false);

  return true;
}

bool ocpnToolBarSimple::Create(wxWindow *parent, wxWindowID id,
                               const wxPoint &pos, const wxSize &size,
                               long style, const wxString &name) {
  if (!wxWindow::Create(parent, id, pos, size, style, name)) return false;

  // Set it to grey (or other 3D face colour)
  SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));

  if (GetWindowStyleFlag() & wxTB_VERTICAL) {
    m_lastX = 7;
    m_lastY = 3;

    m_maxRows = 32000;  // a lot
    m_maxCols = 1;
  } else {
    m_lastX = 3;
    m_lastY = 7;

    m_maxRows = 1;
    m_maxCols = 32000;  // a lot
  }

  SetCursor(*wxSTANDARD_CURSOR);

  m_tooltip_timer.SetOwner(this, TOOLTIPON_TIMER);
  m_tooltipoff_timer.SetOwner(this, TOOLTIPOFF_TIMER);
  m_tooltip_off = 3000;

  m_tbenableRolloverBitmaps = false;

  return true;
}

ocpnToolBarSimple::~ocpnToolBarSimple() {
  if (m_pToolTipWin) {
    m_pToolTipWin->Destroy();
    m_pToolTipWin = NULL;
  }
}

void ocpnToolBarSimple::EnableTooltips() {
#ifndef __OCPN__ANDROID__
  m_btooltip_show = true;
#endif
}

void ocpnToolBarSimple::DisableTooltips() {
#ifndef __OCPN__ANDROID__
  ocpnToolBarSimple::m_btooltip_show = false;
#endif
}

void ocpnToolBarSimple::KillTooltip() {
  m_btooltip_show = false;

  if (m_pToolTipWin) {
    m_pToolTipWin->Hide();
    m_pToolTipWin->Destroy();
    m_pToolTipWin = NULL;
  }
  m_tooltip_timer.Stop();

  /*
      if( m_last_ro_tool ) {
          if( m_last_ro_tool->IsEnabled() ) {
              if( m_last_ro_tool->IsToggled() ) {
                  if(
     m_style->NativeToolIconExists(m_last_ro_tool->GetToolname()) ) {
                      m_last_ro_tool->SetNormalBitmap( m_style->GetToolIcon(
     m_last_ro_tool->GetToolname(), TOOLICON_TOGGLED ) );
                  }
                  else{
                      m_last_ro_tool->SetNormalBitmap(m_style->BuildPluginIcon(
     m_last_ro_tool->pluginNormalIcon, TOOLICON_TOGGLED ));
                  }

              }
              else {
                  if(
     m_style->NativeToolIconExists(m_last_ro_tool->GetToolname()) ) {
                      m_last_ro_tool->SetNormalBitmap( m_style->GetToolIcon(
     m_last_ro_tool->GetToolname(), TOOLICON_NORMAL ) );
                  }
                  else{
                      m_last_ro_tool->SetNormalBitmap(m_style->BuildPluginIcon(
     m_last_ro_tool->pluginNormalIcon, TOOLICON_NORMAL ));
                  }
              }
          }
      }
  */
  gFrame->Raise();
  gFrame->GetFocusCanvas()->TriggerDeferredFocus();
}

void ocpnToolBarSimple::HideTooltip() {
#ifndef __OCPN__ANDROID__
  if (m_pToolTipWin) {
    m_pToolTipWin->Hide();
  }
#endif
}

void ocpnToolBarSimple::SetColorScheme(ColorScheme cs) {
#ifndef __OCPN__ANDROID__
  if (m_pToolTipWin) {
    m_pToolTipWin->Destroy();
    m_pToolTipWin = NULL;
  }
#endif
  m_toolOutlineColour = GetGlobalColor(_T("UIBDR"));

  m_currentColorScheme = cs;
}

bool ocpnToolBarSimple::Realize() {
  if (IsVertical())
    m_style->SetOrientation(wxTB_VERTICAL);
  else
    m_style->SetOrientation(wxTB_HORIZONTAL);

  wxSize toolSize = wxSize(-1, -1);
  int separatorSize = m_style->GetToolSeparation() * m_sizefactor;
  int topMargin = m_style->GetTopMargin() * m_sizefactor;
  int leftMargin = m_style->GetLeftMargin() * m_sizefactor;

  m_currentRowsOrColumns = 0;
  m_LineCount = 1;
  m_lastX = leftMargin;
  m_lastY = topMargin;
  m_maxWidth = 0;
  m_maxHeight = 0;

  ocpnToolBarTool *lastTool = NULL;
  bool firstNode = true;
  wxToolBarToolsList::compatibility_iterator node = m_tools.GetFirst();

  int iNode = 0;

  while (node) {
    if (iNode >= m_nShowTools) break;

    ocpnToolBarTool *tool = (ocpnToolBarTool *)node->GetData();

    // Set the tool size to be the size of the first non-separator tool, usually
    // the first one
    if (toolSize.x == -1) {
      if (!tool->IsSeparator()) {
        toolSize.x = tool->m_width;
        toolSize.y = tool->m_height;
      }
    }

    tool->firstInLine = firstNode;
    tool->lastInLine = false;
    firstNode = false;

    tool->last_rect.width = 0;  // mark it invalid

    if (tool->IsSeparator()) {
      if (GetWindowStyleFlag() & wxTB_HORIZONTAL) {
        if (m_currentRowsOrColumns >= m_maxCols)
          m_lastY += separatorSize;
        else
          m_lastX += separatorSize;
      } else {
        if (m_currentRowsOrColumns >= m_maxRows)
          m_lastX += separatorSize;
        else
          m_lastY += separatorSize;
      }
    } else if (tool->IsButton()) {
      if (!IsVertical()) {
        if (m_currentRowsOrColumns >= m_maxCols) {
          tool->firstInLine = true;
          if (lastTool && m_LineCount > 1) lastTool->lastInLine = true;
          m_LineCount++;
          m_currentRowsOrColumns = 0;
          m_lastX = leftMargin;
          m_lastY += toolSize.y + topMargin;
        }
        tool->m_x = (wxCoord)m_lastX;
        tool->m_y = (wxCoord)m_lastY;

        tool->trect = wxRect(tool->m_x, tool->m_y, toolSize.x, toolSize.y);
        tool->trect.Inflate(separatorSize / 2, topMargin);

        m_lastX += toolSize.x + separatorSize;
      } else {
        if (m_currentRowsOrColumns >= m_maxRows) {
          tool->firstInLine = true;
          if (lastTool) lastTool->lastInLine = true;
          m_LineCount++;
          m_currentRowsOrColumns = 0;
          m_lastX += toolSize.x + leftMargin;
          m_lastY = topMargin;
        }
        tool->m_x = (wxCoord)m_lastX;
        tool->m_y = (wxCoord)m_lastY;

        tool->trect = wxRect(tool->m_x, tool->m_y, toolSize.x, toolSize.y);
        tool->trect.Inflate((separatorSize / 2), topMargin);

        m_lastY += toolSize.y + separatorSize;
      }
      m_currentRowsOrColumns++;
    } else if (tool->IsControl()) {
      tool->m_x = (wxCoord)(m_lastX);
      tool->m_y = (wxCoord)(m_lastY - (topMargin / 2));

      tool->trect =
          wxRect(tool->m_x, tool->m_y, tool->GetWidth(), tool->GetHeight());
      tool->trect.Inflate(separatorSize / 2, topMargin);

      wxSize s = tool->GetControl()->GetSize();
      m_lastX += s.x + separatorSize;
    }

    if (m_lastX > m_maxWidth) m_maxWidth = m_lastX;
    if (m_lastY > m_maxHeight) m_maxHeight = m_lastY;

    lastTool = tool;
    node = node->GetNext();
    iNode++;
  }
  if (lastTool && (m_LineCount > 1 || IsVertical()))
    lastTool->lastInLine = true;

  if (GetWindowStyleFlag() & wxTB_HORIZONTAL) {
    m_maxHeight += toolSize.y;
    m_maxHeight += m_style->GetBottomMargin();
  } else {
    m_maxWidth += toolSize.x;
    m_maxWidth += m_style->GetRightMargin() * m_sizefactor;
  }

  SetSize(m_maxWidth, m_maxHeight);
  SetMinSize(wxSize(m_maxWidth, m_maxHeight));

  return true;
}

// ----------------------------------------------------------------------------
// event handlers
// ----------------------------------------------------------------------------

void ocpnToolBarSimple::OnPaint(wxPaintEvent &WXUNUSED(event)) {
  wxPaintDC dc(this);
  PrepareDC(dc);

  wxRegion ru = GetUpdateRegion();
  wxRect upRect = ru.GetBox();

  static int count = 0;
  // Prevent reentry of OnPaint which would cause wxMemoryDC errors.
  if (count > 0) return;
  count++;

  for (wxToolBarToolsList::compatibility_iterator node = m_tools.GetFirst();
       node; node = node->GetNext()) {
    wxToolBarToolBase *tool = node->GetData();
    ocpnToolBarTool *tools = (ocpnToolBarTool *)tool;
    wxRect toolRect = tools->trect;

    if (toolRect.Intersects(upRect)) {
      if (tool->IsButton()) {
        DrawTool(dc, tool);
      } else if (tool->IsControl()) {
        if (tool->GetControl()->IsKindOf(CLASSINFO(wxStaticBitmap))) {
          wxStaticBitmap *psbm = (wxStaticBitmap *)tool->GetControl();
          ocpnToolBarTool *toolsimp = (ocpnToolBarTool *)tool;
          dc.DrawBitmap(psbm->GetBitmap(), toolsimp->m_x, toolsimp->m_y, false);
        }
      }
    }
  }

  count--;
}

void ocpnToolBarSimple::OnSize(wxSizeEvent &WXUNUSED(event)) {
  /*if( GetAutoLayout() )*/ Layout();
}

void ocpnToolBarSimple::OnKillFocus(wxFocusEvent &WXUNUSED(event)) {
  OnMouseEnter(m_pressedTool = m_currentTool = -1);
}

void ocpnToolBarSimple::OnToolTipTimerEvent(wxTimerEvent &event) {
  if (!gFrame)  // In case gFrame was already destroyed, but the toolbar still
                // exists (Which should not happen, ever.)
    return;

  // if( !gFrame->IsActive() )
  //    return;

  if (m_btooltip_show && IsShown() && m_pToolTipWin &&
      (!m_pToolTipWin->IsShown())) {
    if (m_last_ro_tool) {
      wxString s = m_last_ro_tool->GetShortHelp();

      if (s.Len()) {
        m_pToolTipWin->SetString(s);
        m_pToolTipWin->SetHiviz(m_last_ro_tool->m_btooltip_hiviz);

        wxPoint pos_in_toolbar(m_last_ro_tool->m_x, m_last_ro_tool->m_y);
        pos_in_toolbar.x += m_last_ro_tool->m_width + 2;

        m_pToolTipWin->Move(
            0, 0);  // workaround for gtk autocentre dialog behavior

        wxPoint screenPosition = ClientToScreen(pos_in_toolbar);
        wxPoint framePosition = gFrame->ScreenToClient(screenPosition);
        wxSize tipSize = m_pToolTipWin->GetRenderedSize();

        if ((framePosition.x + tipSize.x) > gFrame->GetSize().x)
          screenPosition.x -= (tipSize.x + m_last_ro_tool->m_width + 4);

        m_pToolTipWin->SetPosition(screenPosition);
        m_pToolTipWin->SetBitmap();
        m_pToolTipWin->Show();
#ifndef __WXOSX__
        gFrame->Raise();
#endif

#ifndef __OCPN__ANDROID__
        if (g_btouch) m_tooltipoff_timer.Start(m_tooltip_off, wxTIMER_ONE_SHOT);
#endif
      }
    }
  }
}

void ocpnToolBarSimple::OnToolTipOffTimerEvent(wxTimerEvent &event) {
  HideTooltip();
}

int s_dragx, s_dragy;
bool leftDown;

void ocpnToolBarSimple::OnMouseEvent(wxMouseEvent &event) {
#ifdef __OCPN__ANDROID__
  if (!event.IsButton()) return;
#endif

  wxCoord x, y;
  event.GetPosition(&x, &y);
  ocpnToolBarTool *tool = (ocpnToolBarTool *)FindToolForPosition(x, y);

#ifndef __OCPN__ANDROID__
  if (event.LeftDown()) {
    ///        CaptureMouse();
    s_dragx = x;
    s_dragy = y;
  }
  if (event.LeftUp()) {
    ///        if( HasCapture() ) ReleaseMouse();
  }
#endif

  static wxPoint s_pos_m_old;
  static bool s_drag;

  if (tool && (s_drag || tool->GetId() == ID_MASTERTOGGLE)) {
    wxPoint pos_m = ClientToScreen(wxPoint(x, y));
    if (event.LeftDown()) {
      s_pos_m_old = pos_m;
    }

    if (!g_btouch && event.Dragging()) {
      s_drag = true;
      wxPoint pos_old = GetScreenPosition();
      wxPoint pos_new = pos_old;

      int dx = abs(pos_m.x - s_pos_m_old.x);
      int dy = abs(pos_m.y - s_pos_m_old.y);
      if ((dx < 10) && (dy < 10)) {
        // s_pos_m_old = pos_m;
        // return;
      }

      pos_new.x += pos_m.x - s_pos_m_old.x;
      pos_new.y += pos_m.y - s_pos_m_old.y;

      ocpnFloatingToolbarDialog *parentFloatingToolBar =
          dynamic_cast<ocpnFloatingToolbarDialog *>(GetParent());
      // if( (dx > 4) || (dy > 4))
      parentFloatingToolBar->MoveDialogInScreenCoords(pos_new, pos_old);
      ocpnFloatingToolbarDialog *parent =
          wxDynamicCast(GetParent(), ocpnFloatingToolbarDialog);
      if (parent)
        parent->GetFrameRelativePosition(&g_maintoolbar_x, &g_maintoolbar_y);
      s_pos_m_old = pos_m;
      return;
    }

    if (event.LeftUp() && s_drag) {
      s_drag = false;
      return;
    }
  }

  if (tool && tool->IsButton() && IsShown()) {
    if (m_btooltip_show) {
      //    ToolTips
      if (NULL == m_pToolTipWin) {
        m_pToolTipWin = new ToolTipWin(gFrame /*GetParent()*/);
        m_pToolTipWin->SetColorScheme(m_currentColorScheme);
        m_pToolTipWin->Hide();
      }

      if (tool != m_last_ro_tool) {
        m_pToolTipWin->Hide();
      }

#ifndef __OCPN__ANDROID__
      if (!m_pToolTipWin->IsShown()) {
        if (!m_tooltip_timer.IsRunning()) {
          m_tooltip_timer.Start(m_one_shot, wxTIMER_ONE_SHOT);
        }
      }
#endif
    }
    //    Tool Rollover highlighting
    if (!g_btouch && m_tbenableRolloverBitmaps) {
      if (tool != m_last_ro_tool) {
        if (tool->IsEnabled()) {
          tool->rollover = true;
        }
        if (m_last_ro_tool) {
          if (m_last_ro_tool->IsEnabled()) {
            m_last_ro_tool->rollover = false;
          }
        }
        tool->bitmapOK = false;
        m_last_ro_tool = tool;

        Refresh(false);
      }
    }
  } else {
    //    Tooltips
    if (m_pToolTipWin && m_pToolTipWin->IsShown()) {
      m_pToolTipWin->Hide();
      KillTooltip();
      m_btooltip_show = true;
    }

    //    Remove Highlighting
    if (m_last_ro_tool && m_tbenableRolloverBitmaps) {
      if (m_last_ro_tool->IsEnabled()) {
        m_last_ro_tool->rollover = false;
        m_last_ro_tool->bitmapOK = false;
      }
      Refresh(false);
    }
  }

  m_last_ro_tool = tool;

  // allow smooth zooming while toolbutton is held down
  if (g_bsmoothpanzoom && !g_btouch) {
    ChartCanvas *pcc = NULL;
    ocpnFloatingToolbarDialog *parent =
        wxDynamicCast(GetParent(), ocpnFloatingToolbarDialog);
    if (parent) pcc = wxDynamicCast(parent->GetParent(), ChartCanvas);

    if (event.LeftUp() && m_btoolbar_is_zooming) {
      if (pcc) {
        pcc->StopMovement();
        m_btoolbar_is_zooming = false;
      }
      return;
    }

    if (event.LeftDown() && tool &&
        (tool->GetId() == ID_ZOOMIN || tool->GetId() == ID_ZOOMOUT)) {
      if (pcc) {
        pcc->ZoomCanvas(tool->GetId() == ID_ZOOMIN
                            ? g_plus_minus_zoom_factor
                            : 1.0 / g_plus_minus_zoom_factor,
                        false, false);
        m_btoolbar_is_zooming = true;
      }
      return;
    }
  }

  if (!tool) {
    if (m_currentTool > -1) {
      if (event.LeftIsDown()) SpringUpButton(m_currentTool);
      m_currentTool = -1;
      OnMouseEnter(-1);
    }

    wxMouseEvent *pev = (wxMouseEvent *)event.Clone();
    GetParent()->GetEventHandler()->AddPendingEvent(*pev);
    wxDELETE(pev);

    return;
  }

  if (!event.IsButton()) {
    if (tool->GetId() != m_currentTool) {
      // If the left button is kept down and moved over buttons,
      // press those buttons.
      if (event.LeftIsDown() && tool->IsEnabled()) {
        SpringUpButton(m_currentTool);

        if (tool->CanBeToggled()) {
          tool->Toggle();
        }

        DrawTool(tool);
      }

      m_currentTool = tool->GetId();
      OnMouseEnter(m_currentTool);
    }

    wxMouseEvent *pev = (wxMouseEvent *)event.Clone();
    GetParent()->GetEventHandler()->AddPendingEvent(*pev);
    wxDELETE(pev);

    return;
  }

  // Left button pressed.
  if (event.LeftIsDown()) m_leftDown = true;  // trigger on

  if (event.LeftDown() && tool->IsEnabled()) {
    if (tool->CanBeToggled()) {
      tool->Toggle();
      tool->bitmapOK = false;
    }

    DrawTool(tool);

    //        Look for PlugIn tools
    //        If found, make the callback.
    if (g_pi_manager) {
      ArrayOfPlugInToolbarTools tool_array =
          g_pi_manager->GetPluginToolbarToolArray();
      for (unsigned int i = 0; i < tool_array.GetCount(); i++) {
        PlugInToolbarToolContainer *pttc = tool_array[i];
        if (tool->GetId() == pttc->id) {
          opencpn_plugin_113 *ppi =
              dynamic_cast<opencpn_plugin_113 *>(pttc->m_pplugin);
          if (ppi) {
            ppi->OnToolbarToolDownCallback(pttc->id);
            m_last_plugin_down_id = pttc->id;
          }
        }
      }
    }
  } else if (event.RightDown()) {
    OnRightClick(tool->GetId(), x, y);
  }

  // Left Button Released.  Only this action confirms selection.
  // If the button is enabled and it is not a toggle tool and it is
  // in the pressed state, then raise the button and call OnLeftClick.
  //
  // Unfortunately, some touch screen drivers do not send "LeftIsDown" events.
  // Nor do they report "LeftIsDown" in any state.
  // c.f rPI "official" 7" panel.

  // So, for this logic, assume in touch mode that the m_leftDown flag may not
  // be set, and process the left-up event anyway.
  if (event.LeftUp() && tool->IsEnabled() && (m_leftDown || g_btouch)) {
    // Pass the OnLeftClick event to tool
    if (!OnLeftClick(tool->GetId(), tool->IsToggled()) &&
        tool->CanBeToggled()) {
      // If it was a toggle, and OnLeftClick says No Toggle allowed,
      // then change it back
      tool->Toggle();
      tool->bitmapOK = false;
    }

    DoPluginToolUp();
    m_leftDown = false;
  }

  wxMouseEvent *pev = (wxMouseEvent *)event.Clone();
  GetParent()->GetEventHandler()->AddPendingEvent(*pev);
  wxDELETE(pev);
  event.Skip();
}

// ----------------------------------------------------------------------------
// drawing
// ----------------------------------------------------------------------------

void ocpnToolBarSimple::DrawTool(wxToolBarToolBase *tool) {
  wxClientDC dc(this);
  DrawTool(dc, tool);
}

// NB! The current DrawTool code assumes that plugin tools are never disabled
// when they are present on the toolbar, since disabled plugins are removed.

void ocpnToolBarSimple::DrawTool(wxDC &dc, wxToolBarToolBase *toolBase) {
  ocpnToolBarTool *tool = (ocpnToolBarTool *)toolBase;
  PrepareDC(dc);

  wxPoint drawAt(tool->m_x, tool->m_y);
  wxBitmap bmp = wxNullBitmap;

  bool bNeedClear = !tool->bitmapOK;

  if (tool->bitmapOK) {
    if (tool->IsEnabled()) {
      bmp = tool->GetNormalBitmap();
      if (!bmp.IsOk()) {
        bmp =
            m_style->GetToolIcon(tool->GetToolname(), TOOLICON_NORMAL,
                                 tool->rollover, tool->m_width, tool->m_height);
        tool->SetNormalBitmap(bmp);
        tool->bitmapOK = true;
      }
    } else {
      bmp = tool->GetDisabledBitmap();
      if (!bmp.IsOk()) {
        bmp = m_style->GetToolIcon(tool->GetToolname(), TOOLICON_DISABLED,
                                   false, tool->m_width, tool->m_height);
        tool->SetDisabledBitmap(bmp);
        tool->bitmapOK = true;
      }
    }
  } else {
    if (tool->isPluginTool) {
      int toggleFlag = tool->IsToggled() ? TOOLICON_TOGGLED : TOOLICON_NORMAL;

      // First try getting the icon from an SVG definition.
      // If it is not found, try to see if it is available in the style
      // If not there, we build a new icon from the style BG and the (default)
      // plugin icon.

      wxString svgFile = tool->pluginNormalIconSVG;
      if (toggleFlag) {
        if (tool->pluginToggledIconSVG.Length())
          svgFile = tool->pluginToggledIconSVG;
      }
      if (tool->rollover) {
        if (tool->pluginRolloverIconSVG.Length())
          svgFile = tool->pluginRolloverIconSVG;
      }

      if (!svgFile.IsEmpty()) {  // try SVG
#ifdef ocpnUSE_SVG
        bmp = LoadSVG(svgFile, tool->m_width, tool->m_height);
        if (bmp.IsOk()) {
          bmp = m_style->BuildPluginIcon(bmp, toggleFlag, m_sizefactor);
        } else
          bmp =
              m_style->BuildPluginIcon(tool->pluginNormalIcon, TOOLICON_NORMAL);
#endif
      }

      if (!bmp.IsOk() || bmp.IsNull()) {
        if (m_style->NativeToolIconExists(tool->GetToolname())) {
          bmp = m_style->GetToolIcon(tool->GetToolname(), toggleFlag,
                                     tool->rollover, tool->m_width,
                                     tool->m_height);
        } else {
          bmp = wxNullBitmap;
        }

        if (bmp.IsNull()) {  // Tool icon not found
          if (tool->rollover) {
            bmp =
                m_style->BuildPluginIcon(tool->pluginRolloverIcon, toggleFlag);
            if (!bmp.IsOk()) {
              bmp =
                  m_style->BuildPluginIcon(tool->pluginNormalIcon, toggleFlag);
            }
          } else {
            bmp = m_style->BuildPluginIcon(tool->pluginNormalIcon, toggleFlag);
          }
          if (fabs(m_sizefactor - 1.0) > 0.01) {
            if (tool->m_width && tool->m_height) {
              wxImage scaled_image = bmp.ConvertToImage();
              bmp = wxBitmap(scaled_image.Scale(tool->m_width, tool->m_height,
                                                wxIMAGE_QUALITY_HIGH));
            }
          }
        }
      }
      tool->SetNormalBitmap(bmp);
      tool->bitmapOK = true;
    } else {  // Not a plugin tool
      bmp = tool->GetNormalBitmap();
      if (tool->IsEnabled()) {
        if (tool->IsToggled()) {
          if (!tool->bitmapOK) {
            if (m_style->NativeToolIconExists(tool->GetToolname())) {
              bmp = m_style->GetToolIcon(tool->GetToolname(), TOOLICON_TOGGLED,
                                         tool->rollover, tool->m_width,
                                         tool->m_height);
              tool->SetNormalBitmap(bmp);
            }
          }
        }

        else {
          if (!tool->bitmapOK) {
            if (m_style->NativeToolIconExists(tool->GetToolname())) {
              bmp = m_style->GetToolIcon(tool->GetIconName(), TOOLICON_NORMAL,
                                         tool->rollover, tool->m_width,
                                         tool->m_height);
              tool->SetNormalBitmap(bmp);
            }
          }
        }

        tool->bitmapOK = true;
      } else {
        bmp = m_style->GetToolIcon(tool->GetToolname(), TOOLICON_DISABLED,
                                   false, tool->m_width, tool->m_height);
        tool->SetDisabledBitmap(bmp);
        tool->bitmapOK = true;
      }
    }
  }

  if (tool->firstInLine) {
    m_style->DrawToolbarLineStart(bmp, m_sizefactor);
  }
  if (tool->lastInLine) {
    m_style->DrawToolbarLineEnd(bmp, m_sizefactor);
  }

  if (bmp.GetWidth() != m_style->GetToolSize().x ||
      bmp.GetHeight() != m_style->GetToolSize().y) {
    //        drawAt.x -= ( bmp.GetWidth() - m_style->GetToolSize().x ) / 2;
    //        drawAt.y -= ( bmp.GetHeight() - m_style->GetToolSize().y ) / 2;
  }

  //      Clear the last drawn tool if necessary
  if ((tool->last_rect.width &&
       (tool->last_rect.x != drawAt.x || tool->last_rect.y != drawAt.y)) ||
      bNeedClear) {
    wxBrush bb(GetGlobalColor(_T("GREY3")));
    dc.SetBrush(bb);
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(tool->last_rect.x, tool->last_rect.y,
                     tool->last_rect.width, tool->last_rect.height);
  }

  //  could cache this in the tool...
  //  A bit of a hack here.  We only scale tools if they are to be magnified
  //  globally
  if (0 /*m_sizefactor > 1.0*/) {
    wxImage scaled_image = bmp.ConvertToImage();
    wxBitmap sbmp = wxBitmap(scaled_image.Scale(tool->m_width, tool->m_height,
                                                wxIMAGE_QUALITY_HIGH));
    dc.DrawBitmap(sbmp, drawAt);
    tool->last_rect =
        wxRect(drawAt.x, drawAt.y, sbmp.GetWidth(), sbmp.GetHeight());

  } else {
    dc.DrawBitmap(bmp, drawAt);
    tool->last_rect =
        wxRect(drawAt.x, drawAt.y, bmp.GetWidth(), bmp.GetHeight());
  }
}

// ----------------------------------------------------------------------------
// toolbar geometry
// ----------------------------------------------------------------------------

wxToolBarToolBase *ocpnToolBarSimple::FindToolForPosition(wxCoord x,
                                                          wxCoord y) {
  wxToolBarToolsList::compatibility_iterator node = m_tools.GetFirst();
  while (node) {
    ocpnToolBarTool *tool = (ocpnToolBarTool *)node->GetData();
    if ((x >= tool->m_x) && (y >= tool->m_y) &&
        (x < (tool->m_x + tool->GetWidth())) &&
        (y < (tool->m_y + tool->GetHeight()))) {
      return tool;
    }

    node = node->GetNext();
  }

  return (wxToolBarToolBase *)NULL;
}

void ocpnToolBarSimple::InvalidateBitmaps() {
  wxToolBarToolsList::compatibility_iterator node = m_tools.GetFirst();
  while (node) {
    ocpnToolBarTool *tool = (ocpnToolBarTool *)node->GetData();
    tool->bitmapOK = false;
    node = node->GetNext();
  }
}

wxRect ocpnToolBarSimple::GetToolRect(int tool_id) {
  wxRect rect;
  wxToolBarToolBase *tool = FindById(tool_id);
  if (tool) {
    ocpnToolBarTool *otool = (ocpnToolBarTool *)tool;
    if (otool) rect = otool->trect;
  }

  return rect;
}

// ----------------------------------------------------------------------------
// tool state change handlers
// ----------------------------------------------------------------------------

void ocpnToolBarSimple::DoEnableTool(wxToolBarToolBase *tool,
                                     bool WXUNUSED(enable)) {
  ocpnToolBarTool *t = (ocpnToolBarTool *)tool;
  t->bitmapOK = false;
  DrawTool(tool);
}

void ocpnToolBarSimple::DoToggleTool(wxToolBarToolBase *tool,
                                     bool WXUNUSED(toggle)) {
  ocpnToolBarTool *t = (ocpnToolBarTool *)tool;
  t->bitmapOK = false;
  DrawTool(tool);
}

// Okay, so we've left the tool we're in ... we must check if the tool we're
// leaving was a 'sprung push button' and if so, spring it back to the up
// state.
void ocpnToolBarSimple::SpringUpButton(int id) {
  wxToolBarToolBase *tool = FindById(id);

  if (tool && tool->CanBeToggled()) {
    if (tool->IsToggled()) tool->Toggle();

    DrawTool(tool);
  }
}

// ----------------------------------------------------------------------------
// scrolling implementation
// ----------------------------------------------------------------------------

wxString ocpnToolBarSimple::GetToolShortHelp(int id) const {
  wxToolBarToolBase *tool = FindById(id);
  wxCHECK_MSG(tool, wxEmptyString, _T("no such tool"));

  return tool->GetShortHelp();
}

wxString ocpnToolBarSimple::GetToolLongHelp(int id) const {
  wxToolBarToolBase *tool = FindById(id);
  wxCHECK_MSG(tool, wxEmptyString, _T("no such tool"));

  return tool->GetLongHelp();
}

void ocpnToolBarSimple::SetToolShortHelp(int id, const wxString &help) {
  wxToolBarToolBase *tool = FindById(id);
  if (tool) {
    (void)tool->SetShortHelp(help);
  }
}

void ocpnToolBarSimple::SetToolLongHelp(int id, const wxString &help) {
  wxToolBarToolBase *tool = FindById(id);
  if (tool) {
    (void)tool->SetLongHelp(help);
  }
}

int ocpnToolBarSimple::GetToolPos(int id) const {
  size_t pos = 0;
  wxToolBarToolsList::compatibility_iterator node;

  for (node = m_tools.GetFirst(); node; node = node->GetNext()) {
    if (node->GetData()->GetId() == id) return pos;

    pos++;
  }

  return wxNOT_FOUND;
}
bool ocpnToolBarSimple::GetToolState(int id) const {
  wxToolBarToolBase *tool = FindById(id);
  wxCHECK_MSG(tool, false, _T("no such tool"));

  return tool->IsToggled();
}

bool ocpnToolBarSimple::GetToolEnabled(int id) const {
  wxToolBarToolBase *tool = FindById(id);
  wxCHECK_MSG(tool, false, _T("no such tool"));

  return tool->IsEnabled();
}

void ocpnToolBarSimple::ToggleTool(int id, bool toggle) {
  wxToolBarToolBase *tool = FindById(id);

  if (tool && tool->CanBeToggled() && tool->Toggle(toggle)) {
    DoToggleTool(tool, toggle);
    Refresh();
  }
}

wxObject *ocpnToolBarSimple::GetToolClientData(int id) const {
  wxToolBarToolBase *tool = FindById(id);
  return tool ? tool->GetClientData() : (wxObject *)NULL;
}

void ocpnToolBarSimple::SetToolClientData(int id, wxObject *clientData) {
  wxToolBarToolBase *tool = FindById(id);

  wxCHECK_RET(tool, _T("no such tool in wxToolBar::SetToolClientData"));

  tool->SetClientData(clientData);
}

void ocpnToolBarSimple::EnableTool(int id, bool enable) {
  wxToolBarToolBase *tool = FindById(id);
  if (tool) {
    if (tool->Enable(enable)) {
      DoEnableTool(tool, enable);
    }
  }

  ocpnFloatingToolbarDialog *parent =
      wxDynamicCast(GetParent(), ocpnFloatingToolbarDialog);
  if (parent && parent->m_FloatingToolbarConfigMenu) {
    wxMenuItem *configItem = parent->m_FloatingToolbarConfigMenu->FindItem(id);
    if (configItem) configItem->Check(true);
  }
}

void ocpnToolBarSimple::SetToolTooltipHiViz(int id, bool b_hiviz) {
  ocpnToolBarTool *tool = (ocpnToolBarTool *)FindById(id);
  if (tool) {
    tool->SetTooltipHiviz(b_hiviz);
  }
}

void ocpnToolBarSimple::ClearTools() {
  while (GetToolsCount()) {
    DeleteToolByPos(0);
  }
}

int ocpnToolBarSimple::GetVisibleToolCount() {
  int counter = 0;
  wxToolBarToolsList::compatibility_iterator node = m_tools.GetFirst();
  while (node) {
    ocpnToolBarTool *tool = (ocpnToolBarTool *)node->GetData();
    counter++;
    node = node->GetNext();
  }
  return counter;
}

bool ocpnToolBarSimple::DeleteToolByPos(size_t pos) {
  wxCHECK_MSG(pos < GetToolsCount(), false,
              _T("invalid position in wxToolBar::DeleteToolByPos()"));

  wxToolBarToolsList::compatibility_iterator node = m_tools.Item(pos);

  if (!DoDeleteTool(pos, node->GetData())) {
    return false;
  }

  delete node->GetData();
  m_tools.Erase(node);

  return true;
}

bool ocpnToolBarSimple::DeleteTool(int id) {
  size_t pos = 0;
  wxToolBarToolsList::compatibility_iterator node;
  for (node = m_tools.GetFirst(); node; node = node->GetNext()) {
    if (node->GetData()->GetId() == id) break;

    pos++;
  }

  if (!node || !DoDeleteTool(pos, node->GetData())) {
    return false;
  }

  delete node->GetData();
  m_tools.Erase(node);

  return true;
}

wxToolBarToolBase *ocpnToolBarSimple::AddSeparator() {
  return InsertSeparator(GetToolsCount());
}

wxToolBarToolBase *ocpnToolBarSimple::InsertSeparator(size_t pos) {
  wxCHECK_MSG(pos <= GetToolsCount(), (wxToolBarToolBase *)NULL,
              _T("invalid position in wxToolBar::InsertSeparator()"));

  wxToolBarToolBase *tool = CreateTool(
      wxID_SEPARATOR, wxEmptyString, wxNullBitmap, wxNullBitmap,
      wxITEM_SEPARATOR, (wxObject *)NULL, wxEmptyString, wxEmptyString);

  if (!tool || !DoInsertTool(pos, tool)) {
    delete tool;

    return NULL;
  }

  m_tools.Insert(pos, tool);
  m_nShowTools++;

  return tool;
}

wxToolBarToolBase *ocpnToolBarSimple::RemoveTool(int id) {
  size_t pos = 0;
  wxToolBarToolsList::compatibility_iterator node;
  for (node = m_tools.GetFirst(); node; node = node->GetNext()) {
    if (node->GetData()->GetId() == id) break;

    pos++;
  }

  if (!node) {
    // don't give any error messages - sometimes we might call RemoveTool()
    // without knowing whether the tool is or not in the toolbar
    return (wxToolBarToolBase *)NULL;
  }

  wxToolBarToolBase *tool = node->GetData();
  if (!DoDeleteTool(pos, tool)) {
    return (wxToolBarToolBase *)NULL;
  }

  m_tools.Erase(node);

  return tool;
}

wxControl *ocpnToolBarSimple::FindControl(int id) {
  for (wxToolBarToolsList::compatibility_iterator node = m_tools.GetFirst();
       node; node = node->GetNext()) {
    const wxToolBarToolBase *const tool = node->GetData();
    if (tool->IsControl()) {
      wxControl *const control = tool->GetControl();

      if (!control) {
        wxFAIL_MSG(_T("NULL control in toolbar?"));
      } else if (control->GetId() == id) {
        // found
        return control;
      }
    }
  }

  return NULL;
}

wxToolBarToolBase *ocpnToolBarSimple::FindById(int id) const {
  wxToolBarToolBase *tool = (wxToolBarToolBase *)NULL;

  for (wxToolBarToolsList::compatibility_iterator node = m_tools.GetFirst();
       node; node = node->GetNext()) {
    tool = node->GetData();
    if (tool->GetId() == id) {
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
bool ocpnToolBarSimple::OnLeftClick(int id, bool toggleDown) {
  wxCommandEvent event(wxEVT_COMMAND_TOOL_CLICKED, id);
  event.SetEventObject(this);

  // we use SetInt() to make wxCommandEvent::IsChecked() return toggleDown
  event.SetInt((int)toggleDown);

  // and SetExtraLong() for backwards compatibility
  event.SetExtraLong((long)toggleDown);

  // Send events to this toolbar instead (and thence up the window hierarchy)
  GetEventHandler()->ProcessEvent(event);

  return true;
}

// Call when right button down.
void ocpnToolBarSimple::OnRightClick(int id, long WXUNUSED(x),
                                     long WXUNUSED(y)) {
  HideTooltip();

  ocpnFloatingToolbarDialog *parent =
      wxDynamicCast(GetParent(), ocpnFloatingToolbarDialog);
  if (parent) {
    if (parent->m_FloatingToolbarConfigMenu) {
      ToolbarChoicesDialog *dlg =
          new ToolbarChoicesDialog(NULL, GetParent(), -1, _T("OpenCPN"),
                                   wxDefaultPosition, wxSize(100, 100));
      int rc = dlg->ShowModal();
      delete dlg;

      if (rc == wxID_OK) {
        wxCommandEvent event(wxEVT_COMMAND_TOOL_RCLICKED, id);
        event.SetEventObject(this);
        event.SetInt(id);

        gFrame->GetEventHandler()->AddPendingEvent(event);
      }
    }
  }
}

// Called when the mouse cursor enters a tool bitmap (no button pressed).
// Argument is wxID_ANY if mouse is exiting the toolbar.
// Note that for this event, the id of the window is used,
// and the integer parameter of wxCommandEvent is used to retrieve
// the tool id.
void ocpnToolBarSimple::OnMouseEnter(int id) {
  wxCommandEvent event(wxEVT_COMMAND_TOOL_ENTER, GetId());
  event.SetEventObject(this);
  event.SetInt(id);

  wxFrame *frame = wxDynamicCast(GetParent(), wxFrame);
  if (frame) {
    wxString help;
    wxToolBarToolBase *tool =
        id == wxID_ANY ? (wxToolBarToolBase *)NULL : FindById(id);
    if (tool) help = tool->GetLongHelp();
    frame->DoGiveHelp(help, id != wxID_ANY);
  }

  (void)GetEventHandler()->ProcessEvent(event);

  DoPluginToolUp();
}

void ocpnToolBarSimple::DoPluginToolUp() {
  //        Look for PlugIn tools
  //        If found, make the callback.
  if (!g_pi_manager) return;

  ArrayOfPlugInToolbarTools tool_array =
      g_pi_manager->GetPluginToolbarToolArray();
  for (unsigned int i = 0; i < tool_array.GetCount(); i++) {
    PlugInToolbarToolContainer *pttc = tool_array[i];
    if (m_last_plugin_down_id == pttc->id) {
      opencpn_plugin_113 *ppi =
          dynamic_cast<opencpn_plugin_113 *>(pttc->m_pplugin);
      if (ppi) ppi->OnToolbarToolUpCallback(pttc->id);
    }
  }

  m_last_plugin_down_id = -1;
}

void ocpnToolBarSimple::SetToolNormalBitmapEx(wxToolBarToolBase *tool,
                                              const wxString &iconName) {
  if (tool) {
    ocpnToolBarTool *otool = (ocpnToolBarTool *)tool;
    if (otool) {
      ocpnStyle::Style *style = g_StyleManager->GetCurrentStyle();

      wxBitmap bmp = style->GetToolIcon(iconName, TOOLICON_NORMAL, false,
                                        otool->m_width, otool->m_height);
      tool->SetNormalBitmap(bmp);
      otool->SetIconName(iconName);
    }
  }
}

void ocpnToolBarSimple::SetToolNormalBitmapSVG(wxToolBarToolBase *tool,
                                               wxString fileSVG) {
  if (tool) {
    ocpnToolBarTool *otool = (ocpnToolBarTool *)tool;
    if (otool) {
      otool->pluginNormalIconSVG = fileSVG;
    }
  }
}

void ocpnToolBarSimple::SetToolBitmaps(int id, wxBitmap *bmp,
                                       wxBitmap *bmpRollover) {
  ocpnToolBarTool *tool = (ocpnToolBarTool *)FindById(id);
  if (tool) {
    if (tool->isPluginTool) {
      tool->pluginNormalIcon = *bmp;
      tool->pluginRolloverIcon = *bmpRollover;
      tool->bitmapOK = false;
    } else {
      tool->SetNormalBitmap(*bmp);
      tool->bitmapOK = true;
    }
  }
}

void ocpnToolBarSimple::SetToolBitmapsSVG(int id, wxString fileSVGNormal,
                                          wxString fileSVGRollover,
                                          wxString fileSVGToggled) {
  ocpnToolBarTool *tool = (ocpnToolBarTool *)FindById(id);
  if (tool) {
    tool->pluginNormalIconSVG = fileSVGNormal;
    tool->pluginRolloverIconSVG = fileSVGRollover;
    tool->pluginToggledIconSVG = fileSVGToggled;
    tool->bitmapOK = false;
  }
}

//-------------------------------------------------------------------------------------

ToolbarMOBDialog::ToolbarMOBDialog(wxWindow *parent)
    : wxDialog(parent, wxID_ANY, _("OpenCPN Alert"), wxDefaultPosition,
               wxSize(250, 230)) {
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(sizer, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

  choices.push_back(
      new wxRadioButton(this, 0, _("No, I don't want to hide it."),
                        wxDefaultPosition, wxDefaultSize, wxRB_GROUP));

  choices.push_back(new wxRadioButton(
      this, 1, _("No, and permanently remove the option to hide it."),
      wxDefaultPosition));

  choices.push_back(
      new wxRadioButton(this, 2, _("Yes, hide it."), wxDefaultPosition));

  wxStdDialogButtonSizer *buttonSizer =
      CreateStdDialogButtonSizer(wxOK | wxCANCEL);

  wxStaticText *textCtrl =
      new wxStaticText(this, wxID_ANY,
                       _("The Man Over Board button could be an important "
                         "safety feature.\nAre you sure you want to hide it?"));

  sizer->Add(textCtrl, 0, wxEXPAND | wxALL, 5);
  sizer->Add(choices[0], 0, wxEXPAND | wxALL, 5);
  sizer->Add(choices[1], 0, wxEXPAND | wxALL, 5);
  sizer->Add(choices[2], 0, wxEXPAND | wxALL, 5);
  sizer->Add(buttonSizer, 0, wxEXPAND | wxTOP, 5);

  topSizer->SetSizeHints(this);
  SetSizer(topSizer);
}

int ToolbarMOBDialog::GetSelection() {
  for (unsigned int i = 0; i < choices.size(); i++) {
    if (choices[i]->GetValue()) return choices[i]->GetId();
  }
  return 0;
}

IMPLEMENT_DYNAMIC_CLASS(ToolbarChoicesDialog, wxDialog)
/*!
 * ToolbarChoicesDialog event table definition
 */
BEGIN_EVENT_TABLE(ToolbarChoicesDialog, wxDialog)
END_EVENT_TABLE()

/*!
 * ToolbarChoicesDialog constructors
 */

ToolbarChoicesDialog::ToolbarChoicesDialog() {}

ToolbarChoicesDialog::ToolbarChoicesDialog(wxWindow *parent, wxWindow *sponsor,
                                           wxWindowID id,
                                           const wxString &caption,
                                           const wxPoint &pos,
                                           const wxSize &size, long style) {
  long wstyle = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER;
  wxDialog::Create(parent, id, caption, pos, size, wstyle);

  m_configMenu = NULL;
  m_ToolbarDialogAncestor = wxDynamicCast(sponsor, ocpnFloatingToolbarDialog);
  if (m_ToolbarDialogAncestor)
    m_configMenu = m_ToolbarDialogAncestor->m_FloatingToolbarConfigMenu;

  CreateControls();
  GetSizer()->Fit(this);

  RecalculateSize();
}

ToolbarChoicesDialog::~ToolbarChoicesDialog() {}

/*!
 * Control creation for ToolbarChoicesDialog
 */

void ToolbarChoicesDialog::CreateControls() {
  wxBoxSizer *itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
  SetSizer(itemBoxSizer1);

  wxScrolledWindow *itemDialog1 = new wxScrolledWindow(
      this, wxID_ANY, wxDefaultPosition, wxSize(-1, -1), wxHSCROLL | wxVSCROLL);
  itemDialog1->SetScrollRate(2, 2);

#ifdef __OCPN__ANDROID__

  //  Set Dialog Font by custom crafted Qt Stylesheet.
  wxFont *qFont = GetOCPNScaledFont(_("Dialog"));

  wxString wqs = getFontQtStylesheet(qFont);
  wxCharBuffer sbuf = wqs.ToUTF8();
  QString qsb = QString(sbuf.data());

  QString qsbq = getQtStyleSheet();  // basic scrollbars, etc

  this->GetHandle()->setStyleSheet(qsb + qsbq);  // Concatenated style sheets

#endif
  itemBoxSizer1->Add(itemDialog1, 2, wxEXPAND | wxALL, 0);

  wxBoxSizer *itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxStaticBox *itemStaticBoxSizer3Static =
      new wxStaticBox(itemDialog1, wxID_ANY, _("Choose Toolbar Icons"));
  wxStaticBoxSizer *itemStaticBoxSizer3 =
      new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
  itemBoxSizer2->Add(itemStaticBoxSizer3, 0, wxEXPAND | wxALL, 5);

  int nitems = 0;
  int max_width = -1;
  if (m_configMenu) {
    nitems = m_configMenu->GetMenuItemCount();

    cboxes.clear();
    for (int i = 0; i < nitems; i++) {
      if (i + ID_ZOOMIN == ID_MOB && g_bPermanentMOBIcon) continue;
      wxMenuItem *item = m_configMenu->FindItemByPosition(i);

      wxString label = item->GetItemLabel();
      int l = label.Len();
      max_width = wxMax(max_width, l);

      wxString windowName = _T("");
      if (item->GetId() == ID_MOB + 100) windowName = _T("MOBCheck");

      wxCheckBox *cb =
          new wxCheckBox(itemDialog1, -1, label, wxDefaultPosition,
                         wxDefaultSize, 0, wxDefaultValidator, windowName);
      //            wxCheckBox *cb = new wxCheckBox(itemDialog1, -1, label);
      itemStaticBoxSizer3->Add(cb, 0, wxALL | wxEXPAND, 2);
      cb->SetValue(item->IsChecked());

      cboxes.push_back(cb);
    }
  }

  itemBoxSizer1->SetMinSize((max_width + 20) * GetCharWidth(),
                            (nitems + 4) * GetCharHeight() * 2);

  wxBoxSizer *itemBoxSizerBottom = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer1->Add(itemBoxSizerBottom, 0, wxALL | wxEXPAND, 5);

  wxBoxSizer *itemBoxSizerAux = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizerBottom->Add(itemBoxSizerAux, 1, wxALL, 3);

  wxBoxSizer *itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizerBottom->Add(itemBoxSizer16, 0, wxALL, 3);

  m_CancelButton =
      new wxButton(this, -1, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer16->Add(m_CancelButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 1);

  m_OKButton =
      new wxButton(this, -1, _("OK"), wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer16->Add(m_OKButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 1);
  m_OKButton->SetDefault();

  m_CancelButton->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ToolbarChoicesDialog::OnCancelClick), NULL, this);
  m_OKButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                      wxCommandEventHandler(ToolbarChoicesDialog::OnOkClick),
                      NULL, this);

  SetColorScheme((ColorScheme)0);
}

void ToolbarChoicesDialog::SetColorScheme(ColorScheme cs) { DimeControl(this); }

void ToolbarChoicesDialog::OnCancelClick(wxCommandEvent &event) {
  EndModal(wxID_CANCEL);
}

void ToolbarChoicesDialog::OnOkClick(wxCommandEvent &event) {
  unsigned int ncheck = 0;

  wxString toolbarConfigSave = m_ToolbarDialogAncestor->GetToolConfigString();
  wxString new_toolbarConfig = toolbarConfigSave;

  for (unsigned int i = 0; i < cboxes.size(); i++) {
    wxCheckBox *cb = cboxes[i];
    wxString cbName = cb->GetName();  // Special flag passed into checkbox ctor
                                      // to find the "MOB" item
    if (cbName.IsSameAs(_T("MOBCheck")) && !cb->IsChecked()) {
      // Ask if really want to disable MOB button
      ToolbarMOBDialog mdlg(this);
      int dialog_ret = mdlg.ShowModal();
      int answer = mdlg.GetSelection();
      if (dialog_ret == wxID_OK) {
        if (answer == 1) {
          g_bPermanentMOBIcon = true;
          cb->SetValue(true);
        } else if (answer == 0) {
          cb->SetValue(true);
        }
      } else {  // wxID_CANCEL
        new_toolbarConfig = toolbarConfigSave;
        return;
      }
    }
    if (m_configMenu) {
      wxMenuItem *item = m_configMenu->FindItemByPosition(i);
      if (new_toolbarConfig.Len() > i) {
        new_toolbarConfig.SetChar(i, cb->IsChecked() ? _T('X') : _T('.'));
      } else {
        new_toolbarConfig.Append(cb->IsChecked() ? _T('X') : _T('.'));
      }
      item->Check(cb->IsChecked());
      if (cb->IsChecked()) ncheck++;
    }
  }

#if 0
     //  We always must have one Tool enabled.  Make it the Options tool....
     if( 0 == ncheck){
         new_toolbarConfig.SetChar( ID_SETTINGS -ID_ZOOMIN , _T('X') );

         int idOffset = ID_PLUGIN_BASE - ID_ZOOMIN + 100;

         if(m_configMenu){
             wxMenuItem *item = m_configMenu->FindItem(ID_SETTINGS + idOffset);
             if(item)
                item->Check( true );
         }
     }
#endif
  m_ToolbarDialogAncestor->SetToolConfigString(new_toolbarConfig);

  EndModal(wxID_OK);
}

void ToolbarChoicesDialog::RecalculateSize(void) {
  wxSize esize = GetSize();

  if (GetParent()) {
    wxSize dsize = GetParent()->GetClientSize();
    esize.y = wxMin(esize.y, dsize.y - (4 * GetCharHeight()));
    esize.x = wxMin(esize.x, dsize.x - (2 * GetCharHeight()));
    SetSize(esize);
    Centre();

  } else {
    wxSize fsize = g_Platform->getDisplaySize();
    fsize.y = wxMin(esize.y, fsize.y - (4 * GetCharHeight()));
    fsize.x = wxMin(esize.x, fsize.x - (2 * GetCharHeight()));
    SetSize(fsize);
    CentreOnScreen();
#ifdef __OCPN__ANDROID__
    Move(GetPosition().x, 10);
#endif
  }
}
