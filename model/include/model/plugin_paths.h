/**************************************************************************
 *   Copyright (C) 2010 - 2023 by David S. Register                        *
 *   Copyright (C) 2023 - 2025  Alec Leamas                                *
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
 * Plugin installation and data paths support.
 */

#ifndef MODEL__PLUGIN_PATHS_H
#define MODEL__PLUGIN_PATHS_H
#include <string>
#include <vector>

/** Join a list of directories to a string. */
std::string join(std::vector<std::string> v, char c);

/** Accessors for various paths to install plugins and their data. */
class PluginPaths {
public:
  /**  Return the singleton instance. */
  static PluginPaths* GetInstance();

  /** The single, user-writable directory for installing .dll files. */
  std::string UserLibdir() { return m_userLibdir; };

  /** The single, user-writable directory for installing helper binaries. */
  std::string UserBindir() { return m_user_bindir; }

  /**
   * The single, user-writable common parent for plugin data
   * directories, typically ending in 'plugins'.
   */
  std::string UserDatadir() { return m_user_datadir; }

  /** List of directories from which we load plugins.*/
  std::vector<std::string> Libdirs() { return m_libdirs; }

  /** 'List of directories for plugin binary helpers.*/
  std::vector<std::string> Bindirs() { return m_bindirs; };

  /**
   *  List of directories containing plugin data, each
   *  element typically ends in 'plugins'.
   */
  std::vector<std::string> Datadirs() { return m_datadirs; }

  /** home directory, convenience stuff. */
  std::string Homedir() const { return m_home; }

private:
  PluginPaths();

  void InitWindowsPaths();
  void InitFlatpakPaths();
  void InitLinuxPaths();
  void InitApplePaths();
  void InitAndroidPaths();

  std::string m_home;
  std::string m_user_datadir;
  std::string m_user_bindir;
  std::string m_userLibdir;
  std::vector<std::string> m_libdirs;
  std::vector<std::string> m_bindirs;
  std::vector<std::string> m_datadirs;
};
#endif  // MODEL__PLUGIN_PATHS_H
