#include   <sstream>

#include   "config.h"
#include   "OCPNPlatform.h"
#include   "PluginPaths.h"

/*
 * The user-writable paths for libraries, binaries and plugin data, 
 * one path each. And the list of paths used fo loading plugin
 * plugin libraries, locating helper binaries and storing plugin
 * data.
 */


const char* const LINUX_DATA_PATH =
    "~/.local/share:/usr/local/share:/usr/share";

extern OCPNPlatform*  g_Platform;


static std::vector<std::string> split(const std::string& s, char delimiter)
{
   std::vector<std::string> tokens;
   std::string token;
   std::istringstream tokenStream(s);
   while (std::getline(tokenStream, token, delimiter)) {
      tokens.push_back(token);
   }
   return tokens;
}


static std::string expand(const std::string& s)
{
    wxFileName fn(s);
    fn.Normalize();
    return fn.GetFullPath().ToStdString();
}

PluginPaths* PluginPaths::getInstance()
{
    static PluginPaths* instance = 0;
    if (!instance) {
        instance = new(PluginPaths);
    }
    return instance;
}


void PluginPaths::initWindowsPaths()
{
    using namespace std;

    const string platform_dir = g_Platform->GetPluginDir().ToStdString();
    const string winPluginBaseDir =
        g_Platform->GetWinPluginBaseDir().ToStdString();
    m_userLibdir = winPluginBaseDir + "\\plugins";
    m_userBindir = winPluginBaseDir + "\\plugins";
    m_userDatadir = winPluginBaseDir;
    m_unknownPathDir = winPluginBaseDir + "\\unknown-prefix";

    m_libdirs.push_back(m_userLibdir);
    m_libdirs.push_back(g_Platform->GetPluginDir().ToStdString());
    m_bindirs = m_libdirs;

    m_datadirs.push_back(platform_dir + "\\plugins");
    m_datadirs.push_back(winPluginBaseDir + "\\plugins");
}


void PluginPaths::initFlatpackPaths()
{
    using namespace std;

    const string flathome = m_home + "/.var/app/org.opencpn.OpenCPN";
    m_userLibdir = flathome + "/lib";
    m_userBindir = flathome + "/bin";
    m_userDatadir = flathome + "/data";
    m_unknownPathDir = flathome + "/unknown-prefix";

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


void PluginPaths::initLinuxPaths()
{
    using namespace std;

    m_userLibdir = m_home + "/.local/lib";
    m_userBindir = m_home + "/.local/bin";
    m_userDatadir = m_home + "/.local/share";
    m_unknownPathDir = m_home + "/.local/share/opencpn/unknown-prefix";

    const string platform_dir = g_Platform->GetPluginDir().ToStdString();
    const char* const envdirs = getenv("OPENCPN_PLUGIN_DIRS");
    string dirlist = envdirs ? envdirs : OCPN_LINUX_LOAD_PATH;
    if (envdirs == 0 && dirlist.find(platform_dir) == string::npos) {
        dirlist = dirlist + ":" + platform_dir;
    }
    m_libdirs = split(dirlist, ':');
    for (auto& dir: m_libdirs) {
        dir += "/opencpn";
        dir = expand(dir);
    }
    m_bindirs = m_libdirs;
    for (auto& dir: m_bindirs) {
        size_t pos = dir.find_last_of("/lib/opencpn");
        dir = pos == string::npos ? dir : dir.substr(0, pos) + "/bin";
    }
    const char* const xdg_data_dirs = getenv("XDG_DATA_DIRS");
    dirlist = xdg_data_dirs ? xdg_data_dirs : LINUX_DATA_PATH;
    m_datadirs = split(dirlist, ':');
    for (auto& dir: m_datadirs) {
        dir += "/opencpn/plugins";
    }
    if (xdg_data_dirs == 0 && dirlist.find(platform_dir) == string::npos) {
        m_datadirs.push_back(platform_dir + "/plugins");
    }
}


void PluginPaths::initApplePaths()
{
    using namespace std;

    const string mac_home = m_home + "/Library/Application Support/OpenCPN";
    m_userLibdir = mac_home + "/Contents/Plugins";
    m_userBindir = m_userLibdir;
    m_userDatadir = mac_home + "/Contents";
    m_unknownPathDir = mac_home + "/Contents/unknown-paths";

    m_libdirs.push_back(m_userLibdir);
    m_libdirs.push_back("/Applications/OpenCPN.app/Contents/Plugins");
    m_bindirs = m_libdirs;

    m_datadirs.push_back(m_userDatadir);
    m_datadirs.push_back( "/Applications/OpenCPN.app/Contents/plugins");
}


PluginPaths::PluginPaths()
{
    using namespace std;

    wxString wxHome("unusable-$HOME");
    wxGetEnv("HOME", &wxHome);
    m_home = wxHome.ToStdString();

    auto osSystemId = wxPlatformInfo::Get().GetOperatingSystemId();
    if (osSystemId & wxOS_WINDOWS) {
        initWindowsPaths();
    }
    else if (g_Platform->isFlatpacked()) {
        initFlatpackPaths();
    }
    else if (osSystemId & wxOS_UNIX_LINUX) {
        initLinuxPaths();
    }
    else if (osSystemId & wxOS_MAC) {
        initApplePaths();
    }
    else {
        wxLogWarning("PluginPaths: Unknown platform");
    }
}
