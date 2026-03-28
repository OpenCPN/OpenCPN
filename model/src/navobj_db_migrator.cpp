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
 * Implement navobj_db_migrator.h -- MySQL schema update facility
 */

#include "model/navobj_db_migrator.h"
#include <stdexcept>
#include <algorithm>

DbMigrator::DbMigrator(sqlite3* db) : db_(db) {}

void DbMigrator::addMigration(int version, MigrationFn fn) {
  migrations_.push_back({version, fn});
}

int DbMigrator::getUserVersion() {
  sqlite3_stmt* stmt;
  int version = 0;

  if (sqlite3_prepare_v2(db_, "PRAGMA user_version;", -1, &stmt, nullptr) ==
      SQLITE_OK) {
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      version = sqlite3_column_int(stmt, 0);
    }
  }
  sqlite3_finalize(stmt);
  return version;
}

void DbMigrator::setUserVersion(int v) {
  exec("PRAGMA user_version = " + std::to_string(v) + ";");
}

void DbMigrator::exec(const std::string& sql) {
  char* errMsg = nullptr;
  if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
    std::string err = errMsg ? errMsg : "Unknown SQL error";
    sqlite3_free(errMsg);
    throw std::runtime_error(err);
  }
}

void DbMigrator::begin() { exec("BEGIN TRANSACTION;"); }
void DbMigrator::commit() { exec("COMMIT;"); }
void DbMigrator::rollback() { exec("ROLLBACK;"); }

void DbMigrator::migrate() {
  std::sort(migrations_.begin(), migrations_.end(),
            [](const Migration& a, const Migration& b) {
              return a.version < b.version;
            });

  int current = getUserVersion();

  for (const auto& m : migrations_) {
    if (m.version > current) {
      begin();
      try {
        m.fn(db_);
        setUserVersion(m.version);
        commit();
      } catch (...) {
        rollback();
        throw;
      }
    }
  }
}
