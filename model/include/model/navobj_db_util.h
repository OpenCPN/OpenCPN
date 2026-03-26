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
 * navobj_db_util.h -- MySQL support utilities
 */
#include "model/navobj_db.h"

class wxFrame;

// Utility functions for perform database schema updates, to be applied in
// succession.

bool needsMigration_0_1(sqlite3* db);
std::string SchemaUpdate_0_1(sqlite3* db, wxFrame* frame);
