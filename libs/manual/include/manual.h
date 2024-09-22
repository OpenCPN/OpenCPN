
/***************************************************************************
 *   Copyright (C) 2024  Alec Leamas                                       *
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

/** \file manual.h Tools to invoke the manual provided by manual_pi. */

#ifndef MANUAL_H_
#define MANUAL_H_

#include <wx/window.h>

#include <string>
#include <wx/jsonreader.h>

class Manual {
public:
  /**
   * Construct a Manual object.
   *
   * @param path  Manual plugin data directory as returned by
   *              GetPluginDataDir("manual")
   */
  Manual(wxWindow* parent, const std::string& path);

  /**
   * Launch a web browser displaying a manual entry point. If the manual
   * plugin is installed use that, otherwise offer user to use the on-line
   * manual.
   *
   * @param entrypoint Named entry in the entrypoints.json file provided
   *        by the manual_pi plugin. If the plugin is not installed a
   *        hardcoded list of entrypoints provides internet links.  See
   *        kOnlineEntries in manual_pi.cpp for list of valid values.
   * @return true if entrypoint exists in entrypoints.json, else false
   *
   */
  bool Launch(const std::string& entrypoint);

private:
  std::string m_datadir;
  wxJSONValue m_root;
  wxWindow* m_parent;
};

#endif  // MANUAL_H_
