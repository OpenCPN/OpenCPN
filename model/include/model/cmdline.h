/***************************************************************************
 *   Copyright (C) 2023 by David Register, Alec Leamas                     *
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
 * Global variables reflecting command line options and arguments.
 */

#ifndef _CMDLINE_H__
#define _CMDLINE_H__

#include <vector>
#include <string>

extern int g_unit_test_1;
extern int g_unit_test_2;
extern bool g_start_fullscreen;
extern bool g_rebuild_gl_cache;
extern bool g_parse_all_enc;
extern bool g_bportable;
extern bool g_config_wizard;
extern bool g_bdisable_opengl;
extern std::string g_configdir;
extern std::vector<std::string> g_params;

#endif  // _CMDLINE_H__
