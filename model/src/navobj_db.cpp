/***************************************************************************
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Implement navobj_db.h -- MySQL based storage for routes, tracks, etc.
 */

#include <cmath>
#include <iomanip>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <wx/dir.h>
#include <wx/filename.h>

#include "model/base_platform.h"
#include "model/comm_appmsg_bus.h"
#include "model/navobj_db.h"
#include "model/navutil_base.h"
#include "model/notification.h"
#include "model/notification_manager.h"
#include "model/routeman.h"

static void ReportError(const std::string zmsg);  // forward

static bool executeSQL(sqlite3* db, const char* sql) {
  char* errMsg = nullptr;
  if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
    wxString msg =
        wxString::Format(_("navobj database error.") + " %s", errMsg);
    wxLogMessage(msg);
    auto& noteman = NotificationManager::GetInstance();
    noteman.AddNotification(NotificationSeverity::kWarning, msg.ToStdString());
    sqlite3_free(errMsg);
    return false;
  }
  return true;
}

static bool executeSQL(sqlite3* db, wxString& sql) {
  return executeSQL(db, sql.ToStdString().c_str());
}

bool CreateTables(sqlite3* db) {
  // Track tables
  const char* create_tables_sql = R"(
        CREATE TABLE IF NOT EXISTS tracks (
            guid TEXT PRIMARY KEY NOT NULL,
            name TEXT,
            description TEXT,
            visibility INTEGER,
            start_string TEXT,
            end_string TEXT,
            width INTEGER,
            style INTEGER,
            color TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );

        CREATE TABLE IF NOT EXISTS trk_points (
            track_guid TEXT NOT NULL,
            latitude REAL NOT NULL,
            longitude REAL NOT NULL,
            timestamp TEXT NOT NULL,
            point_order INTEGER,
            FOREIGN KEY (track_guid) REFERENCES tracks(guid) ON DELETE CASCADE
        );


        CREATE TABLE IF NOT EXISTS track_html_links (
            guid TEXT PRIMARY KEY,
            track_guid TEXT NOT NULL,
            html_link TEXT,
            html_description TEXT,
            html_type TEXT,
            FOREIGN KEY (track_guid) REFERENCES tracks(guid) ON DELETE CASCADE
        );


        CREATE TABLE IF NOT EXISTS routes (
            guid TEXT PRIMARY KEY NOT NULL,
            name TEXT,
            start_string TEXT,
            end_string TEXT,
            description TEXT,
            planned_departure TEXT,
            plan_speed REAL,
            time_format TEXT,
            style INTEGER,
            width INTEGER,
            color TEXT,
            visibility INTEGER,
            shared_wp_viz INTEGER,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );


        CREATE TABLE IF NOT EXISTS routepoints (
            guid TEXT PRIMARY KEY NOT NULL,
            lat REAL,
            lon REAL,
            Symbol TEXT,
            Name TEXT,
            description TEXT,
            TideStation TEXT,
            plan_speed REAL,
            etd INTEGER,
            Type TEXT,
            Time TEXT,
            ArrivalRadius REAL,
            RangeRingsNumber INTEGER,
            RangeRingsStep REAL,
            RangeRingsStepUnits INTEGER,
            RangeRingsVisible INTEGER,
            RangeRingsColour TEXT,
            ScaleMin INTEGER,
            ScaleMax INTEGER,
            UseScale INTEGER,
            visibility INTEGER,
            viz_name INTEGER,
            shared INTEGER,
            isolated INTEGER,
            linked_layer_guid TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );

        CREATE TABLE IF NOT EXISTS routepoints_link (
            route_guid TEXT,
            point_guid TEXT,
            point_order INTEGER,
            PRIMARY KEY (route_guid, point_order),
            FOREIGN KEY (route_guid) REFERENCES routes(guid) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS route_html_links (
            guid TEXT PRIMARY KEY,
            route_guid TEXT NOT NULL,
            html_link TEXT,
            html_description TEXT,
            html_type TEXT,
            FOREIGN KEY (route_guid) REFERENCES routes(guid) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS routepoint_html_links (
            guid TEXT PRIMARY KEY,
            routepoint_guid TEXT NOT NULL,
            html_link TEXT,
            html_description TEXT,
            html_type TEXT,
            FOREIGN KEY (routepoint_guid) REFERENCES routepoints(guid) ON DELETE CASCADE
        );

        CREATE INDEX IF NOT EXISTS idx_track_points
        ON trk_points (track_guid);

        )";

  if (!executeSQL(db, create_tables_sql)) return false;

  // Add new columns if the database was created with an older schema.
  executeSQL(db, "ALTER TABLE routepoints ADD COLUMN linked_layer_guid TEXT");

  return true;
}

bool TrackExists(sqlite3* db, const std::string& track_guid) {
  const char* sql = "SELECT 1 FROM tracks WHERE guid = ? LIMIT 1";
  sqlite3_stmt* stmt;
  bool exists = false;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, track_guid.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
      exists = true;  // found a match
    }

    sqlite3_finalize(stmt);
  } else {
    ReportError("TrackExists:prepare");
    return false;
  }
  return exists;
}

bool TrackHtmlLinkExists(sqlite3* db, const std::string& link_guid) {
  const char* sql = "SELECT 1 FROM track_html_links WHERE guid = ? LIMIT 1";
  sqlite3_stmt* stmt;
  bool exists = false;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, link_guid.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
      exists = true;  // found a match
    }

    sqlite3_finalize(stmt);
  } else {
    ReportError("TrackHtmlLinkExists:prepare");
    return false;
  }
  return exists;
}

bool DeleteAllCommentsForTrack(sqlite3* db, const std::string& track_guid) {
  const char* sql = "DELETE FROM track_html_links WHERE track_guid = ?";

  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, track_guid.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      ReportError("DeleteAllCommentsForTrack:step");
      return false;
    }

    sqlite3_finalize(stmt);
  } else {
    ReportError("DeleteAllCommentsForTrack:prepare");
    return false;
  }
  return true;
}

bool InsertTrackPoint(sqlite3* db, const std::string& track_guid, double lat,
                      double lon, const std::string& timestamp, int i_point) {
  const char* sql = R"(
        INSERT INTO trk_points (track_guid, latitude, longitude, timestamp, point_order)
        VALUES (?, ?, ?, ?, ?)
    )";
  sqlite3_stmt* stmt;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, track_guid.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 2, lat);
    sqlite3_bind_double(stmt, 3, lon);
    sqlite3_bind_text(stmt, 4, timestamp.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, i_point);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      ReportError("InsertTrackPoint:step");
      sqlite3_finalize(stmt);
      return false;
    }
    sqlite3_finalize(stmt);
  } else {
    return false;
  }
  return true;
}

bool InsertTrackHTML(sqlite3* db, const std::string& track_guid,
                     const std::string& link_guid, const std::string& descrText,
                     const std::string& link, const std::string& ltype) {
  const char* sql = R"(
        INSERT INTO track_html_links (guid, track_guid, html_link, html_description, html_type)
        VALUES (?, ?, ?, ?, ?)
    )";
  sqlite3_stmt* stmt;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, link_guid.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, track_guid.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, link.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, descrText.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, ltype.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      ReportError("InsertTrackHTML:step");
      sqlite3_finalize(stmt);
      return false;
    }
    sqlite3_finalize(stmt);
  } else {
    return false;
  }
  return true;
}

//..Routes

bool DeleteAllCommentsForRoute(sqlite3* db, const std::string& route_guid) {
  const char* sql = R"(
        DELETE FROM route_html_links WHERE route_guid = ?
    )";
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, route_guid.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      ReportError("DeleteAllCommentsForRoute:step");
      sqlite3_finalize(stmt);
      return false;
    }
    sqlite3_finalize(stmt);
  } else {
    return false;
  }
  return true;
}

bool RouteHtmlLinkExists(sqlite3* db, const std::string& link_guid) {
  const char* sql = "SELECT 1 FROM route_html_links WHERE guid = ? LIMIT 1";
  sqlite3_stmt* stmt;
  bool exists = false;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, link_guid.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
      exists = true;  // found a match
    }

    sqlite3_finalize(stmt);
  } else {
    return false;
  }
  return exists;
}

bool RoutePointHtmlLinkExists(sqlite3* db, const std::string& link_guid) {
  const char* sql =
      "SELECT 1 FROM routepoint_html_links WHERE guid = ? LIMIT 1";
  sqlite3_stmt* stmt;
  bool exists = false;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, link_guid.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
      exists = true;  // found a match
    }

    sqlite3_finalize(stmt);
  } else {
    return false;
  }
  return exists;
}

bool RouteExistsDB(sqlite3* db, const std::string& route_guid) {
  const char* sql = "SELECT 1 FROM routes WHERE guid = ? LIMIT 1";
  sqlite3_stmt* stmt;
  bool exists = false;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, route_guid.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
      exists = true;  // found a match
    }

    sqlite3_finalize(stmt);
  } else {
    return false;
  }
  return exists;
}

bool InsertRouteHTML(sqlite3* db, const std::string& route_guid,
                     const std::string& link_guid, const std::string& descrText,
                     const std::string& link, const std::string& ltype) {
  const char* sql = R"(
        INSERT INTO route_html_links (guid, route_guid, html_link, html_description, html_type)
        VALUES (?, ?, ?, ?, ?)
    )";
  sqlite3_stmt* stmt;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, link_guid.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, route_guid.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, link.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, descrText.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, ltype.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      ReportError("InsertRouteHTML:step");
      sqlite3_finalize(stmt);
      return false;
    }
    sqlite3_finalize(stmt);
  } else {
    return false;
  }
  return true;
}

bool RoutePointExists(sqlite3* db, const std::string& routepoint_guid) {
  const char* sql = "SELECT 1 FROM routepoints WHERE guid = ? LIMIT 1";
  sqlite3_stmt* stmt;
  bool exists = false;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, routepoint_guid.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
      exists = true;  // found a match
    }

    sqlite3_finalize(stmt);
  } else {
    return false;
  }
  return exists;
}

bool InsertRoutePointHTML(sqlite3* db, const std::string& point_guid,
                          const std::string& link_guid,
                          const std::string& descrText, const std::string& link,
                          const std::string& ltype) {
  const char* sql = R"(
        INSERT INTO routepoint_html_links (guid, routepoint_guid, html_link, html_description, html_type)
        VALUES (?, ?, ?, ?, ?)
    )";
  sqlite3_stmt* stmt;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, link_guid.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, point_guid.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, link.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, descrText.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, ltype.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      ReportError("InsertRoutePointHTML:step");
      sqlite3_finalize(stmt);
      return false;
    }
    sqlite3_finalize(stmt);
  } else {
    return false;
  }
  return true;
}
bool DeleteAllCommentsForRoutePoint(sqlite3* db,
                                    const std::string& routepoint_guid) {
  const char* sql =
      "DELETE FROM routepoint_html_links WHERE routepoint_guid = ?";

  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, routepoint_guid.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      ReportError("DeleteAllCommentsForRoutepoint:step");
      return false;
    }

    sqlite3_finalize(stmt);
  } else {
    ReportError("DeleteAllCommentsForRoutepoint:prepare");
    return false;
  }
  return true;
}

bool InsertRoutePointDB(sqlite3* db, RoutePoint* point) {
  const char* sql = R"(
        INSERT or REPLACE INTO routepoints(guid)
        VALUES (?)
    )";
  sqlite3_stmt* stmt;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, point->m_GUID.ToStdString().c_str(), -1,
                      SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      ReportError("InsertRoutePointDB:step");
      sqlite3_finalize(stmt);
      return false;
    }
    sqlite3_finalize(stmt);
  } else {
    return false;
  }
  return true;
}

bool InsertRoutePointLink(sqlite3* db, Route* route, RoutePoint* point,
                          int point_order) {
  const char* sql = R"(
        INSERT or IGNORE INTO routepoints_link (route_guid, point_guid, point_order)
        VALUES (?, ?, ?)
    )";

  sqlite3_stmt* stmt;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, route->m_GUID.ToStdString().c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, point->m_GUID.ToStdString().c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, point_order);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      ReportError("InsertTrackPointLink:step");
      sqlite3_finalize(stmt);
      return false;
    }
    sqlite3_finalize(stmt);
  } else {
    return false;
  }
  return true;
}

void DeleteOrphanedRoutepoint(sqlite3* db) {
  const char* sql = R"(
        DELETE FROM routepoints
        WHERE guid NOT IN (SELECT point_guid FROM routepoints_link)
    )";
  char* errMsg = nullptr;

  if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
  } else {
  }
}

void errorLogCallback(void* pArg, int iErrCode, const char* zMsg) {
  wxString msg =
      wxString::Format(_("navobj database error.") + " %d: %s", iErrCode, zMsg);
  wxLogMessage(msg);
  auto& noteman = NotificationManager::GetInstance();
  noteman.AddNotification(NotificationSeverity::kWarning, msg.ToStdString());
}

static void ReportError(const std::string zmsg) {
  wxString msg =
      wxString::Format(_("navobj database error.") + " %s", zmsg.c_str());
  wxLogMessage(msg);
  auto& noteman = NotificationManager::GetInstance();
  noteman.AddNotification(NotificationSeverity::kWarning, msg.ToStdString());
}

NavObj_dB& NavObj_dB::GetInstance() {
  static NavObj_dB instance;
  return instance;
}

NavObj_dB::NavObj_dB() {
  m_pImportProgress = nullptr;

  // Set SQLite per-process config options
  int ie = sqlite3_config(SQLITE_CONFIG_LOG, errorLogCallback, nullptr);

  // Does dB file exist?
  wxString db_filename = g_BasePlatform->GetPrivateDataDir() +
                         wxFileName::GetPathSeparator() + "navobj.db";
  if (!wxFileExists(db_filename)) {
    //  Make a safety backup of current navobj.xml
    wxString xml_filename = g_BasePlatform->GetPrivateDataDir() +
                            wxFileName::GetPathSeparator() + "navobj.xml";
    if (wxFileExists(xml_filename)) {
      wxCopyFile(xml_filename, xml_filename + ".backup");

      // Make another safety backup, one time
      wxString deep_backup_filename = g_BasePlatform->GetPrivateDataDir() +
                                      wxFileName::GetPathSeparator() +
                                      "navobj.xml.import_backup";
      if (!wxFileExists(deep_backup_filename)) {
        wxCopyFile(xml_filename, deep_backup_filename);
      }
    }

    // Create the new database file navobj.db

    int create_result = sqlite3_open_v2(
        db_filename.ToStdString().c_str(),
        &m_db,  // sqlite3 **ppDb,         /* OUT: SQLite db handle */
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,  // int flags, /* Flags */
        NULL  // char *zVfs        /* Name of VFS module to use */
    );
    if (create_result != SQLITE_OK) {
      wxLogMessage("Cannot create new navobj.db database file");
      m_db = nullptr;
      return;
    }

    // Create initial database tables
    CreateTables(m_db);

    // Save/Close the database
    int close_result = sqlite3_close_v2(m_db);
    if (close_result != SQLITE_OK) {
      return;
    }
  }

  // Open the existing database file
  int m_open_result = sqlite3_open_v2(db_filename.ToStdString().c_str(), &m_db,
                                      SQLITE_OPEN_READWRITE, NULL);
  sqlite3_exec(m_db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);

  //  Add any new tables
  CreateTables(m_db);
  sqlite3_close_v2(m_db);

  m_open_result = sqlite3_open_v2(db_filename.ToStdString().c_str(), &m_db,
                                  SQLITE_OPEN_READWRITE, NULL);
  sqlite3_exec(m_db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);

  // Init class members
  m_importing = false;
}

NavObj_dB::~NavObj_dB() { sqlite3_close_v2(m_db); }

void NavObj_dB::Close() {
  sqlite3_close_v2(m_db);
  m_db = nullptr;
}

bool NavObj_dB::ImportLegacyNavobj(wxFrame* frame) {
  wxString navobj_filename = g_BasePlatform->GetPrivateDataDir() +
                             wxFileName::GetPathSeparator() + "navobj.xml";
  bool rv = false;
  if (::wxFileExists(navobj_filename)) {
    m_importing = true;
    CountImportNavObjects();
    m_pImportProgress = new wxProgressDialog(_("Importing Navobj database"), "",
                                             m_nImportObjects, frame);
    m_import_progesscount = 0;

    rv = ImportLegacyPoints();
    rv |= ImportLegacyRoutes();
    rv |= ImportLegacyTracks();
    m_importing = false;
    m_pImportProgress->Destroy();
  }

  // Delete the imported navobj.xml
  if (::wxFileExists(navobj_filename)) ::wxRemoveFile(navobj_filename);

  return rv;
}

void NavObj_dB::CountImportNavObjects() {
  m_nImportObjects = 0;
  m_nimportPoints = 0;
  m_nimportRoutes = 0;
  m_nimportTracks = 0;

  auto input_set = new NavObjectCollection1();
  wxString navobj_filename = g_BasePlatform->GetPrivateDataDir() +
                             wxFileName::GetPathSeparator() + "navobj.xml";

  if (::wxFileExists(navobj_filename) &&
      input_set->load_file(navobj_filename.ToStdString().c_str()).status ==
          pugi::xml_parse_status::status_ok) {
    input_set->LoadAllGPXPointObjects();
    auto pointlist = pWayPointMan->GetWaypointList();
    for (RoutePoint* point : *pointlist) {
      if (point->m_bIsolatedMark) {
        m_nImportObjects++;
        m_nimportPoints++;
      }
    }

    input_set->LoadAllGPXRouteObjects();
    for (Route* route_import : *pRouteList) {
      m_nImportObjects++;
      m_nimportRoutes++;
      m_nImportObjects += route_import->GetnPoints();
    }

    input_set->LoadAllGPXTrackObjects();
    m_nImportObjects += g_TrackList.size();
    m_nimportTracks = g_TrackList.size();

    for (Track* track_import : g_TrackList) {
      m_nImportObjects += track_import->GetnPoints();
    }
  }
  delete input_set;
}

bool NavObj_dB::ImportLegacyTracks() {
  std::vector<Track*> tracks_added;
  //  Add all tracks to database
  int ntrack = 0;
  for (Track* track_import : g_TrackList) {
    if (InsertTrack(track_import)) {
      tracks_added.push_back(track_import);
    }
    ntrack++;
    m_import_progesscount += track_import->GetnPoints() + 1;
    wxString msg = wxString::Format("Tracks  %d/%d", ntrack, m_nimportTracks);
    m_pImportProgress->Update(m_import_progesscount, msg);
    m_pImportProgress->Show();
  }

  //  Delete all tracks that were successfully added
  for (Track* ptrack : tracks_added) {
    if (ptrack->m_bIsInLayer) continue;
    g_pRouteMan->DeleteTrack(ptrack);
  }

  return true;
}

bool NavObj_dB::ImportLegacyRoutes() {
  std::vector<Route*> routes_added;
  //  Add all routes to database
  int nroute = 0;
  for (Route* route_import : *pRouteList) {
    if (InsertRoute(route_import)) {
      routes_added.push_back(route_import);
    }
    nroute++;
    m_import_progesscount += route_import->GetnPoints() + 1;
    wxString msg = wxString::Format("Routes  %d/%d", nroute, m_nimportRoutes);
    m_pImportProgress->Update(m_import_progesscount, msg);
    m_pImportProgress->Show();
  }

  //  Delete all routes that were successfully added
  for (Route* route : routes_added) {
    g_pRouteMan->DeleteRoute(route);
  }

  //  There may have been some points left as isolated orphans
  //  Delete them too.
  pWayPointMan->DeleteAllWaypoints(true);

  return true;
}

bool NavObj_dB::ImportLegacyPoints() {
  std::vector<RoutePoint*> points_added;
  //  Add all isolated points to database
  int npoint = 0;
  int nmod = 1;
  if (m_nimportPoints > 1000) nmod = 10;
  if (m_nimportPoints > 10000) nmod = 100;

  for (RoutePoint* point : *pWayPointMan->GetWaypointList()) {
    if (point->m_bIsolatedMark) {
      if (InsertRoutePointDB(m_db, point)) {
        points_added.push_back(point);
      }

      UpdateDBRoutePointAttributes(point);
      m_import_progesscount += 1;
      if ((npoint % nmod) == 0) {
        wxString msg =
            wxString::Format("Points  %d/%d", npoint, m_nimportPoints);
        m_pImportProgress->Update(m_import_progesscount, msg);
        m_pImportProgress->Show();
      }
      npoint++;
    }
  }

  //  Delete all points that were successfully added
  for (RoutePoint* point : points_added) {
    pWayPointMan->RemoveRoutePoint(point);
    delete point;
  }

  return true;
}

void NavObj_dB::LoadNavObjects() {
  LoadAllPoints();
  LoadAllRoutes();
  LoadAllTracks();
}

bool NavObj_dB::InsertTrack(Track* track) {
  if (TrackExists(m_db, track->m_GUID.ToStdString())) return false;

  bool rv = false;
  char* errMsg = 0;
  sqlite3_exec(m_db, "BEGIN TRANSACTION", 0, 0, &errMsg);
  if (errMsg) {
    ReportError("InsertTrack:BEGIN TRANSACTION");
    return false;
  }

  // Insert a new track
  wxString sql = wxString::Format("INSERT INTO tracks (guid) VALUES ('%s')",
                                  track->m_GUID.ToStdString().c_str());
  if (!executeSQL(m_db, sql)) {
    sqlite3_exec(m_db, "COMMIT", 0, 0, &errMsg);
    return false;
  }

  UpdateDBTrackAttributes(track);

  //  Add any existing trkpoints
  for (int i = 0; i < track->GetnPoints(); i++) {
    auto point = track->GetPoint(i);
    //  Add the bare trkpoint
    InsertTrackPoint(m_db, track->m_GUID.ToStdString(), point->m_lat,
                     point->m_lon, point->GetTimeString(), i);
  }

  //  Add HTML links to track
  int NbrOfLinks = track->m_TrackHyperlinkList->size();
  if (NbrOfLinks > 0) {
    auto& list = track->m_TrackHyperlinkList;
    for (auto it = list->begin(); it != list->end(); ++it) {
      Hyperlink* link = *it;
      if (!TrackHtmlLinkExists(m_db, link->GUID)) {
        InsertTrackHTML(m_db, track->m_GUID.ToStdString(), link->GUID,
                        link->DescrText.ToStdString(), link->Link.ToStdString(),
                        link->LType.ToStdString());
      }
    }
  }
  sqlite3_exec(m_db, "COMMIT", 0, 0, &errMsg);
  rv = true;
  if (errMsg) rv = false;

  return rv;
};

bool NavObj_dB::UpdateTrack(Track* track) {
  bool rv = false;
  char* errMsg = 0;

  if (!TrackExists(m_db, track->m_GUID.ToStdString())) return false;

  sqlite3_exec(m_db, "BEGIN TRANSACTION", 0, 0, &errMsg);
  if (errMsg) {
    ReportError("UpdateTrack:BEGIN TRANSACTION");
    return false;
  }

  UpdateDBTrackAttributes(track);

  // Delete and re-add track points
  const char* sql = "DELETE FROM trk_points WHERE track_guid = ?";
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, track->m_GUID.ToStdString().c_str(), -1,
                      SQLITE_TRANSIENT);
  } else {
    ReportError("UpdateTrack:prepare");
    sqlite3_exec(m_db, "COMMIT", 0, 0, &errMsg);
    return false;
  }
  if (sqlite3_step(stmt) != SQLITE_DONE) {
    ReportError("UpdateTrack:step");
    sqlite3_finalize(stmt);
    sqlite3_exec(m_db, "COMMIT", 0, 0, &errMsg);
    return false;
  }
  sqlite3_finalize(stmt);

  // re-add trackpoints
  for (int i = 0; i < track->GetnPoints(); i++) {
    auto point = track->GetPoint(i);
    //  Add the bare point
    if (point) {
      InsertTrackPoint(m_db, track->m_GUID.ToStdString(), point->m_lat,
                       point->m_lon, point->GetTimeString(), i);
    }
  }

  sqlite3_exec(m_db, "COMMIT", 0, 0, nullptr);

  rv = true;
  if (errMsg) rv = false;
  return rv;
};

bool NavObj_dB::UpdateDBTrackAttributes(Track* track) {
  const char* sql =
      "UPDATE tracks SET "
      "name = ?, "
      "description = ?, "
      "visibility = ?, "
      "start_string = ?, "
      "end_string = ?, "
      "width = ?, "
      "style = ?, "
      "color = ? "
      "WHERE guid = ?";

  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, track->GetName().ToStdString().c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, track->m_TrackDescription.ToStdString().c_str(),
                      -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, track->m_bVisible);
    sqlite3_bind_text(stmt, 4, track->m_TrackStartString.ToStdString().c_str(),
                      -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, track->m_TrackEndString.ToStdString().c_str(),
                      -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 6, track->m_width);
    sqlite3_bind_int(stmt, 7,
                     (int)(track->m_style));  // track->m_style.c_str(),
    sqlite3_bind_text(stmt, 8, track->m_Colour.ToStdString().c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 9, track->m_GUID.c_str(), track->m_GUID.size(),
                      SQLITE_TRANSIENT);
  } else {
    return false;
  }

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    ReportError("UpdateDBTrackAttributesA:step");
    sqlite3_finalize(stmt);
    return false;
  }

  sqlite3_finalize(stmt);

  // Update the HTML links
  // The list of links is freshly rebuilt when this method is called
  // So start by deleting all existing bcomments
  DeleteAllCommentsForTrack(m_db, track->m_GUID.ToStdString());

  // Now add all the links to db
  int NbrOfLinks = track->m_TrackHyperlinkList->size();
  if (NbrOfLinks > 0) {
    auto& list = track->m_TrackHyperlinkList;
    for (auto it = list->begin(); it != list->end(); ++it) {
      Hyperlink* link = *it;

      if (!TrackHtmlLinkExists(m_db, link->GUID)) {
        InsertTrackHTML(m_db, track->m_GUID.ToStdString(), link->GUID,
                        link->DescrText.ToStdString(), link->Link.ToStdString(),
                        link->LType.ToStdString());
      } else {
        const char* sql =
            "UPDATE track_html_links SET "
            "html_link = ?, "
            "html_description = ?, "
            "html_type = ? "
            "WHERE guid = ?";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
          sqlite3_bind_text(stmt, 3, link->Link.ToStdString().c_str(), -1,
                            SQLITE_TRANSIENT);
          sqlite3_bind_text(stmt, 4, link->DescrText.ToStdString().c_str(), -1,
                            SQLITE_TRANSIENT);
          sqlite3_bind_text(stmt, 5, link->LType.ToStdString().c_str(), -1,
                            SQLITE_TRANSIENT);
        }
        if (sqlite3_step(stmt) != SQLITE_DONE) {
          ReportError("UpdateDBTRackAttributesB:step");
          sqlite3_finalize(stmt);
          return false;
        }
        sqlite3_finalize(stmt);
      }
    }
  }

  return true;
}

bool NavObj_dB::AddTrackPoint(Track* track, TrackPoint* point) {
  //  If track does not yet exist in dB, return
  if (!TrackExists(m_db, track->m_GUID.ToStdString())) return false;

  // Get next point order
  int this_point_index = track->GetnPoints();

  // Add the linked point to the dB
  if (!InsertTrackPoint(m_db, track->m_GUID.ToStdString(), point->m_lat,
                        point->m_lon, point->GetTimeString(),
                        this_point_index - 1))
    return false;

  return true;
}

bool NavObj_dB::LoadAllTracks() {
  const char* sql = R"(
        SELECT guid, name,
        description, visibility, start_string, end_string,
        width, style, color,
        created_at
        FROM tracks
        ORDER BY created_at ASC
    )";

  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    return false;
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    std::string guid =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    std::string name =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    std::string description =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    int visibility = sqlite3_column_int(stmt, 3);
    std::string start_string =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    std::string end_string =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
    int width = sqlite3_column_int(stmt, 6);
    int style = sqlite3_column_int(stmt, 7);
    std::string color =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
    std::string created =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));

    Track* new_trk = NULL;

    //  Add the trk_points
    const char* sql = R"(
        SELECT  latitude, longitude, timestamp, point_order
        FROM trk_points
        WHERE track_guid = ?
        ORDER BY point_order ASC
    )";

    sqlite3_stmt* stmtp;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmtp, nullptr) != SQLITE_OK) {
      return false;
    }

    sqlite3_bind_text(stmtp, 1, guid.c_str(), -1, SQLITE_TRANSIENT);

    int GPXTrkSeg = 1;
    while (sqlite3_step(stmtp) == SQLITE_ROW) {
      if (!new_trk) {
        new_trk = new Track;
        new_trk->m_GUID = guid;

        // Set all the track attributes
        new_trk->SetVisible(visibility == 1);
        new_trk->SetName(name.c_str());
        new_trk->m_TrackStartString = start_string.c_str();
        new_trk->m_TrackEndString = end_string.c_str();
        new_trk->m_width = width;
        new_trk->m_style = (wxPenStyle)style;
        new_trk->m_Colour = color;
      }

      double latitude = sqlite3_column_double(stmtp, 0);
      double longitude = sqlite3_column_double(stmtp, 1);
      std::string timestamp =
          reinterpret_cast<const char*>(sqlite3_column_text(stmtp, 2));
      int point_order = sqlite3_column_int(stmtp, 3);

      auto point = new TrackPoint(latitude, longitude, timestamp);

      point->m_GPXTrkSegNo = GPXTrkSeg;
      new_trk->AddPoint(point);
    }
    sqlite3_finalize(stmtp);

    if (new_trk) {
      new_trk->SetCurrentTrackSeg(GPXTrkSeg);

      //    Add the HTML links
      const char* sqlh = R"(
        SELECT guid, html_link, html_description, html_type
        FROM track_html_links
        WHERE track_guid = ?
        ORDER BY html_type ASC
    )";

      sqlite3_stmt* stmt;

      if (sqlite3_prepare_v2(m_db, sqlh, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, new_trk->m_GUID.ToStdString().c_str(), -1,
                          SQLITE_TRANSIENT);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
          std::string link_guid =
              reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
          std::string link_link =
              reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
          std::string link_description =
              reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
          std::string link_type =
              reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

          Hyperlink* h = new Hyperlink();
          h->DescrText = link_description;
          h->Link = link_link;
          h->LType = link_type;

          new_trk->m_TrackHyperlinkList->push_back(h);
          int yyp = 4;
        }

        sqlite3_finalize(stmt);

      } else {
        return false;
      }

      //  Insert the track into the global list
      g_TrackList.push_back(new_trk);
      //    Add the selectable points and segments of the track
      pSelect->AddAllSelectableTrackSegments(new_trk);
    }
  }
  return true;
}

bool NavObj_dB::DeleteTrack(Track* track) {
  if (!track) return false;
  std::string track_guid = track->m_GUID.ToStdString();
  const char* sql = "DELETE FROM tracks WHERE guid = ?";
  sqlite3_stmt* stmt;

  if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, track_guid.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      ReportError("DeleteTrack:step");
      sqlite3_finalize(stmt);
      return false;
    }

    sqlite3_finalize(stmt);
  } else {
    return false;
  }
  return true;
}

//  Route support

bool NavObj_dB::InsertRoute(Route* route) {
  bool rv = false;
  char* errMsg = 0;

  if (!RouteExistsDB(m_db, route->m_GUID.ToStdString())) {
    // Insert a new route
    wxString sql = wxString::Format("INSERT INTO routes (guid) VALUES ('%s')",
                                    route->m_GUID.ToStdString().c_str());
    if (!executeSQL(m_db, sql)) {
      return false;
    }
    UpdateDBRouteAttributes(route);
  }

  sqlite3_exec(m_db, "BEGIN TRANSACTION", 0, 0, &errMsg);
  if (errMsg) {
    ReportError("InsertRoute:BEGIN TRANSACTION");
    return false;
  }

  // insert routepoints
  for (int i = 0; i < route->GetnPoints(); i++) {
    auto point = route->GetPoint(i + 1);
    //  Add the bare point
    if (point) {
      if (!RoutePointExists(m_db, point->m_GUID.ToStdString())) {
        InsertRoutePointDB(m_db, point);
        UpdateDBRoutePointAttributes(point);
      }
    }
  }

  // insert linkages
  for (int i = 0; i < route->GetnPoints(); i++) {
    auto point = route->GetPoint(i + 1);
    //  Add the bare point
    if (point) {
      InsertRoutePointLink(m_db, route, point, i + 1);
    }
  }

  //  Add HTML links to route
  int NbrOfLinks = route->m_HyperlinkList->size();
  if (NbrOfLinks > 0) {
    auto& list = *route->m_HyperlinkList;
    for (auto it = list.begin(); it != list.end(); ++it) {
      Hyperlink* link = *it;
      if (!RouteHtmlLinkExists(m_db, link->GUID)) {
        InsertRouteHTML(m_db, route->m_GUID.ToStdString(), link->GUID,
                        link->DescrText.ToStdString(), link->Link.ToStdString(),
                        link->LType.ToStdString());
      }
    }
  }

  sqlite3_exec(m_db, "COMMIT", 0, 0, &errMsg);
  rv = true;
  if (errMsg) {
    ReportError("InsertRoute:commit");
    rv = false;
  }
  return rv;
};

bool NavObj_dB::UpdateRoute(Route* route) {
  bool rv = false;
  char* errMsg = 0;

  if (!RouteExistsDB(m_db, route->m_GUID.ToStdString())) return false;

  sqlite3_exec(m_db, "BEGIN TRANSACTION", 0, 0, &errMsg);
  if (errMsg) {
    ReportError("UpdateRoute:BEGIN TRANSACTION");
    return false;
  }

  UpdateDBRouteAttributes(route);

  // update routepoints
  for (int i = 0; i < route->GetnPoints(); i++) {
    auto point = route->GetPoint(i + 1);
    //  Add the bare point
    if (point) {
      if (!RoutePointExists(m_db, point->m_GUID.ToStdString())) {
        InsertRoutePointDB(m_db, point);
      }
      UpdateDBRoutePointAttributes(point);
    }
  }

  // Delete and re-add point linkages
  const char* sql = "DELETE FROM routepoints_link WHERE route_guid = ?";
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, route->m_GUID.ToStdString().c_str(), -1,
                      SQLITE_TRANSIENT);
  } else {
    sqlite3_exec(m_db, "COMMIT", 0, 0, &errMsg);
    return false;
  }
  if (sqlite3_step(stmt) != SQLITE_DONE) {
    ReportError("UpdateRoute:step");
    sqlite3_finalize(stmt);
    sqlite3_exec(m_db, "COMMIT", 0, 0, &errMsg);
    return false;
  }

  sqlite3_finalize(stmt);

  for (int i = 0; i < route->GetnPoints(); i++) {
    auto point = route->GetPoint(i + 1);
    if (point) {
      InsertRoutePointLink(m_db, route, point, i + 1);
    }
  }

  //  Add HTML links to route
  int NbrOfLinks = route->m_HyperlinkList->size();
  if (NbrOfLinks > 0) {
    auto& list = *route->m_HyperlinkList;
    for (auto it = list.begin(); it != list.end(); ++it) {
      Hyperlink* link = *it;
      if (!RouteHtmlLinkExists(m_db, link->GUID)) {
        InsertRouteHTML(m_db, route->m_GUID.ToStdString(), link->GUID,
                        link->DescrText.ToStdString(), link->Link.ToStdString(),
                        link->LType.ToStdString());
      }
    }
  }
  sqlite3_exec(m_db, "COMMIT", 0, 0, nullptr);

  rv = true;
  if (errMsg) rv = false;

  return rv;
};

bool NavObj_dB::UpdateRouteViz(Route* route) {
  bool rv = false;
  char* errMsg = 0;
  if (!RouteExistsDB(m_db, route->m_GUID.ToStdString())) return false;

  UpdateDBRouteAttributes(route);
  // update routepoints visibility
  for (int i = 0; i < route->GetnPoints(); i++) {
    auto point = route->GetPoint(i + 1);
    //  Add the bare point
    if (point) {
      UpdateDBRoutePointViz(point);
    }
  }
  rv = true;
  if (errMsg) rv = false;

  return rv;
};

bool NavObj_dB::UpdateDBRouteAttributes(Route* route) {
  const char* sql =
      "UPDATE routes SET "
      "name = ?, "
      "description = ?, "
      "start_string = ?, "
      "end_string = ?, "
      "visibility = ?, "
      "shared_wp_viz = ?, "
      "planned_departure = ?, "
      "plan_speed = ?, "
      "time_format = ?, "
      "width = ?, "
      "style = ?, "
      "color = ? "
      "WHERE guid = ?";

  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, route->GetName().ToStdString().c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, route->m_RouteDescription.ToStdString().c_str(),
                      -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, route->m_RouteStartString.ToStdString().c_str(),
                      -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, route->m_RouteEndString.ToStdString().c_str(),
                      -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, route->IsVisible());
    sqlite3_bind_int(stmt, 6, route->GetSharedWPViz());
    if (route->m_PlannedDeparture.IsValid())
      sqlite3_bind_int(stmt, 7, route->m_PlannedDeparture.GetTicks());
    sqlite3_bind_double(stmt, 8, route->m_PlannedSpeed);
    sqlite3_bind_text(stmt, 9, route->m_TimeDisplayFormat.ToStdString().c_str(),
                      -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 10, route->m_width);
    sqlite3_bind_int(stmt, 11,
                     (int)(route->m_style));  // track->m_style.c_str(),
    sqlite3_bind_text(stmt, 12, route->m_Colour.ToStdString().c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 13, route->m_GUID.c_str(), route->m_GUID.size(),
                      SQLITE_TRANSIENT);
  } else {
    return false;
  }

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    ReportError("UpdateDBRouteAttributesA:step");
    sqlite3_finalize(stmt);
    return false;
  }

  sqlite3_finalize(stmt);

  // Update the HTML links
  // The list of links is freshly rebuilt when this method is called
  // So start by deleting all existing bcomments
  DeleteAllCommentsForRoute(m_db, route->m_GUID.ToStdString());

  // Now add all the links to db
  int NbrOfLinks = route->m_HyperlinkList->size();
  if (NbrOfLinks > 0) {
    auto& list = route->m_HyperlinkList;
    for (auto it = list->begin(); it != list->end(); ++it) {
      Hyperlink* link = *it;
      if (!RouteHtmlLinkExists(m_db, link->GUID)) {
        InsertRouteHTML(m_db, route->m_GUID.ToStdString(), link->GUID,
                        link->DescrText.ToStdString(), link->Link.ToStdString(),
                        link->LType.ToStdString());
      } else {
        const char* sql =
            "UPDATE route_html_links SET "
            "html_link = ?, "
            "html_description = ?, "
            "html_type = ? "
            "WHERE guid = ?";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
          sqlite3_bind_text(stmt, 3, link->Link.ToStdString().c_str(), -1,
                            SQLITE_TRANSIENT);
          sqlite3_bind_text(stmt, 4, link->DescrText.ToStdString().c_str(), -1,
                            SQLITE_TRANSIENT);
          sqlite3_bind_text(stmt, 5, link->LType.ToStdString().c_str(), -1,
                            SQLITE_TRANSIENT);
        }
        if (sqlite3_step(stmt) != SQLITE_DONE) {
          return false;
        }
        if (sqlite3_step(stmt) != SQLITE_DONE) {
          ReportError("UpdateDBRouteAttributesB:step");
          sqlite3_finalize(stmt);
          return false;
        }
        sqlite3_finalize(stmt);
      }
    }
  }
  return true;
}

bool NavObj_dB::UpdateDBRoutePointAttributes(RoutePoint* point) {
  const char* sql =
      "UPDATE routepoints SET "
      "lat = ?, "
      "lon = ?, "
      "Symbol = ?, "
      "Name = ?, "
      "description = ?, "
      "TideStation = ?, "
      "plan_speed = ?, "
      "etd = ?, "
      "Type = ?, "
      "Time = ?, "
      "ArrivalRadius = ?, "
      "RangeRingsNumber = ?, "
      "RangeRingsStep = ?, "
      "RangeRingsStepUnits = ?, "
      "RangeRingsVisible = ?, "
      "RangeRingsColour = ?, "
      "ScaleMin = ?, "
      "ScaleMax = ?, "
      "UseScale = ?, "
      "visibility = ?, "
      "viz_name = ?, "
      "shared = ?, "
      "isolated = ?, "
      "linked_layer_guid = ? "
      "WHERE guid = ?";

  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_double(stmt, 1, point->GetLatitude());
    sqlite3_bind_double(stmt, 2, point->GetLongitude());
    sqlite3_bind_text(stmt, 3, point->GetIconName().ToStdString().c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, point->GetName().ToStdString().c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, point->GetDescription().ToStdString().c_str(),
                      -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, point->m_TideStation.ToStdString().c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 7, point->GetPlannedSpeed());
    time_t etd = -1;
    if (point->GetManualETD().IsValid()) etd = point->GetManualETD().GetTicks();
    sqlite3_bind_int(stmt, 8, etd);
    sqlite3_bind_text(stmt, 9, "type", -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 10, point->m_timestring.ToStdString().c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 11, point->m_WaypointArrivalRadius);

    sqlite3_bind_int(stmt, 12, point->m_iWaypointRangeRingsNumber);
    sqlite3_bind_double(stmt, 13, point->m_fWaypointRangeRingsStep);
    sqlite3_bind_int(stmt, 14, point->m_iWaypointRangeRingsStepUnits);
    sqlite3_bind_int(stmt, 15, point->m_bShowWaypointRangeRings);
    sqlite3_bind_text(
        stmt, 16,
        point->m_wxcWaypointRangeRingsColour.GetAsString(wxC2S_HTML_SYNTAX)
            .ToStdString()
            .c_str(),
        -1, SQLITE_TRANSIENT);

    sqlite3_bind_int(stmt, 17, point->GetScaMin());
    sqlite3_bind_int(stmt, 18, point->GetScaMax());
    sqlite3_bind_int(stmt, 19, point->GetUseSca());

    sqlite3_bind_int(stmt, 20, point->IsVisible());
    sqlite3_bind_int(stmt, 21, point->IsNameShown());
    sqlite3_bind_int(stmt, 22, point->IsShared());
    int iso = point->m_bIsolatedMark;
    sqlite3_bind_int(stmt, 23, iso);  // point->m_bIsolatedMark);

    sqlite3_bind_text(stmt, 24,
                      point->m_LinkedLayerGUID.ToStdString().c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 25, point->m_GUID.ToStdString().c_str(), -1,
                      SQLITE_TRANSIENT);

  } else {
    return false;
  }

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    ReportError("UpdateDBRoutePointAttributesA:step");
    sqlite3_finalize(stmt);
    return false;
  }

  sqlite3_finalize(stmt);

  // Update the HTML links
  // The list of links is freshly rebuilt when this method is called
  // So start by deleting all existing bcomments
  DeleteAllCommentsForRoutePoint(m_db, point->m_GUID.ToStdString());

  // Now add all the links to db
  int NbrOfLinks = point->m_HyperlinkList->size();
  if (NbrOfLinks > 0) {
    auto& list = point->m_HyperlinkList;
    for (auto it = list->begin(); it != list->end(); ++it) {
      Hyperlink* link = *it;
      if (!RoutePointHtmlLinkExists(m_db, link->GUID)) {
        InsertRoutePointHTML(m_db, point->m_GUID.ToStdString(), link->GUID,
                             link->DescrText.ToStdString(),
                             link->Link.ToStdString(),
                             link->LType.ToStdString());
      } else {
        const char* sql =
            "UPDATE routepoint_html_links SET "
            "html_link = ?, "
            "html_description = ?, "
            "html_type = ? "
            "WHERE guid = ?";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
          sqlite3_bind_text(stmt, 3, link->Link.ToStdString().c_str(), -1,
                            SQLITE_TRANSIENT);
          sqlite3_bind_text(stmt, 4, link->DescrText.ToStdString().c_str(), -1,
                            SQLITE_TRANSIENT);
          sqlite3_bind_text(stmt, 5, link->LType.ToStdString().c_str(), -1,
                            SQLITE_TRANSIENT);
        }
        if (sqlite3_step(stmt) != SQLITE_DONE) {
          return false;
        }
        if (sqlite3_step(stmt) != SQLITE_DONE) {
          ReportError("UpdateDBRoutePointAttributesB:step-h");
          sqlite3_finalize(stmt);
          return false;
        }
        sqlite3_finalize(stmt);
      }
    }
  }

  return true;
}

bool NavObj_dB::UpdateDBRoutePointViz(RoutePoint* point) {
  const char* sql =
      "UPDATE routepoints SET "
      "visibility = ? "
      "WHERE guid = ?";

  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_int(stmt, 1, point->IsVisible());
    sqlite3_bind_text(stmt, 2, point->m_GUID.ToStdString().c_str(), -1,
                      SQLITE_TRANSIENT);

  } else {
    return false;
  }

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    ReportError("UpdateDBRoutePointVizA:step");
    sqlite3_finalize(stmt);
    return false;
  }

  sqlite3_finalize(stmt);

  return true;
}

bool NavObj_dB::DeleteRoute(Route* route) {
  if (m_importing) return false;
  if (!route) return false;
  std::string route_guid = route->m_GUID.ToStdString();
  const char* sql = "DELETE FROM routes WHERE guid = ?";
  sqlite3_stmt* stmt;

  if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, route_guid.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      ReportError("DeleteRoute:step");
      sqlite3_finalize(stmt);
      return false;
    }
    sqlite3_finalize(stmt);
  } else {
    return false;
  }
  return true;
}

bool NavObj_dB::LoadAllRoutes() {
  const char* sql =
      "SELECT "
      "guid, "
      "name, "
      "description, "
      "start_string, "
      "end_string, "
      "visibility, "
      "shared_wp_viz, "
      "planned_departure, "
      "plan_speed, "
      "time_format, "
      "width, "
      "style, "
      "color "
      "FROM routes "
      "ORDER BY created_at ASC";

  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    return false;
  }

  int errcode0 = SQLITE_OK;
  while ((errcode0 = sqlite3_step(stmt)) == SQLITE_ROW) {
    std::string guid =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    std::string name =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    std::string description =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    std::string start_string =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    std::string end_string =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    int visibility = sqlite3_column_int(stmt, 5);
    int sharewp_viz = sqlite3_column_int(stmt, 6);
    time_t planned_departure_ticks = sqlite3_column_int(stmt, 7);
    double plan_speed = sqlite3_column_double(stmt, 8);
    std::string time_format =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));

    int width = sqlite3_column_int(stmt, 10);
    int style = sqlite3_column_int(stmt, 11);
    std::string color =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));

    Route* route = NULL;
    std::map<std::string, RoutePoint*> linked_clones;
    const auto resolve_linked_clone =
        [&linked_clones](const std::string& guid) -> RoutePoint* {
      auto it = linked_clones.find(guid);
      if (it != linked_clones.end()) return it->second;
      return NULL;
    };

    //  Add the route_points
    const char* sql = R"(
        SELECT  latitude, longitude, timestamp, point_order
        FROM trk_points
        WHERE track_guid = ?
        ORDER BY point_order ASC
    )";

    const char* sqlp =
        "SELECT p.guid, "
        "p.lat, "
        "p.lon, "
        "p.Symbol, "
        "p.Name, "
        "p.description, "
        "p.TideStation, "
        "p.plan_speed, "
        "p.etd, "
        "p.Type, "
        "p.Time, "
        "p.ArrivalRadius, "
        "p.RangeRingsNumber, "
        "p.RangeRingsStep, "
        "p.RangeRingsStepUnits, "
        "p.RangeRingsVisible, "
        "p.RangeRingsColour, "
        "p.ScaleMin, "
        "p.ScaleMax, "
        "p.UseScale, "
        "p.visibility, "
        "p.viz_name, "
        "p.shared, "
        "p.isolated, "
        "p.linked_layer_guid, "
        "p.created_at "
        "FROM routepoints_link tp "
        "JOIN routepoints p ON p.guid = tp.point_guid "
        "WHERE tp.route_guid = ? "
        "ORDER BY tp.point_order ASC";

    sqlite3_stmt* stmtp;
    if (sqlite3_prepare_v2(m_db, sqlp, -1, &stmtp, nullptr) != SQLITE_OK) {
      ReportError("LoadAllRoutes-B:prepare");
      return false;
    }

    sqlite3_bind_text(stmtp, 1, guid.c_str(), -1, SQLITE_TRANSIENT);

    int GPXSeg = 0;
    int errcode = SQLITE_OK;
    while ((errcode = sqlite3_step(stmtp)) == SQLITE_ROW) {
      if (!route) {
        route = new Route;
        route->m_GUID = guid;

        // Set all the route attributes
        route->SetVisible(visibility == 1);
        route->m_RouteNameString = name.c_str();
        route->m_RouteDescription = description.c_str();
        route->m_RouteStartString = start_string.c_str();
        route->m_RouteEndString = end_string.c_str();
        route->SetVisible(visibility == 1);
        route->SetSharedWPViz(sharewp_viz == 1);
        route->m_PlannedDeparture.Set((time_t)planned_departure_ticks);
        route->m_PlannedSpeed = plan_speed;
        route->m_TimeDisplayFormat = time_format.c_str();

        route->m_width = width;
        route->m_style = (wxPenStyle)style;
        route->m_Colour = color;
      }

      // Grab all the point attributes from the SELECT statement
      int col = 0;
      std::string point_guid =
          reinterpret_cast<const char*>(sqlite3_column_text(stmtp, col++));
      double latitude = sqlite3_column_double(stmtp, col++);
      double longitude = sqlite3_column_double(stmtp, col++);
      std::string symbol =
          reinterpret_cast<const char*>(sqlite3_column_text(stmtp, col++));
      std::string name =
          reinterpret_cast<const char*>(sqlite3_column_text(stmtp, col++));
      std::string description =
          reinterpret_cast<const char*>(sqlite3_column_text(stmtp, col++));
      std::string tide_station =
          reinterpret_cast<const char*>(sqlite3_column_text(stmtp, col++));
      double plan_speed = sqlite3_column_double(stmtp, col++);
      time_t etd_epoch = sqlite3_column_int(stmtp, col++);
      std::string type =
          reinterpret_cast<const char*>(sqlite3_column_text(stmtp, col++));
      std::string time =
          reinterpret_cast<const char*>(sqlite3_column_text(stmtp, col++));
      double arrival_radius = sqlite3_column_double(stmtp, col++);

      int range_ring_number = sqlite3_column_int(stmtp, col++);
      double range_ring_step = sqlite3_column_double(stmtp, col++);
      int range_ring_units = sqlite3_column_int(stmtp, col++);
      int range_ring_visible = sqlite3_column_int(stmtp, col++);
      std::string range_ring_color =
          reinterpret_cast<const char*>(sqlite3_column_text(stmtp, col++));

      int scamin = sqlite3_column_int(stmtp, col++);
      int scamax = sqlite3_column_int(stmtp, col++);
      int use_scaminmax = sqlite3_column_int(stmtp, col++);

      int visibility = sqlite3_column_int(stmtp, col++);
      int viz_name = sqlite3_column_int(stmtp, col++);
      int shared = sqlite3_column_int(stmtp, col++);
      int isolated = sqlite3_column_int(stmtp, col++);
      const unsigned char* linked_guid_text = sqlite3_column_text(stmtp, col++);
      std::string linked_layer_guid;
      if (linked_guid_text)
        linked_layer_guid =
            reinterpret_cast<const char*>(linked_guid_text);
      std::string point_created_at =
          reinterpret_cast<const char*>(sqlite3_column_text(stmtp, col++));

      RoutePoint* point;
      // RoutePoint exists already, in another route or isolated??
      RoutePoint* existing_point = NULL;
      auto containing_route =
          g_pRouteMan->FindRouteContainingWaypoint(point_guid);

      if (containing_route) {  // In a route already?
        existing_point = containing_route->GetPoint(point_guid);
      }
      // Or isolated?
      if (!existing_point) {
        existing_point = pWayPointMan->FindRoutePointByGUID(point_guid.c_str());
      }
      if (!existing_point && !linked_layer_guid.empty()) {
        existing_point = resolve_linked_clone(linked_layer_guid);
      }
      if (!existing_point && !linked_layer_guid.empty()) {
        existing_point =
            pWayPointMan->FindRoutePointByGUID(linked_layer_guid);
        if (existing_point &&
            (!existing_point->m_bIsInLayer ||
             !existing_point->m_bLayerGuidIsPersistent)) {
          existing_point = NULL;
        }
      }

      if (existing_point) {
        point = existing_point;
        point->SetShared(true);  // by definition
        point->m_bIsolatedMark = false;
      } else {
        point =
            new RoutePoint(latitude, longitude, symbol, name, point_guid, true);

        point->m_MarkDescription = description;
        point->m_TideStation = tide_station;
        point->SetPlannedSpeed(plan_speed);

        wxDateTime etd;
        etd.Set((time_t)etd_epoch);
        if (etd.IsValid()) point->SetETD(etd);

        point->m_WaypointArrivalRadius = arrival_radius;

        point->m_iWaypointRangeRingsNumber = range_ring_number;
        point->m_fWaypointRangeRingsStep = range_ring_step;
        point->m_iWaypointRangeRingsStepUnits = range_ring_units;
        point->SetShowWaypointRangeRings(range_ring_visible == 1);
        // TODO
        point->m_wxcWaypointRangeRingsColour.Set(range_ring_color);

        point->SetScaMin(scamin);
        point->SetScaMax(scamax);
        point->SetUseSca(use_scaminmax == 1);

        point->SetVisible(visibility == 1);
        point->SetNameShown(viz_name == 1);
        point->SetShared(shared == 1);
        point->m_bIsolatedMark = (isolated == 1);
        if (!linked_layer_guid.empty())
          point->m_LinkedLayerGUID = linked_layer_guid;

        if (point_created_at.size()) {
          // Convert from sqLite default date/time format to wxDateTime
          // sqLite format uses UTC, so conversion to epoch_time is clear.
          std::tm tm = {};
          std::istringstream ss(point_created_at);
          ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
          time_t epoch_time = mktime(&tm);
          point->m_CreateTimeX = epoch_time;
        }

        //    Add the point HTML links
        const char* sqlh = R"(
        SELECT guid, html_link, html_description, html_type
        FROM routepoint_html_links
        WHERE routepoint_guid = ?
        ORDER BY html_type ASC
        )";

        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(m_db, sqlh, -1, &stmt, nullptr) == SQLITE_OK) {
          sqlite3_bind_text(stmt, 1, point->m_GUID.ToStdString().c_str(), -1,
                            SQLITE_TRANSIENT);

          while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string link_guid =
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            std::string link_link =
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            std::string link_description =
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            std::string link_type =
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

            Hyperlink* h = new Hyperlink();
            h->DescrText = link_description;
            h->Link = link_link;
            h->LType = link_type;

            point->m_HyperlinkList->push_back(h);
          }
        }
      }

      if (point->m_bIsInLayer) {
        RoutePoint* cloned =
            new RoutePoint(point->m_lat, point->m_lon, point->GetIconName(),
                           point->GetName(), point_guid, true);
        cloned->m_MarkDescription = point->m_MarkDescription;
        cloned->m_TideStation = point->m_TideStation;
        cloned->SetPlannedSpeed(point->GetPlannedSpeed());
        cloned->SetETD(point->GetETD());

        cloned->m_WaypointArrivalRadius = point->m_WaypointArrivalRadius;
        cloned->m_iWaypointRangeRingsNumber =
            point->m_iWaypointRangeRingsNumber;
        cloned->m_fWaypointRangeRingsStep = point->m_fWaypointRangeRingsStep;
        cloned->m_iWaypointRangeRingsStepUnits =
            point->m_iWaypointRangeRingsStepUnits;
        cloned->SetShowWaypointRangeRings(point->m_bShowWaypointRangeRings);
        cloned->m_wxcWaypointRangeRingsColour =
            point->m_wxcWaypointRangeRingsColour;
        cloned->SetScaMin(point->GetScaMin());
        cloned->SetScaMax(point->GetScaMax());
        cloned->SetUseSca(point->GetUseSca());
        cloned->SetVisible(point->IsVisible());
        cloned->SetNameShown(point->IsNameShown());
        cloned->SetShared(shared == 1);
        cloned->m_bIsolatedMark = (isolated == 1);
        if (!linked_layer_guid.empty()) {
          cloned->m_LinkedLayerGUID = linked_layer_guid;
        } else if (point->m_bLayerGuidIsPersistent) {
          cloned->m_LinkedLayerGUID = point->m_GUID;
        }
        point = cloned;
        if (!cloned->m_LinkedLayerGUID.IsEmpty()) {
          linked_clones[cloned->m_LinkedLayerGUID.ToStdString()] = cloned;
        }
      }

      route->AddPoint(point);
    }  // route points
    sqlite3_finalize(stmtp);
    if (errcode != SQLITE_DONE) {
      ReportError("LoadAllRoutes-A:step");
      return false;
    }

    // Add route html links
    if (route) {
      //    Add the HTML links
      const char* sqlh = R"(
        SELECT guid, html_link, html_description, html_type
        FROM route_html_links
        WHERE route_guid = ?
        ORDER BY html_type ASC
    )";

      sqlite3_stmt* stmt;

      if (sqlite3_prepare_v2(m_db, sqlh, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, route->m_GUID.ToStdString().c_str(), -1,
                          SQLITE_TRANSIENT);

        int errcode2 = SQLITE_OK;
        while ((errcode2 = sqlite3_step(stmt)) == SQLITE_ROW) {
          std::string link_guid =
              reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
          std::string link_link =
              reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
          std::string link_description =
              reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
          std::string link_type =
              reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

          Hyperlink* h = new Hyperlink();
          h->DescrText = link_description;
          h->Link = link_link;
          h->LType = link_type;

          route->m_HyperlinkList->push_back(h);
        }
        if (errcode != SQLITE_DONE) {
          ReportError("LoadAllRoutes-B:step");
          return false;
        }

        sqlite3_finalize(stmt);

      } else {
        ReportError("LoadAllRoutes-B:prepare");
        return false;
      }
    }

    //  Insert the route into the global list
    InsertRouteA(route,
                 nullptr);  // NavObjectChanges::getInstance()  //TODO adding
                            // changes will force the xml file to be updated?

  }  // routes
  if (errcode0 != SQLITE_DONE) {
    ReportError("LoadAllRoutes-C:step");
    return false;
  }

  return true;
}

bool NavObj_dB::LoadAllPoints() {
  const char* sqlp =
      "SELECT "
      "p.guid, "
      "p.lat, "
      "p.lon, "
      "p.Symbol, "
      "p.Name, "
      "p.description, "
      "p.TideStation, "
      "p.plan_speed, "
      "p.etd, "
      "p.Type, "
      "p.Time, "
      "p.ArrivalRadius, "
      "p.RangeRingsNumber, "
      "p.RangeRingsStep, "
      "p.RangeRingsStepUnits, "
      "p.RangeRingsVisible, "
      "p.RangeRingsColour, "
      "p.ScaleMin, "
      "p.ScaleMax, "
      "p.UseScale, "
      "p.visibility, "
      "p.viz_name, "
      "p.shared, "
      "p.isolated, "
      "p.linked_layer_guid, "
      "p.created_at "
      "FROM routepoints p ";

  RoutePoint* point = nullptr;

  sqlite3_stmt* stmtp;
  if (sqlite3_prepare_v2(m_db, sqlp, -1, &stmtp, nullptr) != SQLITE_OK) {
    return false;
  }

  while (sqlite3_step(stmtp) == SQLITE_ROW) {
    // Grab all the point attributes from the SELECT statement
    int col = 0;
    std::string point_guid =
        reinterpret_cast<const char*>(sqlite3_column_text(stmtp, col++));
    double latitude = sqlite3_column_double(stmtp, col++);
    double longitude = sqlite3_column_double(stmtp, col++);
    std::string symbol =
        reinterpret_cast<const char*>(sqlite3_column_text(stmtp, col++));
    std::string name =
        reinterpret_cast<const char*>(sqlite3_column_text(stmtp, col++));
    std::string description =
        reinterpret_cast<const char*>(sqlite3_column_text(stmtp, col++));
    std::string tide_station =
        reinterpret_cast<const char*>(sqlite3_column_text(stmtp, col++));
    double plan_speed = sqlite3_column_double(stmtp, col++);
    time_t etd = sqlite3_column_int(stmtp, col++);
    std::string type =
        reinterpret_cast<const char*>(sqlite3_column_text(stmtp, col++));
    std::string point_time_string =
        reinterpret_cast<const char*>(sqlite3_column_text(stmtp, col++));
    double arrival_radius = sqlite3_column_double(stmtp, col++);

    int range_ring_number = sqlite3_column_int(stmtp, col++);
    double range_ring_step = sqlite3_column_double(stmtp, col++);
    int range_ring_units = sqlite3_column_int(stmtp, col++);
    int range_ring_visible = sqlite3_column_int(stmtp, col++);
    std::string range_ring_color =
        reinterpret_cast<const char*>(sqlite3_column_text(stmtp, col++));

    int scamin = sqlite3_column_int(stmtp, col++);
    int scamax = sqlite3_column_int(stmtp, col++);
    int use_scaminmax = sqlite3_column_int(stmtp, col++);

    int visibility = sqlite3_column_int(stmtp, col++);
    int viz_name = sqlite3_column_int(stmtp, col++);
    int shared = sqlite3_column_int(stmtp, col++);
    int isolated = sqlite3_column_int(stmtp, col++);
    const unsigned char* linked_guid_text = sqlite3_column_text(stmtp, col++);
    std::string linked_layer_guid;
    if (linked_guid_text)
      linked_layer_guid =
          reinterpret_cast<const char*>(linked_guid_text);
    std::string point_created_at =
        reinterpret_cast<const char*>(sqlite3_column_text(stmtp, col++));

    if (isolated) {
      point =
          new RoutePoint(latitude, longitude, symbol, name, point_guid, false);

      point->m_MarkDescription = description;
      point->m_TideStation = tide_station;
      point->SetPlannedSpeed(plan_speed);
      point->m_WaypointArrivalRadius = arrival_radius;

      point->m_iWaypointRangeRingsNumber = range_ring_number;
      point->m_fWaypointRangeRingsStep = range_ring_step;
      point->m_iWaypointRangeRingsStepUnits = range_ring_units;
      point->SetShowWaypointRangeRings(range_ring_visible == 1);

      point->m_wxcWaypointRangeRingsColour.Set(range_ring_color);

      point->SetScaMin(scamin);
      point->SetScaMax(scamax);
      point->SetUseSca(use_scaminmax == 1);

      point->SetVisible(visibility == 1);
      point->SetNameShown(viz_name == 1);
      point->SetShared(shared == 1);
      point->m_bIsolatedMark = (isolated == 1);
      if (!linked_layer_guid.empty())
        point->m_LinkedLayerGUID = linked_layer_guid;

      if (point_created_at.size()) {
        // Convert from sqLite default date/time format to wxDateTime
        // sqLite format uses UTC, so conversion to epoch_time is clear.
        std::tm tm = {};
        std::istringstream ss(point_created_at);
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        time_t epoch_time = mktime(&tm);
        point->m_CreateTimeX = epoch_time;
      }

      // Add it here
      pWayPointMan->AddRoutePoint(point);
      pSelect->AddSelectableRoutePoint(point->m_lat, point->m_lon, point);
    }
  }  // points
  sqlite3_finalize(stmtp);

  if (point) {
    //    Add the point HTML links
    const char* sqlh = R"(
        SELECT guid, html_link, html_description, html_type
        FROM routepoint_html_links
        WHERE routepoint_guid = ?
        ORDER BY html_type ASC
    )";

    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(m_db, sqlh, -1, &stmt, nullptr) == SQLITE_OK) {
      sqlite3_bind_text(stmt, 1, point->m_GUID.ToStdString().c_str(), -1,
                        SQLITE_TRANSIENT);

      while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string link_guid =
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string link_link =
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string link_description =
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        std::string link_type =
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        Hyperlink* h = new Hyperlink();
        h->DescrText = link_description;
        h->Link = link_link;
        h->LType = link_type;

        point->m_HyperlinkList->push_back(h);
      }

      sqlite3_finalize(stmt);

    } else {
      return false;
    }
  }
  return true;
}
bool NavObj_dB::InsertRoutePoint(RoutePoint* point) {
  bool rv = false;
  char* errMsg = 0;

  if (!RoutePointExists(m_db, point->m_GUID.ToStdString())) {
    // Insert a new route point
    wxString sql =
        wxString::Format("INSERT INTO routepoints (guid) VALUES ('%s')",
                         point->m_GUID.ToStdString().c_str());
    if (!executeSQL(m_db, sql)) {
      return false;
    }
  }

  UpdateDBRoutePointAttributes(point);

  //  Add HTML links to routepoint
  int NbrOfLinks = point->m_HyperlinkList->size();
  if (NbrOfLinks > 0) {
    auto& list = point->m_HyperlinkList;
    for (auto it = list->begin(); it != list->end(); ++it) {
      Hyperlink* link = *it;
      if (!RoutePointHtmlLinkExists(m_db, link->GUID)) {
        InsertRoutePointHTML(m_db, point->m_GUID.ToStdString(), link->GUID,
                             link->DescrText.ToStdString(),
                             link->Link.ToStdString(),
                             link->LType.ToStdString());
      }
    }
  }

  return true;
}

bool NavObj_dB::DeleteRoutePoint(RoutePoint* point) {
  if (m_importing) return false;
  if (!point) return false;

  std::string point_guid = point->m_GUID.ToStdString();

  // DeleteAllCommentsForRoutePoint(m_db, point_guid);

  const char* sql = "DELETE FROM routepoints WHERE guid = ?";
  sqlite3_stmt* stmt;

  if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, point_guid.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      ReportError("DeleteRoutePoint:step");
      sqlite3_finalize(stmt);
      return false;
    }

    sqlite3_finalize(stmt);
  } else {
    return false;
  }
  return true;
}

bool NavObj_dB::UpdateRoutePoint(RoutePoint* point) {
  if (m_importing) return false;
  if (!RoutePointExists(m_db, point->m_GUID.ToStdString())) return false;
  UpdateDBRoutePointAttributes(point);
  return true;
}

bool NavObj_dB::Backup(wxString fileName) {
  sqlite3_backup* pBackup;
  sqlite3* backupDatabase;

  if (sqlite3_open(fileName.c_str(), &backupDatabase) == SQLITE_OK) {
    pBackup = sqlite3_backup_init(backupDatabase, "main", m_db, "main");
    if (pBackup) {
      int result = sqlite3_backup_step(pBackup, -1);
      if ((result == SQLITE_OK) || (result == SQLITE_DONE)) {
        if (sqlite3_backup_finish(pBackup) == SQLITE_OK) {
          sqlite3_close_v2(backupDatabase);
          return true;
        }
      }
    }
  }
  wxLogMessage("navobj database backup error: %s", sqlite3_errmsg(m_db));
  return false;
}
