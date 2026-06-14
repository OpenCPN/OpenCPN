///////////////////////////////////////////////////////////////////////////
// wxFormBuilder-generated layout; implementations split for maintainability.
///////////////////////////////////////////////////////////////////////////

#include "chartdldr_pi.h"
#include "chartdldrgui.h"
#include "chartdldr_prefs_gui.h"
#include "manual.h"
#include "../../../libs/manual/include/manual.h"
#include <wx/msgdlg.h>
#include <wx/scrolwin.h>
#include <wx/spinctrl.h>
#include <wx/textwrapper.h>

#ifdef __OCPN__ANDROID__
#include <QtWidgets/QScroller>
#include "qdebug.h"
#endif

extern chartdldr_pi* g_pi;

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

#ifdef HAVE_WX_GESTURE_EVENTS
  if (!m_scrollWinChartList->EnableTouchEvents(wxTOUCH_PRESS_GESTURES)) {
    wxLogError("Failed to enable touch events on chart downloader");
  }
  Bind(wxEVT_LONG_PRESS, &ChartDldrPanel::OnLongPress, this);
#endif

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

  m_stCatalogInfo = new wxStaticText(chartsPanel, wxID_ANY, "",
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
  m_bDnldCharts->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrPanel::OnDownloadCharts), NULL, this);
  // m_bShowLocal->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED,
  // wxCommandEventHandler( ChartDldrPanel::OnShowLocalDir ), NULL, this );
  this->Disconnect(wxEVT_SIZE, wxSizeEventHandler(ChartDldrPanel::OnSize));
}

#ifdef HAVE_WX_GESTURE_EVENTS
void ChartDldrPanel::OnLongPress(wxLongPressEvent& event) {
  printf(" OnLongPress\n");
  if (1) {
    wxPoint pos = event.GetPosition();
    wxMouseEvent ev(wxEVT_RIGHT_DOWN);
    ev.m_x = pos.x;
    ev.m_y = pos.y;

    // MouseEvent(ev);
  }
}
#endif

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
