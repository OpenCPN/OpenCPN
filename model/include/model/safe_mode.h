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
 * Safe mode non-gui handling.
 *
 * Administrate the safe mode state which is true if openpcn should run in
 * safe mode without OpenGL, plugins, etc.
 */

#include <wx/window.h>

#include "ocpn_utils.h"

namespace safe_mode {

extern bool safe_mode;

/**
 * Return path to file created when start is commenced and removed when
 * completed
 */
std::string CheckFilePath();

/** Return true if we are running in safe mode. */
bool GetMode();

void SetMode(bool mode);

/** Mark last run as successful. */
void MarkStartAsOk();

}  // namespace safe_mode
