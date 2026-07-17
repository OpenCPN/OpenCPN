///////////////////////////////////////////////////////////////////////////
// wxFormBuilder-generated layout; implementations split for maintainability.
///////////////////////////////////////////////////////////////////////////

#include "chartdldr_pi.h"
#include "chartdldrgui.h"
#include "chartdldr_prefs.h"
#include <wx/msgdlg.h>
#include <wx/scrolwin.h>
#include <wx/spinctrl.h>
#include <wx/textwrapper.h>

#ifdef __OCPN__ANDROID__
#include <QtWidgets/QScroller>
#include "qdebug.h"
#endif

#ifdef __OCPN__ANDROID__
extern QString qtStyleSheet;
#endif

class DLDR_ChartDirPanelHardBreakWrapper : public wxTextWrapper {
public:
  DLDR_ChartDirPanelHardBreakWrapper(wxWindow* win, const wxString& text,
                                     int widthMax) {
    m_lineCount = 0;

    // Replace all spaces in the string with a token character '^'
    wxString textMod = text;
    textMod.Replace(" ", "^");

    // Replace all path separators with spaces
    wxString sep = wxFileName::GetPathSeparator();
    textMod.Replace(sep, " ");

    Wrap(win, textMod, widthMax);

    // walk the output array, repairing the substitutions
    for (size_t i = 0; i < m_array.GetCount(); i++) {
      wxString a = m_array[i];
      a.Replace(" ", sep);
      if (m_array.GetCount() > 1) {
        if (i < m_array.GetCount() - 1) a += sep;
      }
      a.Replace("^", " ");
      m_array[i] = a;
    }
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

BEGIN_EVENT_TABLE(DLDR_OCPNChartDirPanel, wxPanel)
EVT_PAINT(DLDR_OCPNChartDirPanel::OnPaint)
END_EVENT_TABLE()

DLDR_OCPNChartDirPanel::DLDR_OCPNChartDirPanel(wxWindow* parent, wxWindowID id,
                                               const wxPoint& pos,
                                               const wxSize& size)
    : wxPanel(parent, id, pos, size, wxBORDER_NONE) {
  // On Android, shorten the displayed path name by removing well-known prefix
  // if
  // (cdi.fullpath.StartsWith("/storage/emulated/0/Android/data/org.opencpn.opencpn/files"))
  //  m_ChartDir = "..." + cdi.fullpath.Mid(58);

  m_refHeight = GetCharHeight();

  m_unselectedHeight = 2 * m_refHeight;

  SetMinSize(wxSize(-1, m_unselectedHeight));

  wxColour colour;
  GetGlobalColor(_T("UIBCK"), &colour);
  m_boxColour = colour;
}

DLDR_OCPNChartDirPanel::~DLDR_OCPNChartDirPanel() {}

void DLDR_OCPNChartDirPanel::SetText(wxString text) {
  m_ChartDir = text;

  int maxWidth = GetParent()->GetSize().x * 75 / 100;
  DLDR_ChartDirPanelHardBreakWrapper wrapper(this, m_ChartDir, maxWidth);
  wxArrayString nameWrapped = wrapper.GetLineArray();
  int lineCount = nameWrapped.GetCount();
  if (lineCount > 1) lineCount++;
  SetMinSize(wxSize(-1, (lineCount * m_refHeight * 3 / 2)));
  GetParent()->Layout();
  Refresh();
}

void DLDR_OCPNChartDirPanel::OnPaint(wxPaintEvent& event) {
  int width, height;
  GetSize(&width, &height);
  wxPaintDC dc(this);

  dc.SetPen(*wxTRANSPARENT_PEN);
  dc.SetBrush(wxBrush(GetBackgroundColour()));
  dc.DrawRectangle(GetVirtualSize());

  wxColour c;

  wxString nameString = m_ChartDir;
  int maxWidth = GetParent()->GetSize().x * 75 / 100;

  DLDR_ChartDirPanelHardBreakWrapper wrapper(this, nameString, maxWidth);
  wxArrayString nameWrapped = wrapper.GetLineArray();

  if (height < (int)(nameWrapped.GetCount() + 1) * m_refHeight) {
    SetMinSize(wxSize(-1, (nameWrapped.GetCount() + 1) * m_refHeight));
    GetParent()->GetSizer()->Layout();
  }

  if (1) {
    dc.SetBrush(wxBrush(m_boxColour));

    GetGlobalColor(_T ( "UITX1" ), &c);
    dc.SetPen(wxPen(wxColor(0xCE, 0xD5, 0xD6), 3));

    dc.DrawRoundedRectangle(0, 0, width - 1, height - 1, height / 10);

    int offset = height / 10;
    int text_x = offset;

    wxFont* dFont = GetOCPNScaledFont_PlugIn(_("Dialog"));
    dc.SetFont(*dFont);

    dc.SetTextForeground(wxColour(64, 64, 64));

    int yd = height * 5 / 100;
    for (size_t i = 0; i < nameWrapped.GetCount(); i++) {
      if (i == 0)
        dc.DrawText(nameWrapped[i], text_x, yd);
      else
        dc.DrawText(nameWrapped[i], text_x + GetCharWidth() / 2, yd);
      yd += GetCharHeight();
    }
  }  // selected
}

void AddSourceDlg::applyStyle() {
#ifdef __OCPN__ANDROID__
  m_panelPredefined->GetHandle()->setStyleSheet(qtStyleSheet);
  QScroller::ungrabGesture(m_panelPredefined->GetHandle());
  ///    QScroller::ungrabGesture(m_treeCtrlPredefSrcs->GetHandle());
#endif
}

AddSourceDlg::AddSourceDlg(wxWindow* parent, wxWindowID id,
                           const wxString& title, const wxPoint& pos,
                           const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style | wxRESIZE_BORDER) {
  this->SetSizeHints(wxSize(500, -1), wxDefaultSize);

  wxBoxSizer* bSizerMain = new wxBoxSizer(wxVERTICAL);
  this->SetSizer(bSizerMain);

  wxStaticBoxSizer* sbSizerSourceSel = new wxStaticBoxSizer(
      new wxStaticBox(this, wxID_ANY, _("Catalog")), wxVERTICAL);
  bSizerMain->Add(sbSizerSourceSel, 1, wxALL | wxEXPAND, 5);

  m_nbChoice = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                              wxNB_TOP);
  sbSizerSourceSel->Add(m_nbChoice, 1, wxEXPAND | wxALL, 5);
  m_nbChoice->SetMinSize(wxSize(-1, 6 * GetCharHeight()));
#ifdef __WXOSX__
  m_nbChoice->SetMinSize(wxSize(-1, 8 * GetCharHeight()));
#endif

  m_panelPredefined = new wxPanel(m_nbChoice, wxID_ANY, wxDefaultPosition,
                                  wxDefaultSize, wxTAB_TRAVERSAL);
  m_nbChoice->AddPage(m_panelPredefined, _("Predefined"), false);

  wxBoxSizer* bSizerPredefTree = new wxBoxSizer(wxVERTICAL);

  m_treeCtrlPredefSrcs = new wxTreeCtrl(
      m_panelPredefined, wxID_ANY, wxDefaultPosition, wxDefaultSize,
      wxTR_DEFAULT_STYLE | wxTR_HIDE_ROOT | wxTR_HAS_BUTTONS);
  bSizerPredefTree->Add(m_treeCtrlPredefSrcs, 1, wxALL | wxEXPAND, 5);
#ifdef __OCPN__ANDROID__
  m_treeCtrlPredefSrcs->SetScrollRate(0, 1);
#endif
  m_panelPredefined->SetSizer(bSizerPredefTree);
  m_panelPredefined->Layout();
  bSizerPredefTree->Fit(m_treeCtrlPredefSrcs /*m_panelPredefined*/);

  m_panelCustom = new wxPanel(m_nbChoice, wxID_ANY, wxDefaultPosition,
                              wxDefaultSize, wxTAB_TRAVERSAL);
  wxFlexGridSizer* fgSizerSourceSel;
  fgSizerSourceSel = new wxFlexGridSizer(0, 2, 0, 0);
  fgSizerSourceSel->AddGrowableCol(1);
  fgSizerSourceSel->SetFlexibleDirection(wxBOTH);
  fgSizerSourceSel->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

  m_stName = new wxStaticText(m_panelCustom, wxID_ANY, _("Name"),
                              wxDefaultPosition, wxDefaultSize, 0);
  m_stName->Wrap(-1);
  fgSizerSourceSel->Add(m_stName, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_tSourceName = new wxTextCtrl(m_panelCustom, wxID_ANY, wxEmptyString,
                                 wxDefaultPosition, wxDefaultSize, 0);
  m_tSourceName->SetMaxLength(0);
  fgSizerSourceSel->Add(m_tSourceName, 0, wxALL | wxEXPAND, 5);

  m_stUrl = new wxStaticText(m_panelCustom, wxID_ANY, _("URL"),
                             wxDefaultPosition, wxDefaultSize, 0);
  m_stUrl->Wrap(-1);
  fgSizerSourceSel->Add(m_stUrl, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_tChartSourceUrl = new wxTextCtrl(m_panelCustom, wxID_ANY, wxEmptyString,
                                     wxDefaultPosition, wxSize(-1, -1), 0);
  m_tChartSourceUrl->SetMaxLength(0);
  fgSizerSourceSel->Add(m_tChartSourceUrl, 0, wxALL | wxEXPAND, 5);

  m_panelCustom->SetSizer(fgSizerSourceSel);
  m_panelCustom->Layout();
  fgSizerSourceSel->Fit(m_panelCustom);
  m_nbChoice->AddPage(m_panelCustom, _("Custom"), true);

  wxStaticBoxSizer* sbSizerChartDir;
  sbSizerChartDir = new wxStaticBoxSizer(
      new wxStaticBox(this, wxID_ANY,
                      _("Proposed chart installation directory")),
      wxVERTICAL);
  bSizerMain->Add(sbSizerChartDir, 0, wxALL | wxEXPAND, 5);

  wxBoxSizer* dirbox = new wxBoxSizer(wxVERTICAL);
  sbSizerChartDir->Add(dirbox, 0, wxEXPAND);

  m_tcChartDirectory = new wxTextCtrl(this, wxID_ANY, _T(""), wxDefaultPosition,
                                      wxSize(200, -1), wxTE_READONLY);
  dirbox->Add(m_tcChartDirectory, 1, wxALL | wxEXPAND, 5);
  m_tcChartDirectory->Hide();

  m_panelChartDirectory = new DLDR_OCPNChartDirPanel(
      this, wxID_ANY, wxDefaultPosition, wxSize(-1, -1));
  // m_panelChartDirectory->SetSizeHints(5000, -1);
  dirbox->Add(m_panelChartDirectory, 1, wxALL | wxEXPAND, 5);

  wxBoxSizer* dirbox1 = new wxBoxSizer(wxHORIZONTAL);
  sbSizerChartDir->Add(dirbox1, 0, wxALIGN_RIGHT);
  m_buttonChartDirectory =
      new wxButton(this, wxID_ANY, _("Select a different directory"));
  dirbox1->Add(m_buttonChartDirectory, 0, wxALL | wxEXPAND, 5);
  m_buttonChartDirectory->Disable();

  wxBoxSizer* buttons = new wxBoxSizer(wxHORIZONTAL);
  bSizerMain->Add(buttons, 0, wxALIGN_RIGHT);

  m_sdbSizerBtnsOK = new wxButton(this, wxID_OK, _("OK"));
  buttons->Add(m_sdbSizerBtnsOK, 1, wxALL, 5);
  m_sdbSizerBtnsCancel = new wxButton(this, wxID_CANCEL, _("Cancel"));
  buttons->Add(m_sdbSizerBtnsCancel, 1, wxALL, 5);

  this->Layout();

  m_sourceswin = NULL;

  // Connect Events (overridable handlers are bound by ChartDldrGuiAddSourceDlg)
  m_buttonChartDirectory->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(AddSourceDlg::OnDirSelClick), NULL, this);
}

void AddSourceDlg::OnNbPage(wxNotebookEvent& event) {
  if (event.GetSelection() == 1) {
    m_buttonChartDirectory->Enable();
  } else {
    wxTreeItemId item = m_treeCtrlPredefSrcs->GetSelection();
    ChartSource* cs = nullptr;
    if (item.IsOk())
      cs = (ChartSource*)(m_treeCtrlPredefSrcs->GetItemData(item));
    if (!cs) m_buttonChartDirectory->Disable();
  }
}

AddSourceDlg::~AddSourceDlg() {
  m_buttonChartDirectory->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(AddSourceDlg::OnDirSelClick), NULL, this);
}

void AddSourceDlg::OnDirSelClick(wxCommandEvent& event) {
  wxString dir_spec;
  int response = PlatformDirSelectorDialog(this, &dir_spec,
                                           _("Choose Chart File Directory"),
                                           m_tcChartDirectory->GetValue());

  if (response == wxID_OK) {
    wxFileName fn(m_dirExpanded);

    if (!dir_spec.EndsWith(fn.GetName())) {
      dir_spec += wxFileName::GetPathSeparator();
      dir_spec += fn.GetName();
    }

    m_tcChartDirectory->SetValue(dir_spec);
    m_panelChartDirectory->SetText(dir_spec);
  }
}
