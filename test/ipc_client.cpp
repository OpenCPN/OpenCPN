#include <iostream>
#include <thread>

#include <wx/app.h>
#include <wx/cmdline.h>
#include <wx/filename.h>
#include <wx/ipc.h>
#include <wx/log.h>
#include <wx/string.h>
#include <wx/timer.h>

#include "ocpn_utils.h"
#include "ipc_api.h"

static std::string SocketPath() {
  wxFileName path("~/.opencpn", "opencpn-ipc");
  path.Normalize(wxPATH_NORM_TILDE);
  return path.GetFullPath().ToStdString();
}

class _IpcClientFactory;  // forward

class _IpcClient: public wxConnection {
public:
  _IpcClient(wxAppConsole* app) : wxConnection(),  exit_timer(app) {}

  class ExitTimer : public wxTimer {
  public:
    wxAppConsole* console_app;
    ExitTimer(wxAppConsole* app) : wxTimer(), console_app(app) {}
    void Notify() { console_app->ExitMainLoop(); }
  };


  bool OnExec(const wxString& tbd, const wxString& data) {
    exit_timer.Start(200, wxTIMER_ONE_SHOT);
    return wxConnection::OnExec(tbd, data);
    /**
    std::cout << data << "\n" << std::flush;
    return (data == "raise" || data == "quit");
    **/
  }

  const void* OnRequest(const wxString& topic, const wxString& wxitem,
                        size_t* size, wxIPCFormat format) {
    exit_timer.Start(200, wxTIMER_ONE_SHOT);
    return wxConnection::OnRequest(topic, wxitem, size, format);
    /***
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
    ***/
  }

private:
  std::string buffer;
  ExitTimer exit_timer;
};


class _IpcClientFactory : public wxClient {
public:
   wxConnectionBase* connection;
   wxAppConsole* app;

   _IpcClientFactory(wxAppConsole* _app)
       : wxClient(), connection(0), app(_app) {}
   _IpcClientFactory(wxAppConsole* _app, const std::string& path)
       : wxClient(), app(_app)
   {
      connection = MakeConnection("localhost", path, "OpenCPN");
   }

   wxConnectionBase* OnMakeConnection() { return new _IpcClient(app); }
};


class ClientApp : public wxAppConsole {
public:
  _IpcClientFactory factory;

  ClientApp() : wxAppConsole(), factory(this, SocketPath()) {}

  void OnInitCmdLine(wxCmdLineParser& parser) override {
    parser.AddParam("command", wxCMD_LINE_VAL_STRING,  wxCMD_LINE_PARAM_MULTIPLE);
    wxLog::SetActiveTarget(new wxLogStderr);
    wxLog::SetTimestamp("");
    wxLog::SetLogLevel(wxLOG_Warning);
  }

  bool OnCmdLineParsed(wxCmdLineParser& parser) override {
    wxInitializer initializer;
    std::vector<std::string> args;
    for (size_t i = 0; i < parser.GetParamCount(); i++)
      args.push_back(parser.GetParam(i).ToStdString());
    if (!factory.connection) {
      std::cout << "Cannot connect to server at " << SocketPath() << "\n" 
              << std::flush;
      exit(1);
    }
    if (args.size() < 1) {
      std::cerr << "que?\n";
      exit(1);
    }
    else if (args[0] == "raise") {
      bool ok = factory.connection->Execute("raise");
      exit(ok ? 0 : 2);
    }
    else if (args[0] == "quit") {
      bool ok = factory.connection->Execute("quit");
      exit(ok ? 0 : 2);
    }
    else if (args[0] == "get_rest_endpoint") {
      auto cmd = std::string(args[0]) + args[1];
      const void* reply = factory.connection->Request(cmd.c_str(), 0);
      std::cout << static_cast<const char*>(reply) << "\n";
      exit(reply ? 0 : 2);
    }
    else if (args[0] == "open") {
      auto cmd = std::string(args[0]) + " " + args[1];
      const void* reply = factory.connection->Request(cmd.c_str(), 0);
      std::cout << static_cast<const char*>(reply) << "\n";
      exit(reply ? 0 : 2);
    }
     else {
      std::cerr << "que?\n";
      exit(1);
    }
  }
};
wxIMPLEMENT_APP_CONSOLE(ClientApp);
