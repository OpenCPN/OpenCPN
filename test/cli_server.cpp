#include <iostream>
#include <thread>

#include <wx/app.h>
#include <wx/cmdline.h>
#include <wx/filename.h>
#include <wx/ipc.h>
#include <wx/log.h>
#include <wx/string.h>
#include <wx/timer.h>

#include "model/ocpn_utils.h"

static std::string GetSocketPath() {
  wxFileName path("~/.opencpn", "opencpn-ipc");
  path.Normalize(wxPATH_NORM_TILDE);
  auto dir = path.GetPath();
  if (!wxFileName::DirExists(dir)) wxFileName::Mkdir(dir);
  return path.GetFullPath().ToStdString();
}

class CliServer: public wxConnection {
public:
  CliServer(wxAppConsole* app)
      : wxConnection(), wx_app(app), exit_timer(app) {}

  class ExitTimer : public wxTimer {
  public:
    wxAppConsole* wx_app;
    ExitTimer(wxAppConsole* app) : wxTimer(), wx_app(app) {}
    void Notify() { wx_app->ExitMainLoop(); }
  };


  bool OnExec(const wxString&, const wxString& data) {
    std::cout << data << "\n" << std::flush;
    exit_timer.Start(200, wxTIMER_ONE_SHOT);
    return data == "raise" || data == "quit";
  }

  const void* OnRequest(const wxString& topic, const wxString& wxitem,
                        size_t* size, wxIPCFormat format) {
    exit_timer.Start(200, wxTIMER_ONE_SHOT);

    std::string item(wxitem);
    item = ocpn::trim(item);
    std::cout << "OnRequest: item: " << item << std::flush;
    if (item == "get_rest_endpoint") {
      buffer = "0.0.0.0/api";
      if (size) *size = buffer.size();
      return static_cast<const void*>(buffer.c_str());
    } else if (ocpn::startswith(item, "open")) {
      auto words = ocpn::split(item.c_str(), " ");
      if (words.size() != 2) return nullptr;
      buffer = words[1];
      if (size) *size = buffer.size();
      return static_cast<const void*>(buffer.c_str());
    } else {
      return nullptr;
    }
  }

private:
  std::string buffer;
  wxAppConsole* wx_app;
  ExitTimer exit_timer;
};


class ServerFactory : public wxServer {
public:
   const bool is_connected;
   wxAppConsole* wx_app;

   ServerFactory() : wxServer(), is_connected(Create(GetSocketPath())) {}

   wxConnectionBase* OnAcceptConnection(const wxString& topic) {
     return new CliServer(wx_app);
   }
};

class ServerApp : public wxAppConsole {

  ServerFactory factory;
  void OnInitCmdLine(wxCmdLineParser& parser) override {
    factory.wx_app = this;
    wxLog::SetActiveTarget(new wxLogStderr);
    wxLog::SetTimestamp("");
    wxLog::SetLogLevel(wxLOG_Warning);
  }


  bool OnCmdLineParsed(wxCmdLineParser& parser) override {
    wxInitializer initializer;
    std::cout << "Listening on " << GetSocketPath() << ", connected: " 
            << (factory.is_connected ? "true\n" : "false\n") << std::flush;
    return true;
  }
};

wxIMPLEMENT_APP_CONSOLE(ServerApp);
