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

/**
 * Handle plugin install from remote repositories and local operations
 * to uninstall and list plugins. The plugin maintains an internal list
 * and is a singleton.
 *
 * Remote repositories are based on XML files describing metadata and
 * addresses to tarballs with compiled and installed files, basically
 * under there different directories.
 *
 *    - The plugin directory e. g., /usr/lib/opencpn/
 *    - The data directory e. g., /usr/share/data/opencpn/plugins.
 *    - The binary directory (additional program) e. g, /usr/bin.
 *
 * The compiled plugins are installed to user-writable location(s)
 *    
 *    - Linux: under ~/.local, possibly relocated by environment variables.
 *    - Windows: under &GetWinPluginBaseDir() which is configurable,
 *      defaults to %APPDATA%/Local/opencpn
 *    - Flatpak: under ~/.var/app/org.opencpn.OpenCPN
 *
 * However, plugins are loaded from multiple locations basically 
 * corresponding to new and old lacations:
 *
 *    - Windows: GetSharedDataDirPtr():GetWinPluginBaseDir()
 *    - linux: As defined  by XDG_DATA_DIRS, defaulting to
 *      ~/.local/lib/opencpn:/usr/local/lib/opencpn:/usr/lib/opencpn
 *    - flatpak:
 *        /app/lib/opencpn:/app/extensions/lib/opencpn:~/.var/app/opencpn/lib
 *        
 */

#ifndef PLUGIN_HANDLER_H__
#define PLUGIN_HANDLER_H__

#include <string>
#include <memory>
#include <vector>

#include <wx/cmdline.h>

#include <archive.h>

#include "catalog_parser.h"

//  Some useful static functions
extern bool isRegularFile(const char* path);
extern std::string fileListPath(std::string name);
extern std::string versionPath(std::string name);
extern void cleanup(const std::string& filelist, const std::string& plugname);

class PluginHandler {

    public:
        static PluginHandler* getInstance();

        /** Check if given plugin can be installed/updated. */
        bool isPluginWritable(std::string name);

        /** Return list of all installed plugins. */
        const std::vector<PluginMetadata> getInstalled();

        /** Return list of available, not installed plugins. */
        const std::vector<PluginMetadata> getAvailable();

        /** Return list of available, unique plugins from configured XML catalog */
        std::vector<PluginMetadata> getAvailableUniquePlugins();

        /** Return path to metadata XML file. */
        std::string getMetadataPath();

        /** Set path to metadata XML file. */
        void setMetadata(std::string path) { metadataPath = path; }

        /** Download and install a new, not installed plugin. */
        bool installPlugin(PluginMetadata plugin);

        /** Install a new, downloaded but not installed plugin tarball. */
        bool installPlugin(PluginMetadata plugin, std::string path);

	/** Uninstall an installed plugin. */
        bool uninstall(const std::string plugin);

        std::string getLastErrorMsg() { return last_error_msg; }

        CatalogData *GetCatalogData(){ return &catalogData; }
        
    protected:
	/** Initiats the handler and set up LD_LIBRARY_PATH. */
        PluginHandler() { m_sOsLike = ""; }

    private:
        std::string metadataPath;
        std::vector<PluginMetadata> installed;
        CatalogData catalogData;
        std::string last_error_msg;
        bool explodeTarball(struct archive* src,
                            struct archive* dest,
                            std::string& filelist);
        bool extractTarball(const std::string path, std::string& filelist);
        bool archive_check(int r, const char* msg, struct archive* a);

        wxString m_sOsLike;
        void find_compat_target(const std::string& plugin_target)
        {
            if (m_sOsLike != "") {
                return;
            }
            if (getenv("OPENCPN_COMPAT_TARGET") != 0) {
                // Undocumented test hook.
                m_sOsLike = getenv("OPENCPN_COMPAT_TARGET");
                return;
            }
            if (plugin_target != "ubuntu") {
                return;
            }
            wxFile file("/etc/os-release");
            if(!file.IsOpened()) {
                return;
            }
            wxString l_InString;
            if(file.ReadAll(&l_InString)) {
                // Find OS_LIKE in string
                int l_nPos = l_InString.Find("ID_LIKE=");
                if(l_nPos != wxNOT_FOUND) {
                    l_nPos += 8;
                    int l_nEnd = l_InString.find('\n', l_nPos);
                    m_sOsLike.append(l_InString.SubString(l_nPos, l_nEnd - 1));
                }
            }
            file.Close();
        }

};

/**
 * Used to compare plugin versions. Versions are basically semantic
 * versioning: major.minor.revision.build for example 1.2.6.1-deadbee. The
 * values major, minor and revision should be integers. The build is a
 * free-format string sorted lexically.
 *
 * Note: The version installed is saved in text files since it's not
 * available in the plugin interface besides major.minor. See
 * https://github.com/OpenCPN/OpenCPN/issues/1443
 */
class OcpnVersion
{
    int major;
    int minor;
    int revision;
    std::string build;

public:
    OcpnVersion();

    OcpnVersion(std::string version_release);
    OcpnVersion(int major, int minor, int revision=0, std::string build = "");
    std::string to_string();

    bool operator < (const OcpnVersion& other)
    {
            if (major < other.major) return true;
            if (minor < other.minor) return true;
            if (revision < other.revision) return true;
            if (build < other.build) return true;
            return false;
    }

    bool operator == (const OcpnVersion& other)
    {
            return major == other.major
                && minor == other.minor
                && revision == other.revision
                && build == other.build;
    }

    bool operator > (const OcpnVersion& other)
    {
            return !(*this == other) && !(*this < other);
    }

    bool operator <= (const OcpnVersion& other)
    {
            return (*this == other) || (*this < other);
    }

    bool operator >= (const OcpnVersion& other)
    {
            return (*this == other) || (*this > other);
    }

    bool operator != (const OcpnVersion& other)
    {
            return !(*this == other);
    }

    friend std::ostream& operator << (std::ostream& s, const OcpnVersion& v)
    {
            s << v.major << '.' << v.minor << '.' << v.revision;
            if (v.build != "" ) {
                s << '.' << v.build;
            }
            return s;
    }

};


#endif // PLUGIN_HANDLER_H__
