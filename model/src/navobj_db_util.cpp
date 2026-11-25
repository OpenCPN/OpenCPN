/***************************************************************************
 *   Copyright (C) 2026 by David S. Register                               *
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
 * Implement navobj_db_util.h -- MySQL support utilities
 */

#include "model/navobj_db.h"
#include "model/navobj_db_migrator.h"
#include "model/navobj_db_util.h"

//  Schema migration 0->1 Helper functions
int getUserVersion(sqlite3* db) {
  sqlite3_stmt* stmt = nullptr;
  int version = 0;

  const char* sql = "PRAGMA user_version;";

  int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    return 0;  // or throw, depending on your error policy
  }

  rc = sqlite3_step(stmt);
  if (rc == SQLITE_ROW) {
    version = sqlite3_column_int(stmt, 0);
  }

  sqlite3_finalize(stmt);
  return version;
}

void setUserVersion(sqlite3* db, int v) {
  std::string sql = "PRAGMA user_version = " + std::to_string(v) + ";";

  char* errMsg = nullptr;
  int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);

  if (rc != SQLITE_OK) {
    std::string err = errMsg ? errMsg : "Failed to set user_version";
    sqlite3_free(errMsg);
    throw std::runtime_error(err);
  }
}

inline bool columnInPrimaryKey(sqlite3* db, const std::string& table,
                               const std::string& column) {
  std::string sql = "PRAGMA table_info(" + table + ");";
  sqlite3_stmt* stmt;

  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    return false;

  bool found = false;

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    std::string name =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    int pk = sqlite3_column_int(stmt, 5);

    if (name == column && pk > 0) {
      found = true;
      break;
    }
  }

  sqlite3_finalize(stmt);
  return found;
}

inline bool columnExists(sqlite3* db, const std::string& table,
                         const std::string& column) {
  std::string sql = "PRAGMA table_info(" + table + ");";
  sqlite3_stmt* stmt;

  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    return false;

  bool found = false;

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    std::string name =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    if (name == column) {
      found = true;
      break;
    }
  }

  sqlite3_finalize(stmt);
  return found;
}

bool needsMigration_0_1(sqlite3* db) {
  int version = getUserVersion(db);
  if (version < 1) {
    // Verify really version 0
    bool hasRouteGuid =
        columnInPrimaryKey(db, "routepoints_link", "route_guid");
    bool hasPointGuid =
        columnInPrimaryKey(db, "routepoints_link", "point_guid");
    bool hasPointOrder =
        columnInPrimaryKey(db, "routepoints_link", "point_order");
    return hasRouteGuid && hasPointGuid && !hasPointOrder;
  }
  return false;
}

// Utility functions for performing database schema updates, to be applied in
// succession.

std::string SchemaUpdate_0_1(sqlite3* db, wxFrame* frame) {
  bool rv = true;

  if (needsMigration_0_1(db)) {
    // Configure and execute migrator for 0->1 migration
    DbMigrator migrator(db);
    migrator.addMigration(1, [](sqlite3* db) {
      char* errMsg = nullptr;

      const char* sql = R"(
        CREATE TABLE routepoints_link_new (
            route_guid TEXT,
            point_guid TEXT,
            point_order INTEGER,
            PRIMARY KEY (route_guid, point_order),
            FOREIGN KEY (route_guid) REFERENCES routes(guid) ON DELETE CASCADE
        );

        INSERT INTO routepoints_link_new
        SELECT t.*
        FROM routepoints_link t
        WHERE rowid = (
            SELECT MIN(rowid)
            FROM routepoints_link t2
            WHERE t.route_guid = t2.route_guid
              AND t.point_order = t2.point_order
        );

        DROP TABLE routepoints_link;

        ALTER TABLE routepoints_link_new RENAME TO routepoints_link;
    )";

      if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::string err = errMsg ? errMsg : "";
        sqlite3_free(errMsg);
        throw std::runtime_error(err);
      }
    });

    try {
      migrator.migrate();
    } catch (const std::runtime_error& e) {
      // Known errors you threw (e.g., SQLite issues)
      return (std::string("Migration 0->1 failed: ") + e.what());
    } catch (const std::exception& e) {
      // Any other standard exception
      return (std::string("Unexpected error on migration 0->1: ") + e.what());
    } catch (...) {
      // Truly unknown (rare, but good safety net)
      return ("Unknown non-standard exception during migration");
    }
  }

  return "";
}

bool needsMigration_1_2(sqlite3* db) {
  int version = getUserVersion(db);
  if (version < 2) {
    return !columnExists(db, "routepoints", "linked_layer_guid");
  }
  return false;
}

std::string SchemaUpdate_1_2(sqlite3* db, wxFrame* frame) {
  if (needsMigration_1_2(db)) {
    DbMigrator migrator(db);
    migrator.addMigration(2, [](sqlite3* db) {
      char* errMsg = nullptr;
      const char* sql =
          "ALTER TABLE routepoints ADD COLUMN linked_layer_guid TEXT;";

      if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::string err = errMsg ? errMsg : "";
        sqlite3_free(errMsg);
        throw std::runtime_error(err);
      }
    });

    try {
      migrator.migrate();
    } catch (const std::runtime_error& e) {
      return (std::string("Migration 1->2 failed: ") + e.what());
    } catch (const std::exception& e) {
      return (std::string("Unexpected error on migration 1->2: ") + e.what());
    } catch (...) {
      return ("Unknown non-standard exception during migration");
    }
  }

  return "";
}
