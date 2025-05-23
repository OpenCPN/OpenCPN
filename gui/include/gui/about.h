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
 * \file
 * class About
 */

#include <functional>

#ifndef _ABOUT_H_
#define _ABOUT_H_

#include <wx/notebook.h>
#include <wx/html/htmlwin.h>
#include <wx/dialog.h>
#include <wx/textctrl.h>

//      Constants

#define ID_DIALOG 10001

#define SYMBOL_ABOUT_TITLE _("About OpenCPN")

#define ID_DONATE 10010
#define ID_COPYINI 10011
#define ID_COPYLOG 10012

#define ID_NOTEBOOK_HELP 10002

/**
 * The OpenCPN About dialog displaying information including
 * version, authors, license, and tips. This is a legacy variant
 * used on Android only.
 */
class About : public wxDialog {
  DECLARE_EVENT_TABLE()

public:
  explicit About();
  explicit About(wxWindow* parent, wxString License_Data_Locn,
                 std::function<void()> launch_local_help,
                 wxWindowID id = ID_DIALOG,
                 const wxString& caption = SYMBOL_ABOUT_TITLE,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxSize(500, 500),
                 long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU |
                              wxCLOSE_BOX);
  bool Create(wxWindow* parent, wxWindowID id = ID_DIALOG,
              const wxString& caption = SYMBOL_ABOUT_TITLE,
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxSize(500, 500),
              long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU |
                           wxCLOSE_BOX);

  void RecalculateSize(void);

private:
  void CreateControls(void);
  void Populate(void);
  void OnXidOkClick(wxCommandEvent& event);
  void OnNBPageChange(wxNotebookEvent& event);
  void OnDonateClick(wxCommandEvent& event);
  void OnCopyClick(wxCommandEvent& event);
  void OnClose(wxCloseEvent& event);
  void SetColorScheme(void);

  wxString m_DataLocn;
  wxWindow* m_parent;
  std::function<void()> m_launch_local_help;
  bool m_btips_loaded;
  bool m_blicensePageSet;

  wxPanel* itemPanelAbout;
  wxPanel* itemPanelAuthors;
  wxPanel* itemPanelLicense;
  wxPanel* itemPanelTips;

  wxNotebook* pNotebook;
  wxHtmlWindow* pAboutHTMLCtl;
  wxHtmlWindow* pLicenseHTMLCtl;
  wxHtmlWindow* pAuthorHTMLCtl;

  wxSize m_displaySize;
};

#endif  // _ABOUT_H_
