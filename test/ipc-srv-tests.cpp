/***************************************************************************
 *   Copyright (C) 2023 Alec Leamas                                        *
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

#include "config.h"

#include <chrono>
#include <cstdlib>
#include <string>
#include <thread>

#include <wx/app.h>
#include <wx/event.h>
#include <wx/filename.h>
#include <wx/log.h>

#include <gtest/gtest.h>

#include "base_platform.h"
#include "observable_evtvar.h"

#include "ipc_api.h"
#include "local_api.h"
#include "logger.h"
#include "ocpn_utils.h"

#ifdef __APPLE__
#include "macutils.h"
#endif

/**
 * \file
 *
 * Tests for the wxwidgets IPC server code.
 *
 * NOTE: The ` std::thread t([&] {` construct in the various Test* methods
 * like TestRaise exposes some kind of bug. The wortk around for now is to
 * always compile this using -O0 i. e. disable optimizations. With higher
 * levels, the methods crashes on exit.
 */

using namespace std::literals::chrono_literals;

BasePlatform* g_BasePlatform = 0;
bool g_bportable;
wxString g_winPluginDir;
float g_selection_radius_mm;
float g_selection_radius_touch_mm;
bool g_btouch;

/** Define an action to be performed when a KeyProvider is notified. */
class IpcServerTest : public wxAppConsole {
public:
  int OnRun() override {
    IpcServer server(GetSocketPath());
    EXPECT_TRUE(server.is_connected);
    TestRaise(server);
    TestQuit(server);
    TestGetRestApiEndpoint(server);
    TestOpenFile(server);
    return 0;
  }

private:
  class ObsListener : public wxEvtHandler {
  public:

    /** Create an object which does not listen until Init(); */
    ObsListener() {}

    /** Create object which invokes action when kp is notified. */
    ObsListener(const KeyProvider& kp,
                std::function<void(ObservedEvt& ev)> action) {
      Init(kp, action);
    }

    /** Create object which invokes action when kp is notified. */
    ObsListener(const KeyProvider& kp, std::function<void()> action)
        : ObsListener(kp, [&](ObservedEvt&) { action(); }) {}

    /** Initiate an object yet not listening. */
    void Init(const KeyProvider& kp,
              std::function<void(ObservedEvt& ev)> action) {
      // i. e. wxDEFINE_EVENT(), avoiding the evil macro.
      const wxEventTypeTag<ObservedEvt> EvtObs(wxNewEventType());
      m_listener.Listen(kp, this, EvtObs);
      Bind(EvtObs, action);
    }

  private:
    ObservableListener m_listener;
  };

  std::string GetSocketPath() {
    wxFileName path("~/.opencpn", "opencpn-ipc");
    path.Normalize(wxPATH_NORM_TILDE);
    return path.GetFullPath().ToStdString();
  }

  void TestRaise(IpcServer& server) {
    int result0 = 5;
    ObsListener listener(server.on_raise,
                         [&result0]() { result0 = 17; });
    auto cmd = std::string(CMAKE_BINARY_DIR) + "/test/ipc-client raise";
    FILE* stream = popen(cmd.c_str(), "r");

    std::thread t([&] {
      for (int i = 0; result0 == 5 && i < 10; i++)
        std::this_thread::sleep_for(10ms);
      ExitMainLoop();
    });
    wxAppConsole::OnRun();
    t.join();

    EXPECT_EQ(result0, 17) << "TestRaise";
    EXPECT_EQ(pclose(stream), 0);
  }

  void TestQuit(IpcServer& server) {
    int result0 = 7;
    ObsListener listener(server.on_quit,
                         [&result0]() { result0 = 13; });
    auto cmd = std::string(CMAKE_BINARY_DIR) + "/test/ipc-client quit";
    FILE* stream = popen(cmd.c_str(), "r");

    std::thread t([&] {
      for (int i = 0; result0 == 7 && i < 10; i++)
        std::this_thread::sleep_for(10ms);
      ExitMainLoop();
    });
    wxAppConsole::OnRun();
    t.join();

    EXPECT_EQ(result0, 13) << "TestQuit";
    EXPECT_EQ(pclose(stream), 0);
  }

  void TestGetRestApiEndpoint(IpcServer& server) {
    server.SetGetRestApiEndpointCb([]() { return "1.2.3.4/22"; });
    auto cmd =
        std::string(CMAKE_BINARY_DIR) + "/test/ipc-client get_rest_endpoint";
    FILE* stream = popen(cmd.c_str(), "r");

    char* line = 0;
    char buff[2048];
    std::thread t([&] {
      line = fgets(buff, sizeof(buff), stream);
      ExitMainLoop();
    });
    wxAppConsole::OnRun();
    t.join();

    std::string reply = ocpn::trim(line);
    EXPECT_EQ(reply, "1.2.3.4/22") << "TestGetRestApiEndpoint";
    EXPECT_EQ(pclose(stream), 0);
  }

  void TestOpenFile(IpcServer& server) {
    server.open_file_cb = [](const std::string&) { return false; };
    auto cmd =
        std::string(CMAKE_BINARY_DIR) + "/test/ipc-client open foobar.txt";
    FILE* stream = popen(cmd.c_str(), "r");

    char buff[2048];
    char* line = 0;
    std::thread t([&] {
      line = fgets(buff, sizeof(buff), stream);
      ExitMainLoop();
    });
    wxAppConsole::OnRun();
    t.join();

    std::string reply = ocpn::trim(line);
    // wxWidgets problems? We do return correct pointer and length
    // from Execute, but somehow trash is added to the end. Needs more
    // investigation
    EXPECT_TRUE(ocpn::startswith(reply, "fail")) << "TestOpen";
    EXPECT_EQ(pclose(stream), 0);
  }
};
wxIMPLEMENT_APP_NO_MAIN(IpcServerTest);

#ifndef OCPN_FLATPAK

TEST(IpcServer, Commands) {
  char arg0[32];
  strcpy(arg0, "ipc-srv");
  char* argv[] = {arg0, 0};
  int argc = 1;
  wxEntry(argc, argv);
}

#endif
