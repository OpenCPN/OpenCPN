#include <sstream>

#include <wx/filename.h>
#include <wx/platinfo.h>
#include <wx/string.h>

#include "config.h"

#include "model/base_platform.h"
#include "model/cmdline.h"
#include "model/ocpn_utils.h"
#include "model/plugin_paths.h"
#include "ocpn_plugin.h"

/*
 * The user-writable paths for libraries, binaries and plugin data,
 * one path each. And the list of paths used fo loading plugin
 * plugin libraries, locating helper binaries and storing plugin
 * data.
 */

static std::vector<std::string> split(const std::string& s, char delimiter) {
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(s);
  while (std::getline(tokenStream, token, delimiter)) {
    tokens.push_back(token);
  }
  return tokens;
}

static std::string expand(const std::string& s) {
  wxFileName fn(s);
  fn.Normalize();
  return fn.GetFullPath().ToStdString();
}

PluginPaths* PluginPaths::GetInstance() {
  static PluginPaths* instance = 0;
  if (!instance) {
    instance = new (PluginPaths);
  }
  return instance;
}

void PluginPaths::InitWindowsPaths() {
  using namespace std;

  if (g_bportable) {
    m_userLibdir =
        g_BasePlatform->GetPrivateDataDir().ToStdString() + "\\plugins";
    m_libdirs.push_back(m_userLibdir);
    m_user_bindir =
        g_BasePlatform->GetPrivateDataDir().ToStdString() + "\\plugins";
    m_bindirs = m_libdirs;
    m_user_datadir =
        g_BasePlatform->GetPrivateDataDir().ToStdString() + "\\plugins";
    m_datadirs.push_back(m_user_datadir);
    return;
  }

  const string platform_dir = g_BasePlatform->GetPluginDir().ToStdString();
  const string winPluginBaseDir =
      g_BasePlatform->GetWinPluginBaseDir().ToStdString();
  m_userLibdir = winPluginBaseDir;
  m_user_bindir = winPluginBaseDir;
  m_user_datadir = winPluginBaseDir;

  m_libdirs.push_back(m_userLibdir);
  m_libdirs.push_back(g_BasePlatform->GetPluginDir().ToStdString());
  m_bindirs = m_libdirs;

  m_datadirs.push_back(platform_dir + "\\plugins");
  m_datadirs.push_back(winPluginBaseDir);
}

void PluginPaths::InitFlatpakPaths() {
  using namespace std;

  const string flathome = m_home + "/.var/app/org.opencpn.OpenCPN";
  m_userLibdir = flathome + "/lib";
  m_user_bindir = flathome + "/bin";
  m_user_datadir = flathome + "/data";

  m_libdirs.push_back(flathome + "/lib");
  m_libdirs.push_back("/app/extensions/lib/opencpn");
  m_libdirs.push_back("/app/lib/opencpn");

  m_bindirs.push_back(flathome + "/bin");
  m_bindirs.push_back("/app/extensions/bin");
  m_bindirs.push_back("/app/bin");

  m_datadirs.push_back(flathome + "/data/plugins");
  m_datadirs.push_back("/app/extensions/share/opencpn/plugins");
  m_datadirs.push_back("/app/share/opencpn/plugins");
}

void PluginPaths::InitLinuxPaths() {
  using namespace std;

  if (g_bportable) {
    m_userLibdir = g_BasePlatform->GetPrivateDataDir().ToStdString() +
                   "/plugins/lib";  // m_home + "/.local/lib";
    m_libdirs.push_back(m_userLibdir);
    m_user_bindir = g_BasePlatform->GetPrivateDataDir().ToStdString() +
                    "/plugins/bin";  // m_home + "/.local/bin";
    m_bindirs = m_libdirs;
    m_user_datadir = g_BasePlatform->GetPrivateDataDir().ToStdString() +
                     "/plugins/share";  // m_home + "/.local/share";
    m_datadirs.push_back(m_user_datadir);
    return;
  }

  m_userLibdir = m_home + "/.local/lib";
  m_user_bindir = m_home + "/.local/bin";
  m_user_datadir = m_home + "/.local/share";

  std::vector<std::string> base_plugin_paths;
#if defined(__WXGTK__) || defined(__WXQT__)
  char exe_buf[100] = {0};
  ssize_t len = readlink("/proc/self/exe", exe_buf, 99);
  if (len > 0) {
    exe_buf[len] = '\0';
    wxFileName fn(exe_buf);
    std::string path = fn.GetPath().ToStdString();
    base_plugin_paths.push_back(expand(path + "/../lib/opencpn"));
    if (g_BasePlatform->GetOSDetail()->osd_arch.find("64") != string::npos) {
      base_plugin_paths.push_back(expand(path + "/../lib64/opencpn"));
    } else {
      base_plugin_paths.push_back(expand(path + "/../lib32/opencpn"));
    }
  }
#endif

  const char* const envdirs = getenv("OPENCPN_PLUGIN_DIRS");
  string dirlist = envdirs ? envdirs : "~/.local/lib/opencpn";
  m_libdirs = split(dirlist, ':');
  for (auto& dir : m_libdirs) {
    dir = expand(dir);
  }
  for (auto& base_plugin_path : base_plugin_paths) {
    if (envdirs == 0 && dirlist.find(base_plugin_path) == string::npos) {
      if (ocpn::exists(base_plugin_path)) {
        m_libdirs.push_back(base_plugin_path);
      }
    }
  }
  m_bindirs = m_libdirs;
  for (auto& dir : m_bindirs) {
    // Fails on Debian multilib paths like /usr/lib/x86_64-linux-gnu.
    // But we don't use those even on Debian.
    size_t pos = dir.rfind("/lib/opencpn");
    if (pos == string::npos) {
      pos = dir.rfind("/lib64/opencpn");
    }
    dir = pos == string::npos ? dir : dir.substr(0, pos) + "/bin";
  }
  const char* const xdg_data_dirs = getenv("XDG_DATA_DIRS");
  dirlist = xdg_data_dirs ? xdg_data_dirs : "~/.local/lib";
  m_datadirs = split(dirlist, ':');
  for (auto& dir : m_datadirs) {
    dir += "/opencpn/plugins";
  }
  for (auto& base_plugin_path : base_plugin_paths) {
    if (xdg_data_dirs == 0 && dirlist.find(base_plugin_path) == string::npos) {
      m_datadirs.push_back(base_plugin_path + "/plugins");
    }
  }
}

void PluginPaths::InitApplePaths() {
  using namespace std;

  const string mac_home = m_home + "/Library/Application Support/OpenCPN";
  m_userLibdir = mac_home + "/Contents/PlugIns";
  m_user_bindir = m_userLibdir;
  m_user_datadir = mac_home + "/Contents";

  m_libdirs.push_back(m_userLibdir);
  wxFileName fn_exe(g_BasePlatform->GetExePath());
  fn_exe.RemoveLastDir();
  string exeLibDir =
      fn_exe.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR).ToStdString() +
      "PlugIns";
  m_libdirs.push_back(exeLibDir);
  // m_libdirs.push_back("/Applications/OpenCPN.app/Contents/Plugins");
  m_bindirs = m_libdirs;

  m_datadirs.push_back(m_user_datadir);
  m_datadirs.push_back("/Applications/OpenCPN.app/Contents/PlugIns");
}

void PluginPaths::InitAndroidPaths() {
  using namespace std;

  const string platform_dir = g_BasePlatform->GetPluginDir().ToStdString();

  m_userLibdir =
      platform_dir + "/manPlug";  //("/data/user/0/org.opencpn.opencpn");
  m_user_bindir =
      platform_dir + "/manPlug";  //("/data/user/0/org.opencpn.opencpn");
  m_user_datadir =
      g_BasePlatform->GetPrivateDataDir()
          .ToStdString();  //(
                           //"/storage/emulated/0/android/data/org.opencpn.opencpn/files");

  m_libdirs.push_back(m_userLibdir);  // Load managed plugins first...
  m_libdirs.push_back(expand(platform_dir));

  m_bindirs = m_libdirs;
}

PluginPaths::PluginPaths() {
  using namespace std;

  wxString wxHome("unusable-$HOME");
  wxGetEnv("HOME", &wxHome);
  m_home = wxHome.ToStdString();

  auto osSystemId = wxPlatformInfo::Get().GetOperatingSystemId();
  if (osSystemId & wxOS_WINDOWS) {
    InitWindowsPaths();
  } else if (g_BasePlatform->isFlatpacked()) {
    InitFlatpakPaths();
  } else if (osSystemId & wxOS_UNIX_LINUX) {
#ifdef __OCPN__ANDROID__
    InitAndroidPaths();
#else
    InitLinuxPaths();
#endif
  } else if (osSystemId & wxOS_MAC) {
    InitApplePaths();
  } else {
    wxString os_name = wxPlatformInfo::Get().GetPortIdName();
    wxLogMessage(_T("OS_NAME: ") + os_name);
    if (os_name.Contains(_T("wxQT"))) {
      InitAndroidPaths();
    } else
      wxLogWarning("PluginPaths: Unknown platform");
  }
}
