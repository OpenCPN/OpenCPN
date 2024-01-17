/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Main wxWidgets Program
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David S. Register                               *
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

#ifndef _OCPN_APP_H
#define _OCPN_APP_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/app.h>
#include <wx/cmdline.h>
#include <wx/event.h>
#endif  // precompiled headers

#include "model/instance_check.h"

#include "model/comm_bridge.h"
#include "model/local_api.h"
#include "model/rest_server.h"

class Track;

class MyApp : public wxApp {
public:
  MyApp();
  ~MyApp(){};

  bool OnInit() override;
  int OnExit() override;
#ifndef __ANDROID__
  void OnInitCmdLine(wxCmdLineParser& parser) override;
  bool OnCmdLineHelp(wxCmdLineParser& parser) override;
  bool OnCmdLineParsed(wxCmdLineParser& parser) override;
#endif
  int OnRun() override;

  void OnActivateApp(wxActivateEvent& event);
  bool OpenFile(const std::string& path);

#ifdef LINUX_CRASHRPT
  //! fatal exeption handling
  void OnFatalException();
#endif

#ifdef __WXMSW__
  //  Catch malloc/new fail exceptions
  //  All the rest will be caught be CrashRpt
  bool OnExceptionInMainLoop();
#endif

  InstanceCheck& m_checker;
  CommBridge m_comm_bridge;

  RestServer m_rest_server;

  DECLARE_EVENT_TABLE()
private:

  /** Remote command deemed to be run from actual argc/argv. */
  struct ParsedCmdline {
    CmdlineAction action;
    std::string arg;
    ParsedCmdline(CmdlineAction a, const std::string& s) : action(a), arg(s) {}
    ParsedCmdline() : ParsedCmdline(CmdlineAction::Skip, "") {}
    ParsedCmdline(CmdlineAction a) : ParsedCmdline(a, "") {}
  };

  ParsedCmdline m_parsed_cmdline;
  int m_exitcode;  ///< by default -2. Otherwise, forces exit(exit_code)

  void InitRestListeners();
  ObsListener rest_srv_listener;

};

wxDECLARE_APP(MyApp);
#endif
