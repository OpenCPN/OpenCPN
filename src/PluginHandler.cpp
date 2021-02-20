/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2019 Alec Leamas                                        *
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
 ***************************************************************************
 */
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <memory>
#include <ostream>
#include <regex>
#include <stdexcept>
#include <streambuf>
#include <sstream>
#include <unordered_map>
#include <set>

#include <wx/jsonreader.h>
#include <wx/string.h>
#include <wx/file.h>
#include <wx/uri.h>

#include <archive.h>
#include <archive_entry.h>
typedef __LA_INT64_T la_int64_t;      //  "older" libarchive versions support

#include <expat.h>

#if defined(__MINGW32__) && defined(Yield)
#undef Yield                 // from win.h, conflicts with mingw headers
#endif

#include "catalog_parser.h"
#include "catalog_handler.h"
#include "Downloader.h"
#include "logger.h"
#include "navutil.h"
#include "OCPNPlatform.h"
#include "ocpn_utils.h"
#include "PluginHandler.h"
#include "pluginmanager.h"
#include "PluginPaths.h"

#ifdef _WIN32
static std::string SEP("\\");
#else
static std::string SEP("/");
#endif

#ifndef F_OK   // windows: missing unistd.h.
#define F_OK 0
#endif


extern OCPNPlatform*  g_Platform;
extern PlugInManager* g_pi_manager;
extern wxString       g_winPluginDir;
extern MyConfig*      pConfig;
extern OCPNPlatform*  g_Platform;
extern bool           g_bportable;
extern MyFrame        *gFrame;

extern wxString       g_compatOS;
extern wxString       g_compatOsVersion;

/** split s on first occurrence of delim, or return s in first result. */
static std::vector<std::string> split(const std::string& s, const std::string& delim)
{
    std::vector<std::string> result;
    size_t pos = s.find(delim);
    if (pos == std::string::npos) {
        result.push_back(s);
        return result;
    }
    result.push_back(s.substr(0, pos));
    result.push_back(s.substr(pos + delim.length()));
    return result;
}


inline std::string basename(const std::string path)
{
    wxFileName wxFile(path);
    return wxFile.GetFullName().ToStdString();
}


bool isRegularFile(const char* path)
{
    wxFileName wxFile(path);
    return wxFile.FileExists() && !wxFile.IsDir();
}


static void mkdir(const std::string path)
{
#if defined(_WIN32) && !defined(__MINGW32__)
    _mkdir(path.c_str());
#elif defined(__MINGW32__)
    mkdir(path.c_str());
#else
    mkdir(path.c_str(), 0755);
#endif
}


/**
 * Return index in ArrayOfPlugins for plugin with given name,
 * or -1 if not found
 */
static ssize_t PlugInIxByName(const std::string name, ArrayOfPlugIns* plugins)
{
    for (unsigned i = 0; i < plugins->GetCount(); i += 1) {
        if (name == plugins->Item(i)->m_common_name.ToStdString()) {
            return i;
        }
    }
    return -1;
}


static std::string pluginsConfigDir()
{
    std::string pluginDataDir = g_Platform->GetPrivateDataDir().ToStdString();
    pluginDataDir += SEP + "plugins";
    if (!ocpn::exists(pluginDataDir)) {
        mkdir(pluginDataDir);
    }
    pluginDataDir += SEP + "install_data";
    if (!ocpn::exists(pluginDataDir)) {
        mkdir(pluginDataDir);
    }
    return pluginDataDir;
}


static std::string dirListPath(std::string name)
{
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    return pluginsConfigDir() + SEP + name + ".dirs";
}


CompatOs* CompatOs::getInstance()
{
    static std::string last_global_os("");
    static CompatOs* instance = 0;

    if (!instance || last_global_os != g_compatOS) {
        instance = new(CompatOs);
        last_global_os = g_compatOS;
    }
    return instance;
}


CompatOs::CompatOs(): _name(PKG_TARGET), _version(PKG_TARGET_VERSION)
{
    // Get the specified system definition,
    //   From the OCPN_OSDetail structure probed at startup.
    //   or the environment override,
    //   or the config file override
    //   or the baked in (build system) values.  Not too useful in cross-build environments...

    OCPN_OSDetail *os_detail = g_Platform->GetOSDetail();

    std::string compatOS(_name);
    std::string compatOsVersion(_version);

    // Handle the most common cross-compile, safely
#ifdef ocpnARM 
    if(os_detail->osd_ID.size())
        compatOS = os_detail->osd_ID;
    if(os_detail->osd_version.size())
        compatOsVersion = os_detail->osd_version;
#endif    

    if (getenv("OPENCPN_COMPAT_TARGET") != 0) {
        _name = getenv("OPENCPN_COMPAT_TARGET");
        if (_name.find(':') != std::string::npos) {
            auto tokens = ocpn::split(_name.c_str(), ":");
            _name = tokens[0];
            _version = tokens[1];
        }
    }
    else if (g_compatOS != "") {
        // CompatOS and CompatOsVersion in opencpn.conf/.ini file.
        _name = g_compatOS;
        if (g_compatOsVersion != ""){
            _version = g_compatOsVersion;
        }
    }
    _name = ocpn::tolower(_name);
    _version = ocpn::tolower(_version);
}


std::string PluginHandler::fileListPath(std::string name)
{
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    return pluginsConfigDir() + SEP + name + ".files";
}


bool PluginHandler::isCompatible(const PluginMetadata& metadata,
                                 const char* os, const char* os_version)

{
    wxLogDebug("Plugin compatibility check");
    wxLogDebug("name: %s, target: %s, target_arch: %s", metadata.name, metadata.target, metadata.target_arch);
    OCPN_OSDetail *os_detail = g_Platform->GetOSDetail();


    auto compat_os = CompatOs::getInstance();
    std::string compatOS(compat_os->name());
    std::string compatOsVersion(compat_os->version());

    //  Compare to the required values in the metadata
    std::string plugin_os = ocpn::tolower(metadata.target);

    // msvc is simple...
    if (compatOS == "msvc") {
        return (plugin_os == "msvc");
    }

    // And so is MacOS...
    if (compatOS == "darwin") {
        return (plugin_os == "darwin");
    }

    //  For linux variants....
    // If the plugin architecture is defined, we can eliminate incompatible plugins immediately
    if(metadata.target_arch.size()){
        wxLogDebug("target_arch: %s, osd_arch: %s, osd_build_arch: %s", ocpn::tolower(metadata.target_arch), ocpn::tolower(os_detail->osd_arch), ocpn::tolower(os_detail->osd_build_arch));
        if(ocpn::tolower(metadata.target_arch) != ocpn::tolower(os_detail->osd_arch) && ocpn::tolower(metadata.target_arch) != ocpn::tolower(os_detail->osd_build_arch)) {
            wxLogDebug("Not compatible");
            return false;
        }
    }

    std::string compatOS_ARCH = compatOS + "-" + ocpn::tolower(os_detail->osd_arch);

    wxLogDebug(wxString::Format(_T("Plugin compatibility check1: %s  OS:%s  Plugin:%s"), metadata.name.c_str(), compatOS_ARCH.c_str(), plugin_os.c_str()));

    bool rv = false;
    std::string plugin_os_version = ocpn::tolower(metadata.target_version);

    auto meta_vers = ocpn::split(plugin_os_version.c_str(), ".")[0];

    wxLogDebug("compatOS_ARCH: %s, compatOS_Build_ARCH: %s, build target: %s, plugin_os: %s, plugin build target: %s", compatOS_ARCH, os_detail->osd_build_arch, PKG_BUILD_TARGET, plugin_os, metadata.build_target);
    if (compatOS_ARCH  == plugin_os || os_detail->osd_build_arch == metadata.target_arch) {
        //  OS matches so far, so must compare versions

        if (ocpn::startswith(plugin_os, "ubuntu")){
            wxLogDebug("plugin_os_version: %s, CompatOsVersion: %s, osd_build_version: %s", plugin_os_version, compatOsVersion, os_detail->osd_build_version);
            if(plugin_os_version == compatOsVersion || plugin_os_version == os_detail->osd_build_version)            // Full version comparison required
                rv = true;
        }
        else{
            auto target_vers = ocpn::split(compatOsVersion.c_str(), ".")[0];
            wxLogDebug("meta_vers: %s, target_vers: %s, osd_build_version: %s, version: %s", meta_vers, target_vers, os_detail->osd_build_version, metadata.version);
            if( meta_vers == target_vers || os_detail->osd_build_version == metadata.version)
                rv = true;;
        }
    }
    else{
        // running OS may be "like" some known OS
        for(unsigned int i=0 ; i < os_detail->osd_name_like.size(); i++){
            std::string osd_like_arch = os_detail->osd_name_like[i] + "-" + os_detail->osd_arch;
            if( osd_like_arch  == plugin_os){
                if (ocpn::startswith(plugin_os, "ubuntu")){
                    if( plugin_os_version == os_detail->osd_version )            // Full version comparison required
                        rv = true;
                }
                else{
                    auto target_vers = ocpn::split(os_detail->osd_version.c_str(), ".")[0];
                    if( meta_vers == target_vers )
                        rv = true;
                }
            }
        }
    }
    
    // Special case tests for vanilla debian, which can use some variants of Ubuntu plugins
    if(!rv){
        wxLogDebug("Checking for debian and ubuntu");
        if (ocpn::startswith(compatOS_ARCH, "debian-x86_64")){
            auto target_vers = ocpn::split(compatOsVersion.c_str(), ".")[0];
            if(target_vers == std::string("9") ){        // Stretch
                if( (plugin_os == std::string("ubuntu-x86_64")) && (plugin_os_version == std::string("16.04")) )
                    rv = true;
            }
            else if (target_vers == "11"  || target_vers == "sid"){        // Sid
                if( (plugin_os == std::string("ubuntu-gtk3-x86_64")) && (plugin_os_version == std::string("20.04")) )
                    rv = true;
            }
        }
    }

    std::string status("REJECTED");
    if(rv)
        status = "ACCEPTED";
    wxLogDebug(wxString::Format(_T("Plugin compatibility checkFinal %s: %s  PluginOS:%s  PluginVersion: %s"), status.c_str(), metadata.name.c_str(), plugin_os.c_str(), plugin_os_version.c_str()));
       

    return rv;
}


std::string PluginHandler::versionPath(std::string name)
{
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    return pluginsConfigDir() + SEP + name + ".version";
}


typedef std::unordered_map<std::string, std::string> pathmap_t;

/**
 * Platform-specific mapping of tarball paths to user-writable
 * installation directories.
 */
static pathmap_t getInstallPaths()
{
    using namespace std;

    pathmap_t pathmap;
    PluginPaths* paths = PluginPaths::getInstance();
    pathmap["bin"] =  paths->UserBindir();
    pathmap["lib"] =  paths->UserLibdir();
    pathmap["lib64"] = paths->UserLibdir();
    pathmap["share"] =  paths->UserDatadir();
    pathmap["unknown"] = paths->UserUnknownPrefixDir();
    return pathmap;
}


static void saveFilelist(std::string filelist, std::string name)
{
    using namespace std;
    string listpath = PluginHandler::fileListPath(name);
    ofstream diskfiles(listpath);
    if (!diskfiles.is_open()) {
        wxLogWarning("Cannot create installed files list.");
        return;
    }
    diskfiles << filelist;
}


static void saveDirlist(std::string name)
{
    using namespace std;
    string path = dirListPath(name);
    ofstream dirs(path);
    if (!dirs.is_open()) {
        wxLogWarning("Cannot create installed files list.");
        return;
    }
    pathmap_t pathmap = getInstallPaths();
    unordered_map<string, string>::iterator it;
    for (it = pathmap.begin(); it != pathmap.end(); it++) {
        dirs << it->first << ": " << it->second << endl;
    }
}

static void saveVersion(const std::string& name, const std::string& version)
{
    using namespace std;
    string path = PluginHandler::versionPath(name);
    ofstream stream(path);
    if (!stream.is_open()) {
        wxLogWarning("Cannot create version file.");
        return;
    }
    stream << version << endl;
}


static int copy_data(struct archive* ar, struct archive* aw)
{
    int r;
    const void* buff;
    size_t size;
    la_int64_t offset;

    while (true) {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
            return (ARCHIVE_OK);
        if (r < ARCHIVE_OK){
            std::string s(archive_error_string(ar));
            return (r);
        }
        r = archive_write_data_block(aw, buff, size, offset);
        if (r < ARCHIVE_OK) {
            std::string s(archive_error_string(aw));
            wxLogWarning("Error copying install data: %s",
                         archive_error_string(aw));
            return (r);
        }
    }
}


static void win_entry_set_install_path(struct archive_entry* entry,
                                       pathmap_t installPaths)
{
    using namespace std;

    string path = archive_entry_pathname(entry);

    // Check # components, drop the single top-level path
    int slashes = count(path.begin(), path.end(), '/');
    if (slashes < 1) {
        archive_entry_set_pathname(entry, "");
        return;
    }
    if (ocpn::startswith(path, "./")) {
        path = path.substr(1);
    }

    // Remove top-level directory part
    int slashpos = path.find_first_of('/', 1);
    if(slashpos < 0){
        archive_entry_set_pathname(entry, "");
        return;
    }
        
    string prefix = path.substr(0, slashpos);
    path = path.substr(prefix.size() + 1);

    // Map remaining path to installation directory
    if (ocpn::endswith(path, ".dll") || ocpn::endswith(path, ".exe")) {
        slashpos = path.find_first_of('/');
        path = path.substr(slashpos + 1);
        path = installPaths["bin"] + "\\" + path;
    } else if (ocpn::startswith(path, "share")) {
        // The "share" directory should be a direct sibling of "plugins" directory
        wxFileName fn(installPaths["share"].c_str(), "");       // should point to .../opencpn/plugins
        fn.RemoveLastDir();     // should point to ".../opencpn
        path = fn.GetFullPath().ToStdString() + path;
    } else if (ocpn::startswith(path, "plugins")) {
        slashpos = path.find_first_of('/');
       // share path already ends in plugins/, drop prefix from archive entry.
        path = path.substr(slashpos + 1);
        path = installPaths["share"] + "\\" + path;

    } else if (archive_entry_filetype(entry) == AE_IFREG) {
        path = installPaths["unknown"] + "\\" + path;
    }
    wxString s(path);
    s.Replace("/", "\\");      // std::regex_replace FTBS on gcc 4.8.4
    s.Replace("\\\\", "\\");
    archive_entry_set_pathname(entry, s.c_str());
}


static void flatpak_entry_set_install_path(struct archive_entry* entry,
                                           pathmap_t installPaths)
{
    using namespace std;

    string path = archive_entry_pathname(entry);
    int slashes = count(path.begin(), path.end(), '/');
    if (slashes < 2) {
        archive_entry_set_pathname(entry, "");
        return;
    }
    if (ocpn::startswith(path, "./")) {
        path = path.substr(2);
    }
    int slashpos = path.find_first_of('/', 1);
    string prefix = path.substr(0, slashpos);
    path = path.substr(prefix.size() + 1);
    slashpos = path.find_first_of('/');
    string location = path.substr(0, slashpos);
    string suffix = path.substr(slashpos + 1);
    if (installPaths.find(location) == installPaths.end()
        && archive_entry_filetype(entry) == AE_IFREG
    ) {
        location = "unknown";
    }
    string dest = installPaths[location] + "/" + suffix;
    archive_entry_set_pathname(entry, dest.c_str());
}



static void linux_entry_set_install_path(struct archive_entry* entry,
                                         pathmap_t installPaths)
{
    using namespace std;

    string path = archive_entry_pathname(entry);
    int slashes = count(path.begin(), path.end(), '/');
    if (slashes < 2) {
        archive_entry_set_pathname(entry, "");
        return;
    }

    int slashpos = path.find_first_of('/', 1);
    if(ocpn::startswith(path, "./"))
        slashpos = path.find_first_of('/', 2);  // skip the './'

    string prefix = path.substr(0, slashpos);
    path = path.substr(prefix.size() + 1);
    if (ocpn::startswith(path, "usr/")) {
        path = path.substr(strlen("usr/"));
    }
    if (ocpn::startswith(path, "local/")) {
        path = path.substr(strlen("local/"));
    }
    slashpos = path.find_first_of('/');
    string location = path.substr(0, slashpos);
    string suffix = path.substr(slashpos + 1);
    if (installPaths.find(location) == installPaths.end()
        && archive_entry_filetype(entry) == AE_IFREG
    ){
        location = "unknown";
    }
    
    string dest = installPaths[location] + "/" + suffix;

    if(g_bportable){
        // A data dir?
        if(ocpn::startswith(location, "share") && ocpn::startswith(suffix, "opencpn/plugins/") ){
            slashpos = suffix.find_first_of("opencpn/plugins/");
            suffix = suffix.substr(16);

            dest = g_Platform->GetPrivateDataDir().ToStdString() + "/plugins/" + suffix;
        }
        if(ocpn::startswith(location, "lib") && ocpn::startswith(suffix, "opencpn/") ){
            suffix = suffix.substr(8);

            dest = g_Platform->GetPrivateDataDir().ToStdString() + "/plugins/lib/" + suffix;
        }
    }
    
    archive_entry_set_pathname(entry, dest.c_str());
}


static void apple_entry_set_install_path(struct archive_entry* entry,
                                         pathmap_t installPaths)
{
    using namespace std;

    const string base = PluginPaths::getInstance()->Homedir()
        + "/Library/Application Support/OpenCPN";

    string path = archive_entry_pathname(entry);
    if(ocpn::startswith(path, "./"))
        path = path.substr(2);

    string dest("");
    size_t slashes = count(path.begin(), path.end(), '/');
    if (slashes < 3) {
        archive_entry_set_pathname(entry, "");
        return;
    }
    auto parts = split(path, "Contents/Resources");
    if (parts.size() >= 2) {
        dest = base  + "/Contents/Resources" + parts[1];
    }
    if (dest == "") {
        parts = split(path, "Contents/SharedSupport");
        if (parts.size() >= 2) {
            dest = base + "/Contents/SharedSupport"  + parts[1];
        }
    }
    if (dest == "") {
        parts = split(path, "Contents/PlugIns");
        if (parts.size() >= 2) {
            dest = base +  "/Contents/PlugIns" + parts[1];
        }
    }
    if (dest == "" && archive_entry_filetype(entry) == AE_IFREG) {
        // Drop uninstalled directories.
        dest = installPaths["unknown"] + "/" + path;
    }
    archive_entry_set_pathname(entry, dest.c_str());
}

static void android_entry_set_install_path(struct archive_entry* entry,
                                         pathmap_t installPaths)
{
    using namespace std;

    string path = archive_entry_pathname(entry);
    int slashes = count(path.begin(), path.end(), '/');
    if (slashes < 2) {
        archive_entry_set_pathname(entry, "");
        return;
    }

    if(path.find("/share") != string::npos)
        int yyp = 4;
    
    int slashpos = path.find_first_of('/', 1);
    if(ocpn::startswith(path, "./"))
        slashpos = path.find_first_of('/', 2);  // skip the './'

    string prefix = path.substr(0, slashpos);
    path = path.substr(prefix.size() + 1);
    if (ocpn::startswith(path, "usr/")) {
        path = path.substr(strlen("usr/"));
    }
    if (ocpn::startswith(path, "local/")) {
        path = path.substr(strlen("local/"));
    }
    slashpos = path.find_first_of('/');
    string location = path.substr(0, slashpos);
    string suffix = path.substr(slashpos + 1);
    if (installPaths.find(location) == installPaths.end()
        && archive_entry_filetype(entry) == AE_IFREG
    ){
        location = "unknown";
    }
    
    if((location == "lib") && ocpn::startswith(suffix, "opencpn")){
        auto parts = split(suffix, "/");
        if(parts.size() == 2)
            suffix = parts[1];
    }

    if((location == "share") && ocpn::startswith(suffix, "opencpn")){
        auto parts = split(suffix, "opencpn/");
        if(parts.size() == 2)
            suffix = parts[1];
    }

    ///storage/emulated/0/android/data/org.opencpn.opencpn/files/opencpn/plugins/oesenc_pi/data/LUPPatch3.xml
    string dest = installPaths[location] + "/" + suffix;

    archive_entry_set_pathname(entry, dest.c_str());
}


static void entry_set_install_path(struct archive_entry* entry,
                                   pathmap_t installPaths)
{
    const std::string src = archive_entry_pathname(entry);
#ifdef __OCPN__ANDROID__
    android_entry_set_install_path(entry, installPaths);
#else
    const auto osSystemId = wxPlatformInfo::Get().GetOperatingSystemId();
    if (g_Platform->isFlatpacked()) {
        flatpak_entry_set_install_path(entry, installPaths);
    }
    else if (osSystemId & wxOS_UNIX_LINUX) {
        linux_entry_set_install_path(entry, installPaths);
    }
    else if (osSystemId & wxOS_WINDOWS) {
        win_entry_set_install_path(entry, installPaths);
    }
    else if (osSystemId & wxOS_MAC) {
        apple_entry_set_install_path(entry, installPaths);
    }
    else {
        wxLogMessage("set_install_path() invoked, unsupported platform %s",
                     wxPlatformInfo::Get().GetOperatingSystemDescription());
    }
#endif
    const std::string dest = archive_entry_pathname(entry);
    if(dest.size()){
        MESSAGE_LOG << "Installing " << src << " into " << dest << std::endl;
    }
}


bool PluginHandler::archive_check(int r, const char* msg, struct archive* a)
{
    if (r < ARCHIVE_OK) {
        std::string s(msg);
        
        if(archive_error_string(a))
            s = s + ": " + archive_error_string(a);
        wxLogMessage(s.c_str());
        last_error_msg = s;
    }
    return r >= ARCHIVE_WARN;
}


bool PluginHandler::explodeTarball(struct archive* src,
                                   struct archive* dest,
                                   std::string& filelist)
{
    struct archive_entry* entry = 0;
    pathmap_t pathmap = getInstallPaths();
    while (true) {
        int r = archive_read_next_header(src, &entry);
        if (r == ARCHIVE_EOF) {
            return true;
        }
        if (!archive_check(r, "archive read header error", src)) {
            return false;
        }
        
        //  Ignore any occurrence of file "metadata.xml"
        std::string path = archive_entry_pathname(entry);
        if(std::string::npos != path.find("metadata.xml"))
            continue;
        
        entry_set_install_path(entry, pathmap);
        if (strlen(archive_entry_pathname(entry)) == 0) {
            continue;
        }
        filelist.append(std::string(archive_entry_pathname(entry)) + "\n");

        r = archive_write_header(dest, entry);
        archive_check(r, "archive write install header error", dest);
        if (r >= ARCHIVE_OK && archive_entry_size(entry) > 0) {
            r = copy_data(src, dest);
            if (!archive_check(r, "archive copy data error", dest)) {
                return false;
            }
        }
        r = archive_write_finish_entry(dest);
        if (!archive_check(r, "archive finish write error", dest)) {
            return false;
        }
        
    }
    return false; // notreached
}


/*
 * Extract tarball into platform-specific user directories.
 *
 * The installed tarball has paths like topdir/dest/suffix_path... e. g.
 * oesenc_pi_ubuntu_10_64/usr/local/share/opencpn/plugins/oesenc_pi/README.
 * In this path, the topdir part must exist but is discarded. Next parts
 * being being standard prefixes like /usr/local or /usr are also
 * discarded. The remaining path (here share) is mapped to a user
 * directory. On linux, it ends up in ~/.local/share. The suffix
 * part is then installed as-is into this directory.
 *
 * Windows tarballs has dll and binary files in the top directory. They
 * go to winInstallDir/Program Files. Message catalogs exists under a
 * share/ toplevel directory, they go in winInstallDir/share. The
 * plugin data is installed under winInstallDir/plugins/<plugin name>,
 * and must be looked up by the plugins using GetPluginDataDir(plugin);
 * Windows requires that PATH is set to include the binary dir and tha
 * a bindtextdomain call is invoked to define the message catalog paths.
 *
 * For linux, the expected destinations are bin, lib and share.
 *
 * Parameters:
 *   - src: Readable libarchive source instance.
 *   - dest: Writable libarchive disk-writer instance.
 *   - filelist: On exit, list of installed files.
 *   - last_error_msg: Updated when returning false.
 *
 */
bool PluginHandler::extractTarball(const std::string path,
                                   std::string& filelist)
{
    struct archive* src = archive_read_new();
    archive_read_support_filter_gzip(src);
    archive_read_support_format_tar(src);
    int r = archive_read_open_filename(src, path.c_str(), 10240);
    if (r != ARCHIVE_OK) {
        std::ostringstream os;
        os << "Cannot read installation tarball: " << path;
        wxLogWarning(os.str().c_str());
        last_error_msg = os.str();
        return false;
    }
    struct archive* dest = archive_write_disk_new();
    archive_write_disk_set_options(dest, ARCHIVE_EXTRACT_TIME);
    bool ok = explodeTarball(src, dest, filelist);
    archive_read_free(src);
    archive_write_free(dest);
    return ok;
}


PluginHandler* PluginHandler::getInstance() {
    static PluginHandler* instance = 0;
    if (!instance) {
        instance = new(PluginHandler);
    }
    return instance;
}


bool PluginHandler::isPluginWritable(std::string name)
{

    if (isRegularFile(PluginHandler::fileListPath(name).c_str())) {
        return true;
    }
    if (!g_pi_manager) {
        return false;
    }
    return PlugInIxByName(name, g_pi_manager->GetPlugInArray()) == -1;
}


std::string PluginHandler::getMetadataPath()
{
    if( metadataPath.size() > 0) {
        return metadataPath;
    }
    std::string path = g_Platform->GetPrivateDataDir().ToStdString();
    path += SEP;
    path += "ocpn-plugins.xml";
    if (ocpn::exists(path)) {
        metadataPath = path;
        return path;
    }
    
    // If default location for composit plugin metadata is not found, 
    //  we look in the plugin cache directory, which will normally contain the last "master" catalog downloaded
    path = g_Platform->GetPrivateDataDir().ToStdString();
    path += SEP;
    path += "plugins" + SEP + "cache" + SEP + "metadata" + SEP + "ocpn-plugins.xml";
    if (ocpn::exists(path)) {
        metadataPath = path;
        return path;
    }
    
    // And if that does not work, use the empty metadata file found in the distribution "data" directory
    metadataPath = g_Platform->GetSharedDataDir().ToStdString();
    metadataPath += SEP ;
    metadataPath += "ocpn-plugins.xml";
    if (!ocpn::exists(metadataPath)) {
        wxLogWarning("Non-existing plugins file: %s", metadataPath);
    }
    return metadataPath;
}

static void parseMetadata(const std::string path, catalog_ctx& ctx)
{
    using namespace std;

    wxLogMessage("PluginHandler: using metadata path: %s", path);
    ctx.depth = 0;
    if (!ocpn::exists(path)) {
        wxLogWarning("Non-existing plugins metadata file: %s", path.c_str());
        return;
    }
    ifstream ifpath(path);
    std::string xml((istreambuf_iterator<char>(ifpath)),
                    istreambuf_iterator<char>());
    ParseCatalog(xml, &ctx);
}

const std::map<std::string, int> PluginHandler::getCountByTarget()
{
    auto plugins = getInstalled();
    auto a = getAvailable();
    plugins.insert(plugins.end(), a.begin(), a.end());
    std::map<std::string, int> count_by_target;
    for (const auto& p: plugins) {
        if (p.target == "") {
            continue;    // Built-in plugins like  dashboard et. al.
        }
        auto key = p.target + ":" + p.target_version;
        if (count_by_target.find(key) == count_by_target.end()) {
            count_by_target[key] = 1;
        }
        else {
            count_by_target[key] += 1;
        }
    }
    return count_by_target;
}

void PluginHandler::cleanupFiles(const std::string& manifestFile,
                                 const std::string& plugname)
{
    std::ifstream diskfiles(manifestFile);
    if (diskfiles.is_open()) {
        std::stringstream buffer;
        buffer << diskfiles.rdbuf();
        PluginHandler::cleanup(buffer.str(), plugname);
    }
}


void PluginHandler::cleanup(const std::string& filelist,
                            const std::string& plugname)
{
    wxLogMessage("Cleaning up failed install of %s", plugname.c_str());
    
    std::istringstream files(filelist);
    while (!files.eof()) {
        char line[256];
        files.getline(line, sizeof(line));
        if (isRegularFile(line)) {
            int r = remove(line);
            if (r != 0) {
                wxLogWarning("Cannot remove file %s: %s", line, strerror(r));
            }
        }
    }
    
        // Make another limited recursive pass, and remove any empty directories
    bool done = false;
    int iloop = 0;
    while(!done && (iloop < 6) ){
        done = true;
        std::istringstream dirs(filelist);
        while (!dirs.eof()) {
            char line[256];
            dirs.getline(line, sizeof(line));
            
            wxFileName wxFile(line);
            if(wxFile.IsDir() && wxFile.DirExists()){
                wxDir dir(wxFile.GetFullPath());
                if(dir.IsOpened()){
                    if(!dir.HasFiles() && !dir.HasSubDirs()){
                        wxFile.Rmdir( wxPATH_RMDIR_RECURSIVE );
                        done = false;
                    }
                }
            }
        }
        iloop++;
    }

    
    
    std::string path = PluginHandler::fileListPath(plugname);
    if (ocpn::exists(path)) {
        remove(path.c_str());
    }
    remove(dirListPath(plugname).c_str());  // Best effort try, failures
    remove(PluginHandler::versionPath(plugname).c_str());  // are non-critical.
}


const std::vector<PluginMetadata> PluginHandler::getAvailable()
{
    using namespace std;
    catalog_ctx ctx;

    auto catalogHandler = CatalogHandler::getInstance();
    
    //std::string path = g_Platform->GetPrivateDataDir().ToStdString();
    //path += SEP;
    //path += "ocpn-plugins.xml";
    std::string path = getMetadataPath();
    if (!ocpn::exists(path)) {
        return ctx.plugins;
    }
    std::ifstream file;
    file.open(path, std::ios::in);
    if (file.is_open()) {
        std::string xml((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
        file.close();
        auto status = catalogHandler->DoParseCatalog(xml, &ctx);
        if (status == CatalogHandler::ServerStatus::OK) {
            catalogData.undef = false;
            catalogData.version = ctx.version;
            catalogData.date = ctx.date;
        }
    }

    return ctx.plugins;
}


const std::vector<PluginMetadata> PluginHandler::getInstalled()
{
    using namespace std;
    vector<PluginMetadata> plugins;

    if (g_pi_manager) {
        ArrayOfPlugIns* mgr_plugins = g_pi_manager->GetPlugInArray();
        for (unsigned int i = 0; i < mgr_plugins->GetCount(); i += 1) {
            PlugInContainer* p = mgr_plugins->Item(i);
            PluginMetadata plugin;
            auto name = string(p->m_common_name);
            //std::transform(name.begin(), name.end(), name.begin(), ::tolower);
            plugin.name = name;
            std::stringstream ss;
            ss << p->m_version_major << "." << p->m_version_minor;
            plugin.version = ss.str();
            plugin.readonly = !isPluginWritable(plugin.name);
            string path = PluginHandler::versionPath(plugin.name);
            if (path != "" && wxFileName::IsFileReadable(path)) {
                std::ifstream stream;
                stream.open(path, ifstream::in);
                stream >> plugin.version;
            }
            plugins.push_back(plugin);
        }
    }
    return plugins;
}


bool PluginHandler::installPlugin(PluginMetadata plugin, std::string path)
{
    std::string filelist;
    if ( !extractTarball(path, filelist)) {
        std::ostringstream os;
        os << "Cannot unpack plugin: " << plugin.name  << " at " << path;
        last_error_msg = os.str();
        PluginHandler::cleanup(filelist, plugin.name);
        return false;
    }
    //remove(path.c_str());
    saveFilelist(filelist, plugin.name);
    saveDirlist(plugin.name);
    saveVersion(plugin.name, plugin.version);

    return true;
}


bool PluginHandler::installPlugin(PluginMetadata plugin)
{
    std::string path;
    char fname[4096];

    if (tmpnam(fname) == NULL) {
        wxLogWarning("Cannot create temporary file");
        path = "";
        return false;
    }
    path = std::string(fname);
    std::ofstream stream;
    stream.open(path.c_str(), std::ios::out|std::ios::binary|std::ios::trunc);
    DEBUG_LOG << "Downloading: " << plugin.name << std::endl;
    auto downloader = Downloader(plugin.tarball_url);
    downloader.download(&stream);

    return installPlugin(plugin, path);
}


bool PluginHandler::uninstall(const std::string plugin_name)
{
    using namespace std;

    auto ix = PlugInIxByName(plugin_name, g_pi_manager->GetPlugInArray());
    auto pic = g_pi_manager->GetPlugInArray()->Item(ix);
    //g_pi_manager->ClosePlugInPanel(pic, wxID_OK);
    g_pi_manager->UnLoadPlugIn(ix);
    string path = PluginHandler::fileListPath(plugin_name);
    if (!ocpn::exists(path)) {
        wxLogWarning("Cannot find installation data for %s (%s)",
                     plugin_name.c_str(), path);
        return false;
    }
    ifstream files(path);
    while (!files.eof()) {
        char line[256];
        files.getline(line, sizeof(line));
        if (isRegularFile(line)) {
            int r = remove(line);
            if (r != 0) {
                wxLogWarning("Cannot remove file %s: %s", line, strerror(r));
            }
        }
    }
    files.close();
    
    // Make another limited recursive pass, and remove any empty directories
    bool done = false;
    int iloop = 0;
    while(!done && (iloop < 6) ){
        done = true;
        ifstream dirs(path);
        while (!dirs.eof()) {
            char line[256];
            dirs.getline(line, sizeof(line));
            string dirc(line);
            
            wxFileName wxFile(line);
            if(wxFile.IsDir() && wxFile.DirExists()){
                wxDir dir(wxFile.GetFullPath());
                if(dir.IsOpened()){
                    if(!dir.HasFiles() && !dir.HasSubDirs()){
                        wxFile.Rmdir( wxPATH_RMDIR_RECURSIVE );
                        done = false;
                    }
                }
            }
        }
        dirs.close();
        
        iloop++;
    }
    
    int r = remove(path.c_str());
    if (r != 0) {
        wxLogWarning("Cannot remove file %s: %s", path.c_str(), strerror(r));
    }
    remove(dirListPath(plugin_name).c_str());  // Best effort try, failures
    remove(PluginHandler::versionPath(plugin_name).c_str());  // are OK.

    return true;
}

bool PluginHandler::installPluginFromCache( PluginMetadata plugin )
{
    // Look for the desired file
    wxURI uri( wxString(plugin.tarball_url.c_str()));
    wxFileName fn(uri.GetPath());
    wxString tarballFile = fn.GetFullName();
    wxString sep = _T("/");
    wxString cacheFile = g_Platform->GetPrivateDataDir() + sep + _T("plugins")
                            + sep + _T("cache") + sep + _T("tarballs")
                            +sep + tarballFile;
 
   if(wxFileExists( cacheFile)){
        wxLogMessage("Installing %s from local cache",  tarballFile.c_str());
        bool bOK = installPlugin( plugin, cacheFile.ToStdString());
        if(!bOK){
            wxLogWarning("Cannot install tarball file %s", cacheFile.c_str());
             wxString message = _("Please check system log for more info.");
            OCPNMessageBox(gFrame, message, _("Installation error"), wxICON_ERROR | wxOK | wxCENTRE);

            return false;
        }
        
        wxString message;
        message.Printf("%s %s\n", plugin.name.c_str(),  plugin.version.c_str());
        message += _(" successfully installed from cache");
        OCPNMessageBox(gFrame, message, _("Installation complete"), wxICON_INFORMATION | wxOK | wxCENTRE);

        return true;
   }
 
   return false;
}
