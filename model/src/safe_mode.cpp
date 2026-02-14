/***************************************************************************
 *   Copyright (C) 2022 Alec Leamas                                        *
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
 * Implement safe_mode.h -- safe mode non-gui handling.
 */

#include <cstdio>
#include <string>
#include <fstream>

#include <wx/dialog.h>
#include <wx/filename.h>
#include <wx/sizer.h>

#include "model/base_platform.h"
#include "model/cmdline.h"
#include "model/ocpn_utils.h"
#include "model/safe_mode.h"

namespace safe_mode {

#ifdef _WIN32
static std::string SEP("\\");
#else
static std::string SEP("/");
#endif

bool safe_mode = false;

std::string check_file_path() {
  std::string path = g_BasePlatform->GetPrivateDataDir().ToStdString();
  path += SEP;
  path += "startcheck.dat";
  return path;
}

void set_mode(bool mode) {
  safe_mode = mode;
  g_bdisable_opengl = g_bdisable_opengl || mode;
}

bool get_mode() { return safe_mode; }

/** Mark last run as successful. */
void clear_check() { remove(check_file_path().c_str()); }

}  // namespace safe_mode
