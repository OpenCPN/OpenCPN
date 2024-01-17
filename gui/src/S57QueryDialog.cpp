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

#include <wx/wxprec.h>
#include <wx/wxhtml.h>

#include "S57QueryDialog.h"
#include "navutil.h"
#include "gui_lib.h"
#include <wx/textwrapper.h>
#include "color_types.h"

extern ColorScheme global_color_scheme;
extern S57QueryDialog* g_pObjectQueryDialog;
extern int g_S57_dialog_sx;
extern int g_S57_dialog_sy;
extern int g_S57_extradialog_sx;
extern int g_S57_extradialog_sy;
extern bool g_bresponsive;

// Private class implementations
class MessageHardBreakWrapper : public wxTextWrapper {
public:
  MessageHardBreakWrapper(wxWindow* win, const wxString& text, int widthMax) {
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

IMPLEMENT_CLASS(S57QueryDialog, wxFrame)
// S57QueryDialog event table definition
BEGIN_EVENT_TABLE(S57QueryDialog, wxFrame)  // ws wxDialog
EVT_SIZE(S57QueryDialog::OnSize)
EVT_CLOSE(S57QueryDialog::OnClose)
EVT_HTML_LINK_CLICKED(wxID_ANY, S57QueryDialog::OnHtmlLinkClicked)
EVT_CHAR_HOOK(S57QueryDialog::OnKey)
END_EVENT_TABLE()

S57QueryDialog::S57QueryDialog() { Init(); }

S57QueryDialog::S57QueryDialog(wxWindow* parent, wxWindowID id,
                               const wxString& caption, const wxPoint& pos,
                               const wxSize& size, long style) {
  Init();
  Create(parent, id, caption, pos, size, style);
}

S57QueryDialog::~S57QueryDialog() {
  g_S57_dialog_sx = GetSize().x;
  g_S57_dialog_sy = GetSize().y;
  m_btnOK->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
                      wxCommandEventHandler(S57QueryDialog::OnOKClick), NULL,
                      this);
}

void S57QueryDialog::Init() {}

bool S57QueryDialog::Create(wxWindow* parent, wxWindowID id,
                            const wxString& caption, const wxPoint& pos,
                            const wxSize& size, long style) {
  //    As a display optimization....
  //    if current color scheme is other than DAY,
  //    Then create the dialog ..WITHOUT.. borders and title bar.
  //    This way, any window decorations set by external themes, etc
  //    will not detract from night-vision

  long wstyle = wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT;

  if ((global_color_scheme != GLOBAL_COLOR_SCHEME_DAY) &&
      (global_color_scheme != GLOBAL_COLOR_SCHEME_RGB))
    wstyle |= (wxNO_BORDER);

  if (!wxFrame::Create(parent, id, caption, pos, size, wstyle)) return false;

  wxFont* dFont = GetOCPNScaledFont(_("ObjectQuery"));

  SetFont(*dFont);
  CreateControls();

  m_createsize = size;
  /*
  // This ensures that the dialog cannot be sized smaller
  // than the minimum size
      GetSizer()->SetSizeHints( this );

  // Explicitely set the size
      SetSize( size );

  // Centre the dialog on the parent or (if none) screen
      Centre();
  */
  RecalculateSize();

  DimeControl(this);
  return true;
}

void S57QueryDialog::RecalculateSize(void) {
  //  Make an estimate of the dialog size, without scrollbars showing

  wxSize esize = m_createsize;
  if (g_bresponsive) {
    esize = GetParent()->GetClientSize();
  }

  wxSize dsize = GetParent()->GetClientSize();
  esize.y = wxMin(esize.y, dsize.y - (1 * GetCharHeight()));
  esize.x = wxMin(esize.x, dsize.x - (1 * GetCharHeight()));
  SetSize(esize);

  wxSize fsize = GetSize();
  fsize.y = wxMin(fsize.y, dsize.y - (2 * GetCharHeight()));
  fsize.x = wxMin(fsize.x, dsize.x - (2 * GetCharHeight()));
  SetSize(fsize);

  Centre();
}

void S57QueryDialog::CreateControls() {
  wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(topSizer);

  long style = wxHW_SCROLLBAR_AUTO;
  if (g_btouch) style |= wxHW_NO_SELECTION;

  m_phtml =
      new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, style);

  m_phtml->SetBorders(5);

  m_phtml->SetMinSize(wxSize(100, 100));  // this will constrain the dialog, too
  topSizer->Add(m_phtml, 1, wxBOTTOM | wxEXPAND, 10);

  topSizer->FitInside(this);

  m_btnOK = new wxButton(this, wxID_OK);
  m_btnOK->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                   wxCommandEventHandler(S57QueryDialog::OnOKClick), NULL,
                   this);
  topSizer->Add(m_btnOK, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 5);
}

void S57QueryDialog::SetColorScheme(void) {
  DimeControl(this);
  wxColor bg = GetBackgroundColour();
  m_phtml->SetBackgroundColour(bg);
  SetBackgroundColour(
      bg);  // This looks like non-sense, but is needed for __WXGTK__
            // to get colours to propagate down the control's family tree.

#ifdef __WXQT__
  //  wxQT has some trouble clearing the background of HTML window...
  wxBitmap tbm(GetSize().x, GetSize().y, -1);
  wxMemoryDC tdc(tbm);
  //    wxColour cback = GetGlobalColor( _T("YELO1") );
  tdc.SetBackground(bg);
  tdc.Clear();
  m_phtml->SetBackgroundImage(tbm);
#endif
}

void S57QueryDialog::OnKey(wxKeyEvent& ke) {
  if (ke.GetKeyCode() == WXK_ESCAPE)
    Close(true);
  else
    ke.Skip();
}

void S57QueryDialog::SetHTMLPage(wxString& page) {
  m_phtml->SetPage(page);
  SetColorScheme();
}

void S57QueryDialog::OnSize(wxSizeEvent& event) {
  g_S57_dialog_sx = GetSize().x;
  g_S57_dialog_sy = GetSize().y;
  wxFrame::OnSize(event);
}

void S57QueryDialog::OnClose(wxCloseEvent& event) {
  g_S57_dialog_sx = GetSize().x;
  g_S57_dialog_sy = GetSize().y;
  Destroy();
  g_pObjectQueryDialog = NULL;
}

void S57QueryDialog::OnHtmlLinkClicked(wxHtmlLinkEvent& event) {
  S57ExtraQueryInfoDlg* ExtraObjInfoDlg = new S57ExtraQueryInfoDlg(
      GetParent(), wxID_ANY, _("Extra Object Info"),
      wxPoint(GetPosition().x + 20, GetPosition().y + 20),
      wxSize(g_S57_extradialog_sx, g_S57_extradialog_sy));

  // Check te kind of file, load text files serial and pictures direct
  wxFileName filen(event.GetLinkInfo().GetHref());
  wxString Extensions = wxString("txt,html,rtf");

  if (Extensions.Find(filen.GetExt().Lower()) == wxNOT_FOUND)
    ExtraObjInfoDlg->m_phtml->LoadPage(event.GetLinkInfo().GetHref());
  else {
    wxTextFile txf(filen.GetFullPath());
    if (txf.Open()) {
      wxString contents;
      wxString str;
      str = txf.GetFirstLine();
      do {
        MessageHardBreakWrapper wrapper(ExtraObjInfoDlg->m_phtml, str,
                                        m_phtml->GetSize().x * 9 / 10);
        contents += wrapper.GetWrapped();
        contents += "<br>";

        str = txf.GetNextLine();
      } while (!txf.Eof());

      ExtraObjInfoDlg->m_phtml->SetPage(contents);
    }
  }

  ExtraObjInfoDlg->SetColorScheme();

#ifdef __OCPN__ANDROID__
  ExtraObjInfoDlg->SetSize(GetSize().x - 40, GetSize().y - 40);
#endif

  ExtraObjInfoDlg->Show(true);
}

///////////////////////////////////////////////////////////////

IMPLEMENT_CLASS(S57ExtraQueryInfoDlg, wxFrame)
// S57QueryDialog event table definition
BEGIN_EVENT_TABLE(S57ExtraQueryInfoDlg, wxFrame)  // ws wxDialog
EVT_SIZE(S57ExtraQueryInfoDlg::OnSize)
EVT_CLOSE(S57ExtraQueryInfoDlg::OnClose)
END_EVENT_TABLE()

S57ExtraQueryInfoDlg::S57ExtraQueryInfoDlg() { Init(); }

S57ExtraQueryInfoDlg::S57ExtraQueryInfoDlg(wxWindow* parent, wxWindowID id,
                                           const wxString& caption,
                                           const wxPoint& pos,
                                           const wxSize& size, long style) {
  Init();
  Create(parent, id, caption, pos, size, style);
}
bool S57ExtraQueryInfoDlg::Create(wxWindow* parent, wxWindowID id,
                                  const wxString& caption, const wxPoint& pos,
                                  const wxSize& size, long style) {
  //    As a display optimization....
  //    if current color scheme is other than DAY,
  //    Then create the dialog ..WITHOUT.. borders and title bar.
  //    This way, any window decorations set by external themes, etc
  //    will not detract from night-vision

  long wstyle = wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT;

  if ((global_color_scheme != GLOBAL_COLOR_SCHEME_DAY) &&
      (global_color_scheme != GLOBAL_COLOR_SCHEME_RGB))
    wstyle |= (wxNO_BORDER);

  if (!wxFrame::Create(parent, id, caption, pos, size, wstyle)) return false;

  wxFont* dFont = GetOCPNScaledFont(_("ObjectQuery"));

  SetFont(*dFont);
  CreateControls();

  DimeControl(this);
  return true;
}
S57ExtraQueryInfoDlg::~S57ExtraQueryInfoDlg() {
  g_S57_extradialog_sx = GetSize().x;
  g_S57_extradialog_sy = GetSize().y;
}

void S57ExtraQueryInfoDlg::OnSize(wxSizeEvent& event) {
  g_S57_extradialog_sx = GetSize().x;
  g_S57_extradialog_sy = GetSize().y;
  wxFrame::OnSize(event);
}

void S57ExtraQueryInfoDlg::OnClose(wxCloseEvent& event) {
  g_S57_extradialog_sx = GetSize().x;
  g_S57_extradialog_sy = GetSize().y;
  Destroy();
}
