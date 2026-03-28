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
 * MySQL schema update facility
 */
#ifndef _NAVOBJ_DB_MIGRATOR_H__
#define _NAVOBJ_DB_MIGRATOR_H__

#pragma once
#include <sqlite3.h>
#include <functional>
#include <vector>
#include <string>

class DbMigrator {
public:
  using MigrationFn = std::function<void(sqlite3*)>;

  struct Migration {
    int version;
    MigrationFn fn;
  };

  explicit DbMigrator(sqlite3* db);

  void addMigration(int version, MigrationFn fn);
  void migrate();

private:
  sqlite3* db_;
  std::vector<Migration> migrations_;

  int getUserVersion();
  void setUserVersion(int v);

  void exec(const std::string& sql);
  void begin();
  void commit();
  void rollback();
};

#endif
