#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include <wx/statline.h>

#include "chcanv.h"
#include "MUIBar.h"
#include "OCPNPlatform.h"
#include "CanvasOptions.h"
#include "DetailSlider.h"
#include "GoToPositionDialog.h"
#include "styles.h"
#include "navutil.h"
#include "svg_utils.h"
#include "idents.h"
#include "color_handler.h"

wxBEGIN_EVENT_TABLE(CustomStatsPanel, wxPanel)
EVT_PAINT(CustomStatsPanel::OnPaint) wxEND_EVENT_TABLE()

CustomStatsPanel::CustomStatsPanel(ChartCanvas* parent, int orientation,
                                   float size_factor, wxWindowID id,
                                   const wxPoint& pos, const wxSize& size,
                                   long style, const wxString& name) {
  m_parentCanvas = parent;
  m_orientation = orientation;

  long mstyle = wxNO_BORDER | wxFRAME_NO_TASKBAR | wxFRAME_SHAPED |
                wxFRAME_FLOAT_ON_PARENT;

  m_scaleFactor = size_factor;
  wxFrame::Create(parent, id, _T(""), pos, size, mstyle, name);
  Init();
  // CreateControls();
}

CustomStatsPanel::~CustomStatsPanel() {}

void CustomStatsPanel::Init() {

  wxBoxSizer* statsBox = new wxBoxSizer(wxVERTICAL);
  SetSizer(statsBox);

  wxFlexGridSizer* statsTable = new wxFlexGridSizer(3,10,5);

  shipLon = statsTextControl(toSDMM(1, 123));
  shipLat = statsTextControl(toSDMM(2, 86.23));
  SOG = statsTextControl(_T("12 Kts."));
  COG = statsTextControl(_T("10 Kts."));

  statsTable->Add(statsTextControl(_T("Ship Name : ")));
  statsTable->Add(shipLon, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
  statsTable->Add(shipLat, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
  statsTable->Add(statsTextControl(_T("SOG : ")));
  statsTable->Add(SOG, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
  statsTable->Add(new wxStaticText(this, wxID_ANY, _T("")));
  statsTable->Add(statsTextControl(_T("COG : ")));
  statsTable->Add(COG, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
  statsTable->Add(new wxStaticText(this, wxID_ANY, _T("")));

  statsBox->Add(statsTable, 1, wxALL, 15); // margin of 15px around statsTable

  GetSizer()->Fit(this);
  GetSizer()->SetSizeHints(this);
  Layout();
}

wxStaticText* CustomStatsPanel::statsTextControl(const wxString& label) {
  wxStaticText* control = new wxStaticText(this, wxID_ANY, label);
  wxFont font(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
              wxFONTWEIGHT_NORMAL);
  wxColour fontColor = GetGlobalColor(_T("GREY1"));
  control->SetFont(font);
  control->SetForegroundColour(fontColor);
  return control;
}

void CustomStatsPanel::OnPaint(wxPaintEvent& event) {
  wxPaintDC dc(this);
  wxRect rect = GetClientRect();
  wxColour statsPanelColor = GetGlobalColor(_T("BLUE1"));
  wxBrush brush(statsPanelColor);
  SetBackgroundColour(statsPanelColor);
  dc.SetBrush(brush);
  dc.DrawRoundedRectangle(rect, 5);  // Set corner radius here
}

void CustomStatsPanel::SetBestPosition(void) {
  int leftOffset = 2;
  int x = (m_parent->GetClientSize().x - (GetSize().x * 1.00));
  x -= leftOffset;
  int topOffset = 60;
  ChartCanvas* pcc = wxDynamicCast(m_parent, ChartCanvas);
  int y = topOffset;
  wxPoint m_position = wxPoint(x, y);
  wxPoint screenPos = pcc->ClientToScreen(m_position);
  Move(screenPos);
  Show();
}
