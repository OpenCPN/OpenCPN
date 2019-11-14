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

struct PluginMetadata {
    std::string name;
    std::string version;
    std::string release;
    std::string summary;

    std::string api_version;
    std::string author;
    std::string description;
    std::string git_commit ;
    std::string git_date;
    std::string source;
    std::string tarball_url;
    std::string target;
    std::string target_version;
    std::string info_url;

    bool openSource;

    bool readonly;                // Can plugin be removed?
    int ix;                       // Index in list of installed or available.
    void clear() { *this = PluginMetadata(); }
    PluginMetadata() :  readonly(true), ix(-1) {}


};

/** Overall metadata for the set of plugins used. */
struct CatalogData {
    std::string version;
    std::string date;
};

class PluginHandler {

    public:
        static PluginHandler* getInstance();

        /** Check if given plugin can be installed/updated. */
        bool isPluginWritable(std::string name);

        /** Return list of all installed plugins. */
        const std::vector<PluginMetadata> getInstalled();

        /** Return list of available, not installed plugins. */
        const std::vector<PluginMetadata> getAvailable();

        /** Return path to metadata XML file. */
        std::string getMetadataPath();

        /** Set path to metadata XML file. */
        void setMetadata(std::string path) { metadataPath = path; }

        /** Download and install a new, not installed plugin. */
        bool install(PluginMetadata plugin);

        /** Install a new, downloaded but not installed plugin tarball. */
        bool install(PluginMetadata plugin, std::string path);

	/** Uninstall an installed plugin. */
        bool uninstall(const std::string plugin);

        std::string getLastErrorMsg();

        /** Plugin catalog metadata for given path, default current file. */
        CatalogData getCatalogData(const char* path = 0);

    protected:
	/** Initiats the handler and set up LD_LIBRARY_PATH. */
        PluginHandler() {}

    private:
        std::string metadataPath;
        std::vector<PluginMetadata> installed;
        CatalogData catalogData;

};


#endif // PLUGIN_HANDLER_H__
