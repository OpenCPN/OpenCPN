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

#include <wx/snglinst.h>

#include "comm_bridge.h"
#include "rest_server.h"

class Track;

class MyApp : public wxApp {
public:
  MyApp();
  ~MyApp(){};

  bool OnInit();
  int OnExit();
  void OnInitCmdLine(wxCmdLineParser& parser);
  bool OnCmdLineParsed(wxCmdLineParser& parser);
  void OnActivateApp(wxActivateEvent& event);

#ifdef LINUX_CRASHRPT
  //! fatal exeption handling
  void OnFatalException();
#endif

#ifdef __WXMSW__
  //  Catch malloc/new fail exceptions
  //  All the rest will be caught be CrashRpt
  bool OnExceptionInMainLoop();
#endif

  Track* TrackOff(void);

  wxSingleInstanceChecker* m_checker;
  CommBridge m_comm_bridge;

  RestServer m_RESTserver;

  DECLARE_EVENT_TABLE()
};

wxDECLARE_APP(MyApp);
#endif
