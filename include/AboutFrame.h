///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 26 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbmp.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/hyperlink.h>
#include <wx/scrolwin.h>
#include <wx/html/htmlwin.h>
#if wxUSE_WEBVIEW && defined(HAVE_WEBVIEW)
#include <wx/webview.h>
#endif
#include <wx/panel.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class AboutFrame
///////////////////////////////////////////////////////////////////////////////
class AboutFrame : public wxFrame {
private:
protected:
  wxStaticBitmap* m_bitmapLogo;
  wxButton* m_btnBack;
  wxScrolledWindow* m_scrolledWindowAbout;
  wxStaticText* m_staticTextOCPN;
  wxStaticText* m_staticTextVersion;
  wxStaticText* m_staticTextCopyYears;
  wxHyperlinkCtrl* m_hyperlinkAuthors;
  wxStaticText* m_staticTextDescription;
  wxStaticText* m_staticTextHelp;
  wxHyperlinkCtrl* m_hyperlinkDonate;
  wxStaticText* m_staticTextOr;
  wxHyperlinkCtrl* m_hyperlinkGetInvolved;
  wxStaticText* m_staticTextLogfile;
  wxHyperlinkCtrl* m_hyperlinkLogFile;
  wxStaticText* m_staticTextIniFile;
  wxHyperlinkCtrl* m_hyperlinkIniFile;
  wxHtmlWindow* m_htmlWinAuthors;
  wxHtmlWindow* m_htmlWinLicense;
#if wxUSE_WEBVIEW && defined(HAVE_WEBVIEW)
  wxWebView* m_htmlWinHelp;
#else
  wxHtmlWindow* m_htmlWinHelp;
#endif
  wxPanel* m_panelMainLinks;
  wxHyperlinkCtrl* m_hyperlinkWebsite;
  wxHyperlinkCtrl* m_hyperlinkHelp;
  wxHyperlinkCtrl* m_hyperlinkLicense;

  // Virtual event handlers, overide them in your derived class
  virtual void AboutFrameOnActivate(wxActivateEvent& event) { event.Skip(); }
  virtual void AboutFrameOnClose(wxCloseEvent& event) { event.Skip(); }
  virtual void m_btnBackOnButtonClick(wxCommandEvent& event) { event.Skip(); }
  virtual void OnLinkAuthors(wxHyperlinkEvent& event) { event.Skip(); }
  virtual void OnLinkDonate(wxHyperlinkEvent& event) { event.Skip(); }
  virtual void OnLinkGetInvolved(wxHyperlinkEvent& event) { event.Skip(); }
  virtual void OnLinkLogfile(wxHyperlinkEvent& event) { event.Skip(); }
  virtual void OnLinkConfigfile(wxHyperlinkEvent& event) { event.Skip(); }
  virtual void m_htmlWinHelpOnHtmlLinkClicked(wxHtmlLinkEvent& event) {
    event.Skip();
  }
  virtual void OnLinkHelp(wxHyperlinkEvent& event) { event.Skip(); }
  virtual void OnLinkLicense(wxHyperlinkEvent& event) { event.Skip(); }

public:
  AboutFrame(wxWindow* parent, wxWindowID id = wxID_ANY,
             const wxString& title = _("About OpenCPN"),
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxSize(500, 300),
             long style = wxCAPTION | wxCLOSE_BOX | wxFRAME_FLOAT_ON_PARENT |
                          wxFRAME_NO_TASKBAR | wxRESIZE_BORDER |
                          wxTAB_TRAVERSAL);

  ~AboutFrame();
};
