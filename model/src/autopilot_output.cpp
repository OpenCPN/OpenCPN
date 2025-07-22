/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Autopilot output support
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2025 by David S. Register                               *
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
#include <cmath>
#include <memory>
#include <vector>

#include <wx/wxprec.h>

#include "model/autopilot_output.h"
#include "model/comm_drv_n2k_serial.h"
#include "model/comm_drv_registry.h"
#include "model/comm_n0183_output.h"
#include "model/comm_vars.h"
#include "model/config_vars.h"
#include "model/georef.h"
#include "model/gui.h"
#include "model/nmea_ctx_factory.h"
#include "model/nmea_log.h"
#include "model/own_ship.h"
#include "model/routeman.h"
#include "N2kMsg.h"
#include "N2kMessages.h"

#include "observable_globvar.h"

#ifdef __ANDROID__
#include "androidUTIL.h"
#endif

static NmeaLog *GetNmeaLog() {
  auto w = wxWindow::FindWindowByName(kDataMonitorWindowName);
  auto log = dynamic_cast<NmeaLog *>(w);
  assert(log);
  return log;
}

bool UpdateAutopilotN0183(Routeman &routeman) {
  NMEA0183 nmea0183 = routeman.GetNMEA0183();
  RoutePoint *pActivePoint = routeman.GetpActivePoint();

  // Set max WP name length
  int maxName = 6;
  if ((g_maxWPNameLength >= 3) && (g_maxWPNameLength <= 32))
    maxName = g_maxWPNameLength;

  // Avoid a possible not initiated SOG/COG. APs can be confused if in NAV mode
  // wo valid GPS
  double r_Sog(0.0), r_Cog(0.0);
  if (!std::isnan(gSog)) r_Sog = gSog;
  if (!std::isnan(gCog)) r_Cog = gCog;

  // RMB
  {
    SENTENCE snt;
    nmea0183.Rmb.IsDataValid = bGPSValid ? NTrue : NFalse;
    nmea0183.Rmb.CrossTrackError = routeman.GetCurrentXTEToActivePoint();
    nmea0183.Rmb.DirectionToSteer = routeman.GetXTEDir() < 0 ? Left : Right;
    nmea0183.Rmb.RangeToDestinationNauticalMiles =
        routeman.GetCurrentRngToActivePoint();
    nmea0183.Rmb.BearingToDestinationDegreesTrue =
        routeman.GetCurrentBrgToActivePoint();

    if (pActivePoint->m_lat < 0.)
      nmea0183.Rmb.DestinationPosition.Latitude.Set(-pActivePoint->m_lat, "S");
    else
      nmea0183.Rmb.DestinationPosition.Latitude.Set(pActivePoint->m_lat, "N");

    if (pActivePoint->m_lon < 0.)
      nmea0183.Rmb.DestinationPosition.Longitude.Set(-pActivePoint->m_lon, "W");
    else
      nmea0183.Rmb.DestinationPosition.Longitude.Set(pActivePoint->m_lon, "E");

    nmea0183.Rmb.DestinationClosingVelocityKnots =
        r_Sog *
        cos((r_Cog - routeman.GetCurrentBrgToActivePoint()) * PI / 180.0);
    nmea0183.Rmb.IsArrivalCircleEntered =
        routeman.GetArrival() ? NTrue : NFalse;
    nmea0183.Rmb.FAAModeIndicator = bGPSValid ? "A" : "N";
    // RMB is close to NMEA0183 length limit
    // Restrict WP names further if necessary
    int wp_len = maxName;
    do {
      nmea0183.Rmb.To = pActivePoint->GetName().Truncate(wp_len);
      nmea0183.Rmb.From =
          routeman.GetpActiveRouteSegmentBeginPoint()->GetName().Truncate(
              wp_len);
      nmea0183.Rmb.Write(snt);
      wp_len -= 1;
    } while (snt.Sentence.size() > 82 && wp_len > 0);

    BroadcastNMEA0183Message(snt.Sentence, GetNmeaLog(),
                             routeman.GetMessageSentEventVar());
  }

  // RMC
  {
    SENTENCE snt;
    nmea0183.Rmc.IsDataValid = NTrue;
    if (!bGPSValid) nmea0183.Rmc.IsDataValid = NFalse;

    if (gLat < 0.)
      nmea0183.Rmc.Position.Latitude.Set(-gLat, _T("S"));
    else
      nmea0183.Rmc.Position.Latitude.Set(gLat, _T("N"));

    if (gLon < 0.)
      nmea0183.Rmc.Position.Longitude.Set(-gLon, _T("W"));
    else
      nmea0183.Rmc.Position.Longitude.Set(gLon, _T("E"));

    nmea0183.Rmc.SpeedOverGroundKnots = r_Sog;
    nmea0183.Rmc.TrackMadeGoodDegreesTrue = r_Cog;

    if (!std::isnan(gVar)) {
      if (gVar < 0.) {
        nmea0183.Rmc.MagneticVariation = -gVar;
        nmea0183.Rmc.MagneticVariationDirection = West;
      } else {
        nmea0183.Rmc.MagneticVariation = gVar;
        nmea0183.Rmc.MagneticVariationDirection = East;
      }
    } else
      nmea0183.Rmc.MagneticVariation =
          361.;  // A signal to NMEA converter, gVAR is unknown

    // Send GPS time to autopilot if available else send local system time
    if (!gRmcTime.IsEmpty() && !gRmcDate.IsEmpty()) {
      nmea0183.Rmc.UTCTime = gRmcTime;
      nmea0183.Rmc.Date = gRmcDate;
    } else {
      wxDateTime now = wxDateTime::Now();
      wxDateTime utc = now.ToUTC();
      wxString time = utc.Format(_T("%H%M%S"));
      nmea0183.Rmc.UTCTime = time;
      wxString date = utc.Format(_T("%d%m%y"));
      nmea0183.Rmc.Date = date;
    }

    nmea0183.Rmc.FAAModeIndicator = "A";
    if (!bGPSValid) nmea0183.Rmc.FAAModeIndicator = "N";

    nmea0183.Rmc.Write(snt);

    BroadcastNMEA0183Message(snt.Sentence, GetNmeaLog(),
                             routeman.GetMessageSentEventVar());
  }

  // APB
  {
    SENTENCE snt;

    nmea0183.Apb.IsLoranBlinkOK =
        NTrue;  // considered as "generic invalid fix" flag
    if (!bGPSValid) nmea0183.Apb.IsLoranBlinkOK = NFalse;

    nmea0183.Apb.IsLoranCCycleLockOK = NTrue;
    if (!bGPSValid) nmea0183.Apb.IsLoranCCycleLockOK = NFalse;

    nmea0183.Apb.CrossTrackErrorMagnitude =
        routeman.GetCurrentXTEToActivePoint();

    if (routeman.GetXTEDir() < 0)
      nmea0183.Apb.DirectionToSteer = Left;
    else
      nmea0183.Apb.DirectionToSteer = Right;

    nmea0183.Apb.CrossTrackUnits = _T("N");

    if (routeman.GetArrival())
      nmea0183.Apb.IsArrivalCircleEntered = NTrue;
    else
      nmea0183.Apb.IsArrivalCircleEntered = NFalse;

    //  We never pass the perpendicular, since we declare arrival before
    //  reaching this point
    nmea0183.Apb.IsPerpendicular = NFalse;

    nmea0183.Apb.To = pActivePoint->GetName().Truncate(maxName);

    double brg1, dist1;
    DistanceBearingMercator(pActivePoint->m_lat, pActivePoint->m_lon,
                            routeman.GetpActiveRouteSegmentBeginPoint()->m_lat,
                            routeman.GetpActiveRouteSegmentBeginPoint()->m_lon,
                            &brg1, &dist1);

    if (g_bMagneticAPB && !std::isnan(gVar)) {
      double brg1m =
          ((brg1 - gVar) >= 0.) ? (brg1 - gVar) : (brg1 - gVar + 360.);
      double bapm = ((routeman.GetCurrentBrgToActivePoint() - gVar) >= 0.)
                        ? (routeman.GetCurrentBrgToActivePoint() - gVar)
                        : (routeman.GetCurrentBrgToActivePoint() - gVar + 360.);

      nmea0183.Apb.BearingOriginToDestination = brg1m;
      nmea0183.Apb.BearingOriginToDestinationUnits = _T("M");

      nmea0183.Apb.BearingPresentPositionToDestination = bapm;
      nmea0183.Apb.BearingPresentPositionToDestinationUnits = _T("M");

      nmea0183.Apb.HeadingToSteer = bapm;
      nmea0183.Apb.HeadingToSteerUnits = _T("M");
    } else {
      nmea0183.Apb.BearingOriginToDestination = brg1;
      nmea0183.Apb.BearingOriginToDestinationUnits = _T("T");

      nmea0183.Apb.BearingPresentPositionToDestination =
          routeman.GetCurrentBrgToActivePoint();
      nmea0183.Apb.BearingPresentPositionToDestinationUnits = _T("T");

      nmea0183.Apb.HeadingToSteer = routeman.GetCurrentBrgToActivePoint();
      nmea0183.Apb.HeadingToSteerUnits = _T("T");
    }

    nmea0183.Apb.Write(snt);
    BroadcastNMEA0183Message(snt.Sentence, GetNmeaLog(),
                             routeman.GetMessageSentEventVar());
  }

  // XTE
  {
    SENTENCE snt;

    nmea0183.Xte.IsLoranBlinkOK =
        NTrue;  // considered as "generic invalid fix" flag
    if (!bGPSValid) nmea0183.Xte.IsLoranBlinkOK = NFalse;

    nmea0183.Xte.IsLoranCCycleLockOK = NTrue;
    if (!bGPSValid) nmea0183.Xte.IsLoranCCycleLockOK = NFalse;

    nmea0183.Xte.CrossTrackErrorDistance =
        routeman.GetCurrentXTEToActivePoint();

    if (routeman.GetXTEDir() < 0)
      nmea0183.Xte.DirectionToSteer = Left;
    else
      nmea0183.Xte.DirectionToSteer = Right;

    nmea0183.Xte.CrossTrackUnits = _T("N");

    nmea0183.Xte.Write(snt);
    BroadcastNMEA0183Message(snt.Sentence, GetNmeaLog(),
                             routeman.GetMessageSentEventVar());
  }

  return true;
}

bool UpdateAutopilotN2K(Routeman &routeman) {
  bool fail_any = false;

  // Get a suitable N2K Driver
  auto &registry = CommDriverRegistry::GetInstance();
  const std::vector<DriverPtr> &drivers = registry.GetDrivers();

  AbstractCommDriver *found = nullptr;
  for (auto key : routeman.GetOutpuDriverArray()) {
    for (auto &d : drivers) {
      if (d->Key() == key) {
        std::unordered_map<std::string, std::string> attributes =
            GetAttributes(key);
        auto protocol_it = attributes.find("protocol");
        if (protocol_it != attributes.end()) {
          std::string protocol = protocol_it->second;

          if (protocol == "nmea2000") {
            found = d.get();
          }
        }
      }
    }
  }
  if (!found) return false;

  // N2K serial drivers require maintenance of an enabled PGN TX list
  auto drv_serial = dynamic_cast<CommDriverN2KSerial *>(found);
  if (drv_serial) {
    drv_serial->AddTxPGN(129283);
    drv_serial->AddTxPGN(129284);
    drv_serial->AddTxPGN(129285);
  }

  fail_any |= !SendPGN129285(routeman, found);
  fail_any |= !SendPGN129284(routeman, found);
  fail_any |= !SendPGN129283(routeman, found);

  return (fail_any == 0);
}

bool SendPGN129285(Routeman &routeman, AbstractCommDriver *driver) {
  bool fail_any = false;

  // Set max WP name length
  int maxName = 6;
  if ((g_maxWPNameLength >= 3) && (g_maxWPNameLength <= 32))
    maxName = g_maxWPNameLength;

  //  Create and transmit PGN 129285
  // The basic PGN129285 head
  tN2kMsg msg129285;
  char route_name[] = "Route";
  SetN2kPGN129285(msg129285, 0, 0, 0, N2kdir_forward, 0, route_name);

  //  Append start point of current leg
  RoutePoint *pLegBeginPoint = routeman.GetpActiveRouteSegmentBeginPoint();
  wxString start_point_name = pLegBeginPoint->GetName().Truncate(maxName);
  std::string sname = start_point_name.ToStdString();
  char *s = (char *)sname.c_str();

  fail_any |= !AppendN2kPGN129285(msg129285, 0, s, pLegBeginPoint->m_lat,
                                  pLegBeginPoint->m_lon);
  // Append destination point of current leg
  RoutePoint *pActivePoint = routeman.GetpActivePoint();
  wxString destination_name = pActivePoint->GetName().Truncate(maxName);
  std::string dname = destination_name.ToStdString();
  char *d = (char *)dname.c_str();
  fail_any |= !AppendN2kPGN129285(msg129285, 1, d, pActivePoint->m_lat,
                                  pActivePoint->m_lon);

  if (fail_any) return false;

  auto dest_addr = std::make_shared<const NavAddr2000>(driver->iface, 255);
  std::vector<uint8_t> payload;
  for (int i = 0; i < msg129285.DataLen; i++)
    payload.push_back(msg129285.Data[i]);
  auto PGN129285 =
      std::make_shared<const Nmea2000Msg>(129285, payload, dest_addr, 6);
  fail_any |= !driver->SendMessage(PGN129285, dest_addr);

  return (fail_any == 0);
}

bool SendPGN129284(Routeman &routeman, AbstractCommDriver *driver) {
  bool fail_any = false;
  tN2kMsg msg129284;
  RoutePoint *pActivePoint = routeman.GetpActivePoint();

  // Calculate closing velocity and ETA
  double vmg = 0.;
  if (!std::isnan(gCog) && !std::isnan(gSog)) {
    double brg = routeman.GetCurrentBrgToActivePoint();
    vmg = gSog * cos((brg - gCog) * PI / 180.);
  }
  wxTimeSpan tttg_span;
  wxDateTime arrival_time = wxDateTime::Now();
  if (vmg > 0.) {
    double tttg_sec = (routeman.GetCurrentRngToActivePoint() / gSog) * 3600;
    tttg_span = wxTimeSpan::Seconds((long)tttg_sec);
    arrival_time += tttg_span;
  }
  double time_days_1979 = arrival_time.GetTicks() / (3600. * 24.);

  //  ETA time_seconds, expressed as seconds since midnight
  //  ETA date, expressed as whole days since 1 January 1970
  double eta_time_days;
  double eta_time_seconds = modf(time_days_1979, &eta_time_days);
  int16_t eta_time_days_16 = static_cast<uint16_t>(eta_time_days);

  SetN2kPGN129284(
      msg129284,
      0xFF,                                           // SID
      routeman.GetCurrentRngToActivePoint() * 1852.,  // DistanceToWaypoint
      N2khr_true,        // tN2kHeadingReference BearingReference
      false,             // PerpendicularCrossed
      false,             // ArrivalCircleEntered
      N2kdct_RhumbLine,  // tN2kDistanceCalculationType CalculationType
      eta_time_seconds,  // double ETATime,
      eta_time_days_16,  // int16_t ETADate,
      routeman.GetCurrentSegmentCourse() * PI /
          180.,  // BearingOriginToDestinationWaypoint,
      routeman.GetCurrentBrgToActivePoint() * PI /
          180.,             // BearingPositionToDestinationWaypoint,
      0,                    //   uint8_t  OriginWaypointNumber,
      1,                    //   uint8_t  DestinationWaypointNumber,
      pActivePoint->m_lat,  //   double DestinationLatitude,
      pActivePoint->m_lon,  //    double DestinationLongitude,
      vmg);                 //    double  WaypointClosingVelocity);

  auto dest_addr = std::make_shared<const NavAddr2000>(driver->iface, 255);
  std::vector<uint8_t> payload;
  for (int i = 0; i < msg129284.DataLen; i++)
    payload.push_back(msg129284.Data[i]);
  auto PGN129284 =
      std::make_shared<const Nmea2000Msg>(129284, payload, dest_addr, 6);
  fail_any |= !driver->SendMessage(PGN129284, dest_addr);

  return (fail_any == 0);
}

bool SendPGN129283(Routeman &routeman, AbstractCommDriver *driver) {
  bool fail_any = false;
  tN2kMsg msg129283;
  RoutePoint *pActivePoint = routeman.GetpActivePoint();
  // N2K, distance units are metres, therefore convert from Nm
  double xte = routeman.GetCurrentXTEToActivePoint() * 1852.;
  // N2K, -ve xte, means to the left of the course, steer right, Reverse of
  // current Routeman logic
  if (routeman.GetXTEDir() > 0) {
    xte = -xte;
  }
  SetN2kPGN129283(msg129283, 0,
                  N2kxtem_Autonomous,  // tN2kXTEMode XTEMode,
                  false,               // bool NavigationTerminated,
                  xte                  // double XTE
  );

  auto dest_addr = std::make_shared<const NavAddr2000>(driver->iface, 255);
  std::vector<uint8_t> payload;
  for (int i = 0; i < msg129283.DataLen; i++)
    payload.push_back(msg129283.Data[i]);
  auto PGN129283 =
      std::make_shared<const Nmea2000Msg>(129283, payload, dest_addr, 6);
  fail_any |= !driver->SendMessage(PGN129283, dest_addr);

  return (fail_any == 0);
}
