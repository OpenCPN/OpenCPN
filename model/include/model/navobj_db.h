/**************************************************************************
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

/**
 * \file
 * Class NavObj_dB.
 */

#ifndef _NAVOBJ_DB_H__
#define _NAVOBJ_DB_H__

#include <wx/timer.h>
#include "notification.h"
#include "observable_evtvar.h"
#include "comm_appmsg.h"
#include <sqlite3.h>
#include "track.h"

/** The navobj SQLite container object, a singleton. */
class NavObj_dB {
public:
  static NavObj_dB &GetInstance();

  NavObj_dB(const NavObj_dB &) = delete;
  NavObj_dB &operator=(const NavObj_dB &) = delete;

  void Close();
  void LoadNavObjects();

  // Tracks
  bool LoadAllTracks();
  bool InsertTrack(Track *track);
  bool UpdateTrack(Track *track);
  bool AddTrackPoint(Track *track, TrackPoint *point);
  bool UpdateDBTrackAttributes(Track *track);
  bool DeleteTrack(Track *track);

  // Routes
  bool LoadAllRoutes();
  bool InsertRoute(Route *route);
  bool UpdateRoute(Route *route);
  bool UpdateRouteViz(Route *route);

  bool UpdateDBRouteAttributes(Route *route);
  bool UpdateDBRoutePointAttributes(RoutePoint *point);
  bool UpdateDBRoutePointViz(RoutePoint *point);
  bool DeleteRoute(Route *route);

  // RoutePoints
  bool LoadAllPoints();
  bool InsertRoutePoint(RoutePoint *point);
  bool DeleteRoutePoint(RoutePoint *point);
  bool UpdateRoutePoint(RoutePoint *point);

  // Legacy navobj import
  bool ImportLegacyNavobj(wxFrame *frame);

private:
  NavObj_dB();
  ~NavObj_dB();

  bool ImportLegacyTracks();
  bool ImportLegacyRoutes();
  bool ImportLegacyPoints();
  void CountImportNavObjects();

  int m_open_result;
  sqlite3 *m_db;
  bool m_importing;
  int m_nimportPoints;
  int m_nimportRoutes;
  int m_nimportTracks;
  int m_nImportObjects;
  int m_import_progesscount;
  wxProgressDialog *m_pImportProgress;
};

#endif
