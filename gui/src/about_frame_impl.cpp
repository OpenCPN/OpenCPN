/**************************************************************************
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

/**
 * \file
 * Implement about_frame_impl.h
 */

#include <fstream>
#include <sstream>
#include <string>

#include "about_frame_impl.h"
#include "config.h"
#include "OCPNPlatform.h"
#include "gui_lib.h"
#include "std_filesystem.h"

#ifdef __WXMSW__
#define EXTEND_WIDTH 70
#define EXTEND_HEIGHT 70
#else
#define EXTEND_WIDTH 50
#define EXTEND_HEIGHT 50
#endif

extern OCPNPlatform* g_Platform;

/** Load utf-8 encoded HTML page, works around wxHtmlWindow LoadFile() bugs */
static void LoadAuthors(wxHtmlWindow& hyperlink) {
  fs::path datadir(g_Platform->GetSharedDataDir().ToStdString());
  std::ifstream stream(datadir / "Authors.html");
  std::stringstream ss;
  ss << stream.rdbuf();
  hyperlink.SetPage(wxString::FromUTF8(ss.str().c_str()));
}

AboutFrameImpl::AboutFrameImpl(wxWindow* parent, wxWindowID id,
                               const wxString& title, const wxPoint& pos,
                               const wxSize& size, long style)
    : AboutFrame(parent, id, title, pos, size, style) {
  if (strlen(DEBIAN_PPA_VERSION))
    m_staticTextVersion->SetLabel(wxString(DEBIAN_PPA_VERSION));
  else
    m_staticTextVersion->SetLabel(PACKAGE_VERSION);

  m_staticTextCopyYears->SetLabel("\u00A9 2000-2024");
  m_hyperlinkIniFile->SetLabel(g_Platform->GetConfigFileName());
  m_hyperlinkIniFile->SetURL(g_Platform->GetConfigFileName());
  m_hyperlinkLogFile->SetLabel(g_Platform->GetLogFileName());
  m_hyperlinkLogFile->SetURL(g_Platform->GetLogFileName());
  m_htmlWinAuthors->Hide();
  m_htmlWinLicense->Hide();
  m_htmlWinHelp->Hide();
  m_btnBack->Hide();
  m_htmlWinLicense->LoadFile(wxString::Format(
      "%s/license.html", g_Platform->GetSharedDataDir().c_str()));
  LoadAuthors(*m_htmlWinAuthors);
  wxBitmap logo(wxString::Format("%s/opencpn.png",
                                 g_Platform->GetSharedDataDir().c_str()),
                wxBITMAP_TYPE_ANY);

  wxString target = wxString::Format("%sdoc/local/toc_flat.html",
                                     g_Platform->GetSharedDataDir().c_str());

  if (!::wxFileExists(target))
    target = wxString::Format("%sdoc/help_web.html",
                              g_Platform->GetSharedDataDir().c_str());

  target.Prepend("file://");

  m_hyperlinkHelp->SetURL(target);
#if wxUSE_WEBVIEW && defined(HAVE_WEBVIEW)
  m_htmlWinHelp->LoadURL(target);
#else
  m_htmlWinHelp->LoadFile(target);
#endif
  m_bitmapLogo->SetBitmap(logo);

  int width = m_scrolledWindowAbout->GetSizer()->GetSize().GetWidth() +
              m_bitmapLogo->GetSize().GetWidth() + EXTEND_WIDTH;
  int height = m_scrolledWindowAbout->GetSizer()->GetSize().GetHeight() +
               m_panelMainLinks->GetSizer()->GetSize().GetHeight() +
               EXTEND_HEIGHT;

  SetMinSize(wxSize(width, height));
  RecalculateSize();
}

void AboutFrameImpl::OnLinkHelp(wxHyperlinkEvent& event) {
#ifdef __WXGTK__
  wxString testFile = wxString::Format("/%s/doc/help_web.html",
                                       g_Platform->GetSharedDataDir().c_str());
  if (!::wxFileExists(testFile)) {
    wxString msg = _("OpenCPN Help documentation is not available locally.");
    msg += _T("\n");
    msg +=
        _("Would you like to visit the opencpn.org website for more "
          "information?");

    if (wxID_YES ==
        OCPNMessageBox(NULL, msg, _("OpenCPN Info"), wxYES_NO | wxCENTER, 60)) {
      wxLaunchDefaultBrowser(_T("https://opencpn.org"));
    }
  } else
#endif
  {
    m_htmlWinAuthors->Hide();
    m_htmlWinLicense->Hide();
    m_htmlWinHelp->Show();
    m_scrolledWindowAbout->Hide();
    m_btnBack->Show();
#if wxUSE_WEBVIEW && defined(HAVE_WEBVIEW)
    m_btnBack->Enable(m_htmlWinHelp->CanGoBack());
#else
    m_btnBack->Enable(m_htmlWinHelp->HistoryCanBack());
#endif
    wxSize parentSize = m_parent->GetSize();
    SetSize(wxSize(parentSize.x * 9 / 10, parentSize.y * 9 / 10));
    Centre();
  }
}

void AboutFrameImpl::OnLinkLicense(wxHyperlinkEvent& event) {
  m_htmlWinAuthors->Hide();
  m_htmlWinLicense->Show();
  m_htmlWinHelp->Hide();
  m_btnBack->Hide();
  m_scrolledWindowAbout->Hide();
  Layout();
}

void AboutFrameImpl::OnLinkAuthors(wxHyperlinkEvent& event) {
  m_htmlWinAuthors->Show();
  m_htmlWinLicense->Hide();
  m_htmlWinHelp->Hide();
  m_btnBack->Hide();
  m_scrolledWindowAbout->Hide();
  Layout();
}

void AboutFrameImpl::AboutFrameOnActivate(wxActivateEvent& event) {
  m_htmlWinAuthors->Hide();
  m_htmlWinLicense->Hide();
  m_htmlWinHelp->Hide();
  m_btnBack->Hide();
  m_scrolledWindowAbout->Show();
  Layout();
  m_scrolledWindowAbout->Refresh();
  m_panelMainLinks->Refresh();
}

void AboutFrameImpl::RecalculateSize(void) {
#ifdef __OCPN__ANDROID__
  //  Make an estimate of the dialog size, without scrollbars showing

  wxSize esize;
  esize.x = GetCharWidth() * 110;
  esize.y = GetCharHeight() * 20;

  wxSize dsize = GetParent()->GetClientSize();
  esize.y = wxMin(esize.y, dsize.y - (2 * GetCharHeight()));
  esize.x = wxMin(esize.x, dsize.x - (1 * GetCharHeight()));
  SetClientSize(esize);

  wxSize fsize = GetSize();
  fsize.y = wxMin(fsize.y, dsize.y - (2 * GetCharHeight()));
  fsize.x = wxMin(fsize.x, dsize.x - (1 * GetCharHeight()));

  SetSize(fsize);
  Centre();

#else
  Fit();
  Centre();
#endif
}
