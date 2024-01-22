#include <iostream>
#include <string>

#include <wx/app.h>
#include <wx/log.h>
#include <wx/snglinst.h>
#include <wx/string.h>

#include "model/base_platform.h"

bool g_bportable;

class WxInstanceApp : public wxAppConsole {
public:

  WxInstanceApp() : wxAppConsole() {}

  void OnInitCmdLine(wxCmdLineParser& parser) override {}

  bool OnInit() override {
    SetAppName("opencpn");
    g_BasePlatform = new BasePlatform();
    auto path = g_BasePlatform->GetPrivateDataDir();
    wxSingleInstanceChecker m_checker("_OpenCPN_SILock", path);

    std::string line;
    std::getline(std::cin, line);
    return false;
  }
};
wxIMPLEMENT_APP_CONSOLE(WxInstanceApp);
