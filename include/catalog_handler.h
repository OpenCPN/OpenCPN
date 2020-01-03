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
 * Plugin catalog management: Check for available versions and branches,
 * download as required.
 */

#ifndef CATALOG_HANDLER_H__
#define CATALOG_HANDLER_H__

#include <string>
#include <memory>
#include <ostream>
#include <vector>

#include "catalog_parser.h"


/**
 * A local proxy for the catalog server. The server has a number
 * of branches, some of which containing a plugin catalog.
 *
 * Backend code for channel management (which catalog to get) and
 * the important download function. 
 *
 * Also: CatalogData handling, basically version and date for 
 * various ocpn-plugins.xml.
 */
class CatalogHandler {

    public:
        
        enum class ServerStatus {
            UNKNOWN, OK, OK_MSG, CURL_ERROR, JSON_ERROR, XML_ERROR, OS_ERROR
        };

        static CatalogHandler* getInstance();

        /** Download channel json data, possibly return error code. */
        ServerStatus LoadChannels(std::ostream* json);

        /** Parse and store json channel data, possibly return error code. */
        ServerStatus LoadChannels(const std::string& json);

        /** Get the downloaded list of channels, empty on errors. */
        std::vector<std::string> GetChannels();

        /** Set the active channel used when downloading catalog. */
        bool SetActiveChannel(const char* channel);

        /** Get the branch (a. k. a. channel) used to download. */
        std::string GetActiveChannel();

        /** Set a custom url, overrides also channel settings. */
        void SetCustomUrl(const char* url);

        /** Set a custom url, overrides also channel settings. */
        std::string  GetCustomUrl();

        /** Get the default URL, with actual channel included */
        std::string GetDefaultUrl();

        /** Download the latest catalog to given stream. */
        ServerStatus DownloadCatalog(std::ostream* stream);

        /** Download the latest catalog to local path. */
        ServerStatus DownloadCatalog(std::string& path);

        /** Download the specified catalog to given stream. */
        ServerStatus DownloadCatalog(std::ostream* stream, std::string url);

        /** Download the specified catalog to local path. */
        ServerStatus DownloadCatalog(std::string& filePath, std::string url);

        /** Parse XML contents, save as latest data if latest is true. */
        ServerStatus ParseCatalog(const std::string xml, bool latest = false);

        /** Data for default version, installed with main opencpn. */
        CatalogData DefaultCatalogData();

        /** Data for user catalog which overrides the default one. */
        CatalogData UserCatalogData();

        /** Data for latest parsed data marked as latest. */
        CatalogData LatestCatalogData();

        /** Invalidate *CatalogData caches */
        void ClearCatalogData();

        /** Last error message, free format. */
        std::string LastErrorMsg();


    protected:
	/** Initiate the handler. */
        CatalogHandler();

        void LoadCatalogData(const std::string& path, CatalogData& data);

        const char* const GET_BRANCHES_PATH =
            "/repos/OpenCPN/plugins/branches";
        const char* const GITHUB_API =
            "https://api.github.com";

        const char* const REPO_URL = 
            "https://raw.githubusercontent.com";
        const char* const REPO_PATH = 
            "/OpenCPN/plugins/@branch@/ocpn-plugins.xml";

        const char* const DEFAULT_CHANNEL = "master";

    private:
        std::vector<std::string> channels;
        ServerStatus status;
        std::ostream* stream;
        std::string error_msg;
        CatalogData latest_data;
        CatalogData default_data;
        CatalogData user_data;
};

typedef CatalogHandler::ServerStatus catalog_status;


#endif // CATALOG_HANDLER_H__
