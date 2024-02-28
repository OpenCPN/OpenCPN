/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Main wxWidgets Program
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

#include <wx/artprov.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>

#include "gui_lib.h"
#include "timers.h"
#include "FontMgr.h"
#include "OCPNPlatform.h"
#include "ocpn_plugin.h"
#include "displays.h"

#ifdef __ANDROID__
#include "androidUTIL.h"
#include "qdebug.h"
#endif

extern bool g_bresponsive;
extern OCPNPlatform* g_Platform;
extern int g_GUIScaleFactor;

wxFont* GetOCPNScaledFont(wxString item, int default_size) {
  wxFont* dFont = FontMgr::Get().GetFont(item, default_size);
  int req_size = dFont->GetPointSize();

  if (g_bresponsive) {
    //      Adjust font size to be no smaller than xx mm actual size
    double scaled_font_size = dFont->GetPointSize();

    {
      double points_per_mm =
          g_Platform->getFontPointsperPixel() * g_Platform->GetDisplayDPmm();
      double min_scaled_font_size =
          3 * points_per_mm;  // smaller than 3 mm is unreadable
      int nscaled_font_size =
          wxMax(wxRound(scaled_font_size), min_scaled_font_size);

      if (req_size >= nscaled_font_size)
        return dFont;
      else {
        wxFont* qFont = FontMgr::Get().FindOrCreateFont(
            nscaled_font_size, dFont->GetFamily(), dFont->GetStyle(),
            dFont->GetWeight());
        return qFont;
      }
    }
  }
  return dFont;
}

wxFont GetOCPNGUIScaledFont(wxString item) {
  wxFont* dFont = FontMgr::Get().GetFont(item, 0);
  int req_size = dFont->GetPointSize();
  wxFont qFont = *dFont;

  if (g_bresponsive) {
    double postmult = exp(g_GUIScaleFactor * (0.693 / 5.0));  //  exp(2)
    double scaled_font_size = dFont->GetPointSize() * postmult;

    double points_per_mm =
        g_Platform->getFontPointsperPixel() * g_Platform->GetDisplayDPmm();
    double min_scaled_font_size =
        3 * points_per_mm;  // smaller than 3 mm is unreadable
    int nscaled_font_size =
        wxMax(wxRound(scaled_font_size), min_scaled_font_size);

    //        wxFont *qFont = wxTheFontList->FindOrCreateFont(
    //        nscaled_font_size,
    //                                                                  dFont->GetFamily(),
    //                                                                  dFont->GetStyle(),
    //                                                                  dFont->GetWeight());
    qFont.SetPointSize(nscaled_font_size);
  }

  return qFont;
}

int OCPNMessageBox( wxWindow *parent, const wxString& message, const wxString& caption, int style,
                    int timeout_sec, int x, int y  )
{
#ifdef __OCPN__ANDROID__
    androidDisableRotation();
    int style_mod = style;

    auto dlg = new wxMessageDialog(parent, message, caption,  style_mod);
    int ret = dlg->ShowModal();
    qDebug() << "OCPNMB-1 ret" << ret;

    //int ret= dlg->GetReturnCode();

    //  Not sure why we need this, maybe on wx3?
    if( ((style & wxYES_NO) == wxYES_NO) && (ret == wxID_OK))
        ret = wxID_YES;

    dlg->Destroy();

    androidEnableRotation();
    qDebug() << "OCPNMB-2 ret" << ret;
    return ret;

#else
    int ret =  wxID_OK;

    TimedMessageBox tbox(parent, message, caption, style, timeout_sec, wxPoint( x, y )  );
    ret = tbox.GetRetVal() ;
#endif

    return ret;
}


BEGIN_EVENT_TABLE(OCPNMessageDialog, wxDialog)
EVT_BUTTON(wxID_YES, OCPNMessageDialog::OnYes)
EVT_BUTTON(wxID_NO, OCPNMessageDialog::OnNo)
EVT_BUTTON(wxID_CANCEL, OCPNMessageDialog::OnCancel)
EVT_CLOSE(OCPNMessageDialog::OnClose)
END_EVENT_TABLE()

OCPNMessageDialog::OCPNMessageDialog(wxWindow* parent, const wxString& message,
                                     const wxString& caption, long style,
                                     const wxPoint& pos)
    : wxDialog(parent, wxID_ANY, caption, pos, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP) {
  m_style = style;
  wxFont* qFont = GetOCPNScaledFont(_("Dialog"));
  SetFont(*qFont);

  wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer* icon_text = new wxBoxSizer(wxHORIZONTAL);

#if wxUSE_STATBMP
  // 1) icon
  if (style & wxICON_MASK) {
    wxBitmap bitmap;
    switch (style & wxICON_MASK) {
      default:
        wxFAIL_MSG(_T("incorrect log style"));
        // fall through

      case wxICON_ERROR:
        bitmap = wxArtProvider::GetIcon(wxART_ERROR, wxART_MESSAGE_BOX);
        break;

      case wxICON_INFORMATION:
        bitmap = wxArtProvider::GetIcon(wxART_INFORMATION, wxART_MESSAGE_BOX);
        break;

      case wxICON_WARNING:
        bitmap = wxArtProvider::GetIcon(wxART_WARNING, wxART_MESSAGE_BOX);
        break;

      case wxICON_QUESTION:
        bitmap = wxArtProvider::GetIcon(wxART_QUESTION, wxART_MESSAGE_BOX);
        break;
    }
    wxStaticBitmap* icon = new wxStaticBitmap(this, wxID_ANY, bitmap);
    icon_text->Add(icon, 0, wxCENTER);
  }
#endif  // wxUSE_STATBMP

#if wxUSE_STATTEXT
  // 2) text
  icon_text->Add(CreateTextSizer(message), 0, wxALIGN_CENTER | wxLEFT, 10);

  topsizer->Add(icon_text, 1, wxCENTER | wxLEFT | wxRIGHT | wxTOP, 10);
#endif  // wxUSE_STATTEXT

  // 3) buttons
  int AllButtonSizerFlags =
      wxOK | wxCANCEL | wxYES | wxNO | wxHELP | wxNO_DEFAULT;
  int center_flag = wxEXPAND;
  if (style & wxYES_NO) center_flag = wxALIGN_CENTRE;
  wxSizer* sizerBtn = CreateSeparatedButtonSizer(style & AllButtonSizerFlags);
  if (sizerBtn) topsizer->Add(sizerBtn, 0, center_flag | wxALL, 10);

  SetAutoLayout(true);
  SetSizer(topsizer);

  topsizer->SetSizeHints(this);
  topsizer->Fit(this);
  wxSize size(GetSize());
  if (size.x < size.y * 3 / 2) {
    size.x = size.y * 3 / 2;
    SetSize(size);
  }

  Centre(wxBOTH | wxCENTER_FRAME);
}

void OCPNMessageDialog::OnYes(wxCommandEvent& WXUNUSED(event)) {
  SetReturnCode(wxID_YES);
  EndModal(wxID_YES);
}

void OCPNMessageDialog::OnNo(wxCommandEvent& WXUNUSED(event)) {
  SetReturnCode(wxID_NO);
  EndModal(wxID_NO);
}

void OCPNMessageDialog::OnCancel(wxCommandEvent& WXUNUSED(event)) {
  // Allow cancellation via ESC/Close button except if
  // only YES and NO are specified.
  if ((m_style & wxYES_NO) != wxYES_NO || (m_style & wxCANCEL)) {
    SetReturnCode(wxID_CANCEL);
    EndModal(wxID_CANCEL);
  }
}

void OCPNMessageDialog::OnClose(wxCloseEvent& event) {
  SetReturnCode(wxID_CANCEL);
  EndModal(wxID_CANCEL);
}

BEGIN_EVENT_TABLE(TimedMessageBox, wxEvtHandler)
EVT_TIMER(-1, TimedMessageBox::OnTimer)
END_EVENT_TABLE()

TimedMessageBox::TimedMessageBox(wxWindow* parent, const wxString& message,
                                 const wxString& caption, long style,
                                 int timeout_sec, const wxPoint& pos) {
  ret_val = 0;
  m_timer.SetOwner(this, -1);

  if (timeout_sec > 0) m_timer.Start(timeout_sec * 1000, wxTIMER_ONE_SHOT);

  dlg = new OCPNMessageDialog(parent, message, caption, style, pos);
  dlg->ShowModal();

  int ret = dlg->GetReturnCode();

  //  Not sure why we need this, maybe on wx3?
  if (((style & wxYES_NO) == wxYES_NO) && (ret == wxID_OK)) ret = wxID_YES;

  delete dlg;
  dlg = NULL;

  ret_val = ret;
}

TimedMessageBox::~TimedMessageBox() {}

void TimedMessageBox::OnTimer(wxTimerEvent& evt) {
  if (dlg) dlg->EndModal(wxID_CANCEL);
}

BEGIN_EVENT_TABLE(OCPN_TimedHTMLMessageDialog, wxDialog)
EVT_BUTTON(wxID_YES, OCPN_TimedHTMLMessageDialog::OnYes)
EVT_BUTTON(wxID_NO, OCPN_TimedHTMLMessageDialog::OnNo)
EVT_BUTTON(wxID_CANCEL, OCPN_TimedHTMLMessageDialog::OnCancel)
EVT_CLOSE(OCPN_TimedHTMLMessageDialog::OnClose)
EVT_TIMER(-1, OCPN_TimedHTMLMessageDialog::OnTimer)
EVT_HTML_LINK_CLICKED( wxID_ANY, OCPN_TimedHTMLMessageDialog::OnHtmlLinkClicked )
END_EVENT_TABLE()

OCPN_TimedHTMLMessageDialog::OCPN_TimedHTMLMessageDialog(
    wxWindow* parent, const wxString& message, const wxString& caption,
    int tSeconds, long style, bool bFixedFont, const wxPoint& pos)
    : wxDialog(parent, wxID_ANY, caption, pos, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP) {
  m_style = style;
  if (bFixedFont) {
    wxFont* dFont = GetOCPNScaledFont_PlugIn(_("Dialog"));
    double font_size = dFont->GetPointSize();
    wxFont* qFont =
        wxTheFontList->FindOrCreateFont(font_size, wxFONTFAMILY_TELETYPE,
                                        dFont->GetStyle(), dFont->GetWeight());
    SetFont(*qFont);
  }

  wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);

  msgWindow = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                               wxHW_SCROLLBAR_AUTO | wxHW_NO_SELECTION);
  msgWindow->SetBorders(30);

  topsizer->Add(msgWindow, 1, wxEXPAND, 5);

  wxString html;
  html << message;

  wxCharBuffer buf = html.ToUTF8();
  if (buf.data())  // string OK?
    msgWindow->SetPage(html);

  // 3) buttons
  int AllButtonSizerFlags =
      wxOK | wxCANCEL | wxYES | wxNO | wxHELP | wxNO_DEFAULT;
  int center_flag = wxEXPAND;
  if (style & wxYES_NO) center_flag = wxALIGN_CENTRE;
  wxSizer* sizerBtn = CreateSeparatedButtonSizer(style & AllButtonSizerFlags);
  if (sizerBtn) topsizer->Add(sizerBtn, 0, center_flag | wxALL, 10);

  SetSizer(topsizer);

  topsizer->Fit(this);

  RecalculateSize();
  //       wxSize szyv = msgWindow->GetVirtualSize();

  //       SetClientSize(szyv.x + 20, szyv.y + 20);

  CentreOnScreen();

  // msgWindow->SetBackgroundColour(wxColour(191, 183, 180));
  msgWindow->SetBackgroundColour(GetBackgroundColour());

  m_timer.SetOwner(this, -1);

  if (tSeconds > 0) m_timer.Start(tSeconds * 1000, wxTIMER_ONE_SHOT);
}

void OCPN_TimedHTMLMessageDialog::RecalculateSize(void) {
  wxSize esize;
  esize.x = GetCharWidth() * 60;
  int sx, sy;
  sx = g_monitor_info[g_current_monitor].width;
  esize.x = wxMin(esize.x, sx * 6 / 10);
  esize.y = -1;
  SetClientSize(esize);  // This will force a recalc of internal representation

  int height1 = msgWindow->GetInternalRepresentation()->GetHeight();

  int client_size_y =
      wxMin(::wxGetDisplaySize().y - 100, height1 + 70);  // Must fit on screen

  SetClientSize(wxSize(
      esize.x, client_size_y));  // constant is 2xBorders + a little slop.
}

void OCPN_TimedHTMLMessageDialog::OnYes(wxCommandEvent& WXUNUSED(event)) {
  SetReturnCode(wxID_YES);
  if (IsModal())
    EndModal(wxID_YES);
  else
    Hide();
}

void OCPN_TimedHTMLMessageDialog::OnNo(wxCommandEvent& WXUNUSED(event)) {
  SetReturnCode(wxID_NO);
  if (IsModal())
    EndModal(wxID_NO);
  else
    Hide();
}

void OCPN_TimedHTMLMessageDialog::OnCancel(wxCommandEvent& WXUNUSED(event)) {
  // Allow cancellation via ESC/Close button except if
  // only YES and NO are specified.
  if ((m_style & wxYES_NO) != wxYES_NO || (m_style & wxCANCEL)) {
    SetReturnCode(wxID_CANCEL);
    EndModal(wxID_CANCEL);
  }
}

void OCPN_TimedHTMLMessageDialog::OnClose(wxCloseEvent& event) {
  SetReturnCode(wxID_CANCEL);
  if (IsModal())
    EndModal(wxID_CANCEL);
  else
    Hide();
}

void OCPN_TimedHTMLMessageDialog::OnTimer(wxTimerEvent& evt) {
  if (IsModal())
    EndModal(m_style & wxNO_DEFAULT ? wxID_NO : wxID_YES);
  else
    Hide();
}


//      Auto timed popup Window implementation

BEGIN_EVENT_TABLE(TimedPopupWin, wxWindow)
EVT_PAINT(TimedPopupWin::OnPaint)
EVT_TIMER(POPUP_TIMER, TimedPopupWin::OnTimer)

END_EVENT_TABLE()

// Define a constructor
TimedPopupWin::TimedPopupWin(wxWindow *parent, int timeout)
    : wxWindow(parent, wxID_ANY, wxPoint(0, 0), wxSize(1, 1), wxNO_BORDER) {
  m_pbm = NULL;

  m_timer_timeout.SetOwner(this, POPUP_TIMER);
  m_timeout_sec = timeout;
  isActive = false;
  Hide();
}

TimedPopupWin::~TimedPopupWin() { delete m_pbm; }
void TimedPopupWin::OnTimer(wxTimerEvent &event) {
  if (IsShown()) Hide();
}

void TimedPopupWin::SetBitmap(wxBitmap &bmp) {
  delete m_pbm;
  m_pbm = new wxBitmap(bmp);

  // Retrigger the auto timeout
  if (m_timeout_sec > 0)
    m_timer_timeout.Start(m_timeout_sec * 1000, wxTIMER_ONE_SHOT);
}

void TimedPopupWin::OnPaint(wxPaintEvent &event) {
  int width, height;
  GetClientSize(&width, &height);
  wxPaintDC dc(this);

  wxMemoryDC mdc;
  mdc.SelectObject(*m_pbm);
  dc.Blit(0, 0, width, height, &mdc, 0, 0);
}
