/**************************************************************************
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Implement route_props_dlg_impl.h -- route properties dialog
 */

#include <wx/clipbrd.h>

#include "gl_headers.h"  // Must be included before anything using GL stuff

#include "model/georef.h"
#include "model/own_ship.h"
#include "model/routeman.h"
#include "model/select.h"

#include "model/navobj_db.h"
#include "model/navutil_base.h"

#include "chcanv.h"
#include "gui_lib.h"
#include "mark_info.h"
#include "navutil.h"
#include "ocpn_plugin.h"
#include "print_dialog.h"
#include "routemanagerdialog.h"
#include "route_printout.h"
#include "route_prop_dlg_impl.h"
#include "tcmgr.h"
#include "top_frame.h"

#define UTCINPUT 0  //!< Format date/time in UTC.
#define LTINPUT \
  1  //!< Format date/time using timezone configured in the operating system.
#define LMTINPUT 2  //!< Format date/time using the remote location LMT time.
/** Format date/time according to global OpenCPN settings. */
#define GLOBAL_SETTINGS_INPUT 3

#define ID_RCLK_MENU_COPY_TEXT 7013
#define ID_RCLK_MENU_EDIT_WP 7014
#define ID_RCLK_MENU_DELETE 7015
#define ID_RCLK_MENU_MOVEUP_WP 7026
#define ID_RCLK_MENU_MOVEDOWN_WP 7027

#define COLUMN_PLANNED_SPEED 9
#define COLUMN_ETD 13

#ifndef PI
#define PI (4. * atan(1.0))
#endif
#define TPI (2. * PI)
#define DEGS (180. / PI)
#define RADS (PI / 180.)

#define MOTWILIGHT \
  1  // in some languages there may be a distinction between morning/evening
#define SUNRISE 2
#define DAY 3
#define SUNSET 4
#define EVTWILIGHT 5
#define NIGHT 6

RoutePropDlgImpl* pRoutePropDialog;

extern wxString GetLayerName(int id);  // in ocpn_frame FIXME leamas)

// Sunrise/twilight calculation for route properties.
// limitations: latitude below 60, year between 2000 and 2100
// riset is +1 for rise -1 for set
// adapted by author's permission from QBASIC source as published at
//     http://www.stargazing.net/kepler

static wxString GetDaylightString(int index) {
  switch (index) {
    case 0:
      return " - ";
    case 1:
      return _("MoTwilight");
    case 2:
      return _("Sunrise");
    case 3:
      return _("Daytime");
    case 4:
      return _("Sunset");
    case 5:
      return _("EvTwilight");
    case 6:
      return _("Nighttime");

    default:
      return "";
  }
}

static double sign(double x) {
  if (x < 0.)
    return -1.;
  else
    return 1.;
}

static double FNipart(double x) { return (sign(x) * (int)(fabs(x))); }

static double FNday(int y, int m, int d, int h) {
  long fd = (367 * y - 7 * (y + (m + 9) / 12) / 4 + 275 * m / 9 + d);
  return ((double)fd - 730531.5 + h / 24.);
}

static double FNrange(double x) {
  double b = x / TPI;
  double a = TPI * (b - FNipart(b));
  if (a < 0.) a = TPI + a;
  return (a);
}

static double getDaylightEvent(double glat, double glong, int riset,
                               double altitude, int y, int m, int d) {
  double day = FNday(y, m, d, 0);
  double days, correction;
  double utold = PI;
  double utnew = 0.;
  double sinalt =
      sin(altitude * RADS);  // go for the sunrise/sunset altitude first
  double sinphi = sin(glat * RADS);
  double cosphi = cos(glat * RADS);
  double g = glong * RADS;
  double t, L, G, ec, lambda, E, obl, delta, GHA, cosc;
  int limit = 12;
  while ((fabs(utold - utnew) > .001)) {
    if (limit-- <= 0) return (-1.);
    days = day + utnew / TPI;
    t = days / 36525.;
    //     get arguments of Sun's orbit
    L = FNrange(4.8949504201433 + 628.331969753199 * t);
    G = FNrange(6.2400408 + 628.3019501 * t);
    ec = .033423 * sin(G) + .00034907 * sin(2 * G);
    lambda = L + ec;
    E = -1. * ec + .0430398 * sin(2 * lambda) - .00092502 * sin(4. * lambda);
    obl = .409093 - .0002269 * t;
    delta = asin(sin(obl) * sin(lambda));
    GHA = utold - PI + E;
    cosc = (sinalt - sinphi * sin(delta)) / (cosphi * cos(delta));
    if (cosc > 1.)
      correction = 0.;
    else if (cosc < -1.)
      correction = PI;
    else
      correction = acos(cosc);
    double tmp = utnew;
    utnew = FNrange(utold - (GHA + g + riset * correction));
    utold = tmp;
  }
  return (utnew * DEGS / 15.);  // returns decimal hours UTC
}

static double getLMT(double ut, double lon) {
  double t = ut + lon / 15.;
  if (t >= 0.)
    if (t <= 24.)
      return (t);
    else
      return (t - 24.);
  else
    return (t + 24.);
}

/**
 * Return the date/time timezone setting for the given selection.
 */
static wxString getDatetimeTimezoneSelector(int selection) {
  switch (selection) {
    case UTCINPUT:
      return "UTC";
    case LTINPUT:
      return "Local Time";
    case LMTINPUT:
      return "LMT";
    case GLOBAL_SETTINGS_INPUT:
    default:
      return "";
  }
}

static int getDaylightStatus(double lat, double lon, wxDateTime utcDateTime) {
  if (fabs(lat) > 60.) return (0);
  int y = utcDateTime.GetYear();
  int m = utcDateTime.GetMonth() + 1;  // wxBug? months seem to run 0..11 ?
  int d = utcDateTime.GetDay();
  int h = utcDateTime.GetHour();
  int n = utcDateTime.GetMinute();
  int s = utcDateTime.GetSecond();
  if (y < 2000 || y > 2100) return (0);

  double ut = (double)h + (double)n / 60. + (double)s / 3600.;
  double lt = getLMT(ut, lon);
  double rsalt = -0.833;
  double twalt = -12.;

  if (lt <= 12.) {
    double sunrise = getDaylightEvent(lat, lon, +1, rsalt, y, m, d);
    if (sunrise < 0.)
      return (0);
    else
      sunrise = getLMT(sunrise, lon);

    if (fabs(lt - sunrise) < 0.15) return (SUNRISE);
    if (lt > sunrise) return (DAY);
    double twilight = getDaylightEvent(lat, lon, +1, twalt, y, m, d);
    if (twilight < 0.)
      return (0);
    else
      twilight = getLMT(twilight, lon);
    if (lt > twilight)
      return (MOTWILIGHT);
    else
      return (NIGHT);
  } else {
    double sunset = getDaylightEvent(lat, lon, -1, rsalt, y, m, d);
    if (sunset < 0.)
      return (0);
    else
      sunset = getLMT(sunset, lon);
    if (fabs(lt - sunset) < 0.15) return (SUNSET);
    if (lt < sunset) return (DAY);
    double twilight = getDaylightEvent(lat, lon, -1, twalt, y, m, d);
    if (twilight < 0.)
      return (0);
    else
      twilight = getLMT(twilight, lon);
    if (lt < twilight)
      return (EVTWILIGHT);
    else
      return (NIGHT);
  }
}

RoutePropDlgImpl::RoutePropDlgImpl(wxWindow* parent, wxWindowID id,
                                   const wxString& title, const wxPoint& pos,
                                   const wxSize& size, long style)
    : RoutePropDlg(parent, id, title, pos, size, style) {
  m_pRoute = nullptr;

  SetColorScheme(global_color_scheme);

  if (g_route_prop_sx > 0 && g_route_prop_sy > 0 &&
      g_route_prop_sx < wxGetDisplaySize().x &&
      g_route_prop_sy < wxGetDisplaySize().y) {
    SetSize(g_route_prop_sx, g_route_prop_sy);
  }

  if (g_route_prop_x > 0 && g_route_prop_y > 0 &&
      g_route_prop_x < wxGetDisplaySize().x &&
      g_route_prop_y < wxGetDisplaySize().y) {
    SetPosition(wxPoint(10, 10));
  }
  RecalculateSize();

  Connect(wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(RoutePropDlgImpl::OnRoutePropMenuSelected),
          NULL, this);

#ifdef __WXOSX__
  Connect(wxEVT_ACTIVATE, wxActivateEventHandler(RoutePropDlgImpl::OnActivate),
          NULL, this);
#endif
}

RoutePropDlgImpl::~RoutePropDlgImpl() {
  Disconnect(wxEVT_COMMAND_MENU_SELECTED,
             wxCommandEventHandler(RoutePropDlgImpl::OnRoutePropMenuSelected),
             NULL, this);
  instanceFlag = false;
}

bool RoutePropDlgImpl::instanceFlag = false;
bool RoutePropDlgImpl::getInstanceFlag() {
  return RoutePropDlgImpl::instanceFlag;
}

RoutePropDlgImpl* RoutePropDlgImpl::single = NULL;
RoutePropDlgImpl* RoutePropDlgImpl::getInstance(wxWindow* parent) {
  if (!instanceFlag) {
    single = new RoutePropDlgImpl(parent);
    instanceFlag = true;
  }
  return single;
}

void RoutePropDlgImpl::OnActivate(wxActivateEvent& event) {
  auto pWin = dynamic_cast<wxFrame*>(event.GetEventObject());
  long int style = pWin->GetWindowStyle();
  if (event.GetActive())
    pWin->SetWindowStyle(style | wxSTAY_ON_TOP);
  else
    pWin->SetWindowStyle(style ^ wxSTAY_ON_TOP);
}

void RoutePropDlgImpl::RecalculateSize() {
  wxSize esize;
  esize.x = GetCharWidth() * 110;
  esize.y = GetCharHeight() * 40;

  wxSize dsize = GetParent()->GetSize();  // GetClientSize();
  esize.y = wxMin(esize.y, dsize.y - 0 /*(2 * GetCharHeight())*/);
  esize.x = wxMin(esize.x, dsize.x - 0 /*(2 * GetCharHeight())*/);
  SetSize(esize);

  wxSize fsize = GetSize();
  wxSize canvas_size = GetParent()->GetSize();
  wxPoint screen_pos = GetParent()->GetScreenPosition();
  int xp = (canvas_size.x - fsize.x) / 2;
  int yp = (canvas_size.y - fsize.y) / 2;
  Move(screen_pos.x + xp, screen_pos.y + yp);
}

void RoutePropDlgImpl::UpdatePoints() {
  if (!m_pRoute) return;
  wxDataViewItem selection = m_dvlcWaypoints->GetSelection();
  int selected_row = m_dvlcWaypoints->GetSelectedRow();
  m_dvlcWaypoints->DeleteAllItems();

  wxVector<wxVariant> data;

  m_pRoute->UpdateSegmentDistances(
      m_pRoute->m_PlannedSpeed);  // to fix ETA properties
  m_tcDistance->SetValue(
      wxString::Format("%5.1f " + getUsrDistanceUnit(),
                       toUsrDistance(m_pRoute->m_route_length)));
  m_tcEnroute->SetValue(formatTimeDelta(wxLongLong(m_pRoute->m_route_time)));
  //  Iterate on Route Points, inserting blank fields starting with index 0
  int in = 0;
  wxString slen, eta, ete;
  double bearing, distance, speed;
  double totalDistance = 0;
  wxDateTime eta_dt = wxInvalidDateTime;
  auto pnode = m_pRoute->pRoutePointList->begin();
  while (pnode != m_pRoute->pRoutePointList->end()) {
    speed = (*pnode)->GetPlannedSpeed();
    if (speed < .1) {
      speed = m_pRoute->m_PlannedSpeed;
    }
    if (in == 0) {
      DistanceBearingMercator((*pnode)->GetLatitude(), (*pnode)->GetLongitude(),
                              gLat, gLon, &bearing, &distance);
      if (m_pRoute->m_PlannedDeparture.IsValid()) {
        DateTimeFormatOptions opts =
            DateTimeFormatOptions()
                .SetTimezone(getDatetimeTimezoneSelector(m_tz_selection))
                .SetLongitude((*pnode)->m_lon);
        eta = wxString::Format(
            "Start: %s", ocpn::toUsrDateTimeFormat(
                             m_pRoute->m_PlannedDeparture.FromUTC(), opts));
        eta.Append(wxString::Format(
            " (%s)", GetDaylightString(
                         getDaylightStatus((*pnode)->m_lat, (*pnode)->m_lon,
                                           m_pRoute->m_PlannedDeparture))
                         .c_str()));
        eta_dt = m_pRoute->m_PlannedDeparture;
      } else {
        eta = _("N/A");
      }
      if (speed > .1) {
        ete = formatTimeDelta(wxLongLong(3600. * distance / speed));
      } else {
        ete = _("N/A");
      }
    } else {
      distance = (*pnode)->GetDistance();
      bearing = (*pnode)->GetCourse();
      if ((*pnode)->GetETA().IsValid()) {
        DateTimeFormatOptions opts =
            DateTimeFormatOptions()
                .SetTimezone(getDatetimeTimezoneSelector(m_tz_selection))
                .SetLongitude((*pnode)->m_lon);
        eta = ocpn::toUsrDateTimeFormat((*pnode)->GetETA().FromUTC(), opts);
        eta.Append(wxString::Format(
            " (%s)", GetDaylightString(getDaylightStatus((*pnode)->m_lat,
                                                         (*pnode)->m_lon,
                                                         (*pnode)->GetETA()))
                         .c_str()));
        eta_dt = (*pnode)->GetETA();
      } else {
        eta = "";
      }
      ete = (*pnode)->GetETE();
      totalDistance += distance;
    }
    wxString name = (*pnode)->GetName();
    double lat = (*pnode)->GetLatitude();
    double lon = (*pnode)->GetLongitude();
    wxString tide_station = (*pnode)->m_TideStation;
    wxString desc = (*pnode)->GetDescription();
    wxString etd;
    if ((*pnode)->GetManualETD().IsValid()) {
      // GetManualETD() returns time in UTC, always. So use it as such.
      RoutePoint* rt = (*pnode);
      DateTimeFormatOptions opts =
          DateTimeFormatOptions()
              .SetTimezone(getDatetimeTimezoneSelector(m_tz_selection))
              .SetLongitude(rt->m_lon);
      etd = ocpn::toUsrDateTimeFormat(rt->GetManualETD().FromUTC(), opts);
      if (rt->GetManualETD().IsValid() && rt->GetETA().IsValid() &&
          rt->GetManualETD() < rt->GetETA()) {
        etd.Prepend("!! ");  // Manually entered ETD is before we arrive here!
      }
    } else {
      etd = "";
    }
    ++pnode;
    wxString crs;
    if (pnode != m_pRoute->pRoutePointList->end()) {
      crs = formatAngle((*pnode)->GetCourse());
    } else {
      crs = _("Arrived");
    }

    if (in == 0)
      data.push_back(wxVariant("---"));
    else {
      std::ostringstream stm;
      stm << in;
      data.push_back(wxVariant(stm.str()));
    }

    wxString schar = "";
#ifdef __ANDROID__
    schar = wxString(" ");
#endif
    data.push_back(wxVariant(name + schar));  // To
    slen.Printf("%5.1f " + getUsrDistanceUnit(), toUsrDistance(distance));
    data.push_back(wxVariant(schar + slen + schar));          // Distance
    data.push_back(wxVariant(schar + formatAngle(bearing)));  // Bearing
    slen.Printf("%5.1f " + getUsrDistanceUnit(), toUsrDistance(totalDistance));
    data.push_back(wxVariant(schar + slen + schar));  // Total Distance
    data.push_back(wxVariant(schar + ::toSDMM(1, lat, FALSE) + schar));  // Lat
    data.push_back(wxVariant(schar + ::toSDMM(2, lon, FALSE) + schar));  // Lon
    data.push_back(wxVariant(schar + ete + schar));                      // ETE
    data.push_back(schar + eta + schar);                                 // ETA
    data.push_back(
        wxVariant(wxString::FromDouble(toUsrSpeed(speed))));  // Speed
    data.push_back(wxVariant(
        MakeTideInfo(tide_station, lat, lon, eta_dt)));  // Next Tide event
    data.push_back(wxVariant(desc));                     // Description
    data.push_back(wxVariant(crs));
    data.push_back(wxVariant(etd));
    data.push_back(
        wxVariant(""));  // Empty column to fill the remaining space (Usually
                         // gets squeezed to zero, even if not empty)
    m_dvlcWaypoints->AppendItem(data);
    data.clear();
    in++;
  }
  if (selected_row > 0) {
    m_dvlcWaypoints->SelectRow(selected_row);
    m_dvlcWaypoints->EnsureVisible(selection);
  }
}

void RoutePropDlgImpl::SetRouteAndUpdate(Route* pR, bool only_points) {
  if (NULL == pR) return;

  if (m_pRoute &&
      m_pRoute != pR)  // We had unsaved changes, but now display another route
    ResetChanges();

  m_OrigRoute.m_PlannedDeparture = pR->m_PlannedDeparture;
  m_OrigRoute.m_PlannedSpeed = pR->m_PlannedSpeed;

  wxString title = pR->GetName() == "" ? _("Route Properties") : pR->GetName();
  if (!pR->m_bIsInLayer)
    SetTitle(title);
  else {
    wxString caption(wxString::Format("%s, %s: %s", title, _("Layer"),
                                      GetLayerName(pR->m_LayerID)));
    SetTitle(caption);
  }

  //  Fetch any config file values
  if (!only_points) {
    if (!pR->m_PlannedDeparture.IsValid()) {
      pR->m_PlannedDeparture = wxDateTime::Now().ToUTC();
    }

    m_tz_selection = GLOBAL_SETTINGS_INPUT;  // Honor global setting by default
    if (pR != m_pRoute) {
      if (pR->m_TimeDisplayFormat == RTE_TIME_DISP_UTC)
        m_tz_selection = UTCINPUT;
      else if (pR->m_TimeDisplayFormat == RTE_TIME_DISP_LOCAL)
        m_tz_selection = LMTINPUT;
      m_pEnroutePoint = NULL;
      m_bStartNow = false;
    }

    m_pRoute = pR;

    m_tcPlanSpeed->SetValue(
        wxString::FromDouble(toUsrSpeed(m_pRoute->m_PlannedSpeed)));

    if (m_scrolledWindowLinks) {
      wxWindowList kids = m_scrolledWindowLinks->GetChildren();
      for (unsigned int i = 0; i < kids.GetCount(); i++) {
        wxWindowListNode* node = kids.Item(i);
        wxWindow* win = node->GetData();
        auto link_win = dynamic_cast<wxHyperlinkCtrl*>(win);
        if (link_win) {
          link_win->Disconnect(
              wxEVT_COMMAND_HYPERLINK,
              wxHyperlinkEventHandler(RoutePropDlgImpl::OnHyperlinkClick));
          link_win->Disconnect(
              wxEVT_RIGHT_DOWN,
              wxMouseEventHandler(RoutePropDlgImpl::HyperlinkContextMenu));
          win->Destroy();
        }
      }
      int NbrOfLinks = m_pRoute->m_HyperlinkList->size();
      HyperlinkList* list = m_pRoute->m_HyperlinkList;
      for (Hyperlink* link : *m_pRoute->m_HyperlinkList) {
        wxString Link = link->Link;
        wxString Descr = link->DescrText;

        wxHyperlinkCtrl* ctrl = new wxHyperlinkCtrl(
            m_scrolledWindowLinks, wxID_ANY, Descr, Link, wxDefaultPosition,
            wxDefaultSize, wxHL_DEFAULT_STYLE);
        ctrl->Connect(
            wxEVT_COMMAND_HYPERLINK,
            wxHyperlinkEventHandler(RoutePropDlgImpl::OnHyperlinkClick), NULL,
            this);
        if (!m_pRoute->m_bIsInLayer) {
          ctrl->Connect(
              wxEVT_RIGHT_DOWN,
              wxMouseEventHandler(RoutePropDlgImpl::HyperlinkContextMenu), NULL,
              this);
        }
        bSizerLinks->Add(ctrl, 0, wxALL, 5);
      }
      m_scrolledWindowLinks->InvalidateBestSize();
      m_scrolledWindowLinks->Layout();
      bSizerLinks->Layout();
    }

    m_choiceTimezone->SetSelection(m_tz_selection);

    // Reorganize dialog for route or track display
    m_tcName->SetValue(m_pRoute->m_RouteNameString);
    m_tcFrom->SetValue(m_pRoute->m_RouteStartString);
    m_tcTo->SetValue(m_pRoute->m_RouteEndString);
    m_tcDescription->SetValue(m_pRoute->m_RouteDescription);

    m_tcName->SetFocus();
    if (m_pRoute->m_PlannedDeparture.IsValid() &&
        m_pRoute->m_PlannedDeparture.GetValue() > 0) {
      wxDateTime t =
          toUsrDateTime(m_pRoute->m_PlannedDeparture, m_tz_selection,
                        (*m_pRoute->pRoutePointList->begin())->m_lon);
      m_dpDepartureDate->SetValue(t.GetDateOnly());
      m_tpDepartureTime->SetValue(t);
    } else {
      wxDateTime t =
          toUsrDateTime(wxDateTime::Now().ToUTC(), m_tz_selection,
                        (*m_pRoute->pRoutePointList->begin())->m_lon);
      m_dpDepartureDate->SetValue(t.GetDateOnly());
      m_tpDepartureTime->SetValue(t);
    }
  }

  m_btnSplit->Enable(false);
  if (!m_pRoute) return;

  if (m_pRoute->m_Colour == "") {
    m_choiceColor->Select(0);
  } else {
    for (unsigned int i = 0; i < sizeof(::GpxxColorNames) / sizeof(wxString);
         i++) {
      if (m_pRoute->m_Colour == ::GpxxColorNames[i]) {
        m_choiceColor->Select(i + 1);
        break;
      }
    }
  }

  for (unsigned int i = 0; i < sizeof(::StyleValues) / sizeof(int); i++) {
    if (m_pRoute->m_style == ::StyleValues[i]) {
      m_choiceStyle->Select(i);
      break;
    }
  }

  for (unsigned int i = 0; i < sizeof(::WidthValues) / sizeof(int); i++) {
    if (m_pRoute->m_width == ::WidthValues[i]) {
      m_choiceWidth->Select(i);
      break;
    }
  }

  UpdatePoints();

  m_btnExtend->Enable(IsThisRouteExtendable());
}

void RoutePropDlgImpl::DepartureDateOnDateChanged(wxDateEvent& event) {
  if (!m_pRoute) return;
  m_pRoute->SetDepartureDate(GetDepartureTS());
  UpdatePoints();
  event.Skip();
}

void RoutePropDlgImpl::DepartureTimeOnTimeChanged(wxDateEvent& event) {
  if (!m_pRoute) return;
  m_pRoute->SetDepartureDate(GetDepartureTS());
  UpdatePoints();
  event.Skip();
}

void RoutePropDlgImpl::TimezoneOnChoice(wxCommandEvent& event) {
  if (!m_pRoute) return;
  m_tz_selection = m_choiceTimezone->GetSelection();
  wxDateTime t = toUsrDateTime(m_pRoute->m_PlannedDeparture, m_tz_selection,
                               (*m_pRoute->pRoutePointList->begin())->m_lon);
  m_dpDepartureDate->SetValue(t.GetDateOnly());
  m_tpDepartureTime->SetValue(t);
  UpdatePoints();
  event.Skip();
}

void RoutePropDlgImpl::PlanSpeedOnTextEnter(wxCommandEvent& event) {
  if (!m_pRoute) return;
  double spd;
  if (m_tcPlanSpeed->GetValue().ToDouble(&spd)) {
    if (m_pRoute->m_PlannedSpeed != fromUsrSpeed(spd)) {
      m_pRoute->m_PlannedSpeed = fromUsrSpeed(spd);
      UpdatePoints();
    }
  } else {
    m_tcPlanSpeed->SetValue(
        wxString::FromDouble(toUsrSpeed(m_pRoute->m_PlannedSpeed)));
  }
}

void RoutePropDlgImpl::PlanSpeedOnKillFocus(wxFocusEvent& event) {
  if (!m_pRoute) return;
  double spd;
  if (m_tcPlanSpeed->GetValue().ToDouble(&spd)) {
    if (m_pRoute->m_PlannedSpeed != fromUsrSpeed(spd)) {
      m_pRoute->m_PlannedSpeed = fromUsrSpeed(spd);
      UpdatePoints();
    }
  } else {
    m_tcPlanSpeed->SetValue(
        wxString::FromDouble(toUsrSpeed(m_pRoute->m_PlannedSpeed)));
  }
  event.Skip();
}

static int ev_col;
void RoutePropDlgImpl::WaypointsOnDataViewListCtrlItemEditingDone(
    wxDataViewEvent& event) {
  // There is a bug in wxWidgets, the EDITING_DONE event does not contain the
  // new value, so we must save the data and do the work later in the value
  // changed event.
  ev_col = event.GetColumn();
}

void RoutePropDlgImpl::WaypointsOnDataViewListCtrlItemValueChanged(
    wxDataViewEvent& event) {
#if wxCHECK_VERSION(3, 1, 2)
  // wx 3.0.x crashes in the below code
  if (!m_pRoute) return;
  wxDataViewModel* const model = event.GetModel();
  wxVariant value;
  model->GetValue(value, event.GetItem(), ev_col);
  RoutePoint* p = m_pRoute->GetPoint(
      static_cast<int>(reinterpret_cast<long long>(event.GetItem().GetID())));
  if (ev_col == COLUMN_PLANNED_SPEED) {
    double spd;
    if (!value.GetString().ToDouble(&spd)) {
      spd = 0.0;
    }
    p->SetPlannedSpeed(fromUsrSpeed(spd));
  } else if (ev_col == COLUMN_ETD) {
    wxString::const_iterator end;
    wxDateTime etd;

    wxString ts = value.GetString();
    if (ts.StartsWith("!")) {
      ts.Replace("!", "", true);
    }
    ts.Trim(true);
    ts.Trim(false);

    if (!ts.IsEmpty()) {
      if (!etd.ParseDateTime(ts, &end)) {
        p->SetETD(wxInvalidDateTime);
      } else {
        p->SetETD(
            fromUsrDateTime(etd, m_tz_selection, p->m_lon).FormatISOCombined());
      }
    } else {
      p->SetETD(wxInvalidDateTime);
    }
  }
  UpdatePoints();
#endif
}

void RoutePropDlgImpl::WaypointsOnDataViewListCtrlSelectionChanged(
    wxDataViewEvent& event) {
  long selected_row = m_dvlcWaypoints->GetSelectedRow();
  if (selected_row > 0 && selected_row < m_dvlcWaypoints->GetItemCount() - 1) {
    m_btnSplit->Enable(true);
  } else {
    m_btnSplit->Enable(false);
  }
  if (IsThisRouteExtendable()) {
    m_btnExtend->Enable(true);
  } else {
    m_btnExtend->Enable(false);
  }
  if (selected_row >= 0 && selected_row < m_dvlcWaypoints->GetItemCount()) {
    RoutePoint* prp = m_pRoute->GetPoint(selected_row + 1);
    if (prp) {
      top_frame::Get()->JumpToPosition(prp->m_lat, prp->m_lon);
#ifdef __WXMSW__
      if (m_dvlcWaypoints) m_dvlcWaypoints->SetFocus();
#endif
    }
  }
}

wxDateTime RoutePropDlgImpl::GetDepartureTS() {
  wxDateTime dt = m_dpDepartureDate->GetValue();
  dt.SetHour(m_tpDepartureTime->GetValue().GetHour());
  dt.SetMinute(m_tpDepartureTime->GetValue().GetMinute());
  dt.SetSecond(m_tpDepartureTime->GetValue().GetSecond());
  return fromUsrDateTime(dt, m_tz_selection,
                         (*m_pRoute->pRoutePointList->begin())->m_lon);
  ;
}

void RoutePropDlgImpl::OnRoutepropCopyTxtClick(wxCommandEvent& event) {
  wxString tab("\t", wxConvUTF8);
  wxString eol("\n", wxConvUTF8);
  wxString csvString;

  csvString << this->GetTitle() << eol << _("Name") << tab
            << m_pRoute->m_RouteNameString << eol << _("Depart From") << tab
            << m_pRoute->m_RouteStartString << eol << _("Destination") << tab
            << m_pRoute->m_RouteEndString << eol << _("Total distance") << tab
            << m_tcDistance->GetValue() << eol << _("Speed (Kts)") << tab
            << m_tcPlanSpeed->GetValue() << eol
            << _("Departure Time") + " (" + ETA_FORMAT_STR + ")" << tab
            << GetDepartureTS().Format(ETA_FORMAT_STR) << eol
            << _("Time enroute") << tab << m_tcEnroute->GetValue() << eol
            << eol;

  int noCols;
  int noRows;
  noCols = m_dvlcWaypoints->GetColumnCount();
  noRows = m_dvlcWaypoints->GetItemCount();
  wxListItem item;
  item.SetMask(wxLIST_MASK_TEXT);

  for (int i = 0; i < noCols; i++) {
    wxDataViewColumn* col = m_dvlcWaypoints->GetColumn(i);
    csvString << col->GetTitle() << tab;
  }
  csvString << eol;

  wxVariant value;
  for (int j = 0; j < noRows; j++) {
    for (int i = 0; i < noCols; i++) {
      m_dvlcWaypoints->GetValue(value, j, i);
      csvString << value.MakeString() << tab;
    }
    csvString << eol;
  }

  if (wxTheClipboard->Open()) {
    wxTextDataObject* data = new wxTextDataObject;
    data->SetText(csvString);
    wxTheClipboard->SetData(data);
    wxTheClipboard->Close();
  }
}

void RoutePropDlgImpl::OnRoutePropMenuSelected(wxCommandEvent& event) {
  bool moveup = false;
  switch (event.GetId()) {
    case ID_RCLK_MENU_COPY_TEXT: {
      OnRoutepropCopyTxtClick(event);
      break;
    }
    case ID_RCLK_MENU_MOVEUP_WP: {
      moveup = true;
    }
    case ID_RCLK_MENU_MOVEDOWN_WP: {
      wxString mess =
          moveup ? _("Are you sure you want to move Up this waypoint?")
                 : _("Are you sure you want to move Down this waypoint?");
      int dlg_return =
          OCPNMessageBox(this, mess, _("OpenCPN Move Waypoint"),
                         (long)wxYES_NO | wxCANCEL | wxYES_DEFAULT);

      if (dlg_return == wxID_YES) {
        wxDataViewItem selection = m_dvlcWaypoints->GetSelection();
        RoutePoint* pRP = m_pRoute->GetPoint(
            static_cast<int>(reinterpret_cast<long long>(selection.GetID())));
        auto& list = m_pRoute->pRoutePointList;
        auto pos = std::find(list->begin(), list->end(), pRP);

        pSelect->DeleteAllSelectableRoutePoints(m_pRoute);
        pSelect->DeleteAllSelectableRouteSegments(m_pRoute);

        m_pRoute->pRoutePointList->erase(pos);
        pos += moveup ? -1 : 1;
        m_pRoute->pRoutePointList->insert(pos, pRP);

        pSelect->AddAllSelectableRouteSegments(m_pRoute);
        pSelect->AddAllSelectableRoutePoints(m_pRoute);

        // pConfig->UpdateRoute(m_pRoute);
        NavObj_dB::GetInstance().UpdateRoute(m_pRoute);

        m_pRoute->FinalizeForRendering();
        m_pRoute->UpdateSegmentDistances();
        ;

        top_frame::Get()->InvalidateAllGL();

        m_dvlcWaypoints->SelectRow(pos - list->begin());

        SetRouteAndUpdate(m_pRoute, true);
      }
      break;
    }
    case ID_RCLK_MENU_DELETE: {
      int dlg_return = OCPNMessageBox(
          this, _("Are you sure you want to remove this waypoint?"),
          _("OpenCPN Remove Waypoint"),
          (long)wxYES_NO | wxCANCEL | wxYES_DEFAULT);

      if (dlg_return == wxID_YES) {
        int sel = m_dvlcWaypoints->GetSelectedRow();
        m_dvlcWaypoints->SelectRow(sel);

        wxDataViewItem selection = m_dvlcWaypoints->GetSelection();
        RoutePoint* pRP = m_pRoute->GetPoint(
            static_cast<int>(reinterpret_cast<long long>(selection.GetID())));

        g_pRouteMan->RemovePointFromRoute(pRP, m_pRoute, 0);

        top_frame::Get()->InvalidateAllGL();
        UpdatePoints();
      }
      break;
    }
    case ID_RCLK_MENU_EDIT_WP: {
      wxDataViewItem selection = m_dvlcWaypoints->GetSelection();
      RoutePoint* pRP = m_pRoute->GetPoint(
          static_cast<int>(reinterpret_cast<long long>(selection.GetID())));

      RouteManagerDialog::WptShowPropertiesDialog(pRP, this);
      break;
    }
  }
}

void RoutePropDlgImpl::WaypointsOnDataViewListCtrlItemContextMenu(
    wxDataViewEvent& event) {
  wxMenu menu;
  if (!m_pRoute->m_bIsInLayer) {
    wxMenuItem* editItem = new wxMenuItem(&menu, ID_RCLK_MENU_EDIT_WP,
                                          _("Waypoint Properties") + "...");
    wxMenuItem* moveUpItem =
        new wxMenuItem(&menu, ID_RCLK_MENU_MOVEUP_WP, _("Move Up"));
    wxMenuItem* moveDownItem =
        new wxMenuItem(&menu, ID_RCLK_MENU_MOVEDOWN_WP, _("Move Down"));
    wxMenuItem* delItem =
        new wxMenuItem(&menu, ID_RCLK_MENU_DELETE, _("Remove Selected"));
#ifdef __ANDROID__
    wxFont* pf = OCPNGetFont(_("Menu"));
    editItem->SetFont(*pf);
    moveUpItem->SetFont(*pf);
    moveDownItem->SetFont(*pf);
    delItem->SetFont(*pf);
#endif
#if defined(__WXMSW__)
    wxFont* pf = GetOCPNScaledFont(_("Menu"));
    editItem->SetFont(*pf);
    moveUpItem->SetFont(*pf);
    moveDownItem->SetFont(*pf);
    delItem->SetFont(*pf);
#endif

    menu.Append(editItem);
    if (g_btouch) menu.AppendSeparator();
    menu.Append(moveUpItem);
    if (g_btouch) menu.AppendSeparator();
    menu.Append(moveDownItem);
    if (g_btouch) menu.AppendSeparator();
    menu.Append(delItem);

    editItem->Enable(m_dvlcWaypoints->GetSelectedRow() >= 0);
    moveUpItem->Enable(m_dvlcWaypoints->GetSelectedRow() >= 1 &&
                       m_dvlcWaypoints->GetItemCount() > 2);
    moveDownItem->Enable(m_dvlcWaypoints->GetSelectedRow() >= 0 &&
                         m_dvlcWaypoints->GetSelectedRow() <
                             m_dvlcWaypoints->GetItemCount() - 1 &&
                         m_dvlcWaypoints->GetItemCount() > 2);
    delItem->Enable(m_dvlcWaypoints->GetSelectedRow() >= 0 &&
                    m_dvlcWaypoints->GetItemCount() > 2);
  }
#ifndef __WXQT__
  wxMenuItem* copyItem =
      new wxMenuItem(&menu, ID_RCLK_MENU_COPY_TEXT, _("&Copy all as text"));

#if defined(__WXMSW__)
  wxFont* qFont = GetOCPNScaledFont(_("Menu"));
  copyItem->SetFont(*qFont);
#endif

  if (g_btouch) menu.AppendSeparator();
  menu.Append(copyItem);
#endif

  PopupMenu(&menu);
}

void RoutePropDlgImpl::ResetChanges() {
  if (!m_pRoute) return;
  m_pRoute->m_PlannedSpeed = m_OrigRoute.m_PlannedSpeed;
  m_pRoute->m_PlannedDeparture = m_OrigRoute.m_PlannedDeparture;
  m_pRoute = nullptr;
}

void RoutePropDlgImpl::SaveChanges() {
  if (m_pRoute && !m_pRoute->m_bIsInLayer) {
    //  Get User input Text Fields
    m_pRoute->m_RouteNameString = m_tcName->GetValue();
    m_pRoute->m_RouteStartString = m_tcFrom->GetValue();
    m_pRoute->m_RouteEndString = m_tcTo->GetValue();
    m_pRoute->m_RouteDescription = m_tcDescription->GetValue();
    if (m_choiceColor->GetSelection() == 0) {
      m_pRoute->m_Colour = "";
    } else {
      m_pRoute->m_Colour = ::GpxxColorNames[m_choiceColor->GetSelection() - 1];
    }
    m_pRoute->m_style =
        (wxPenStyle)::StyleValues[m_choiceStyle->GetSelection()];
    m_pRoute->m_width = ::WidthValues[m_choiceWidth->GetSelection()];
    switch (m_tz_selection) {
      case LTINPUT:
        m_pRoute->m_TimeDisplayFormat = RTE_TIME_DISP_PC;
        break;
      case LMTINPUT:
        m_pRoute->m_TimeDisplayFormat = RTE_TIME_DISP_LOCAL;
        break;
      case GLOBAL_SETTINGS_INPUT:
        m_pRoute->m_TimeDisplayFormat = RTE_TIME_DISP_GLOBAL;
        break;
      case UTCINPUT:
      default:
        m_pRoute->m_TimeDisplayFormat = RTE_TIME_DISP_UTC;
    }

    // pConfig->UpdateRoute(m_pRoute);
    NavObj_dB::GetInstance().UpdateRoute(m_pRoute);
    pConfig->UpdateSettings();
    m_pRoute = nullptr;
  }
}

void RoutePropDlgImpl::SetColorScheme(ColorScheme cs) { DimeControl(this); }

void RoutePropDlgImpl::SaveGeometry() {
  GetSize(&g_route_prop_sx, &g_route_prop_sy);
  GetPosition(&g_route_prop_x, &g_route_prop_y);
}

void RoutePropDlgImpl::BtnsOnOKButtonClick(wxCommandEvent& event) {
  SaveChanges();
  if (pRouteManagerDialog && pRouteManagerDialog->IsShown()) {
    pRouteManagerDialog->UpdateRouteListCtrl();
  }
  Hide();
  SaveGeometry();
}

void RoutePropDlgImpl::SplitOnButtonClick(wxCommandEvent& event) {
  m_btnSplit->Enable(false);

  if (m_pRoute->m_bIsInLayer) return;

  int nSelected = m_dvlcWaypoints->GetSelectedRow() + 1;
  if ((nSelected > 1) && (nSelected < m_pRoute->GetnPoints())) {
    m_pHead = new Route();
    m_pTail = new Route();
    m_pHead->CloneRoute(m_pRoute, 1, nSelected, _("_A"));
    m_pTail->CloneRoute(m_pRoute, nSelected, m_pRoute->GetnPoints(), _("_B"),
                        true);
    pRouteList->push_back(m_pHead);
    // pConfig->AddNewRoute(m_pHead);
    NavObj_dB::GetInstance().InsertRoute(m_pHead);

    pRouteList->push_back(m_pTail);
    // pConfig->AddNewRoute(m_pTail);
    NavObj_dB::GetInstance().InsertRoute(m_pTail);

    // pConfig->DeleteConfigRoute(m_pRoute);
    NavObj_dB::GetInstance().DeleteRoute(m_pRoute);

    pSelect->DeleteAllSelectableRoutePoints(m_pRoute);
    pSelect->DeleteAllSelectableRouteSegments(m_pRoute);
    g_pRouteMan->DeleteRoute(m_pRoute);
    pSelect->AddAllSelectableRouteSegments(m_pTail);
    pSelect->AddAllSelectableRoutePoints(m_pTail);
    pSelect->AddAllSelectableRouteSegments(m_pHead);
    pSelect->AddAllSelectableRoutePoints(m_pHead);

    SetRouteAndUpdate(m_pTail);
    UpdatePoints();

    if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
      pRouteManagerDialog->UpdateRouteListCtrl();
  }
}

void RoutePropDlgImpl::PrintOnButtonClick(wxCommandEvent& event) {
  static std::set<int> s_options;  // keep selected options
  RoutePrintDialog dlg(this, s_options);
  int result = dlg.ShowModal();

  if (result == wxID_OK) {
    dlg.GetSelected(s_options);
    RoutePrintout printout(m_pRoute, s_options, m_tz_selection);
    auto& printer = PrintDialog::GetInstance();
    printer.Initialize(wxPORTRAIT);
    printer.EnablePageNumbers(true);
    printer.Print(this, &printout);
  }

  event.Skip();
}

void RoutePropDlgImpl::ExtendOnButtonClick(wxCommandEvent& event) {
  m_btnExtend->Enable(false);

  if (IsThisRouteExtendable()) {
    int extend_idx = m_pExtendRoute->GetIndexOf(m_pExtendPoint);
    if (extend_idx < 0) {
      m_btnExtend->Enable(true);
      return;
    }
    int fm = extend_idx + 1;
    int to = m_pExtendRoute->GetnPoints();
    if (fm <= to) {
      pSelect->DeleteAllSelectableRouteSegments(m_pRoute);
      m_pRoute->CloneRoute(m_pExtendRoute, fm, to, _("_plus"));
      pSelect->AddAllSelectableRouteSegments(m_pRoute);
      SetRouteAndUpdate(m_pRoute);
      UpdatePoints();
    }
  }
  m_btnExtend->Enable(true);
}

bool RoutePropDlgImpl::IsThisRouteExtendable() {
  m_pExtendRoute = NULL;
  m_pExtendPoint = NULL;
  if (!m_pRoute || m_pRoute->m_bRtIsActive || m_pRoute->m_bIsInLayer)
    return false;

  RoutePoint* pLastPoint = m_pRoute->GetLastPoint();
  wxArrayPtrVoid* pEditRouteArray;

  pEditRouteArray = g_pRouteMan->GetRouteArrayContaining(pLastPoint);
  // remove invisible & own routes from choices
  int i;
  for (i = pEditRouteArray->GetCount(); i > 0; i--) {
    Route* p = (Route*)pEditRouteArray->Item(i - 1);
    if (!p->IsVisible() || (p->m_GUID == m_pRoute->m_GUID))
      pEditRouteArray->RemoveAt(i - 1);
  }
  if (pEditRouteArray->GetCount() == 1) {
    m_pExtendPoint = pLastPoint;
  } else {
    if (pEditRouteArray->GetCount() == 0) {
      int nearby_radius_meters =
          (int)(8. / top_frame::Get()->GetCanvasTrueScale());
      double rlat = pLastPoint->m_lat;
      double rlon = pLastPoint->m_lon;

      m_pExtendPoint = pWayPointMan->GetOtherNearbyWaypoint(
          rlat, rlon, nearby_radius_meters, pLastPoint->m_GUID);
      if (m_pExtendPoint) {
        wxArrayPtrVoid* pCloseWPRouteArray =
            g_pRouteMan->GetRouteArrayContaining(m_pExtendPoint);
        if (pCloseWPRouteArray) {
          pEditRouteArray = pCloseWPRouteArray;

          // remove invisible & own routes from choices
          for (i = pEditRouteArray->GetCount(); i > 0; i--) {
            Route* p = (Route*)pEditRouteArray->Item(i - 1);
            if (!p->IsVisible() || (p->m_GUID == m_pRoute->m_GUID))
              pEditRouteArray->RemoveAt(i - 1);
          }
        }
      }
    }
  }
  if (pEditRouteArray->GetCount() == 1) {
    Route* p = (Route*)pEditRouteArray->Item(0);
    int extend_idx = p->GetIndexOf(m_pExtendPoint);
    if (extend_idx < 0) {
      delete pEditRouteArray;
      return false;
    }
    int fm = extend_idx + 1;
    int to = p->GetnPoints();
    if (fm <= to) {
      m_pExtendRoute = p;
      delete pEditRouteArray;
      return true;
    }
  }
  delete pEditRouteArray;

  return false;
}

wxString RoutePropDlgImpl::MakeTideInfo(wxString stationName, double lat,
                                        double lon, wxDateTime utcTime) {
  if (stationName.Find("lind") != wxNOT_FOUND) int yyp = 4;

  if (stationName.IsEmpty()) {
    return "";
  }
  if (!utcTime.IsValid()) {
    return _("Invalid date/time!");
  }
  int stationID = ptcmgr->GetStationIDXbyName(stationName, lat, lon);
  if (stationID == 0) {
    return _("Unknown station!");
  }
  time_t dtmtt = utcTime.FromUTC().GetTicks();
  int ev = ptcmgr->GetNextBigEvent(&dtmtt, stationID);

  wxDateTime dtm;
  dtm.Set(dtmtt).MakeUTC();

  wxString tide_form = "";

  if (ev == 1) {
    tide_form.Append("LW: ");  // High Water
  } else if (ev == 2) {
    tide_form.Append("HW: ");  // Low Water
  } else if (ev == 0) {
    tide_form.Append(_("Unavailable: "));
  }

  int offset =
      ptcmgr->GetStationTimeOffset((IDX_entry*)ptcmgr->GetIDX_entry(stationID));
  DateTimeFormatOptions opts =
      DateTimeFormatOptions()
          .SetTimezone(getDatetimeTimezoneSelector(m_tz_selection))
          .SetLongitude(lon);
  wxString tideDateTime = ocpn::toUsrDateTimeFormat(dtm.FromUTC(), opts);
  tide_form.Append(tideDateTime);
  dtm.Add(wxTimeSpan(0, offset, 0));
  // Write next tide event using station timezone, formatted with explicit HH:MM
  // offset from UTC.
  tide_form.Append(wxString::Format(" (" + _("Local") + ": %s%+03d:%02d) @ %s",
                                    dtm.Format("%a %x %H:%M:%S"), (offset / 60),
                                    abs(offset) % 60, stationName.c_str()));
  return tide_form;
}

void RoutePropDlgImpl::ItemEditOnMenuSelection(wxCommandEvent& event) {
  wxString findurl = m_pEditedLink->GetURL();
  wxString findlabel = m_pEditedLink->GetLabel();

  LinkPropImpl* LinkPropDlg = new LinkPropImpl(this);
  LinkPropDlg->m_textCtrlLinkDescription->SetValue(findlabel);
  LinkPropDlg->m_textCtrlLinkUrl->SetValue(findurl);
  DimeControl(LinkPropDlg);
  LinkPropDlg->ShowWindowModalThenDo([this, LinkPropDlg, findurl,
                                      findlabel](int retcode) {
    if (retcode == wxID_OK) {
      for (Hyperlink* link : *m_pRoute->m_HyperlinkList) {
        wxString Link = link->Link;
        wxString Descr = link->DescrText;
        if (Link == findurl &&
            (Descr == findlabel || (Link == findlabel && Descr == ""))) {
          link->Link = LinkPropDlg->m_textCtrlLinkUrl->GetValue();
          link->DescrText = LinkPropDlg->m_textCtrlLinkDescription->GetValue();
          wxHyperlinkCtrl* h =
              (wxHyperlinkCtrl*)m_scrolledWindowLinks->FindWindowByLabel(
                  findlabel);
          if (h) {
            h->SetLabel(LinkPropDlg->m_textCtrlLinkDescription->GetValue());
            h->SetURL(LinkPropDlg->m_textCtrlLinkUrl->GetValue());
          }
        }
      }

      m_scrolledWindowLinks->InvalidateBestSize();
      m_scrolledWindowLinks->Layout();
      bSizerLinks->Layout();
    }
  });
  event.Skip();
}

void RoutePropDlgImpl::ItemAddOnMenuSelection(wxCommandEvent& event) {
  AddLinkOnButtonClick(event);
}

void RoutePropDlgImpl::ItemDeleteOnMenuSelection(wxCommandEvent& event) {
  wxString findurl = m_pEditedLink->GetURL();
  wxString findlabel = m_pEditedLink->GetLabel();

  wxWindowList kids = m_scrolledWindowLinks->GetChildren();
  for (unsigned int i = 0; i < kids.GetCount(); i++) {
    wxWindowListNode* node = kids.Item(i);
    wxWindow* win = node->GetData();

    auto link_win = dynamic_cast<wxHyperlinkCtrl*>(win);
    if (link_win) {
      link_win->Disconnect(
          wxEVT_COMMAND_HYPERLINK,
          wxHyperlinkEventHandler(RoutePropDlgImpl::OnHyperlinkClick));
      link_win->Disconnect(
          wxEVT_RIGHT_DOWN,
          wxMouseEventHandler(RoutePropDlgImpl::HyperlinkContextMenu));
      win->Destroy();
    }
  }

  ///    m_scrolledWindowLinks->DestroyChildren();
  int NbrOfLinks = m_pRoute->m_HyperlinkList->size();
  HyperlinkList* hyperlinklist = m_pRoute->m_HyperlinkList;
  //      int len = 0;
  auto nodeToDelete = hyperlinklist->end();
  if (NbrOfLinks > 0) {
    auto it = hyperlinklist->begin();
    while (it != hyperlinklist->end()) {
      Hyperlink* link = *it;
      wxString Link = link->Link;
      wxString Descr = link->DescrText;
      if (Link == findurl &&
          (Descr == findlabel || (Link == findlabel && Descr == "")))
        nodeToDelete = it;
      else {
        wxHyperlinkCtrl* ctrl = new wxHyperlinkCtrl(
            m_scrolledWindowLinks, wxID_ANY, Descr, Link, wxDefaultPosition,
            wxDefaultSize, wxHL_DEFAULT_STYLE);
        ctrl->Connect(
            wxEVT_COMMAND_HYPERLINK,
            wxHyperlinkEventHandler(RoutePropDlgImpl::OnHyperlinkClick), NULL,
            this);
        ctrl->Connect(
            wxEVT_RIGHT_DOWN,
            wxMouseEventHandler(RoutePropDlgImpl::HyperlinkContextMenu), NULL,
            this);

        bSizerLinks->Add(ctrl, 0, wxALL, 5);
      }
      it++;
    }
  }
  if (nodeToDelete != hyperlinklist->end()) {
    hyperlinklist->erase(nodeToDelete);
  }
  m_scrolledWindowLinks->InvalidateBestSize();
  m_scrolledWindowLinks->Layout();
  bSizerLinks->Layout();
  event.Skip();
}

void RoutePropDlgImpl::AddLinkOnButtonClick(wxCommandEvent& event) {
  LinkPropImpl* LinkPropDlg = new LinkPropImpl(this);
  LinkPropDlg->m_textCtrlLinkDescription->SetValue("");
  LinkPropDlg->m_textCtrlLinkUrl->SetValue("");
  DimeControl(LinkPropDlg);
  LinkPropDlg->ShowWindowModalThenDo([this, LinkPropDlg](int retcode) {
    if (retcode == wxID_OK) {
      wxString desc = LinkPropDlg->m_textCtrlLinkDescription->GetValue();
      if (desc == "") desc = LinkPropDlg->m_textCtrlLinkUrl->GetValue();
      wxHyperlinkCtrl* ctrl = new wxHyperlinkCtrl(
          m_scrolledWindowLinks, wxID_ANY, desc,
          LinkPropDlg->m_textCtrlLinkUrl->GetValue(), wxDefaultPosition,
          wxDefaultSize, wxHL_DEFAULT_STYLE);
      ctrl->Connect(wxEVT_COMMAND_HYPERLINK,
                    wxHyperlinkEventHandler(RoutePropDlgImpl::OnHyperlinkClick),
                    NULL, this);
      ctrl->Connect(wxEVT_RIGHT_DOWN,
                    wxMouseEventHandler(RoutePropDlgImpl::HyperlinkContextMenu),
                    NULL, this);

      bSizerLinks->Add(ctrl, 0, wxALL, 5);
      m_scrolledWindowLinks->InvalidateBestSize();
      m_scrolledWindowLinks->Layout();
      bSizerLinks->Layout();

      Hyperlink* h = new Hyperlink();
      h->DescrText = LinkPropDlg->m_textCtrlLinkDescription->GetValue();
      h->Link = LinkPropDlg->m_textCtrlLinkUrl->GetValue();
      h->LType = "";
      m_pRoute->m_HyperlinkList->push_back(h);
    }
  });
}

void RoutePropDlgImpl::BtnEditOnToggleButton(wxCommandEvent& event) {
  if (m_toggleBtnEdit->GetValue()) {
    m_stEditEnabled->SetLabel(_("Links are opened for editing."));
  } else {
    m_stEditEnabled->SetLabel(_("Links are opened in the default browser."));
  }
  event.Skip();
}

void RoutePropDlgImpl::OnHyperlinkClick(wxHyperlinkEvent& event) {
  if (m_toggleBtnEdit->GetValue()) {
    m_pEditedLink = (wxHyperlinkCtrl*)event.GetEventObject();
    ItemEditOnMenuSelection(event);
    event.Skip(false);
    return;
  }
  //    Windows has trouble handling local file URLs with embedded anchor
  //    points, e.g file://testfile.html#point1 The trouble is with the
  //    wxLaunchDefaultBrowser with verb "open" Workaround is to probe the
  //    registry to get the default browser, and open directly
  //
  //    But, we will do this only if the URL contains the anchor point character
  //    '#' What a hack......

#ifdef __WXMSW__
  wxString cc = event.GetURL();
  if (cc.Find("#") != wxNOT_FOUND) {
    wxRegKey RegKey(wxString("HKEY_CLASSES_ROOT\\HTTP\\shell\\open\\command"));
    if (RegKey.Exists()) {
      wxString command_line;
      RegKey.QueryValue(wxString(""), command_line);

      //  Remove "
      command_line.Replace(wxString("\""), wxString(""));

      //  Strip arguments
      int l = command_line.Find(".exe");
      if (wxNOT_FOUND == l) l = command_line.Find(".EXE");

      if (wxNOT_FOUND != l) {
        wxString cl = command_line.Mid(0, l + 4);
        cl += " ";
        cc.Prepend("\"");
        cc.Append("\"");
        cl += cc;
        wxExecute(cl);  // Async, so Fire and Forget...
      }
    }
  } else
    event.Skip();
#else
  wxString url = event.GetURL();
  url.Replace(" ", "%20");
  ::wxLaunchDefaultBrowser(url);
#endif
}

void RoutePropDlgImpl::HyperlinkContextMenu(wxMouseEvent& event) {
  m_pEditedLink = (wxHyperlinkCtrl*)event.GetEventObject();
  m_scrolledWindowLinks->PopupMenu(
      m_menuLink, m_pEditedLink->GetPosition().x + event.GetPosition().x,
      m_pEditedLink->GetPosition().y + event.GetPosition().y);
}
