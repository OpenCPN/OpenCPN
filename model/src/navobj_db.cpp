/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  NavObj_dB
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
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <wx/dir.h>

#include "model/base_platform.h"
#include "model/navobj_db.h"
#include "model/navutil_base.h"
#include "model/notification.h"
#include "model/notification_manager.h"
#include "wx/filename.h"
#include "model/datetime.h"
#include "model/comm_appmsg_bus.h"

extern BasePlatform* g_BasePlatform;
extern std::shared_ptr<ObservableListener> ack_listener;

static bool executeSQL(sqlite3* db, const char* sql) {
  char* errMsg = nullptr;
  if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
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
            guid TEXT PRIMARY KEY,
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

        )";

  if (!executeSQL(db, create_tables_sql)) return false;

  return true;
}

bool TrackExists(sqlite3* db, const std::string& track_guid) {
  const char* sql = "SELECT 1 FROM tracks WHERE guid = ? LIMIT 1";
  sqlite3_stmt* stmt;
  bool exists = false;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, track_guid.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
      exists = true;  // found a match
    }

    sqlite3_finalize(stmt);
  } else {
    return false;
  }
  return exists;
}

bool TrackHtmlLinkExists(sqlite3* db, const std::string& link_guid) {
  const char* sql = "SELECT 1 FROM track_html_links WHERE guid = ? LIMIT 1";
  sqlite3_stmt* stmt;
  bool exists = false;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, link_guid.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
      exists = true;  // found a match
    }

    sqlite3_finalize(stmt);
  } else {
    return false;
  }
  return exists;
}

bool DeleteAllCommentsForTrack(sqlite3* db, const std::string& track_guid) {
  const char* sql = R"(
        DELETE FROM track_html_links WHERE track_guid = ?
    )";
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, track_guid.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      std::cerr << "Failed to delete comments: " << sqlite3_errmsg(db) << "\n";
    }
    sqlite3_finalize(stmt);
  } else {
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
    sqlite3_step(stmt);
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
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
  } else {
    return false;
  }
  return true;
}

void errorLogCallback(void* pArg, int iErrCode, const char* zMsg) {
  wxString msg =
      wxString::Format("navobj database error. %d: %s", iErrCode, zMsg);
  wxLogMessage(msg);
  auto& noteman = NotificationManager::GetInstance();
  noteman.AddNotification(NotificationSeverity::kCritical, msg.ToStdString());
}

NavObj_dB& NavObj_dB::GetInstance() {
  static NavObj_dB instance;
  return instance;
}

NavObj_dB::NavObj_dB() {
  // Set SQLite per-process config options
  int ie = sqlite3_config(SQLITE_CONFIG_LOG, errorLogCallback, nullptr);

  // Does dB file exist?
  wxString db_filename = g_BasePlatform->GetPrivateDataDir() +
                         wxFileName::GetPathSeparator() + "navobj.db";
  if (!wxFileExists(db_filename)) {
    //  Make a safety backup of current navobj.xml
    wxString noxml_filename = g_BasePlatform->GetPrivateDataDir() +
                              wxFileName::GetPathSeparator() + "navobj.xml";
    if (wxFileExists(noxml_filename)) {
      wxCopyFile(noxml_filename, noxml_filename + ".backup");
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
  ImportLegacyNavobj();
  sqlite3_close_v2(m_db);

  m_open_result = sqlite3_open_v2(db_filename.ToStdString().c_str(), &m_db,
                                  SQLITE_OPEN_READWRITE, NULL);
  sqlite3_exec(m_db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
}

NavObj_dB::~NavObj_dB() { sqlite3_close_v2(m_db); }

void NavObj_dB::Close() {
  sqlite3_close_v2(m_db);
  m_db = nullptr;
}

bool NavObj_dB::ImportLegacyNavobj() {
  bool rv = ImportLegacyTracks();
  return rv;
}

bool NavObj_dB::ImportLegacyTracks() {
  auto input_set = new NavObjectCollection1();
  wxString navobj_filename = g_BasePlatform->GetPrivateDataDir() +
                             wxFileName::GetPathSeparator() + "navobj.xml";

  if (::wxFileExists(navobj_filename) &&
      input_set->load_file(navobj_filename.ToStdString().c_str()).status ==
          pugi::xml_parse_status::status_ok) {
    input_set->LoadAllGPXTrackObjects();
  }

  std::vector<Track*> tracks_added;
  //  Add all tracks to database
  for (Track* track_import : g_TrackList) {
    if (AddNewTrack(track_import)) {
      tracks_added.push_back(track_import);
    }
  }

  //  Delete all tracks that were successfully added
  for (Track* ptrack : tracks_added) {
    if (ptrack->m_bIsInLayer) continue;
    g_pRouteMan->DeleteTrack(ptrack);
  }

  delete input_set;
  return true;
}

void NavObj_dB::LoadNavObjects() { LoadAllTracks(); }

bool NavObj_dB::AddNewTrack(Track* track) {
  if (TrackExists(m_db, track->m_GUID.ToStdString())) return false;

  bool rv = false;
  char* errMsg = 0;
  sqlite3_exec(m_db, "BEGIN TRANSACTION", 0, 0, &errMsg);

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
  int NbrOfLinks = track->m_TrackHyperlinkList->GetCount();
  if (NbrOfLinks > 0) {
    wxHyperlinkListNode* linknode = track->m_TrackHyperlinkList->GetFirst();
    while (linknode) {
      Hyperlink* link = linknode->GetData();

      if (!TrackHtmlLinkExists(m_db, link->GUID)) {
        InsertTrackHTML(m_db, track->m_GUID.ToStdString(), link->GUID,
                        link->DescrText.ToStdString(), link->Link.ToStdString(),
                        link->LType.ToStdString());
      }
      linknode = linknode->GetNext();
    }
  }
  sqlite3_exec(m_db, "COMMIT", 0, 0, &errMsg);
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
    sqlite3_bind_text(stmt, 1, track->GetName().ToStdString().c_str(),
                      track->GetName().Length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, track->m_TrackDescription.ToStdString().c_str(),
                      track->m_TrackDescription.Length(), SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, track->m_bVisible);
    sqlite3_bind_text(stmt, 4, track->m_TrackStartString.ToStdString().c_str(),
                      track->m_TrackStartString.Length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, track->m_TrackEndString.ToStdString().c_str(),
                      track->m_TrackEndString.Length(), SQLITE_TRANSIENT);
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
    return false;
  }

  sqlite3_finalize(stmt);

  // Update the HTML links
  // The list of links is freshly rebuilt when this method is called
  // So start by deleting all existing bcomments
  DeleteAllCommentsForTrack(m_db, track->m_GUID.ToStdString());

  // Now add all the links to db
  int NbrOfLinks = track->m_TrackHyperlinkList->GetCount();
  if (NbrOfLinks > 0) {
    wxHyperlinkListNode* linknode = track->m_TrackHyperlinkList->GetFirst();
    while (linknode) {
      Hyperlink* link = linknode->GetData();

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
          return false;
        }
        sqlite3_finalize(stmt);
      }

      linknode = linknode->GetNext();
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

    sqlite3_bind_text(stmtp, 1, guid.c_str(), -1, SQLITE_STATIC);

    int GPXSeg = 0;
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

      GPXSeg += 1;

      double latitude = sqlite3_column_double(stmtp, 0);
      double longitude = sqlite3_column_double(stmtp, 1);
      std::string timestamp =
          reinterpret_cast<const char*>(sqlite3_column_text(stmtp, 2));
      int point_order = sqlite3_column_int(stmtp, 3);

      auto point = new TrackPoint(latitude, longitude, timestamp);

      point->m_GPXTrkSegNo = GPXSeg;
      new_trk->AddPoint(point);
    }
    sqlite3_finalize(stmtp);

    if (new_trk) {
      new_trk->SetCurrentTrackSeg(GPXSeg);

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

          new_trk->m_TrackHyperlinkList->Append(h);
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
  std::string track_guid = track->m_GUID.ToStdString();
  const char* sql = "DELETE FROM tracks WHERE guid = ?";
  sqlite3_stmt* stmt;

  if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, track_guid.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      return false;
    }
    sqlite3_finalize(stmt);
  } else {
    return false;
  }
  return true;
}

#if 0

void printTrackPoints(sqlite3* db, const std::string& track_guid) {
    const char* sql = R"(
        SELECT p.guid, p.latitude, p.longitude, p.elevation, p.timestamp, tp.point_order
        FROM track_points tp
        JOIN points p ON p.guid = tp.point_guid
        WHERE tp.track_guid = ?
        ORDER BY tp.point_order ASC
    )";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << "\n";
        return;
    }

    sqlite3_bind_text(stmt, 1, track_guid.c_str(), -1, SQLITE_STATIC);

    std::cout << "Points for track " << track_guid << ":\n";

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string guid       = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        double latitude        = sqlite3_column_double(stmt, 1);
        double longitude       = sqlite3_column_double(stmt, 2);
        double elevation       = sqlite3_column_double(stmt, 3);
        std::string timestamp  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        int order              = sqlite3_column_int(stmt, 5);

        std::cout << "  [" << order << "] " << guid
                  << "  lat: " << latitude
                  << "  lon: " << longitude
                  << "  elev: " << elevation
                  << "  time: " << timestamp << "\n";
    }

    sqlite3_finalize(stmt);
}


void updateTrackName(sqlite3* db, const std::string& track_guid, const std::string& new_name) {
    const char* sql = "UPDATE tracks SET name = ? WHERE guid = ?";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, new_name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, track_guid.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Failed to update track name: " << sqlite3_errmsg(db) << "\n";
        } else {
            std::cout << "Track '" << track_guid << "' updated to name: " << new_name << "\n";
        }

        sqlite3_finalize(stmt);
    } else {
        std::cerr << "Failed to prepare update statement: " << sqlite3_errmsg(db) << "\n";
    }
}


void deleteTrack(sqlite3* db, const std::string& track_guid) {
    const char* sql = "DELETE FROM tracks WHERE guid = ?";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, track_guid.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Failed to delete track: " << sqlite3_errmsg(db) << "\n";
        } else {
            std::cout << "Track '" << track_guid << "' deleted.\n";
        }
        sqlite3_finalize(stmt);
    } else {
        std::cerr << "Failed to prepare delete: " << sqlite3_errmsg(db) << "\n";
    }
}

void deleteOrphanedPoints(sqlite3* db) {
    const char* sql = R"(
        DELETE FROM points
        WHERE guid NOT IN (SELECT point_guid FROM track_points)
    )";
    char* errMsg = nullptr;

    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Failed to delete orphaned points: " << errMsg << "\n";
        sqlite3_free(errMsg);
    }
}

void printAllTracks(sqlite3* db) {
    const char* sql = R"(
        SELECT guid, name, created_at
        FROM tracks
        ORDER BY created_at ASC
    )";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare track query: " << sqlite3_errmsg(db) << "\n";
        return;
    }

    std::cout << "Tracks in database:\n";

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string guid      = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string name      = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string created   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

        std::cout << "  - GUID: " << guid
                  << ", Name: " << name
                  << ", Created: " << created << "\n";
    }

    sqlite3_finalize(stmt);
}

void printPointsInTrack(sqlite3* db, const std::string& track_guid) {
    const char* sql = R"(
        SELECT tp.point_order, p.guid, p.latitude, p.longitude, p.elevation, p.timestamp
        FROM track_points tp
        JOIN points p ON p.guid = tp.point_guid
        WHERE tp.track_guid = ?
        ORDER BY tp.point_order ASC
    )";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare point query: " << sqlite3_errmsg(db) << "\n";
        return;
    }

    sqlite3_bind_text(stmt, 1, track_guid.c_str(), -1, SQLITE_STATIC);

    std::cout << "Points in track '" << track_guid << "':\n";

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int point_order         = sqlite3_column_int(stmt, 0);
        std::string point_guid  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        double latitude         = sqlite3_column_double(stmt, 2);
        double longitude        = sqlite3_column_double(stmt, 3);
        double elevation        = sqlite3_column_double(stmt, 4);
        std::string timestamp   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

        std::cout << "  [" << point_order << "] "
                  << point_guid
                  << " | lat: " << latitude
                  << " | lon: " << longitude
                  << " | elev: " << elevation
                  << " | time: " << timestamp << "\n";
    }

    sqlite3_finalize(stmt);
}

bool trackExists(sqlite3* db, const std::string& track_guid) {
    const char* sql = "SELECT 1 FROM tracks WHERE guid = ? LIMIT 1";
    sqlite3_stmt* stmt;
    bool exists = false;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, track_guid.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            exists = true;  // found a match
        }

        sqlite3_finalize(stmt);
    } else {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << "\n";
    }

    return exists;
}

void printCommentsForTrack(sqlite3* db, const std::string& track_guid) {
    const char* sql = R"(
        SELECT guid, author, content, timestamp
        FROM comments
        WHERE track_guid = ?
        ORDER BY timestamp ASC
    )";

    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, track_guid.c_str(), -1, SQLITE_STATIC);

        std::cout << "Comments for track '" << track_guid << "':\n";

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string guid      = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            std::string author    = sqlite3_column_text(stmt, 1)
                                      ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))
                                      : "(anonymous)";
            std::string content   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            std::string timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

            std::cout << "  [" << timestamp << "] "
                      << author << " (" << guid << "): "
                      << content << "\n";
        }

        sqlite3_finalize(stmt);
    } else {
        std::cerr << "Failed to prepare comment query: " << sqlite3_errmsg(db) << "\n";
    }
}

#endif
