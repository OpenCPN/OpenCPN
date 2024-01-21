#include <iostream>
#include <string>

#include <wx/app.h>
#include <wx/log.h>
#include <wx/string.h>

#include "model/std_instance_chk.h"
#include "model/base_platform.h"

bool g_bportable;

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
