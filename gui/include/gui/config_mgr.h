/**************************************************************************
 *   Copyright (C) 2018 by David S. Register                               *
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
 * %Config file user configuration interface.
 */

#ifndef __CONFIGMGR_H__
#define __CONFIGMGR_H__

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/event.h>
#include <wx/panel.h>
#include <wx/string.h>

#include "canvas_config.h"

class OCPNConfigCatalog;  // forward -- in config_mgr.cpp
class OCPNConfigObject;   // forward -- in config_mgr.cpp

using ConfigObjectList = std::list<OCPNConfigObject *>;

/**
 * Manages the user configuration matrix. Singleton that handles the creation,
 * deletion, and management of user configurations in OpenCPN.
 */
class ConfigMgr {
public:
  static ConfigMgr &Get();
  static void Shutdown();

  wxString CreateNamedConfig(const wxString &title, const wxString &description,
                             wxString UUID);
  bool DeleteConfig(wxString GUID);
  wxArrayString GetConfigGUIDArray();

  wxPanel *GetConfigPanel(wxWindow *parent, wxString GUID);
  wxString GetTemplateTitle(wxString GUID);
  bool ApplyConfigGUID(wxString GUID);
  bool CheckTemplateGUID(wxString GUID);
  arrayofCanvasConfigPtr &GetCanvasConfigArray() { return g_canvasConfigArray; }

private:  // private for singleton
  ConfigMgr();
  ~ConfigMgr();
  ConfigMgr(const ConfigMgr &) {}
  ConfigMgr &operator=(const ConfigMgr &) { return *this; }
  static ConfigMgr *instance;

  void Init();
  bool LoadCatalog();
  bool SaveCatalog();
  wxString GetUUID(void);
  bool SaveTemplate(wxString fileName);
  wxString GetConfigDir() { return m_configDir; }
  std::list<OCPNConfigObject *> *GetConfigList() { return configList; }
  OCPNConfigObject *GetConfig(wxString GUID);
  bool CheckTemplate(wxString fileName);

  wxString m_configDir;
  wxString m_configCatalogName;
  OCPNConfigCatalog *m_configCatalog;
  std::list<OCPNConfigObject *> *configList;
  arrayofCanvasConfigPtr g_canvasConfigArray;
};

/**
 * Represents a panel for displaying and editing a configuration. Creates a
 * wxPanel that shows the details of a specific configuration object and allows
 * for user interaction.
 */
class ConfigPanel : public wxPanel {
public:
  ConfigPanel(OCPNConfigObject *config, wxWindow *parent,
              wxWindowID id = wxID_ANY, const wxPoint &pos = wxDefaultPosition,
              const wxSize &size = wxDefaultSize);
  ~ConfigPanel();

  wxString GetConfigGUID();

private:
  void OnConfigPanelMouseSelected(wxMouseEvent &event);
  OCPNConfigObject *m_config;
};

#endif
