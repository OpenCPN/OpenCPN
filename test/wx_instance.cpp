#include <iostream>
#include <string>

#include <wx/app.h>
#include <wx/log.h>
#include <wx/snglinst.h>
#include <wx/string.h>

#include "base_platform.h"

BasePlatform* g_BasePlatform = 0;
float g_selection_radius_mm;
float g_selection_radius_touch_mm;
bool g_btouch;
bool g_bportable;

wxString g_winPluginDir;

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
