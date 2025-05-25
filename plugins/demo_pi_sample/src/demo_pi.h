/***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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
 ***************************************************************************/

/**
 *  \file
 *  Minimal example plugin header
 */

#ifndef _DEMOPI_H_
#define _DEMOPI_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include "version.h"

#define MY_API_VERSION_MAJOR 1
#define MY_API_VERSION_MINOR 8

#define PLUGIN_VERSION_MAJOR 0
#define PLUGIN_VERSION_MINOR 1

#include "ocpn_plugin.h"

#include "nmea0183/nmea0183.h"

class DemoWindow;

//----------------------------------------------------------------------------------------------------------
//    The PlugIn Class Definition
//----------------------------------------------------------------------------------------------------------

class DemoPi : public opencpn_plugin_118 {
public:
  DemoPi(void *ppimgr) : opencpn_plugin_118(ppimgr) {}

  //    The required PlugIn Methods
  int Init(void);
  bool DeInit(void);

  int GetAPIVersionMajor();
  int GetAPIVersionMinor();
  int GetPlugInVersionMajor();
  int GetPlugInVersionMinor();

  wxString GetCommonName();
  wxString GetShortDescription();
  wxString GetLongDescription();

  //    The optional method overrides
  void OnContextMenuItemCallback(int id);
  void UpdateAuiStatus(void);

  //    The override PlugIn Methods
  bool RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp);
  void SetCursorLatLon(double lat, double lon);
  bool RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp);
  int GetToolbarToolCount(void);
  void ShowPreferencesDialog(wxWindow *parent);
  void OnToolbarToolCallback(int id);
  void SetPluginMessage(wxString &message_id, wxString &message_body);
  void SetPositionFixEx(PlugIn_Position_Fix_Ex &pfix);

private:
  wxWindow *m_parent_window;

  std::unique_ptr<DemoWindow> m_demo_window;
  wxAuiManager *m_aui_mgr;
  int m_show_id;
  int m_hide_id;
};

class DemoWindow : public wxWindow {
public:
  DemoWindow(wxWindow *pparent, wxWindowID id);
  virtual ~DemoWindow() = default;

  void OnPaint(wxPaintEvent &event);
  void OnSize(wxSizeEvent &event);
  void SetNavdata(ObservedEvt ev);
  void HandleGga(ObservedEvt ev);

  NMEA0183 m_nmea0183;  // Used to parse NMEA Sentences

  wxString m_nmea_sentence;
  double m_lat;
  double m_lon;
  double m_sog;
  double m_cog;
  double m_var;
  std::string m_utc;

private:
  std::shared_ptr<ObservableListener> m_navdata_listener;
  std::shared_ptr<ObservableListener> m_gga_listener;
};

#endif
