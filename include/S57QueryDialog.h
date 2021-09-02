/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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
 */

#ifndef __S57QUERYDIALOG_H__
#define __S57QUERYDIALOG_H__

#include <wx/button.h>
#include <wx/frame.h>
#include <wx/html/htmlwin.h>

class wxHtmlWindow;

class S57QueryDialog : public wxFrame {
  DECLARE_CLASS(S57QueryDialog)
  DECLARE_EVENT_TABLE()
public:
  /// Constructors

  S57QueryDialog();
  S57QueryDialog(wxWindow* parent, wxWindowID id = wxID_ANY,
                 const wxString& caption = _("Object Query"),
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

  ~S57QueryDialog();
  void Init();

  bool Create(wxWindow* parent, wxWindowID id = wxID_ANY,
              const wxString& caption = _("Object Query"),
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxDefaultSize,
              long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

  void SetColorScheme(void);

  void CreateControls();
  void OnSize(wxSizeEvent& event);
  void RecalculateSize(void);

  void OnClose(wxCloseEvent& event);
  void OnHtmlLinkClicked(wxHtmlLinkEvent& event);

  void OnOKClick(wxCommandEvent& event) { Close(); }
  void OnKey(wxKeyEvent& ke);

  //    Overrides
  void OnPaint(wxPaintEvent& event);

  void SetHTMLPage(wxString& page);

  //    Data
  wxHtmlWindow* m_phtml;
  wxSize m_createsize;

  wxButton* m_btnOK;
};

class S57ExtraQueryInfoDlg : public S57QueryDialog {
  DECLARE_CLASS(S57ExtraQueryInfoDlg)
  DECLARE_EVENT_TABLE()
public:
  /// Constructors

  S57ExtraQueryInfoDlg();
  S57ExtraQueryInfoDlg(wxWindow* parent, wxWindowID id = wxID_ANY,
                       const wxString& caption = _("Extra Object Info"),
                       const wxPoint& pos = wxDefaultPosition,
                       const wxSize& size = wxDefaultSize,
                       long style = wxCAPTION | wxRESIZE_BORDER |
                                    wxSYSTEM_MENU);
  bool Create(wxWindow* parent, wxWindowID id = wxID_ANY,
              const wxString& caption = _("Extra Object Info"),
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxDefaultSize,
              long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

  ~S57ExtraQueryInfoDlg();
  void OnClose(wxCloseEvent& event);
  void OnSize(wxSizeEvent& event);
  void RecalculateSize(void);

private:
  wxButton* m_btnOK;
};

#endif
