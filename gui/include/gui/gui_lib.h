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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

/**
 *  \file
 *  General purpose GUI support.
 */

#ifndef GUI_LIB_H__
#define GUI_LIB_H__

#include <wx/font.h>
#include <wx/html/htmlwin.h>
#include <wx/msgdlg.h>
#include <wx/textctrl.h>
#include <wx/timer.h>
#include <wx/window.h>
#include <wx/utils.h>

/** Non-editable TextCtrl, used like wxStaticText but is copyable. */
class CopyableText : public wxTextCtrl {
public:
  CopyableText(wxWindow* parent, const char* text);
};

/**
 * Retrieves a font from FontMgr, optionally scaled for physical readability.
 *
 * Returns a font configured for a specific UI context, scaling based on
 * system settings and preserving readability across different displays.
 *
 * @param item UI element identifier (e.g., "AISTargetAlert", "StatusBar")
 * @param default_size Optional base font size in points. 0 uses platform
 * default.
 *
 * @return Pointer to a dynamically scaled wxFont
 *
 * @note Font is managed by OpenCPN's central font cache
 * @note Pointer is shared and should not be deleted by caller
 */
wxFont* GetOCPNScaledFont(wxString item, int default_size = 0);
/**
 * Retrieves a font optimized for touch and high-resolution interfaces.
 *
 * Generates a font specifically tuned for responsive and touch-friendly
 * interfaces, with more aggressive scaling than standard font methods.
 *
 * @param item UI element identifier (e.g., "AISTargetAlert", "StatusBar")
 *
 * @return A wxFont object scaled for touch and high-resolution interfaces
 *
 * @note Ensures minimum physical font size for improved readability
 * @note Particularly suitable for toolbars, buttons, and touch controls
 */
wxFont GetOCPNGUIScaledFont(wxString item);

extern int OCPNMessageBox(wxWindow* parent, const wxString& message,
                          const wxString& caption = _T("Message"),
                          int style = wxOK, int timout_sec = -1, int x = -1,
                          int y = -1);

class OCPNMessageDialog : public wxDialog {
public:
  OCPNMessageDialog(wxWindow* parent, const wxString& message,
                    const wxString& caption = wxMessageBoxCaptionStr,
                    long style = wxOK | wxCENTRE,
                    const wxPoint& pos = wxDefaultPosition);

  void OnYes(wxCommandEvent& event);
  void OnNo(wxCommandEvent& event);
  void OnCancel(wxCommandEvent& event);
  void OnClose(wxCloseEvent& event);

private:
  int m_style;
  DECLARE_EVENT_TABLE()
};

class TimedMessageBox : public wxEvtHandler {
public:
  TimedMessageBox(wxWindow* parent, const wxString& message,
                  const wxString& caption = _T("Message box"),
                  long style = wxOK | wxCANCEL, int timeout_sec = -1,
                  const wxPoint& pos = wxDefaultPosition);
  ~TimedMessageBox();
  int GetRetVal(void) { return ret_val; }
  void OnTimer(wxTimerEvent& evt);

  wxTimer m_timer;
  OCPNMessageDialog* dlg;
  int ret_val;

  DECLARE_EVENT_TABLE()
};

class OCPN_TimedHTMLMessageDialog : public wxDialog {
public:
  OCPN_TimedHTMLMessageDialog(wxWindow* parent, const wxString& message,
                              const wxString& caption = wxMessageBoxCaptionStr,
                              int tSeconds = -1, long style = wxOK | wxCENTRE,
                              bool bFixedFont = false,
                              const wxPoint& pos = wxDefaultPosition);

  void OnYes(wxCommandEvent& event);
  void OnNo(wxCommandEvent& event);
  void OnCancel(wxCommandEvent& event);
  void OnClose(wxCloseEvent& event);
  void OnTimer(wxTimerEvent& evt);
  void RecalculateSize(void);
  void OnHtmlLinkClicked(wxHtmlLinkEvent& event) {
    wxLaunchDefaultBrowser(event.GetLinkInfo().GetHref());
  }

private:
  int m_style;
  wxTimer m_timer;
  wxHtmlWindow* msgWindow;

  DECLARE_EVENT_TABLE()
};

//----------------------------------------------------------------------------
// Generic Auto Timed Window
// Belongs to the creator, not deleted automatically on application close
//----------------------------------------------------------------------------

class TimedPopupWin : public wxWindow {
public:
  TimedPopupWin(wxWindow* parent, int timeout = -1);
  ~TimedPopupWin();

  void OnPaint(wxPaintEvent& event);

  void SetBitmap(wxBitmap& bmp);
  wxBitmap* GetBitmap() { return m_pbm; }
  void OnTimer(wxTimerEvent& event);
  bool IsActive() { return isActive; }
  void IsActive(bool state) { isActive = state; }

private:
  wxBitmap* m_pbm;
  wxTimer m_timer_timeout;
  int m_timeout_sec;
  bool isActive;

  DECLARE_EVENT_TABLE()
};

#endif  // GUI_LIB_H__
