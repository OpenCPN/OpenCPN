// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#include "wx/listctrl.h"
#include <wx/choice.h>

#include "TCWin.h"
#include "timers.h"
#include "chcanv.h"
#include "tide_time.h"
#include "tcmgr.h"
#include "dychart.h"
#include "cutil.h"
#include "FontMgr.h"
#include "wx28compat.h"
#include "OCPNPlatform.h"
#include "RolloverWin.h"
#include "navutil.h"
#include "gui_lib.h"
#include "ocpn_frame.h"

extern ColorScheme global_color_scheme;
extern IDX_entry *gpIDX;
extern int gpIDXn;
extern TCMgr *ptcmgr;
extern wxString g_locale;
extern OCPNPlatform *g_Platform;
extern MyConfig *pConfig;

int g_tcwin_scale;

enum { ID_TCWIN_NX, ID_TCWIN_PR };

enum { TIDE_PLOT, CURRENT_PLOT };

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(SplineList);

BEGIN_EVENT_TABLE(TCWin, wxWindow)
EVT_PAINT(TCWin::OnPaint) EVT_SIZE(TCWin::OnSize) EVT_MOTION(TCWin::MouseEvent)
    EVT_BUTTON(wxID_OK, TCWin::OKEvent) EVT_BUTTON(ID_TCWIN_NX, TCWin::NXEvent)
        EVT_BUTTON(ID_TCWIN_PR, TCWin::PREvent) EVT_CLOSE(TCWin::OnCloseWindow)
            EVT_TIMER(TCWININF_TIMER, TCWin::OnTCWinPopupTimerEvent)
                END_EVENT_TABLE()

    // Define a constructor
    extern wxDateTime gTimeSource;
TCWin::TCWin(ChartCanvas *parent, int x, int y, void *pvIDX) {
  //    As a display optimization....
  //    if current color scheme is other than DAY,
  //    Then create the dialog ..WITHOUT.. borders and title bar.
  //    This way, any window decorations set by external themes, etc
  //    will not detract from night-vision

  m_created = false;
  xSpot = 0;
  ySpot = 0;

  m_pTCRolloverWin = NULL;

  long wstyle = wxCLIP_CHILDREN | wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER |
                wxFRAME_FLOAT_ON_PARENT;
  if ((global_color_scheme != GLOBAL_COLOR_SCHEME_DAY) &&
      (global_color_scheme != GLOBAL_COLOR_SCHEME_RGB))
    wstyle |= (wxNO_BORDER);

  pParent = parent;
  m_x = x;
  m_y = y;

  RecalculateSize();

  // Read the config file to get the user specified time zone.
  if (pConfig) {
    pConfig->SetPath(_T ( "/Settings/Others" ));
    pConfig->Read(_T ( "TCWindowTimeZone" ), &m_tzoneDisplay, 0);
  }

  wxFrame::Create(parent, wxID_ANY, wxString(_T ( "" )), m_position, m_tc_size,
                  wstyle);

  m_created = true;
  wxFont *qFont = GetOCPNScaledFont(_("Dialog"));
  SetFont(*qFont);

  pIDX = (IDX_entry *)pvIDX;
  gpIDXn++;

  //    Set up plot type
  if (strchr("Tt", pIDX->IDX_type)) {
    m_plot_type = TIDE_PLOT;
    SetTitle(wxString(_("Tide")));
    gpIDX = pIDX;  // remember pointer for routeplan

  } else {
    m_plot_type = CURRENT_PLOT;
    SetTitle(wxString(_("Current")));
  }

  int sx, sy;
  GetClientSize(&sx, &sy);

  SetTimeFactors();

  btc_valid = false;

  wxString *TClist = NULL;
  m_tList = new wxListCtrl(this, -1, wxPoint(sx * 65 / 100, 11),
                           wxSize((sx * 32 / 100), (sy * 20 / 100)),
                           wxLC_REPORT | wxLC_NO_HEADER);

  // Add first column
  wxListItem col0;
  col0.SetId(0);
  col0.SetText(_T(""));
  col0.SetAlign(wxLIST_FORMAT_LEFT);
  col0.SetWidth(sx * 30 / 100);
  m_tList->InsertColumn(0, col0);

  //  Measure the size of a generic button, with label
  wxButton *test_button =
      new wxButton(this, wxID_OK, _("OK"), wxPoint(-1, -1), wxDefaultSize);
  test_button->GetSize(&m_tsx, &m_tsy);
  delete test_button;

  //  In the interest of readability, if the width of the dialog is too narrow,
  //  simply skip showing the "Hi/Lo" list control.

  if ((m_tsy * 15) > sx) m_tList->Hide();

  OK_button = new wxButton(this, wxID_OK, _("OK"),
                           wxPoint(sx - (2 * m_tsy + 10), sy - (m_tsy + 10)),
                           wxDefaultSize);

  PR_button = new wxButton(this, ID_TCWIN_PR, _("Prev"),
                           wxPoint(10, sy - (m_tsy + 10)), wxSize(-1, -1));

  wxSize texc_size = wxSize((sx * 60 / 100), (sy * 29 / 100));
  if (!m_tList->IsShown()) {
    texc_size = wxSize((sx * 90 / 100), (sy * 29 / 100));
  }

  m_ptextctrl =
      new wxTextCtrl(this, -1, _T(""), wxPoint(sx * 3 / 100, 6), texc_size,
                     wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP);
  int bsx, bsy, bpx, bpy;
  PR_button->GetSize(&bsx, &bsy);
  PR_button->GetPosition(&bpx, &bpy);

  NX_button =
      new wxButton(this, ID_TCWIN_NX, _("Next"),
                   wxPoint(bpx + bsx + 5, sy - (m_tsy + 10)), wxSize(-1, -1));

  wxString m_choiceTimezoneChoices[] = {_("LMT@Station"), _("UTC")};
  int m_choiceTimezoneNChoices =
      sizeof(m_choiceTimezoneChoices) / sizeof(wxString);
  m_choiceTimezone = new wxChoice(
      this, wxID_ANY, wxPoint((sx - (bsx * 2)) / 2, sy - (m_tsy + 10)),
      wxSize(2 * bsx, bsy), m_choiceTimezoneNChoices, m_choiceTimezoneChoices,
      0);

  m_choiceTimezone->SetSelection(m_tzoneDisplay);
  m_choiceTimezone->Connect(wxEVT_COMMAND_CHOICE_SELECTED,
                            wxCommandEventHandler(TCWin::TimezoneOnChoice),
                            NULL, this);

  m_TCWinPopupTimer.SetOwner(this, TCWININF_TIMER);

  wxScreenDC dc;
  int text_height;
  dc.GetTextExtent(_T("W"), NULL, &text_height);
  m_button_height = m_tsy;  // text_height + 20;

  // Build graphics tools

  wxFont *dlg_font = FontMgr::Get().GetFont(_("Dialog"));
  int dlg_font_size = dlg_font->GetPointSize();

  pSFont = FontMgr::Get().FindOrCreateFont(
      dlg_font_size - 2, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL,
      wxFONTWEIGHT_NORMAL, FALSE, wxString(_T ( "Arial" )));
  pSMFont = FontMgr::Get().FindOrCreateFont(
      dlg_font_size - 1, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL,
      wxFONTWEIGHT_NORMAL, FALSE, wxString(_T ( "Arial" )));
  pMFont = FontMgr::Get().FindOrCreateFont(
      dlg_font_size, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD,
      FALSE, wxString(_T ( "Arial" )));
  pLFont = FontMgr::Get().FindOrCreateFont(
      dlg_font_size + 1, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL,
      wxFONTWEIGHT_BOLD, FALSE, wxString(_T ( "Arial" )));

  pblack_1 = wxThePenList->FindOrCreatePen(
      GetGlobalColor(_T ( "UINFD" )), wxMax(1, (int)(m_tcwin_scaler + 0.5)),
      wxPENSTYLE_SOLID);
  pblack_2 = wxThePenList->FindOrCreatePen(
      GetGlobalColor(_T ( "UINFD" )), wxMax(2, (int)(2 * m_tcwin_scaler + 0.5)),
      wxPENSTYLE_SOLID);
  pblack_3 = wxThePenList->FindOrCreatePen(
      GetGlobalColor(_T ( "UWHIT" )), wxMax(1, (int)(m_tcwin_scaler + 0.5)),
      wxPENSTYLE_SOLID);
  pred_2 = wxThePenList->FindOrCreatePen(
      GetGlobalColor(_T ( "UINFR" )), wxMax(4, (int)(4 * m_tcwin_scaler + 0.5)),
      wxPENSTYLE_SOLID);
  pltgray = wxTheBrushList->FindOrCreateBrush(GetGlobalColor(_T ( "UIBCK" )),
                                              wxBRUSHSTYLE_SOLID);
  pltgray2 = wxTheBrushList->FindOrCreateBrush(GetGlobalColor(_T ( "DILG1" )),
                                               wxBRUSHSTYLE_SOLID);

  DimeControl(this);

  //  Fill in some static text control information

  //  Tidi station information
  m_ptextctrl->Clear();

  wxString locn(pIDX->IDX_station_name, wxConvUTF8);
  wxString locna, locnb;
  if (locn.Contains(wxString(_T ( "," )))) {
    locna = locn.BeforeFirst(',');
    locnb = locn.AfterFirst(',');
  } else {
    locna = locn;
    locnb.Empty();
  }

  // write the first line
  wxTextAttr style;
  style.SetFont(*pLFont);
  m_ptextctrl->SetDefaultStyle(style);

  m_ptextctrl->AppendText(locna);
  m_ptextctrl->AppendText(_T("\n"));

  style.SetFont(*pSMFont);
  m_ptextctrl->SetDefaultStyle(style);

  if (!locnb.IsEmpty()) m_ptextctrl->AppendText(locnb);
  m_ptextctrl->AppendText(_T("\n"));

  // Reference to the master station
  if (('t' == pIDX->IDX_type) || ('c' == pIDX->IDX_type)) {
    wxString mref(pIDX->IDX_reference_name, wxConvUTF8);
    mref.Prepend(_T(" "));

    m_ptextctrl->AppendText(_("Reference Station :"));
    m_ptextctrl->AppendText(_T("\n"));

    m_ptextctrl->AppendText(mref);
    m_ptextctrl->AppendText(_T("\n"));

  } else {
    m_ptextctrl->AppendText(_T("\n"));
  }

  //      Show the data source
  wxString dsource(pIDX->source_ident, wxConvUTF8);
  dsource.Prepend(_T(" "));

  m_ptextctrl->AppendText(_("Data Source :"));
  m_ptextctrl->AppendText(_T("\n"));

  m_ptextctrl->AppendText(dsource);

  m_ptextctrl->ShowPosition(0);
}

TCWin::~TCWin() { pParent->Refresh(false); }

void TCWin::SetTimeFactors() {
  //    Figure out this computer timezone minute offset
  wxDateTime this_now = gTimeSource;
  bool cur_time = !gTimeSource.IsValid();

  if (cur_time) {
    this_now = wxDateTime::Now();
  }
  wxDateTime this_gmt = this_now.ToGMT();

#if wxCHECK_VERSION(2, 6, 2)
  wxTimeSpan diff = this_now.Subtract(this_gmt);
#else
  wxTimeSpan diff = this_gmt.Subtract(this_now);
#endif

  m_diff_mins = diff.GetMinutes();

  //  Correct a bug in wx3.0.2
  //  If the system TZ happens to be GMT, with DST active (e.g.summer in
  //  London), then wxDateTime returns incorrect results for toGMT() method
#if wxCHECK_VERSION(3, 0, 2)
  if (m_diff_mins == 0 && this_now.IsDST()) m_diff_mins += 60;
#endif

  int station_offset = ptcmgr->GetStationTimeOffset(pIDX);

  m_stationOffset_mins = station_offset;
  if (this_now.IsDST()) m_stationOffset_mins += 60;

    //  Correct a bug in wx3.0.2
    //  If the system TZ happens to be GMT, with DST active (e.g.summer in
    //  London), then wxDateTime returns incorrect results for toGMT() method
#if wxCHECK_VERSION(3, 0, 2)
//    if(  this_now.IsDST() )
//        m_corr_mins +=60;
#endif

  //    Establish the inital drawing day as today, in the timezone of the
  //    station
  m_graphday = this_gmt;

  int day_gmt = this_gmt.GetDayOfYear();

  time_t ttNow = this_now.GetTicks();
  time_t tt_at_station =
      ttNow - (m_diff_mins * 60) + (m_stationOffset_mins * 60);
  wxDateTime atStation(tt_at_station);
  int day_at_station = atStation.GetDayOfYear();

  if (day_gmt > day_at_station) {
    wxTimeSpan dt(24, 0, 0, 0);
    m_graphday.Subtract(dt);
  } else if (day_gmt < day_at_station) {
    wxTimeSpan dt(24, 0, 0, 0);
    m_graphday.Add(dt);
  }

  wxDateTime graphday_00 = m_graphday;  // this_gmt;
  graphday_00.ResetTime();
  time_t t_graphday_00 = graphday_00.GetTicks();

  //    Correct a Bug in wxWidgets time support
  //    if( !graphday_00.IsDST() && m_graphday.IsDST() ) t_graphday_00 -= 3600;
  //    if( graphday_00.IsDST() && !m_graphday.IsDST() ) t_graphday_00 += 3600;

  m_t_graphday_GMT = t_graphday_00;

  btc_valid = false;  // Force re-calculation
}

void TCWin::TimezoneOnChoice(wxCommandEvent &event) {
  m_tzoneDisplay = m_choiceTimezone->GetSelection();
  SetTimeFactors();

  Refresh();
}

void TCWin::RecalculateSize() {
  wxSize parent_size(2000, 2000);
  if (pParent) parent_size = pParent->GetClientSize();

  int unscaledheight = 600;
  int unscaledwidth = 650;

  // value of m_tcwin_scaler should be about unity on a 100 dpi display,
  // when scale parameter g_tcwin_scale is 100
  // parameter g_tcwin_scale is set in config file as value of
  // TideCurrentWindowScale
  g_tcwin_scale = wxMax(g_tcwin_scale, 10);  // sanity check on g_tcwin_scale
  m_tcwin_scaler = g_Platform->GetDisplayDPmm() * 0.254 * g_tcwin_scale / 100.0;

  m_tc_size.x = (int)(unscaledwidth * m_tcwin_scaler + 0.5);
  m_tc_size.y = (int)(unscaledheight * m_tcwin_scaler + 0.5);

  m_tc_size.x = wxMin(m_tc_size.x, parent_size.x);
  m_tc_size.y = wxMin(m_tc_size.y, parent_size.y);

  int xc = m_x + 8;
  int yc = m_y;

  //  Arrange for tcWindow to be always totally visible
  //  by shifting left and/or up
  if ((m_x + 8 + m_tc_size.x) > parent_size.x) xc = xc - m_tc_size.x - 16;
  if ((m_y + m_tc_size.y) > parent_size.y) yc = yc - m_tc_size.y;

  //  Don't let the window origin move out of client area
  if (yc < 0) yc = 0;
  if (xc < 0) xc = 0;

  if (pParent) pParent->ClientToScreen(&xc, &yc);
  m_position = wxPoint(xc, yc);

  if (m_created) {
    SetSize(m_tc_size);
    Move(m_position);
  }
}

void TCWin::OKEvent(wxCommandEvent &event) {
  Hide();
  pParent->pCwin = NULL;
  if (--gpIDXn == 0) gpIDX = NULL;
  delete m_pTCRolloverWin;
  delete m_tList;
  pParent->Refresh(false);

  // Update the config file to set the user specified time zone.
  if (pConfig) {
    pConfig->SetPath(_T ( "/Settings/Others" ));
    pConfig->Write(_T ( "TCWindowTimeZone" ), m_tzoneDisplay);
  }

  Destroy();  // that hurts
}

void TCWin::OnCloseWindow(wxCloseEvent &event) {
  Hide();
  pParent->pCwin = NULL;
  if (--gpIDXn == 0) gpIDX = NULL;
  delete m_pTCRolloverWin;
  delete m_tList;

  // Update the config file to set the user specified time zone.
  if (pConfig) {
    pConfig->SetPath(_T ( "/Settings/Others" ));
    pConfig->Write(_T ( "TCWindowTimeZone" ), m_tzoneDisplay);
  }

  Destroy();  // that hurts
}

void TCWin::NXEvent(wxCommandEvent &event) {
  wxTimeSpan dt(24, 0, 0, 0);
  m_graphday.Add(dt);
  wxDateTime dm = m_graphday;

  wxDateTime graphday_00 = dm.ResetTime();
  time_t t_graphday_00 = graphday_00.GetTicks();

  if (!graphday_00.IsDST() && m_graphday.IsDST()) t_graphday_00 -= 3600;
  if (graphday_00.IsDST() && !m_graphday.IsDST()) t_graphday_00 += 3600;

  m_t_graphday_GMT = t_graphday_00;

  btc_valid = false;
  Refresh();
}

void TCWin::PREvent(wxCommandEvent &event) {
  wxTimeSpan dt(-24, 0, 0, 0);
  m_graphday.Add(dt);
  wxDateTime dm = m_graphday;

  wxDateTime graphday_00 = dm.ResetTime();
  time_t t_graphday_00 = graphday_00.GetTicks();

  if (!graphday_00.IsDST() && m_graphday.IsDST()) t_graphday_00 -= 3600;
  if (graphday_00.IsDST() && !m_graphday.IsDST()) t_graphday_00 += 3600;

  m_t_graphday_GMT = t_graphday_00;

  btc_valid = false;
  Refresh();
}

void TCWin::RePosition(void) {
  //    Position the window
  double lon = pIDX->IDX_lon;
  double lat = pIDX->IDX_lat;

  wxPoint r;
  pParent->GetCanvasPointPix(lat, lon, &r);
  pParent->ClientToScreen(&r.x, &r.y);
  Move(r);
}

void TCWin::OnPaint(wxPaintEvent &event) {
  if (!IsShown()) {
    return;
  }
  int x, y;
  int i;
  char sbuf[100];
  int w;
  float tcmax, tcmin;

  if (m_graph_rect.x == 0) return;

  GetClientSize(&x, &y);
  //    qDebug() << "OnPaint" << x << y;

#if 0
    //  establish some graphic element sizes/locations
    int x_graph = x * 1 / 10;
    int y_graph = y * 32 / 100;
    int x_graph_w = x * 8 / 10;
    int y_graph_h = (y * .7)  - (3 * m_button_height);
    m_graph_rect = wxRect(x_graph, y_graph, x_graph_w, y_graph_h);

    wxSize texc_size = wxSize( ( x * 60 / 100 ), ( y *29 / 100 ) );
    if( !m_tList->IsShown()){
        texc_size = wxSize( ( x * 90 / 100 ), ( y *29 / 100 ) );
    }

    m_ptextctrl->SetSize(texc_size);
#endif

  wxPaintDC dc(this);

  wxString tlocn(pIDX->IDX_station_name, wxConvUTF8);

  //     if(1/*bForceRedraw*/)
  {
    int x_textbox = x * 5 / 100;
    int y_textbox = 6;

    int x_textbox_w = x * 51 / 100;
    int y_textbox_h = y * 25 / 100;

    // box the location text & tide-current table
    dc.SetPen(*pblack_3);
    dc.SetBrush(*pltgray2);
    dc.DrawRoundedRectangle(x_textbox, y_textbox, x_textbox_w, y_textbox_h,
                            4);  // location text box

    if (m_tList->IsShown()) {
      wxRect tab_rect = m_tList->GetRect();
      dc.DrawRoundedRectangle(tab_rect.x - 4, y_textbox, tab_rect.width + 8,
                              y_textbox_h, 4);  // tide-current table box
    }

    //    Box the graph
    dc.SetPen(*pblack_1);
    dc.SetBrush(*pltgray);
    dc.DrawRectangle(m_graph_rect.x, m_graph_rect.y, m_graph_rect.width,
                     m_graph_rect.height);

    //  On some platforms, we cannot draw rotated text.
    //  So, reduce the complexity of horizontal axis time labels
#ifndef __WXMSW__
    const int hour_delta = 4;
#else
    const int hour_delta = 1;
#endif

    int hour_start = 0;
    //        if(m_tzoneDisplay == 1){                // UTC
    //            hour_start = m_diff_mins / 60;
    //        }

    //    Horizontal axis
    dc.SetFont(*pSFont);
    for (i = 0; i < 25; i++) {
      int xd = m_graph_rect.x + ((i)*m_graph_rect.width / 25);
      if (hour_delta != 1) {
        if (i % hour_delta == 0) {
          dc.SetPen(*pblack_2);
          dc.DrawLine(xd, m_graph_rect.y, xd,
                      m_graph_rect.y + m_graph_rect.height + 5);
          char sbuf[5];
          int hour_show = hour_start + i;
          if (hour_show >= 24) hour_show -= 24;
          sprintf(sbuf, "%02d", hour_show);
          int x_shim = -20;
          dc.DrawText(wxString(sbuf, wxConvUTF8),
                      xd + x_shim + (m_graph_rect.width / 25) / 2,
                      m_graph_rect.y + m_graph_rect.height + 8);
        } else {
          dc.SetPen(*pblack_1);
          dc.DrawLine(xd, m_graph_rect.y, xd,
                      m_graph_rect.y + m_graph_rect.height + 5);
        }
      } else {
        dc.SetPen(*pblack_1);
        dc.DrawLine(xd, m_graph_rect.y, xd,
                    m_graph_rect.y + m_graph_rect.height + 5);
        wxString sst;
        sst.Printf(_T("%02d"), i);
        dc.DrawRotatedText(sst, xd + (m_graph_rect.width / 25) / 2,
                           m_graph_rect.y + m_graph_rect.height + 8, 270.);
      }
    }

    //    Make a line for "right now"
    wxDateTime this_now = gTimeSource;
    bool cur_time = !gTimeSource.IsValid();
    if (cur_time) this_now = wxDateTime::Now();

    time_t t_now = this_now.GetTicks();  // now, in ticks
    t_now -= m_diff_mins * 60;
    if (m_tzoneDisplay == 0)  // LMT @ Station
      t_now += m_stationOffset_mins * 60;

    float t_ratio =
        m_graph_rect.width * (t_now - m_t_graphday_GMT) / (25 * 3600.0f);

    // must eliminate line outside the graph (in that case put it outside the
    // window)
    int xnow = (t_ratio < 0 || t_ratio > m_graph_rect.width)
                   ? -1
                   : m_graph_rect.x + (int)t_ratio;
    dc.SetPen(*pred_2);
    dc.DrawLine(xnow, m_graph_rect.y, xnow,
                m_graph_rect.y + m_graph_rect.height);
    dc.SetPen(*pblack_1);

    //    Build the array of values, capturing max and min and HW/LW list

    if (!btc_valid) {
      float dir;
      tcmax = -10;
      tcmin = 10;
      float val = -100;
      m_tList->DeleteAllItems();
      int list_index = 0;
      bool wt = false;

      wxBeginBusyCursor();

      // The tide/current modules calculate values based on PC local time
      // We want UTC, so adjust accordingly
      int tt_localtz = m_t_graphday_GMT + (m_diff_mins * 60);

      // get tide flow sens ( flood or ebb ? )
      ptcmgr->GetTideFlowSens(tt_localtz, BACKWARD_TEN_MINUTES_STEP,
                              pIDX->IDX_rec_num, tcv[0], val, wt);

      if (m_tzoneDisplay == 0)
        tt_localtz -= m_stationOffset_mins * 60;  // LMT at station

      for (i = 0; i < 26; i++) {
        int tt = tt_localtz + (i * FORWARD_ONE_HOUR_STEP);

        ptcmgr->GetTideOrCurrent(tt, pIDX->IDX_rec_num, tcv[i], dir);
        tt_tcv[i] = tt;  // store the corresponding time_t value
        if (tcv[i] > tcmax) tcmax = tcv[i];

        if (tcv[i] < tcmin) tcmin = tcv[i];
        if (TIDE_PLOT == m_plot_type) {
          if (!((tcv[i] > val) == wt) && (i > 0))  // if tide flow sense change
          {
            float tcvalue;  // look backward for HW or LW
            time_t tctime;
            ptcmgr->GetHightOrLowTide(tt, BACKWARD_TEN_MINUTES_STEP,
                                      BACKWARD_ONE_MINUTES_STEP, tcv[i], wt,
                                      pIDX->IDX_rec_num, tcvalue, tctime);
            if (tctime > tt_localtz) {  // Only show events visible in graphic
                                        // presently shown
              wxDateTime tcd;           // write date
              wxString s, s1;
              tcd.Set(tctime - (m_diff_mins * 60));
              if (m_tzoneDisplay == 0)  // LMT @ Station
                tcd.Set(tctime + (m_stationOffset_mins - m_diff_mins) * 60);

              s.Printf(tcd.Format(_T("%H:%M  ")));
              s1.Printf(_T("%05.2f "), tcvalue);  // write value
              s.Append(s1);
              Station_Data *pmsd = pIDX->pref_sta_data;  // write unit
              if (pmsd) s.Append(wxString(pmsd->units_abbrv, wxConvUTF8));
              s.Append(_T("   "));
              (wt) ? s.Append(_("HW")) : s.Append(_("LW"));  // write HW or LT

              wxListItem li;
              li.SetId(list_index);
              li.SetAlign(wxLIST_FORMAT_LEFT);
              li.SetText(s);
              li.SetColumn(0);
              m_tList->InsertItem(li);
              list_index++;
            }
            wt = !wt;  // change tide flow sens
          }
          val = tcv[i];
        }
        if (CURRENT_PLOT == m_plot_type) {
          wxDateTime thx;  // write date
          wxString s, s1;

          thx.Set((time_t)tt - (m_diff_mins * 60));
          if (m_tzoneDisplay == 0)  // LMT @ Station
                thx.Set((time_t)tt + (m_stationOffset_mins - m_diff_mins) * 60);

          s.Printf(thx.Format(_T("%H:%M  ")));
          s1.Printf(_T("%05.2f "), fabs(tcv[i]));  // write value
          s.Append(s1);
          Station_Data *pmsd = pIDX->pref_sta_data;  // write unit
          if (pmsd) s.Append(wxString(pmsd->units_abbrv, wxConvUTF8));
          s1.Printf(_T("  %03.0f"), dir);  // write direction
          s.Append(s1);

          wxListItem li;
          li.SetId(list_index);
          li.SetAlign(wxLIST_FORMAT_LEFT);
          li.SetText(s);
          li.SetColumn(0);
          m_tList->InsertItem(li);
          list_index++;
        }
      }

      wxEndBusyCursor();

      //    Set up the vertical parameters based on Tide or Current plot
      if (CURRENT_PLOT == m_plot_type) {
        it = std::max(abs((int)tcmin - 1), abs((int)tcmax + 1));
        ib = -it;

        im = 2 * it;
        m_plot_y_offset = m_graph_rect.height / 2;
        val_off = 0;
      } else {
        ib = (int)tcmin;
        if (tcmin < 0) ib -= 1;
        it = (int)tcmax + 1;

        im = it - ib;  // abs ( ib ) + abs ( it );
        m_plot_y_offset = (m_graph_rect.height * (it - ib)) / im;
        val_off = ib;
      }

      // Arrange to skip some lines and legends if there are too many for the
      // vertical space we have
      int height_stext;
      dc.GetTextExtent(_T("1"), NULL, &height_stext);
      float available_lines = (float)m_graph_rect.height / height_stext;
      i_skip = (int)ceil(im / available_lines);

      if (CURRENT_PLOT == m_plot_type && i_skip != 1) {
        // Adjust steps so slack current "0" line is always drawn on graph
        ib -= it % i_skip;
        it = -ib;
        im = 2 * it;
      }

      //    Build spline list of points

      m_sList.DeleteContents(true);
      m_sList.Clear();

      for (i = 0; i < 26; i++) {
        wxPoint *pp = new wxPoint;
        pp->x = m_graph_rect.x + ((i)*m_graph_rect.width / 25);
        pp->y = m_graph_rect.y + (m_plot_y_offset) -
                (int)((tcv[i] - val_off) * m_graph_rect.height / im);

        m_sList.Append(pp);
      }

      btc_valid = true;
    }

    dc.SetTextForeground(GetGlobalColor(_T ( "DILG3" )));

    //    Vertical Axis

    i = ib;
    while (i < it + 1) {
      int yd = m_graph_rect.y + (m_plot_y_offset) -
               ((i - val_off) * m_graph_rect.height / im);

      if ((m_plot_y_offset + m_graph_rect.y) == yd)
        dc.SetPen(*pblack_2);
      else
        dc.SetPen(*pblack_1);

      dc.DrawLine(m_graph_rect.x, yd, m_graph_rect.x + m_graph_rect.width, yd);
      snprintf(sbuf, 99, "%d", i);
      dc.DrawText(wxString(sbuf, wxConvUTF8), m_graph_rect.x - 20, yd - 5);
      i += i_skip;
    }

    //    Draw the Value curve
#if wxCHECK_VERSION(2, 9, 0)
    wxPointList *list = (wxPointList *)&m_sList;
#else
    wxList *list = (wxList *)&m_sList;
#endif

    dc.SetPen(*pblack_2);
#if wxUSE_SPLINES
    dc.DrawSpline(list);
#else
    dc.DrawLines(list);
#endif
    //  More Info

    if (m_tzoneDisplay == 0) {
      int station_offset = ptcmgr->GetStationTimeOffset(pIDX);
      int h = station_offset / 60;
      int m = station_offset - (h * 60);
      if (m_graphday.IsDST()) h += 1;
      m_stz.Printf(_T("UTC %+03d:%02d"), h, m);

      //    Make the "nice" (for the US) station time-zone string, brutally by
      //    hand
      double lat = ptcmgr->GetStationLat(pIDX);

      if (lat > 20.0) {
        wxString mtz;
        switch (ptcmgr->GetStationTimeOffset(pIDX)) {
          case -240:
            mtz = _T( "AST" );
            break;
          case -300:
            mtz = _T( "EST" );
            break;
          case -360:
            mtz = _T( "CST" );
            break;
        }

        if (mtz.Len()) {
          if (m_graphday.IsDST()) mtz[1] = 'D';
          m_stz = mtz;
        }
      }
    }

    else
      m_stz = _T("UTC");

    int h;
    dc.SetFont(*pSFont);
    dc.GetTextExtent(m_stz, &w, &h);
    dc.DrawText(m_stz, x / 2 - w / 2, y - 2.5 * m_button_height);

    wxString sdate;
    if (g_locale == _T("en_US"))
      sdate = m_graphday.Format(_T ( "%A %b %d, %Y" ));
    else
      sdate = m_graphday.Format(_T ( "%A %d %b %Y" ));

    dc.SetFont(*pMFont);
    dc.GetTextExtent(sdate, &w, &h);
    dc.DrawText(sdate, x / 2 - w / 2, y - 2.0 * m_button_height);

    Station_Data *pmsd = pIDX->pref_sta_data;
    if (pmsd) {
      dc.GetTextExtent(wxString(pmsd->units_conv, wxConvUTF8), &w, &h);
      dc.DrawRotatedText(wxString(pmsd->units_conv, wxConvUTF8), 5,
                         m_graph_rect.y + m_graph_rect.height / 2 + w / 2, 90.);
    }

    //      Show flood and ebb directions
    if ((strchr("c", pIDX->IDX_type)) || (strchr("C", pIDX->IDX_type))) {
      dc.SetFont(*pSFont);

      wxString fdir;
      fdir.Printf(_T("%03d"), pIDX->IDX_flood_dir);
      dc.DrawText(fdir, m_graph_rect.x + m_graph_rect.width + 4,
                  m_graph_rect.y + m_graph_rect.height * 1 / 4);

      wxString edir;
      edir.Printf(_T("%03d"), pIDX->IDX_ebb_dir);
      dc.DrawText(edir, m_graph_rect.x + m_graph_rect.width + 4,
                  m_graph_rect.y + m_graph_rect.height * 3 / 4);
    }

    //    Today or tomorrow
    if ((m_button_height * 15) < x && cur_time) {  // large enough horizontally?
      wxString sday;

      int day = m_graphday.GetDayOfYear();
      if (m_graphday.GetYear() == this_now.GetYear()) {
        if (day == this_now.GetDayOfYear())
          sday.Append(_("Today"));
        else if (day == this_now.GetDayOfYear() + 1)
          sday.Append(_("Tomorrow"));
        else
          sday.Append(m_graphday.GetWeekDayName(m_graphday.GetWeekDay()));
      } else if (m_graphday.GetYear() == this_now.GetYear() + 1 &&
                 day == this_now.Add(wxTimeSpan::Day()).GetDayOfYear())
        sday.Append(_("Tomorrow"));

      dc.SetFont(*pSFont);
      dc.GetTextExtent(sday, &w, &h);
      dc.DrawText(sday, 55 - w / 2, y - 2 * m_button_height);
    }

    //  Render "Spot of interest"
    double spotDim = 4 * g_Platform->GetDisplayDPmm();

    dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(
        GetGlobalColor(_T ( "YELO1" )), wxBRUSHSTYLE_SOLID));
    dc.SetPen(wxPen(GetGlobalColor(_T ( "URED" )),
                    wxMax(2, 0.5 * g_Platform->GetDisplayDPmm())));
    dc.DrawRoundedRectangle(xSpot - spotDim / 2, ySpot - spotDim / 2, spotDim,
                            spotDim, spotDim / 2);

    dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(
        GetGlobalColor(_T ( "UBLCK" )), wxBRUSHSTYLE_SOLID));
    dc.SetPen(wxPen(GetGlobalColor(_T ( "UBLCK" )), 1));

    double ispotDim = spotDim / 5.;
    dc.DrawRoundedRectangle(xSpot - ispotDim / 2, ySpot - ispotDim / 2,
                            ispotDim, ispotDim, ispotDim / 2);
  }
}

void TCWin::OnSize(wxSizeEvent &event) {
  if (!m_created) return;

  int x, y;
  GetClientSize(&x, &y);

  //  establish some graphic element sizes/locations
  int x_graph = x * 1 / 10;
  int y_graph = y * 32 / 100;
  int x_graph_w = x * 8 / 10;
  int y_graph_h = (y * .7) - (7 * m_button_height / 2);
  y_graph_h =
      wxMax(y_graph_h, 2);  // ensure minimum size is positive, at least.

  m_graph_rect = wxRect(x_graph, y_graph, x_graph_w, y_graph_h);

  //  In the interest of readability, if the width of the dialog is too narrow,
  //  simply skip showing the "Hi/Lo" list control.

  if ((m_tsy * 15) > x)
    m_tList->Hide();
  else {
    m_tList->Move(wxPoint(x * 65 / 100, 11));
    m_tList->Show();
  }

  wxSize texc_size = wxSize((x * 60 / 100), (y * 29 / 100));
  if (!m_tList->IsShown()) {
    texc_size = wxSize((x * 90 / 100), (y * 29 / 100));
  }
  m_ptextctrl->SetSize(texc_size);

#ifdef __WXOSX__
  OK_button->Move(wxPoint(x - (4 * m_tsy + 10), y - (m_tsy + 10)));
#else
  OK_button->Move(wxPoint(x - (3 * m_tsy + 10), y - (m_tsy + 10)));
#endif
  PR_button->Move(wxPoint(10, y - (m_tsy + 10)));

  int bsx, bsy, bpx, bpy;
  PR_button->GetSize(&bsx, &bsy);
  PR_button->GetPosition(&bpx, &bpy);

  NX_button->Move(wxPoint(bpx + bsx + 5, y - (m_tsy + 10)));

  btc_valid = false;

  Refresh(true);
  Update();
}

void TCWin::MouseEvent(wxMouseEvent &event) {
  event.GetPosition(&curs_x, &curs_y);

  if (!m_TCWinPopupTimer.IsRunning())
    m_TCWinPopupTimer.Start(20, wxTIMER_ONE_SHOT);
}

void TCWin::OnTCWinPopupTimerEvent(wxTimerEvent &event) {
  int x, y;
  bool ShowRollover;

  GetClientSize(&x, &y);
  wxRegion cursorarea(m_graph_rect);
  if (cursorarea.Contains(curs_x, curs_y)) {
    ShowRollover = true;
    SetCursor(*pParent->pCursorCross);
    if (NULL == m_pTCRolloverWin) {
      m_pTCRolloverWin = new RolloverWin(this, -1, false);
      // doesn't really work, mouse positions are relative to rollover window
      // not this window.
      // effect: hide rollover window if mouse on rollover
      m_pTCRolloverWin->SetMousePropogation(1);
      m_pTCRolloverWin->Hide();
    }
    float t, d;
    wxString p, s;
    // set time on x cursor position
    t = (25 / ((float)x * 8 / 10)) * ((float)curs_x - ((float)x * 1 / 10));

    int tt = m_t_graphday_GMT + (int)(t * 3600);
    time_t ths = tt;

    wxDateTime thd;
    thd.Set(ths);
    p.Printf(thd.Format(_T("%Hh %Mmn")));
    p.Append(_T("\n"));

    // The tide/current modules calculate values based on PC local time
    // We want UTC, so adjust accordingly
    int tt_localtz = m_t_graphday_GMT + (m_diff_mins * 60);

    int ttv = tt_localtz + (int)(t * 3600);
    if (m_tzoneDisplay == 0) {
      ttv -= m_stationOffset_mins * 60;  // LMT at station
    }

    time_t tts = ttv;

    // set tide level or current speed at that time
    ptcmgr->GetTideOrCurrent(tts, pIDX->IDX_rec_num, t, d);
    s.Printf(_T("%3.2f "), (t < 0 && CURRENT_PLOT == m_plot_type)
                               ? -t
                               : t);  // always positive if current
    p.Append(s);

    // set unit
    Station_Data *pmsd = pIDX->pref_sta_data;
    if (pmsd) p.Append(wxString(pmsd->units_abbrv, wxConvUTF8));

    // set current direction
    if (CURRENT_PLOT == m_plot_type) {
      s.Printf("%3.0f%c", d, 0x00B0);
      p.Append(_T("\n"));
      p.Append(s);
    }

    // set rollover area size
    wxSize win_size;
    win_size.Set(x * 90 / 100, y * 80 / 100);

    m_pTCRolloverWin->SetString(p);
    m_pTCRolloverWin->SetBestPosition(curs_x, curs_y, 1, 1, TC_ROLLOVER,
                                      win_size);
    m_pTCRolloverWin->SetBitmap(TC_ROLLOVER);
    m_pTCRolloverWin->Refresh();
    m_pTCRolloverWin->Show();

    //  Mark the actual spot on the curve
    // x value is clear...
    //  Find the point in the window that is used for the curev rendering,
    //  rounding as necessary

    int idx = 1;  // in case m_graph_rect.width is weird ie ppx never > curs_x
    for (int i = 0; i < 26; i++) {
      float ppx = m_graph_rect.x + ((i)*m_graph_rect.width / 25.f);
      if (ppx > curs_x) {
        idx = i;
        break;
      }
    }

    wxPointList *list = (wxPointList *)&m_sList;
    wxPoint *a = list->Item(idx - 1)->GetData();
    wxPoint *b = list->Item(idx)->GetData();

    float pct = (curs_x - a->x) / (float)((b->x - a->x));
    float dy = pct * (b->y - a->y);

    ySpot = a->y + dy;
    xSpot = curs_x;

    Refresh(true);

  } else {
    SetCursor(*pParent->pCursorArrow);
    ShowRollover = false;
  }

  if (m_pTCRolloverWin && m_pTCRolloverWin->IsShown() && !ShowRollover) {
    m_pTCRolloverWin->Hide();
  }
}
