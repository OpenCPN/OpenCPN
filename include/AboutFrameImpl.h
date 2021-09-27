/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2019 by David S. Register                               *
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

#pragma once
#ifndef _ABOUTFRAMEIMPL_H_
#define _ABOUTFRAMEIMPL_H_

#include "AboutFrame.h"

class AboutFrameImpl : public AboutFrame {
protected:
  void AboutFrameOnClose(wxCloseEvent& event) {
    Hide();
    SetSize(GetMinSize());
    Centre();
    event.Veto();
  }
  void OnLinkDonate(wxHyperlinkEvent& event) { event.Skip(); }
  void OnLinkGetInvolved(wxHyperlinkEvent& event) { event.Skip(); }
  void OnLinkLogfile(wxHyperlinkEvent& event) { event.Skip(); }
  void OnLinkConfigfile(wxHyperlinkEvent& event) { event.Skip(); }
  void OnLinkHelp(wxHyperlinkEvent& event);
  void OnLinkLicense(wxHyperlinkEvent& event);
  void OnLinkAuthors(wxHyperlinkEvent& event);
  void AboutFrameOnActivate(wxActivateEvent& event);
#if wxUSE_WEBVIEW && defined(HAVE_WEBVIEW)
  void m_btnBackOnButtonClick(wxCommandEvent& event) {
    m_htmlWinHelp->GoBack();
    m_btnBack->Enable(m_htmlWinHelp->CanGoBack());
  }
#else
  void m_btnBackOnButtonClick(wxCommandEvent& event) {
    m_htmlWinHelp->HistoryBack();
    m_btnBack->Enable(m_htmlWinHelp->HistoryCanBack());
  }
#endif
  void m_htmlWinHelpOnHtmlLinkClicked(wxHtmlLinkEvent& event) {
    m_btnBack->Enable();
    event.Skip();
  }

  void RecalculateSize(void);

public:
  ~AboutFrameImpl(){};

  AboutFrameImpl(wxWindow* parent, wxWindowID id = wxID_ANY,
                 const wxString& title = _("About OpenCPN"),
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxSize(500, 300),
                 long style = wxCAPTION | wxCLOSE_BOX |
                              wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR |
                              wxRESIZE_BORDER | wxTAB_TRAVERSAL);
};

#endif  // _ABOUTFRAMEIMPL_H_
