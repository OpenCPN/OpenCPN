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

#ifdef HAVE_WX_GESTURE_EVENTS
  Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(ChartPanel::OnContextMenu),
          NULL, this);

  if (!EnableTouchEvents(wxTOUCH_PRESS_GESTURES)) {
    wxLogError("Failed to enable touch events on chart downloader");
  }

  Bind(wxEVT_LONG_PRESS, &ChartPanel::OnLongPress, this);
  Bind(wxEVT_LEFT_UP, &ChartPanel::OnLeftUp, this);
#endif
  m_popupWanted = false;
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

#ifdef HAVE_WX_GESTURE_EVENTS
void ChartPanel::OnLongPress(wxLongPressEvent& event) {
  /* we defer the popup menu call upon the leftup event
  else the menu disappears immediately, */
  m_popupWanted = true;
}
#endif

void ChartPanel::OnLeftUp(wxMouseEvent& event) {
  wxPoint pos = event.GetPosition();

  if (m_popupWanted) {
    m_popupWanted = false;
    wxMouseEvent ev(wxEVT_RIGHT_DOWN);
    ev.m_x = pos.x;
    ev.m_y = pos.y;
    wxPostEvent(this, ev);
  }
}

#endif /* CHART_LIST */
