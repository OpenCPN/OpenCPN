/**************************************************************************
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 *  \file
 *
 *  Implement gui_lib.h
 */

#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/dialog.h>
#include <wx/font.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <wx/statline.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/utils.h>

#include "gl_headers.h"  // Must come before anything using GL stuff

#include "model/config_vars.h"
#include "model/gui_events.h"

#include "gui_lib.h"
#include "timers.h"
#include "font_mgr.h"
#include "ocpn_platform.h"
#include "ocpn_plugin.h"
#include "displays.h"

#ifdef __ANDROID__
#include "androidUTIL.h"
#include "qdebug.h"
#endif

CopyableText::CopyableText(wxWindow* parent, const char* text)
    : wxTextCtrl(parent, wxID_ANY, text, wxDefaultPosition, wxDefaultSize,
                 wxBORDER_NONE) {
  SetEditable(false);
  wxStaticText tmp(parent, wxID_ANY, text);
  SetBackgroundColour(tmp.GetBackgroundColour());
}

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

int OCPNMessageBox(wxWindow* parent, const wxString& message,
                   const wxString& caption, int style, int timeout_sec, int x,
                   int y) {
#ifdef __ANDROID__
  androidDisableRotation();
  int style_mod = style;

  auto dlg = new wxMessageDialog(parent, message, caption, style_mod);
  int ret = dlg->ShowModal();
  qDebug() << "OCPNMB-1 ret" << ret;

  // int ret= dlg->GetReturnCode();

  //  Not sure why we need this, maybe on wx3?
  if (((style & wxYES_NO) == wxYES_NO) && (ret == wxID_OK)) ret = wxID_YES;

  dlg->Destroy();

  androidEnableRotation();
  qDebug() << "OCPNMB-2 ret" << ret;
  return ret;

#else
  int ret = wxID_OK;

  TimedMessageBox tbox(parent, message, caption, style, timeout_sec,
                       wxPoint(x, y));
  ret = tbox.GetRetVal();
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
        wxFAIL_MSG("incorrect log style");
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

// Auto timed popup Window implementation
BEGIN_EVENT_TABLE(TimedPopupWin, wxWindow)
EVT_PAINT(TimedPopupWin::OnPaint)
EVT_TIMER(POPUP_TIMER, TimedPopupWin::OnTimer)

END_EVENT_TABLE()

// Define a constructor
TimedPopupWin::TimedPopupWin(wxWindow* parent, int timeout)
    : wxWindow(parent, wxID_ANY, wxPoint(0, 0), wxSize(1, 1), wxNO_BORDER) {
  m_pbm = NULL;

  m_timer_timeout.SetOwner(this, POPUP_TIMER);
  m_timeout_sec = timeout;
  isActive = false;
  Hide();
}

TimedPopupWin::~TimedPopupWin() { delete m_pbm; }
void TimedPopupWin::OnTimer(wxTimerEvent& event) {
  if (IsShown()) Hide();
}

void TimedPopupWin::SetBitmap(wxBitmap& bmp) {
  delete m_pbm;
  m_pbm = new wxBitmap(bmp);

  // Retrigger the auto timeout
  if (m_timeout_sec > 0)
    m_timer_timeout.Start(m_timeout_sec * 1000, wxTIMER_ONE_SHOT);
}

void TimedPopupWin::OnPaint(wxPaintEvent& event) {
  int width, height;
  GetClientSize(&width, &height);
  wxPaintDC dc(this);

  wxMemoryDC mdc;
  mdc.SelectObject(*m_pbm);
  dc.Blit(0, 0, width, height, &mdc, 0, 0);
}

class InfoButton::InfoFrame : public wxFrame {
public:
  explicit InfoFrame(wxWindow* parent, const char* header, const char* info)
      : wxFrame(parent, wxID_ANY, info) {
    auto flags = wxSizerFlags().Expand();
    auto vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(new wxStaticText(this, wxID_ANY, info), flags.Border());
    vbox->Add(new wxStaticLine(this, wxID_ANY), flags);
    auto button_sizer = new wxStdDialogButtonSizer();
    auto ok_btn = new wxButton(this, wxID_OK);
    ok_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                 [&](wxCommandEvent&) { Hide(); });
    button_sizer->SetAffirmativeButton(ok_btn);
    vbox->Add(button_sizer, flags.Border());
    button_sizer->Realize();
    SetSizer(vbox);
    wxWindow::Layout();
    Hide();
  }
};

InfoButton::InfoButton(wxWindow* parent, bool touch, const char* header,
                       const char* info)
    : wxButton(parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
               wxBU_EXACTFIT | wxBORDER_NONE),
      m_icon(parent, "help-info.svg",
             GuiEvents::GetInstance().color_scheme_change, touch),
      m_info_frame(new InfoFrame(parent, header, info)) {
  SetBitmap(m_icon.GetBitmap());
  Bind(wxEVT_COMMAND_BUTTON_CLICKED, [&](wxCommandEvent&) {
    m_info_frame->Fit();
    m_info_frame->Show();
  });
}
