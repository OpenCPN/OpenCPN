///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun  6 2014)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "chartdldr_pi.h"
#include "chartdldrgui.h"
#include <wx/msgdlg.h>
#include <wx/scrolwin.h>
#include <wx/textwrapper.h>

///////////////////////////////////////////////////////////////////////////

#ifdef __OCPN__ANDROID__

QString qtStyleSheet =
    "QScrollBar:horizontal {\
border: 0px solid grey;\
background-color: rgb(240, 240, 240);\
height: 50px;\
margin: 0px 1px 0 1px;\
}\
QScrollBar::handle:horizontal {\
background-color: rgb(200, 200, 200);\
min-width: 20px;\
border-radius: 10px;\
}\
QScrollBar::add-line:horizontal {\
border: 0px solid grey;\
background: #32CC99;\
width: 0px;\
subcontrol-position: right;\
subcontrol-origin: margin;\
}\
QScrollBar::sub-line:horizontal {\
border: 0px solid grey;\
background: #32CC99;\
width: 0px;\
subcontrol-position: left;\
subcontrol-origin: margin;\
}\
QScrollBar:vertical {\
border: 0px solid grey;\
background-color: rgb(240, 240, 240);\
width: 50px;\
margin: 1px 0px 1px 0px;\
}\
QScrollBar::handle:vertical {\
background-color: rgb(200, 200, 200);\
min-height: 50px;\
border-radius: 10px;\
}\
QScrollBar::add-line:vertical {\
border: 0px solid grey;\
background: #32CC99;\
height: 0px;\
subcontrol-position: top;\
subcontrol-origin: margin;\
}\
QScrollBar::sub-line:vertical {\
border: 0px solid grey;\
background: #32CC99;\
height: 0px;\
subcontrol-position: bottom;\
subcontrol-origin: margin;\
}\
QCheckBox {\
spacing: 25px;\
}\
QCheckBox::indicator {\
width: 30px;\
height: 30px;\
}\
";

#endif

#ifdef __OCPN__ANDROID__
#include <QtWidgets/QScroller>
#include "qdebug.h"
#endif

extern chartdldr_pi* g_pi;

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

  // Connect Events
  m_treeCtrlPredefSrcs->Connect(
      wxEVT_COMMAND_TREE_SEL_CHANGED,
      wxTreeEventHandler(AddSourceDlg::OnSourceSelected), NULL, this);
  m_sdbSizerBtnsOK->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                            wxCommandEventHandler(AddSourceDlg::OnOkClick),
                            NULL, this);
  m_sdbSizerBtnsCancel->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(AddSourceDlg::OnCancelClick), NULL, this);
  m_buttonChartDirectory->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(AddSourceDlg::OnDirSelClick), NULL, this);
  m_nbChoice->Connect(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,
                      wxNotebookEventHandler(AddSourceDlg::OnNbPage), NULL,
                      this);
}

void AddSourceDlg::OnNbPage(wxNotebookEvent& event) {
  if (event.GetSelection() == 1) {
    m_buttonChartDirectory->Enable();
  } else {
    wxTreeItemId item = m_treeCtrlPredefSrcs->GetSelection();
    ChartSource* cs = nullptr;
    if (item.IsOk())
      cs = (ChartSource *)(m_treeCtrlPredefSrcs->GetItemData(item));
    if(!cs)
      m_buttonChartDirectory->Disable();
  }
}

AddSourceDlg::~AddSourceDlg() {
  // Disconnect Events
  m_treeCtrlPredefSrcs->Disconnect(
      wxEVT_COMMAND_TREE_SEL_CHANGED,
      wxTreeEventHandler(AddSourceDlg::OnSourceSelected), NULL, this);
  m_sdbSizerBtnsOK->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
                               wxCommandEventHandler(AddSourceDlg::OnOkClick),
                               NULL, this);
  m_sdbSizerBtnsCancel->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(AddSourceDlg::OnCancelClick), NULL, this);
  m_buttonChartDirectory->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(AddSourceDlg::OnDirSelClick), NULL, this);
  m_nbChoice->Disconnect(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,
                         wxNotebookEventHandler(AddSourceDlg::OnNbPage), NULL,
                         this);
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

ChartDldrPanel::ChartDldrPanel(wxWindow* parent, wxWindowID id,
                               const wxPoint& pos, const wxSize& size,
                               long style)
    : wxPanel(parent, id, pos, size, style) {
  int border_size = 2;

  //   Main Sizer
  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(mainSizer);

  mainSizer->AddSpacer(GetCharHeight());

  m_DLoadNB = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                             wxNB_TOP);
  mainSizer->Add(m_DLoadNB, 1, wxEXPAND);

  // Catalogs
  wxPanel* catalogPanel =
      new wxPanel(m_DLoadNB, wxID_ANY, wxDefaultPosition, wxDefaultSize);
  m_DLoadNB->AddPage(catalogPanel, _("Select Catalog..."));

  wxBoxSizer* catalogPanelBoxSizer = new wxBoxSizer(wxVERTICAL);
  catalogPanel->SetSizer(catalogPanelBoxSizer);

  wxStaticText* catalogLabel =
      new wxStaticText(catalogPanel, wxID_ANY, _("Chart Catalogs Available"));
  catalogLabel->Wrap(-1);
  catalogPanelBoxSizer->Add(catalogLabel, 0, wxTOP | wxRIGHT | wxLEFT,
                            border_size);

  //   Sources list box
  wxBoxSizer* activeListSizer = new wxBoxSizer(wxVERTICAL);
  catalogPanelBoxSizer->Add(activeListSizer, 0, wxALL | wxEXPAND, 5);

  m_lbChartSources =
      new wxListCtrl(catalogPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                     wxLC_NO_SORT_HEADER | wxLC_REPORT | wxLC_SINGLE_SEL);
  activeListSizer->Add(m_lbChartSources, 1, wxEXPAND | wxRIGHT, 60);
  /// m_lbChartSources->SetMinSize( wxSize( -1, 10 * GetCharHeight() ) );

  //  Buttons

  wxBoxSizer* bSizerCatalogBtns = new wxBoxSizer(wxHORIZONTAL);
  catalogPanelBoxSizer->Add(bSizerCatalogBtns, 0, wxALL | wxEXPAND,
                            border_size);

  m_bAddSource = new wxButton(catalogPanel, wxID_ANY, _("Add Catalog"),
                              wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
  m_bAddSource->SetToolTip(_("Add a new chart catalog."));

  bSizerCatalogBtns->Add(m_bAddSource, 0, wxALL | wxEXPAND, 5);

  m_bDeleteSource = new wxButton(catalogPanel, wxID_ANY, _("Delete"),
                                 wxDefaultPosition, wxDefaultSize, 0);
  m_bDeleteSource->SetToolTip(
      _("Delete the chart catalog. The downloaded charts are not deleted."));

  bSizerCatalogBtns->Add(m_bDeleteSource, 0, wxALL | wxEXPAND, 5);

  m_bEditSource = new wxButton(catalogPanel, wxID_ANY, _("Edit..."),
                               wxDefaultPosition, wxDefaultSize, 0);
  bSizerCatalogBtns->Add(m_bEditSource, 0, wxALL | wxEXPAND, 5);

#ifdef __OCPN__ANDROID__
  m_bEditSource->Hide();
#endif

  m_bUpdateChartList = new wxButton(catalogPanel, wxID_ANY, _("Update"),
                                    wxDefaultPosition, wxDefaultSize, 0);
  m_bUpdateChartList->SetDefault();
  m_bUpdateChartList->SetToolTip(
      _("Download the current chart catalog and update the data."));

  bSizerCatalogBtns->Add(m_bUpdateChartList, 0, wxALL | wxEXPAND, 5);

  m_bUpdateAllCharts = new wxButton(catalogPanel, wxID_ANY, _("Update All"),
                                    wxDefaultPosition, wxDefaultSize, 0);
  m_bUpdateAllCharts->SetToolTip(
      _("Download all chart catalogs, and then update charts."));
  bSizerCatalogBtns->Add(m_bUpdateAllCharts, 0, wxALL | wxEXPAND, 5);

  // Charts
  wxPanel* chartsPanel = new wxPanel(m_DLoadNB, wxID_ANY, wxDefaultPosition,
                                     wxDefaultSize, wxBG_STYLE_ERASE);
  m_DLoadNB->AddPage(chartsPanel, _("Download Charts..."));

  wxBoxSizer* chartsPanelBoxSizer = new wxBoxSizer(wxVERTICAL);
  chartsPanel->SetSizer(chartsPanelBoxSizer);

  //  Chart list
  m_chartsLabel = new wxStaticText(chartsPanel, wxID_ANY, _("Charts"));
  chartsPanelBoxSizer->Add(m_chartsLabel, 0, wxALL, 4 * border_size);

#if defined(CHART_LIST)
  m_scrollWinChartList = new wxDataViewListCtrl(
      chartsPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_ROW_LINES);
  chartsPanelBoxSizer->Add(m_scrollWinChartList, 0, wxEXPAND);

  int h = m_scrollWinChartList->GetCharHeight();
  int w = m_scrollWinChartList->GetCharWidth();

  m_scrollWinChartList->SetMinSize(wxSize(-1, 12 * h));
  m_scrollWinChartList->SetRowHeight(m_scrollWinChartList->GetCharHeight() + 6);
  // Add 4 columns (checkbox, chart status, update date, description)
  wxDataViewColumn* colFlag = m_scrollWinChartList->AppendToggleColumn(
      _T(" "), wxDATAVIEW_CELL_ACTIVATABLE, wxDVC_TOGGLE_DEFAULT_WIDTH,
      wxALIGN_LEFT);

  wxDataViewColumn* colStat = m_scrollWinChartList->AppendTextColumn(
      _("Status"), wxDATAVIEW_CELL_INERT, w * 12, wxALIGN_CENTER,
      wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_RESIZABLE);

  wxDataViewColumn* colLast = m_scrollWinChartList->AppendTextColumn(
      _("Date"), wxDATAVIEW_CELL_INERT, w * 12, wxALIGN_LEFT,
      wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_RESIZABLE);

  wxDataViewColumn* colDesc = m_scrollWinChartList->AppendTextColumn(
      _("Chart Description"), wxDATAVIEW_CELL_INERT, -1, wxALIGN_LEFT,
      wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_RESIZABLE);

#else

  m_scrollWinChartList = new wxScrolledWindow(
      chartsPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
  chartsPanelBoxSizer->Add(m_scrollWinChartList, 0, wxEXPAND);
  m_scrollWinChartList->SetScrollRate(5, 5);
  m_scrollWinChartList->SetMinSize(wxSize(-1, 12 * GetCharHeight()));

  m_boxSizerCharts = new wxBoxSizer(wxVERTICAL);
  m_scrollWinChartList->SetSizer(m_boxSizerCharts);

#endif /* CHART_LIST */
  //  Buttons

  chartsPanelBoxSizer->AddSpacer(GetCharHeight());

#if defined(CHART_LIST)
  wxBoxSizer* chartsButtonBoxSizer = new wxBoxSizer(wxHORIZONTAL);
  chartsPanelBoxSizer->Add(chartsButtonBoxSizer, 0, wxEXPAND);

  m_bDnldCharts =
      new wxButton(chartsPanel, wxID_ANY, _("Download selected charts"),
                   wxDefaultPosition, wxDefaultSize, 0);
  chartsButtonBoxSizer->Add(m_bDnldCharts, 0, wxALL, 5);

  m_bSelectNew = new wxButton(chartsPanel, wxID_ANY, _("Select New"),
                              wxDefaultPosition, wxDefaultSize, 0);
  m_bSelectNew->SetToolTip(_("Check all charts that are 'New' in the list."));
  chartsButtonBoxSizer->Add(m_bSelectNew, 0, wxALL, 5);

  m_bSelectUpdated = new wxButton(chartsPanel, wxID_ANY, _("Select Updated"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  m_bSelectUpdated->SetToolTip(
      _("Select all charts that are 'Out of Date' in the list."));
  chartsButtonBoxSizer->Add(m_bSelectUpdated, 0, wxALL, 5);

  // Set text to longest string so it sizes right
  m_bSelectAll = new wxButton(chartsPanel, wxID_ANY, _("Select All"),
                              wxDefaultPosition, wxDefaultSize, 0);
  // Now change button size
  m_bSelectAll->SetSize(m_bSelectAll->GetSizeFromTextSize(
      m_bSelectAll->GetTextExtent(_("Select None"))));
  m_bSelectAll->SetToolTip(_("Select all charts in the list."));
  chartsButtonBoxSizer->Add(m_bSelectAll, 0, wxALL, 5);

  m_stCatalogInfo =
      new wxStaticText(chartsPanel, wxID_ANY,
                       _("%u charts total, %u updated, %u new, %u selected"),
                       wxDefaultPosition, wxDefaultSize, 0);
  chartsPanelBoxSizer->Add(m_stCatalogInfo, 1, wxEXPAND | wxALL, 5);

#else
  m_bDnldCharts =
      new wxButton(chartsPanel, wxID_ANY, _("Download selected charts"),
                   wxDefaultPosition, wxDefaultSize, 0);
  chartsPanelBoxSizer->Add(m_bDnldCharts, 0, wxALIGN_LEFT | wxALL, 5);

  m_stCatalogInfo = new wxStaticText(chartsPanel, wxID_ANY,
                                     "",
                                     wxDefaultPosition, wxDefaultSize, 0);
  chartsPanelBoxSizer->Add(m_stCatalogInfo, 1, wxEXPAND | wxALL, 5);
  /// mainSizer->Add( m_stCatalogInfo, 0, wxEXPAND| wxALL, 5 );

#endif /* CHART_LIST */

  this->Layout();

  //     m_lbChartSources->GetHandle()->setStyleSheet( qtStyleSheet);
  //     m_clCharts->GetHandle()->setStyleSheet( qtStyleSheet);

  // Connect Events
  this->Connect(wxEVT_PAINT, wxPaintEventHandler(ChartDldrPanel::OnPaint));
  m_lbChartSources->Connect(wxEVT_LEFT_DCLICK,
                            wxMouseEventHandler(ChartDldrPanel::OnLeftDClick),
                            NULL, this);
  m_lbChartSources->Connect(wxEVT_COMMAND_LIST_ITEM_SELECTED,
                            wxListEventHandler(ChartDldrPanel::SelectSource),
                            NULL, this);
  m_bAddSource->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                        wxCommandEventHandler(ChartDldrPanel::AddSource), NULL,
                        this);
  m_bDeleteSource->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(ChartDldrPanel::DeleteSource),
                           NULL, this);
  m_bEditSource->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                         wxCommandEventHandler(ChartDldrPanel::EditSource),
                         NULL, this);
  m_bUpdateChartList->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPanel::UpdateChartList), NULL, this);
  m_bUpdateAllCharts->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPanel::UpdateAllCharts), NULL, this);
#if defined(CHART_LIST)
  m_bSelectAll->SetSize(m_bSelectAll->GetSizeFromTextSize(
      m_bSelectAll->GetTextExtent(_("Select None"))));
  m_bSelectNew->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPanel::OnSelectNewCharts), NULL, this);
  m_bSelectUpdated->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPanel::OnSelectUpdatedCharts), NULL, this);
  m_bSelectAll->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPanel::OnSelectAllCharts), NULL, this);
#if !defined(__WXOSX__)  // This event is not available under OSX and is seldom,
                         // if ever, used
  m_scrollWinChartList->Connect(
      wxEVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK,
      wxMouseEventHandler(ChartDldrPanel::OnContextMenu), NULL, this);
#endif /* __WXOSX__ */
  m_scrollWinChartList->Connect(
      wxEVT_DATAVIEW_ITEM_VALUE_CHANGED,
      wxCommandEventHandler(ChartDldrPanel::OnSelectChartItem), NULL, this);
  m_scrollWinChartList->Connect(
      wxEVT_DATAVIEW_ITEM_CONTEXT_MENU,
      wxMouseEventHandler(ChartDldrPanel::OnContextMenu), NULL, this);
#else
  m_scrollWinChartList->Connect(
      wxEVT_RIGHT_DOWN, wxMouseEventHandler(ChartDldrPanel::OnContextMenu),
      NULL, this);
#endif /* CHART_LIST */
  // m_bHelp->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(
  // ChartDldrPanel::DoHelp ), NULL, this );
  m_bDnldCharts->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPanel::OnDownloadCharts), NULL, this);
  // m_bShowLocal->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(
  // ChartDldrPanel::OnShowLocalDir ), NULL, this );

  this->Connect(wxEVT_SIZE, wxSizeEventHandler(ChartDldrPanel::OnSize));
}
ChartDldrPanel::~ChartDldrPanel() {
  // Disconnect Events
  this->Disconnect(wxEVT_PAINT, wxPaintEventHandler(ChartDldrPanel::OnPaint));
  m_lbChartSources->Disconnect(
      wxEVT_LEFT_DCLICK, wxMouseEventHandler(ChartDldrPanel::OnLeftDClick),
      NULL, this);
  m_lbChartSources->Disconnect(wxEVT_COMMAND_LIST_ITEM_SELECTED,
                               wxListEventHandler(ChartDldrPanel::SelectSource),
                               NULL, this);
  m_bAddSource->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(ChartDldrPanel::AddSource),
                           NULL, this);
  m_bDeleteSource->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPanel::DeleteSource), NULL, this);
  m_bEditSource->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
                            wxCommandEventHandler(ChartDldrPanel::EditSource),
                            NULL, this);
  m_bUpdateChartList->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPanel::UpdateChartList), NULL, this);
  m_bUpdateAllCharts->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPanel::UpdateAllCharts), NULL, this);

#if defined(CHART_LIST)
  m_bSelectNew->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPanel::OnSelectNewCharts), NULL, this);
  m_bSelectUpdated->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPanel::OnSelectUpdatedCharts), NULL, this);
  m_bSelectAll->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPanel::OnSelectAllCharts), NULL, this);
#if !defined(__WXOSX__)
  m_scrollWinChartList->Disconnect(
      wxEVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK,
      wxMouseEventHandler(ChartDldrPanel::OnContextMenu), NULL, this);
#endif /* __WXOSX__ */
  m_scrollWinChartList->Disconnect(
      wxEVT_DATAVIEW_ITEM_VALUE_CHANGED,
      wxCommandEventHandler(ChartDldrPanel::OnSelectChartItem), NULL, this);
  m_scrollWinChartList->Disconnect(
      wxEVT_DATAVIEW_ITEM_CONTEXT_MENU,
      wxMouseEventHandler(ChartDldrPanel::OnContextMenu), NULL, this);
#else
  m_scrollWinChartList->Disconnect(
      wxEVT_RIGHT_DOWN, wxMouseEventHandler(ChartDldrPanel::OnContextMenu),
      NULL, this);
#endif /* CHART_LIST */

  // m_bHelp->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(
  // ChartDldrPanel::DoHelp ), NULL, this );
  m_bDnldCharts->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPanel::OnDownloadCharts), NULL, this);
  // m_bShowLocal->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED,
  // wxCommandEventHandler( ChartDldrPanel::OnShowLocalDir ), NULL, this );
  this->Disconnect(wxEVT_SIZE, wxSizeEventHandler(ChartDldrPanel::OnSize));
}

void ChartDldrPanel::OnSize(wxSizeEvent& event) {
  //     wxSize newSize = event.GetSize();
  //     qDebug() << "Size: " << newSize.x << newSize.y;
  //
  //     int width, height;
  //     ::wxDisplaySize(&width, &height);
  //     qDebug() << "wxDisplaySize: " << width << height;

  // We want the variable sized windows to to be small enough so that
  // no vertical scroll bar is necessary to access the control buttons.
  // Use this method to estimate the available vertical size to allow this
  wxWindow* win = GetOCPNCanvasWindow();
  wxSize sz = win->GetSize();
  // qDebug() << "CanvasSize: " << sz.x << sz.y;

  if (GetGrandParent()) sz = GetGrandParent()->GetSize();
  // qDebug() << "MetricSize: " << sz.x << sz.y;

  int yAvail = sz.y - (8 * GetCharHeight());  // Roughly 8 chars of decoration
                                              // on-screen for this dialog.
  double ratio = 0.7;
  if (sz.y > sz.x)  // Portait mode
    ratio = 0.8;

  m_lbChartSources->SetMinSize(wxSize(-1, yAvail * ratio));
  m_scrollWinChartList->SetMinSize(wxSize(-1, yAvail * ratio));
  Layout();

  event.Skip();
}

#if !defined(CHART_LIST)
ChartPanel::ChartPanel(wxWindow* parent, wxWindowID id, const wxPoint& pos,
                       const wxSize& size, wxString Name, wxString stat,
                       wxString latest, ChartDldrPanel* DldrPanel, bool bcheck)
    : wxPanel(parent, id, pos, size, wxBORDER_NONE) {
  wxString Descriptor = Name + _T("\n    ") + stat + _T("   ") + latest;
  wxColour bColor;
  GetGlobalColor(_T("DILG0"), &bColor);
  bool bUseSysColors = false;
#ifdef __WXOSX__
  if (wxPlatformInfo::Get().CheckOSVersion(10, 14)) bUseSysColors = true;
#endif
#ifdef __WXGTK__
  bUseSysColors = true;
#endif

  if (bUseSysColors) {
    wxColour bg = wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE);
    if (bg.Red() < 128) {
      bColor = wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE);
    }
  }

#ifndef __WXOSX__
  SetBackgroundColour(bColor);
#endif

  wxBoxSizer* m_sizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(m_sizer);
  m_cb = new wxCheckBox(this, wxID_ANY, Descriptor);
  m_cb->SetValue(bcheck);
  m_sizer->Add(m_cb, 0, wxTOP | wxLEFT | wxRIGHT, 4);
  m_cb->Connect(wxEVT_RIGHT_DOWN,
                wxMouseEventHandler(ChartPanel::OnContextMenu), NULL, this);

  m_stat = stat;
  m_latest = latest;

  //    wxBoxSizer* statSizer = new wxBoxSizer(wxHORIZONTAL);
  //    m_sizer->Add(statSizer, 0, wxALIGN_LEFT | wxALL, 1);

  //    m_chartInfo = new wxStaticText( this, wxID_ANY, stat );
  //    statSizer->Add(m_chartInfo, 0, wxLEFT, 4 * GetCharHeight());
  //    m_chartInfo->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler(
  //    ChartPanel::OnContextMenu ), NULL, this );

  //    m_chartInfo2 = new wxStaticText( this, wxID_ANY, latest );
  //    statSizer->Add(m_chartInfo2, 0, wxLEFT, 2 * GetCharHeight());
  //    m_chartInfo2->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler(
  //    ChartPanel::OnContextMenu ), NULL, this );

  //     wxString info = _T("           ") + stat + _T("   ") + latest;
  //     m_chartInfo = new wxStaticText( this, wxID_ANY, info );
  //     m_chartInfo->Wrap(-1);
  //     m_sizer->Add( m_chartInfo, 0, wxALL, 1 );

  //    wxStaticLine *divLine = new wxStaticLine( this, wxID_ANY,
  //    wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL ); m_sizer->Add(
  //    divLine, 0, wxEXPAND | wxALL, 5 );

  m_dldrPanel = DldrPanel;
  Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(ChartPanel::OnContextMenu),
          NULL, this);
}

ChartPanel::~ChartPanel() {
  m_cb->Disconnect(wxEVT_RIGHT_DOWN,
                   wxMouseEventHandler(ChartPanel::OnContextMenu), NULL, this);
  //    m_chartInfo->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler(
  //    ChartPanel::OnContextMenu ), NULL, this ); m_chartInfo2->Disconnect(
  //    wxEVT_RIGHT_DOWN, wxMouseEventHandler( ChartPanel::OnContextMenu ),
  //    NULL, this );

  delete m_cb;
  //   delete m_chartInfo;
  //   delete m_chartInfo2;
}

void ChartPanel::OnContextMenu(wxMouseEvent& event) {
  if (m_dldrPanel) return m_dldrPanel->OnContextMenu(event);
  event.Skip();
}
#endif /* CHART_LIST */

ChartDldrPrefsDlg::ChartDldrPrefsDlg(wxWindow* parent, wxWindowID id,
                                     const wxString& title, const wxPoint& pos,
                                     const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style) {
  wxFont* pFont = OCPNGetFont(_T("Dialog"), 0);
  if (pFont) SetFont(*pFont);

  this->SetSizeHints(wxDefaultSize, wxDefaultSize);

  wxBoxSizer *itemBoxSizerMainPanel = new wxBoxSizer(wxVERTICAL);
  SetSizer(itemBoxSizerMainPanel);

  wxScrolledWindow *scrollWin = new wxScrolledWindow(
     this, wxID_ANY, wxDefaultPosition, wxSize(-1, -1), wxVSCROLL);

  scrollWin->SetScrollRate(1, 1);
  itemBoxSizerMainPanel->Add(scrollWin, 1, wxEXPAND | wxALL, 0);

  m_sdbSizerBtns = new wxStdDialogButtonSizer();
  m_sdbSizerBtnsOK = new wxButton(this, wxID_OK);
  m_sdbSizerBtns->AddButton(m_sdbSizerBtnsOK);
  m_sdbSizerBtnsCancel = new wxButton(this, wxID_CANCEL, _("Cancel"));
  m_sdbSizerBtns->AddButton(m_sdbSizerBtnsCancel);
  m_sdbSizerBtns->Realize();

  itemBoxSizerMainPanel->Add(m_sdbSizerBtns, 0, wxALL | wxEXPAND, 5);

  wxBoxSizer *bSizerPrefsMain = new wxBoxSizer(wxVERTICAL);
  scrollWin->SetSizer(bSizerPrefsMain);

  wxStaticBoxSizer* sbSizerPaths;
  sbSizerPaths = new wxStaticBoxSizer(
      new wxStaticBox(scrollWin, wxID_ANY, _("Default Path to Charts")), wxVERTICAL);

  //      m_dpDefaultDir = new wxDirPickerCtrl( this, wxID_ANY, wxEmptyString,
  //      _("Select a root folder for your charts"), wxDefaultPosition,
  //      wxDefaultSize, wxDIRP_USE_TEXTCTRL ); sbSizerPaths->Add(
  //      m_dpDefaultDir, 0, wxALL|wxEXPAND, 5 );

  m_tcDefaultDir = new wxTextCtrl(scrollWin, wxID_ANY, _T(""), wxDefaultPosition,
                                  wxSize(-1, -1), wxHSCROLL);
  sbSizerPaths->Add(m_tcDefaultDir, 3, wxALL | wxEXPAND, 5);

  m_buttonChartDirectory = new wxButton(scrollWin, wxID_ANY, _("Select a folder"));
  sbSizerPaths->Add(m_buttonChartDirectory, 1, wxALIGN_RIGHT | wxALL, 5);

  bSizerPrefsMain->Add(sbSizerPaths, 0, wxALL | wxEXPAND, 5);

  wxStaticBoxSizer* sbSizerBehavior;
  sbSizerBehavior = new wxStaticBoxSizer(
      new wxStaticBox(scrollWin, wxID_ANY, _("Preferences")), wxVERTICAL);

  sbSizerBehavior->SetMinSize(-1, 200);

  m_stPreselect = new wxStaticText(
      scrollWin, wxID_ANY, _("After catalog update select for download"),
      wxDefaultPosition, wxDefaultSize, 0);
  m_stPreselect->Wrap(-1);
  sbSizerBehavior->Add(m_stPreselect, 0, wxALL, 5);

  m_cbSelectUpdated = new wxCheckBox(scrollWin, wxID_ANY, _("All updated charts"),
                                     wxDefaultPosition, wxDefaultSize, 0);
  m_cbSelectUpdated->SetValue(true);
  sbSizerBehavior->Add(m_cbSelectUpdated, 0, wxALL, 5);

  m_cbSelectNew = new wxCheckBox(scrollWin, wxID_ANY, _("All new charts"),
                                 wxDefaultPosition, wxDefaultSize, 0);
  sbSizerBehavior->Add(m_cbSelectNew, 0, wxALL, 5);

  m_staticline1 = new wxStaticLine(scrollWin, wxID_ANY, wxDefaultPosition,
                                   wxDefaultSize, wxLI_HORIZONTAL);
  sbSizerBehavior->Add(m_staticline1, 0, wxEXPAND | wxALL, 5);

  m_cbBulkUpdate = new wxCheckBox(
      scrollWin, wxID_ANY,
      _("Allow bulk update of all configured chart sources and charts"),
      wxDefaultPosition, wxDefaultSize, 0);
  sbSizerBehavior->Add(m_cbBulkUpdate, 0, wxALL, 5);

  m_buttonDownloadMasterCatalog =
      new wxButton(scrollWin, wxID_ANY, _("Update chart source catalog"),
                   wxDefaultPosition, wxDefaultSize, 0);

  sbSizerBehavior->Add(m_buttonDownloadMasterCatalog, 0, wxALL, 5);

  bSizerPrefsMain->Add(sbSizerBehavior, 1, wxALL | wxEXPAND, 5);


  this->Fit();

  // Constrain size on small displays
  int display_width, display_height;
  wxDisplaySize(&display_width, &display_height);

  wxSize canvas_size = GetOCPNCanvasWindow()->GetSize();
  if(display_height < 600){
    SetMaxSize(GetOCPNCanvasWindow()->GetSize());
    SetSize(wxSize(60 * GetCharWidth(), canvas_size.y * 8 / 10));
  }
  else {
    SetMaxSize(GetOCPNCanvasWindow()->GetSize());
    SetSize(wxSize(60 * GetCharWidth(), canvas_size.y * 8 / 10));
  }
  this->CentreOnScreen();

  // Connect Events
  m_sdbSizerBtnsCancel->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPrefsDlg::OnCancelClick), NULL, this);
  m_sdbSizerBtnsOK->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                            wxCommandEventHandler(ChartDldrPrefsDlg::OnOkClick),
                            NULL, this);
  m_buttonChartDirectory->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPrefsDlg::OnDirSelClick), NULL, this);
  m_buttonDownloadMasterCatalog->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPrefsDlg::OnDownloadMasterCatalog), NULL,
      this);
}

void ChartDldrPrefsDlg::OnDownloadMasterCatalog(wxCommandEvent& event) {
  wxFileName tfn = wxFileName::CreateTempFileName(_T("chartdldr"));
  wxString url =
      "https://raw.githubusercontent.com/OpenCPN/OpenCPN/master/plugins/"
      "chartdldr_pi/data/chart_sources.xml";

  _OCPN_DLStatus ret = OCPN_downloadFile(
      url, tfn.GetFullPath(), _("Downloading chart sources"),
      _("Downloading chart sources"), wxNullBitmap, this,
      OCPN_DLDS_ELAPSED_TIME | OCPN_DLDS_ESTIMATED_TIME |
          OCPN_DLDS_REMAINING_TIME | OCPN_DLDS_SPEED | OCPN_DLDS_SIZE |
          OCPN_DLDS_URL | OCPN_DLDS_CAN_PAUSE | OCPN_DLDS_CAN_ABORT |
          OCPN_DLDS_AUTO_CLOSE,
      15);
  wxFileName fn;
  fn.SetPath(*GetpPrivateApplicationDataLocation());
  fn.SetFullName(_T("chartdldr_pi-chart_sources.xml"));

  switch (ret) {
    case OCPN_DL_NO_ERROR: {
      if (!wxCopyFile(tfn.GetFullPath(), fn.GetFullPath())) {
        OCPNMessageBox_PlugIn(this,
                              wxString::Format(_("Failed to save: %s "),
                                               fn.GetFullPath().c_str()),
                              _("Chart downloader"), wxOK | wxICON_ERROR);
      }
      break;
    }
    case OCPN_DL_FAILED: {
      OCPNMessageBox_PlugIn(
          this,
          wxString::Format(_("Failed to download: %s \nVerify there is a "
                             "working Internet connection."),
                           url.c_str()),
          _("Chart downloader"), wxOK | wxICON_ERROR);
      break;
    }
    case OCPN_DL_USER_TIMEOUT:
    case OCPN_DL_ABORTED: {
      break;
    }
    case OCPN_DL_UNKNOWN:
    case OCPN_DL_STARTED: {
      break;
    }

    default:
      wxASSERT(false);  // This should never happen because we handle all
                        // possible cases of ret
  }
  if (wxFileExists(tfn.GetFullPath())) wxRemoveFile(tfn.GetFullPath());
}

void ChartDldrPrefsDlg::OnDirSelClick(wxCommandEvent& event) {
  wxString dir_spec;
  int response = PlatformDirSelectorDialog(this, &dir_spec,
                                           _("Choose Chart File Directory"),
                                           m_tcDefaultDir->GetValue());

  if (response == wxID_OK) {
    m_tcDefaultDir->SetValue(dir_spec);
  }
}

ChartDldrPrefsDlg::~ChartDldrPrefsDlg() {
  // Disconnect Events
  m_sdbSizerBtnsCancel->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPrefsDlg::OnCancelClick), NULL, this);
  m_sdbSizerBtnsOK->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPrefsDlg::OnOkClick), NULL, this);
  m_buttonChartDirectory->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPrefsDlg::OnDirSelClick), NULL, this);
  m_buttonDownloadMasterCatalog->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPrefsDlg::OnDownloadMasterCatalog), NULL,
      this);
}
