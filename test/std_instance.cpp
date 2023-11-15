#include <iostream>
#include <string>

#include <wx/app.h>
#include <wx/log.h>
#include <wx/string.h>

#include "std_instance_chk.h"
#include "base_platform.h"

float g_selection_radius_mm;
float g_selection_radius_touch_mm;
bool g_btouch;
bool g_bportable;
BasePlatform* g_BasePlatform = 0;

wxString g_winPluginDir;

class StdInstanceApp : public wxAppConsole {
public:

  StdInstanceApp() : wxAppConsole() {}

  bool OnInit() {
    SetAppName("opencpn");
    g_BasePlatform = new BasePlatform();
    StdInstanceCheck m_checker; 
    std::string line;
    std::getline(std::cin, line);
    return false;
  }
};
wxIMPLEMENT_APP_CONSOLE(StdInstanceApp);
