/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Notification Manager GUI
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2025 by David S. Register                               *
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
#include <cmath>
#include <memory>
#include <vector>
#include <wx/statline.h>
#include <wx/textwrapper.h>

#include "notification_manager_gui.h"
#include "model/notification_manager.h"
#include "model/notification.h"
#include "observable_globvar.h"
#include "color_handler.h"
#include "styles.h"
#include "OCPNPlatform.h"
#include "chcanv.h"
#include "glChartCanvas.h"
#include "gui_lib.h"
#include "svg_utils.h"
#include "model/datetime.h"
#include "navutil.h"

extern OCPNPlatform* g_Platform;
extern ocpnStyle::StyleManager* g_StyleManager;

class PanelHardBreakWrapper : public wxTextWrapper {
public:
  PanelHardBreakWrapper(wxWindow* win, const wxString& text, int widthMax) {
    m_lineCount = 0;
    Wrap(win, text, widthMax);
  }
  wxString const& GetWrapped() const { return m_wrapped; }
  int const GetLineCount() const { return m_lineCount; }
  wxArrayString GetLineArray() { return m_array; }

protected:
  virtual void OnOutputLine(const wxString& line) {
    m_wrapped += line;
    m_array.Add(line);
  }
  virtual void OnNewLine() {
    m_wrapped += '\n';
    m_lineCount++;
  }

private:
  wxString m_wrapped;
  int m_lineCount;
  wxArrayString m_array;
};

#
BEGIN_EVENT_TABLE(NotificationPanel, wxPanel)
EVT_PAINT(NotificationPanel::OnPaint)
END_EVENT_TABLE()

NotificationPanel::NotificationPanel(
    wxPanel* parent, wxWindowID id, const wxPoint& pos, const wxSize& size,
    std::shared_ptr<Notification> _notification, int _repeat_count)
    : wxPanel(parent, id, pos, size, wxBORDER_NONE),
      repeat_count(_repeat_count) {
  notification = _notification;

  wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(topSizer);

  wxBoxSizer* itemBoxSizer01 = new wxBoxSizer(wxHORIZONTAL);
  topSizer->Add(itemBoxSizer01, 0, wxEXPAND);

  double iconSize = GetCharWidth() * 3;
  double dpi_mult = g_Platform->GetDisplayDIPMult(this);
  int icon_scale = iconSize * dpi_mult;

  wxImage notification_icon;
  ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
  wxBitmap bitmap;
  wxFileName path;
  if (notification->GetSeverity() == NotificationSeverity::kInformational) {
    path =
        wxFileName(g_Platform->GetSharedDataDir(), "notification-info-2.svg");
  } else if (notification->GetSeverity() == NotificationSeverity::kWarning) {
    path = wxFileName(g_Platform->GetSharedDataDir(),
                      "notification-warning-2.svg");
  } else {
    path = wxFileName(g_Platform->GetSharedDataDir(),
                      "notification-critical-2.svg");
  }
  path.AppendDir("uidata");
  path.AppendDir("MUI_flat");
  bitmap = LoadSVG(path.GetFullPath(), icon_scale, icon_scale);
  m_itemStaticBitmap = new wxStaticBitmap(this, wxID_ANY, bitmap);

  itemBoxSizer01->Add(m_itemStaticBitmap, 0, wxEXPAND | wxALL, 10);

  // Repeat Count
  wxString rp = _("Repeat:");
  wxString sCount = rp + wxString::Format("\n  %d", repeat_count);
  auto counttextbox = new wxStaticText(this, wxID_ANY, sCount);
  itemBoxSizer01->Add(counttextbox, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  // Time
  wxDateTime act_time = wxDateTime(notification->GetActivateTime());
  wxString stime = wxString::Format(
      "%s",
      ocpn::toUsrDateTimeFormat(act_time, DateTimeFormatOptions()
                                              .SetFormatString("$short_date")
                                              .SetShowTimezone(false)));
  stime = stime.BeforeFirst(' ');
  wxString stime1 = wxString::Format(
      "%s", ocpn::toUsrDateTimeFormat(act_time,
                                      DateTimeFormatOptions().SetFormatString(
                                          "$24_hour_minutes_seconds")));
  stime += "\n";
  stime += stime1;

  auto timetextbox = new wxStaticText(this, wxID_ANY, stime);
  itemBoxSizer01->Add(timetextbox, 0,
                      /*wxEXPAND|*/ wxALL | wxALIGN_CENTER_VERTICAL, 5);

  PanelHardBreakWrapper wrapper(this, notification->GetMessage(),
                                GetSize().x * 60 / 100);

  auto textbox = new wxStaticText(this, wxID_ANY, wrapper.GetWrapped());
  itemBoxSizer01->Add(textbox, 0, wxALIGN_CENTER_VERTICAL | wxALL, 10);

  itemBoxSizer01->AddStretchSpacer(1);

  // Ack button
  m_ack_button = new wxButton(this, wxID_OK);
  itemBoxSizer01->Add(m_ack_button, 0, wxALIGN_CENTER_VERTICAL | wxALL, 10);
  m_ack_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                     &NotificationPanel::OnAckButton, this);

  DimeControl(m_ack_button);
}

NotificationPanel::~NotificationPanel() {}

void NotificationPanel::OnAckButton(wxCommandEvent& event) {
  NotificationManager& noteman = NotificationManager::GetInstance();
  noteman.AcknowledgeNotification(notification->GetGuid());
}

void NotificationPanel::OnPaint(wxPaintEvent& event) {
  wxPaintDC dc(this);
  wxColor back_color = GetDialogColor(DLG_UNSELECTED_BACKGROUND);
  wxBrush bg(back_color, wxBRUSHSTYLE_SOLID);
  dc.SetBackground(bg);
  dc.Clear();

  int penWidth = 2;  // m_penWidthUnselected;
  wxColour box_color = GetDialogColor(DLG_UNSELECTED_BACKGROUND);
  wxColour box_border = GetGlobalColor("GREY3");

  wxBrush b(box_color, wxBRUSHSTYLE_SOLID);
  dc.SetBrush(b);
  dc.SetPen(wxPen(box_border, penWidth));

  dc.DrawRoundedRectangle(5, 5, GetSize().x - 10, GetSize().y - 10, 5);
}

NotificationListPanel::NotificationListPanel(wxWindow* parent, wxWindowID id,
                                             const wxPoint& pos,
                                             const wxSize& size)
    : wxScrolledWindow(parent, id, pos, size,
                       wxTAB_TRAVERSAL | wxVSCROLL | wxHSCROLL) {
  SetSizer(new wxBoxSizer(wxVERTICAL));
  SetScrollRate(5, 5);
  ShowScrollbars(wxSHOW_SB_NEVER, wxSHOW_SB_DEFAULT);
  if (g_btouch) {
    ShowScrollbars(wxSHOW_SB_NEVER, wxSHOW_SB_NEVER);
    SetScrollRate(1, 1);
  }
  ReloadNotificationPanels();
  DimeControl(this);
}

NotificationListPanel::~NotificationListPanel() {}

void NotificationListPanel::ReloadNotificationPanels() {
  wxWindowList kids = GetChildren();
  for (unsigned int i = 0; i < kids.GetCount(); i++) {
    wxWindowListNode* node = kids.Item(i);
    wxWindow* win = node->GetData();
    NotificationPanel* pp = dynamic_cast<NotificationPanel*>(win);
    if (pp) win->Destroy();
  }
  GetSizer()->Clear();
  Hide();
  panels.clear();

  NotificationManager& noteman = NotificationManager::GetInstance();
  auto notifications = noteman.GetNotifications();

  int panel_size_x = 60 * GetCharWidth();
  panel_size_x = wxMax(panel_size_x, GetParent()->GetClientSize().x);
  wxSize panel_size = wxSize(panel_size_x, -1);

  for (auto notification : notifications) {
    size_t this_hash = notification->GetStringHash();
    int repeat_count = 0;
    for (auto hash_test : notifications) {
      if (hash_test->GetStringHash() == this_hash) {
        repeat_count++;
      }
    }

    // Do not create duplicate panels
    bool skip = false;
    for (auto tpanel : panels) {
      auto note = tpanel->GetNotification();

      if ((note->GetStringHash() == this_hash) && (repeat_count > 1)) {
        skip = true;
      }
    }
    if (skip) continue;

    NotificationPanel* panel =
        new NotificationPanel(this, wxID_ANY, wxDefaultPosition, panel_size,
                              notification, repeat_count);
    panels.push_back(panel);
  }

  for (auto panel : panels) {
    AddNotificationPanel(panel);
    DimeControl(panel);
  }

  Show();
  Layout();
  Refresh(true);
  Scroll(0, 0);
}

void NotificationListPanel::AddNotificationPanel(NotificationPanel* _panel) {
  GetSizer()->Add(_panel, 0);  //| wxALL, 10);
}

//------------------------------------------------------------------------------
//    NotificationsList
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(NotificationsList, wxDialog)
EVT_CLOSE(NotificationsList::OnClose)
END_EVENT_TABLE()

NotificationsList::NotificationsList(wxWindow* parent) : wxDialog() {
  wxFont* qFont = GetOCPNScaledFont(_("Dialog"));
  SetFont(*qFont);

  long mstyle = wxRESIZE_BORDER | wxCAPTION | wxCLOSE_BOX | wxFRAME_NO_TASKBAR;
#ifdef __WXOSX__
  mstyle |= wxSTAY_ON_TOP;
#endif

  wxDialog::Create(parent, wxID_ANY, _("OpenCPN Notifications"),
                   wxDefaultPosition, wxDefaultSize, mstyle);

  wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(topsizer);

  int border_size = 2;
  int group_item_spacing = 0;
  int interGroupSpace = border_size * 2;

  auto acksizer = new wxBoxSizer(wxHORIZONTAL);
  topsizer->Add(acksizer, 0, wxEXPAND);

  // Ack All button
  acksizer->AddStretchSpacer(1);

  m_ackall_button = new wxButton(this, wxID_ANY, _("Acknowledge All"));
  acksizer->Add(m_ackall_button, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2);
  m_ackall_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                        &NotificationsList::OnAckAllButton, this);
  acksizer->AddSpacer(10);

  // spacer
  topsizer->Add(0, interGroupSpace);

  m_notifications_list_panel = new NotificationListPanel(
      this, wxID_ANY, wxDefaultPosition, wxSize(-1, parent->GetSize().y));
  topsizer->Add(m_notifications_list_panel, 0, wxALL | wxEXPAND, border_size);

  DimeControl(this);
}
void NotificationsList::SetColorScheme() { DimeControl(this); }

void NotificationsList::ReloadNotificationList() {
  m_notifications_list_panel->ReloadNotificationPanels();
  if (!m_notifications_list_panel->GetPanels().size()) {
    Hide();
    GetParent()->Refresh();
  }
}

void NotificationsList::OnAckAllButton(wxCommandEvent& event) {
  NotificationManager& noteman = NotificationManager::GetInstance();
  noteman.AcknowledgeAllNotifications();
}

void NotificationsList::RecalculateSize() {
  // calculate and set best size and position for Notification list
  wxSize parent_size = GetParent()->GetSize();
  wxPoint ClientUpperRight =
      GetParent()->ClientToScreen(wxPoint(parent_size.x, 0));
  wxPoint list_bottom =
      GetParent()->ClientToScreen(wxPoint(0, parent_size.y / 3));
  int size_y = list_bottom.y - (ClientUpperRight.y + 5);
  size_y -= GetParent()->GetCharHeight();
  size_y =
      wxMax(size_y, 8 * GetCharHeight());  // ensure always big enough to see

  wxSize target_size = wxSize(GetCharWidth() * 80, size_y);

  wxPoint targetNLPos = GetParent()->ClientToScreen(
      wxPoint(parent_size.x / 2, 3 * GetParent()->GetCharHeight()));

  // Adjust the size for smaller devices
  wxSize display_size = g_Platform->getDisplaySize();
  if (target_size.x * 2 > display_size.x) {
    target_size.x =
        display_size.x * 85 / 100 - (2 * GetParent()->GetCharWidth());
    targetNLPos.x =
        GetParent()->ClientToScreen(wxPoint(display_size.x * 15 / 100, 0)).x;
  }

  SetSize(target_size);
  Move(targetNLPos);
  Layout();
}

void NotificationsList::OnClose(wxCloseEvent& event) { Hide(); }

/*
 * Notification Button Widget
 */

extern ocpnStyle::StyleManager* g_StyleManager;
extern bool g_bSatValid;
extern int g_SatsInView;
extern bool g_bopengl;
extern bool g_btenhertz;

#ifndef GL_RGBA8
#define GL_RGBA8 0x8058
#endif

NotificationButton::NotificationButton(ChartCanvas* parent) {
  m_parent = parent;

  ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
  _img_gpsRed = style->GetIcon(_T("gpsRed"));

  m_pStatBoxToolStaticBmp = NULL;

  m_rect = wxRect(style->GetCompassXOffset(), style->GetCompassYOffset(),
                  _img_gpsRed.GetWidth() + style->GetCompassLeftMargin() * 2 +
                      style->GetToolSeparation(),
                  _img_gpsRed.GetHeight() + style->GetCompassTopMargin() +
                      style->GetCompassBottomMargin());

#ifdef ocpnUSE_GL
  m_texobj = 0;
#endif
  m_texOK = false;

  m_scale = 1.0;
  m_cs = GLOBAL_COLOR_SCHEME_RGB;
  m_NoteIconName = "notification-info-2";
}

NotificationButton::~NotificationButton() {
#ifdef ocpnUSE_GL
  if (m_texobj) {
    glDeleteTextures(1, &m_texobj);
    m_texobj = 0;
  }
#endif

  delete m_pStatBoxToolStaticBmp;
}

void NotificationButton::SetIconSeverity(NotificationSeverity _severity) {
  wxString icon_name;
  if (_severity == NotificationSeverity::kInformational) {
    icon_name = "notification-info-2";
  } else if (_severity == NotificationSeverity::kWarning) {
    icon_name = "notification-warning-2";
  } else {
    icon_name = "notification-critical-2";
  }

  SetIconName(icon_name);
}

void NotificationButton::Paint(ocpnDC& dc) {
  if (m_shown && m_StatBmp.IsOk()) {
#if defined(ocpnUSE_GLES) || defined(ocpnUSE_GL)
    if (g_bopengl && !m_texobj) {
      // The glContext is known active here,
      // so safe to create a texture.
      glGenTextures(1, &m_texobj);
      CreateTexture();
    }

    if (g_bopengl && m_texobj /*&& m_texOK*/) {
      glBindTexture(GL_TEXTURE_2D, m_texobj);
      glEnable(GL_TEXTURE_2D);

#if defined(USE_ANDROID_GLES2) || defined(ocpnUSE_GLSL)
      float coords[8];
      float uv[8];

      // normal uv, normalized to POT
      uv[0] = 0;
      uv[1] = 0;
      uv[2] = (float)m_image_width / m_tex_w;
      uv[3] = 0;
      uv[4] = (float)m_image_width / m_tex_w;
      uv[5] = (float)m_image_height / m_tex_h;
      uv[6] = 0;
      uv[7] = (float)m_image_height / m_tex_h;

      // pixels
      coords[0] = m_rect.x;
      coords[1] = m_rect.y;
      coords[2] = m_rect.x + m_rect.width;
      coords[3] = m_rect.y;
      coords[4] = m_rect.x + m_rect.width;
      coords[5] = m_rect.y + m_rect.height;
      coords[6] = m_rect.x;
      coords[7] = m_rect.y + m_rect.height;

      m_parent->GetglCanvas()->RenderTextures(dc, coords, uv, 4,
                                              m_parent->GetpVP());
#else

      glBegin(GL_QUADS);

      glTexCoord2f(0, 0);
      glVertex2i(m_rect.x, m_rect.y);
      glTexCoord2f((float)m_image_width / m_tex_w, 0);
      glVertex2i(m_rect.x + m_rect.width, m_rect.y);
      glTexCoord2f((float)m_image_width / m_tex_w,
                   (float)m_image_height / m_tex_h);
      glVertex2i(m_rect.x + m_rect.width, m_rect.y + m_rect.height);
      glTexCoord2f(0, (float)m_image_height / m_tex_h);
      glVertex2i(m_rect.x, m_rect.y + m_rect.height);

      glEnd();
#endif

      glDisable(GL_TEXTURE_2D);

    } else {
#ifdef __WXOSX__
      // Support MacBook Retina display
      if (g_bopengl) {
        double scale = m_parent->GetContentScaleFactor();
        if (scale > 1) {
          wxImage image = m_StatBmp.ConvertToImage();
          image.Rescale(image.GetWidth() * scale, image.GetHeight() * scale);
          wxBitmap bmp(image);
          dc.DrawBitmap(bmp, m_rect.x, m_rect.y, true);
        } else
          dc.DrawBitmap(m_StatBmp, m_rect.x, m_rect.y, true);
      } else
        dc.DrawBitmap(m_StatBmp, m_rect.x, m_rect.y, true);
#else
      dc.DrawBitmap(m_StatBmp, m_rect.x, m_rect.y, true);
#endif
    }

#else
    dc.DrawBitmap(m_StatBmp, m_rect.x, m_rect.y, true);
#endif
  }
}

void NotificationButton::SetColorScheme(ColorScheme cs) {
  m_cs = cs;
  UpdateStatus(true);
}

wxRect NotificationButton::GetLogicalRect(void) const {
#ifdef wxHAS_DPI_INDEPENDENT_PIXELS
#if wxCHECK_VERSION(3, 1, 6)
  wxRect logicalRect = wxRect(m_parent->FromPhys(m_rect.GetPosition()),
                              m_parent->FromPhys(m_rect.GetSize()));
#else
  double scaleFactor = m_parent->GetContentScaleFactor();
  wxRect logicalRect(
      wxPoint(m_rect.GetX() / scaleFactor, m_rect.GetY() / scaleFactor),
      wxSize(m_rect.GetWidth() / scaleFactor,
             m_rect.GetHeight() / scaleFactor));
#endif
#else
  // On platforms without DPI-independent pixels, logical = physical.
  wxRect logicalRect = m_rect;
#endif
  return logicalRect;
}

bool NotificationButton::UpdateStatus(bool bnew) {
  bool rv = false;
  if (bnew) m_lastNoteIconName.Clear();  // force an update to occur
  if (m_lastNoteIconName != m_NoteIconName) {
    CreateBmp(bnew);
    rv = true;
  }

#ifdef ocpnUSE_GL
  if (g_bopengl && m_texobj) CreateTexture();
#endif
  return rv;
}

void NotificationButton::SetScaleFactor(float factor) {
  ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

  if (factor > 0.1)
    m_scale = factor;
  else
    m_scale = 1.0;

  //  Precalculate the background sizes to get m_rect width/height
  wxBitmap noteBg;
  int orient = style->GetOrientation();
  style->SetOrientation(wxTB_HORIZONTAL);
  if (style->HasBackground()) {
    noteBg = style->GetNormalBG();
    style->DrawToolbarLineEnd(noteBg);
    noteBg = style->SetBitmapBrightness(noteBg, m_cs);
  }

  if (fabs(m_scale - 1.0) > 0.1) {
    // noteBg = noteBg.ConvertToImage();
    // noteBg.Rescale(noteBg.GetWidth() * m_scale, noteBg.GetHeight() * m_scale,
    //                wxIMAGE_QUALITY_NORMAL);
    // noteBg = wxBitmap(noteBg);
  }

  int width = noteBg.GetWidth() + style->GetCompassLeftMargin();
  if (!style->marginsInvisible)
    width += style->GetCompassLeftMargin() + style->GetToolSeparation();

  m_rect = wxRect(style->GetCompassXOffset(), style->GetCompassYOffset(), width,
                  noteBg.GetHeight() + style->GetCompassTopMargin() +
                      style->GetCompassBottomMargin());
}

void NotificationButton::CreateBmp(bool newColorScheme) {
  // wxString gpsIconName;
  ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

  // In order to draw a horizontal compass window when the toolbar is vertical,
  // we need to save away the sizes and backgrounds for the two icons.

  // static wxBitmap compassBg;
  static wxBitmap noteBg;
  static wxSize toolsize;
  static int topmargin, leftmargin, radius;

  if (!noteBg.IsOk() || newColorScheme) {
    int orient = style->GetOrientation();
    style->SetOrientation(wxTB_HORIZONTAL);
    if (style->HasBackground()) {
      noteBg = style->GetNormalBG();
      style->DrawToolbarLineEnd(noteBg);
      noteBg = style->SetBitmapBrightness(noteBg, m_cs);
    }

    if (fabs(m_scale - 1.0) > 0.1) {
      wxImage bg_img = noteBg.ConvertToImage();
      bg_img.Rescale(noteBg.GetWidth() * m_scale, noteBg.GetHeight() * m_scale,
                     wxIMAGE_QUALITY_NORMAL);
      noteBg = wxBitmap(bg_img);
    }

    leftmargin = style->GetCompassLeftMargin();
    topmargin = style->GetCompassTopMargin();
    radius = style->GetCompassCornerRadius();

    if (orient == wxTB_VERTICAL) style->SetOrientation(wxTB_VERTICAL);
  }

  // int width = noteBg.GetWidth();// + leftmargin;
  int height = noteBg.GetHeight() + topmargin + style->GetCompassBottomMargin();
  int width = height;  // + leftmargin;
  // if (!style->marginsInvisible)
  // width += leftmargin + style->GetToolSeparation();

  m_StatBmp.Create(width, height);

  m_rect.width = m_StatBmp.GetWidth();
  m_rect.height = m_StatBmp.GetHeight();

  m_MaskBmp = wxBitmap(m_StatBmp.GetWidth(), m_StatBmp.GetHeight());
  if (style->marginsInvisible) {
    wxMemoryDC sdc(m_MaskBmp);
    sdc.SetBackground(*wxWHITE_BRUSH);
    sdc.Clear();
    sdc.SetBrush(*wxBLACK_BRUSH);
    sdc.SetPen(*wxBLACK_PEN);
    wxSize maskSize = wxSize(m_MaskBmp.GetWidth() - leftmargin,
                             m_MaskBmp.GetHeight() - (2 * topmargin));
    sdc.DrawRoundedRectangle(wxPoint(leftmargin, topmargin), maskSize, radius);
    sdc.SelectObject(wxNullBitmap);
  } else if (radius) {
    wxMemoryDC sdc(m_MaskBmp);
    sdc.SetBackground(*wxWHITE_BRUSH);
    sdc.Clear();
    sdc.SetBrush(*wxBLACK_BRUSH);
    sdc.SetPen(*wxBLACK_PEN);
    sdc.DrawRoundedRectangle(0, 0, m_MaskBmp.GetWidth(), m_MaskBmp.GetHeight(),
                             radius);
    sdc.SelectObject(wxNullBitmap);
  }
#if !defined(USE_ANDROID_GLES2) && !defined(ocpnUSE_GLSL)
  m_StatBmp.SetMask(new wxMask(m_MaskBmp, *wxWHITE));
#endif

  wxMemoryDC mdc;
  mdc.SelectObject(m_StatBmp);
  mdc.SetBackground(wxBrush(GetGlobalColor(_T("COMP1")), wxBRUSHSTYLE_SOLID));
  mdc.Clear();

  mdc.SetPen(wxPen(GetGlobalColor(_T("UITX1")), 1));
  mdc.SetBrush(wxBrush(GetGlobalColor(_T("UITX1")), wxBRUSHSTYLE_TRANSPARENT));

  if (!style->marginsInvisible)
    mdc.DrawRoundedRectangle(0, 0, m_StatBmp.GetWidth(), m_StatBmp.GetHeight(),
                             radius);
  wxPoint offset(leftmargin, topmargin);

  //  Notification Icon
  int twidth = style->GetToolSize().x * m_scale;
  int theight = style->GetToolSize().y * m_scale;
  int swidth = wxMax(twidth, theight);
  int sheight = wxMin(twidth, theight);

  swidth = swidth * 45 / 50;
  sheight = sheight * 45 / 50;

  offset.x = ((m_StatBmp.GetWidth() - swidth) / 2);
  offset.y = ((m_StatBmp.GetHeight() - sheight) / 2);

  wxFileName icon_path;
  wxString file_name = m_NoteIconName + ".svg";
  icon_path = wxFileName(g_Platform->GetSharedDataDir(), file_name);
  icon_path.AppendDir("uidata");
  icon_path.AppendDir("MUI_flat");
  wxBitmap gicon = LoadSVG(icon_path.GetFullPath(), swidth, sheight);

  wxBitmap iconBm;
  iconBm = gicon;

  mdc.DrawBitmap(iconBm, offset);
  mdc.SelectObject(wxNullBitmap);

  m_lastNoteIconName = m_NoteIconName;
}

void NotificationButton::CreateTexture() {
#if defined(USE_ANDROID_GLES2) || defined(ocpnUSE_GLSL)
  // GLES does not do ocpnDC::DrawBitmap(), so use
  // texture
  if (g_bopengl) {
    wxImage image = m_StatBmp.ConvertToImage();
    unsigned char* imgdata = image.GetData();
    unsigned char* imgalpha = image.GetAlpha();
    m_tex_w = image.GetWidth();
    m_tex_h = image.GetHeight();
    m_image_width = m_tex_w;
    m_image_height = m_tex_h;

    // Make it POT
    int width_pot = m_tex_w;
    int height_pot = m_tex_h;

    int xp = image.GetWidth();
    if (((xp != 0) && !(xp & (xp - 1))))  // detect POT
      width_pot = xp;
    else {
      int a = 0;
      while (xp) {
        xp = xp >> 1;
        a++;
      }
      width_pot = 1 << a;
    }

    xp = image.GetHeight();
    if (((xp != 0) && !(xp & (xp - 1))))
      height_pot = xp;
    else {
      int a = 0;
      while (xp) {
        xp = xp >> 1;
        a++;
      }
      height_pot = 1 << a;
    }

    m_tex_w = width_pot;
    m_tex_h = height_pot;

    GLuint format = GL_RGBA;
    GLuint internalformat = GL_RGBA8;  // format;
    int stride = 4;

    if (imgdata) {
      unsigned char* teximage =
          (unsigned char*)malloc(stride * m_tex_w * m_tex_h);

      for (int i = 0; i < m_image_height; i++) {
        for (int j = 0; j < m_image_width; j++) {
          int s = (i * 3 * m_image_width) + (j * 3);
          int d = (i * stride * m_tex_w) + (j * stride);

          teximage[d + 0] = imgdata[s + 0];
          teximage[d + 1] = imgdata[s + 1];
          teximage[d + 2] = imgdata[s + 2];
          teximage[d + 3] = 255;
        }
      }

      glBindTexture(GL_TEXTURE_2D, m_texobj);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                      GL_NEAREST);  // No mipmapping
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

      glTexImage2D(GL_TEXTURE_2D, 0, internalformat, m_tex_w, m_tex_h, 0,
                   format, GL_UNSIGNED_BYTE, teximage);

      free(teximage);
      glBindTexture(GL_TEXTURE_2D, 0);
      m_texOK = true;
    }
  }
#endif
}

void NotificationButton::UpdateTexture() {
#if defined(USE_ANDROID_GLES2) || defined(ocpnUSE_GLSL)
  // GLES does not do ocpnDC::DrawBitmap(), so use
  // texture
  if (g_bopengl) {
    wxImage image = m_StatBmp.ConvertToImage();
    unsigned char* imgdata = image.GetData();
    unsigned char* imgalpha = image.GetAlpha();
    m_tex_w = image.GetWidth();
    m_tex_h = image.GetHeight();
    m_image_width = m_tex_w;
    m_image_height = m_tex_h;

    // Make it POT
    int width_pot = m_tex_w;
    int height_pot = m_tex_h;

    int xp = image.GetWidth();
    if (((xp != 0) && !(xp & (xp - 1))))  // detect POT
      width_pot = xp;
    else {
      int a = 0;
      while (xp) {
        xp = xp >> 1;
        a++;
      }
      width_pot = 1 << a;
    }

    xp = image.GetHeight();
    if (((xp != 0) && !(xp & (xp - 1))))
      height_pot = xp;
    else {
      int a = 0;
      while (xp) {
        xp = xp >> 1;
        a++;
      }
      height_pot = 1 << a;
    }

    m_tex_w = width_pot;
    m_tex_h = height_pot;

    GLuint format = GL_RGBA;
    GLuint internalformat = GL_RGBA8;  // format;
    int stride = 4;

    if (imgdata) {
      unsigned char* teximage =
          (unsigned char*)malloc(stride * m_tex_w * m_tex_h);

      for (int i = 0; i < m_image_height; i++) {
        for (int j = 0; j < m_image_width; j++) {
          int s = (i * 3 * m_image_width) + (j * 3);
          int d = (i * stride * m_tex_w) + (j * stride);

          teximage[d + 0] = imgdata[s + 0];
          teximage[d + 1] = imgdata[s + 1];
          teximage[d + 2] = imgdata[s + 2];
          teximage[d + 3] = 255;
        }
      }

      glBindTexture(GL_TEXTURE_2D, m_texobj);
      glEnable(GL_TEXTURE_2D);

      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_tex_w, m_tex_h, format,
                      GL_UNSIGNED_BYTE, teximage);

      free(teximage);
      glBindTexture(GL_TEXTURE_2D, 0);
    }
  }
#endif
}
