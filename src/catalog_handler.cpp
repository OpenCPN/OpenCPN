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

#include "config.h"

#include <fstream>
#include <sstream>

#include <wx/filename.h>
#include <wx/jsonreader.h>
#include <wx/log.h>

#include <curl/curl.h>

#include "catalog_handler.h"
#include "catalog_parser.h"
#include "Downloader.h"
#include "ocpn_utils.h"
#include "OCPNPlatform.h"
#include "PluginHandler.h"


#ifdef _WIN32
static const std::string SEP("\\");
#else
static const std::string SEP("/");
#endif


extern wxString                 g_catalog_custom_url;
extern wxString                 g_catalog_channel;
extern OCPNPlatform*            g_Platform;

//static const char* const DOWNLOAD_REPO = "https://raw.githubusercontent.com/leamas/plugins";
static const char* const DOWNLOAD_REPO = "https://raw.githubusercontent.com/opencpn/plugins";

static const char* const DOWNLOAD_PATH = "/@branch@/ocpn-plugins.xml";

static const char* const API_ENDPOINT = "https://api.github.com/repos";
//static const char* const API_PATH = "/leamas/plugins/branches";
static const char* const API_PATH = "/opencpn/plugins/branches";


CatalogHandler::CatalogHandler()
    : status(ServerStatus::UNKNOWN)
{
    if (g_catalog_channel == "") {
        g_catalog_channel = DEFAULT_CHANNEL;
    }
}


CatalogHandler* CatalogHandler::getInstance()
{
    static CatalogHandler* instance = 0;
    if (!instance) {
        instance = new(CatalogHandler);
    }
    return instance;
}


std::string CatalogHandler::GetDefaultUrl()
{
    std::string url = std::string(DOWNLOAD_REPO) + DOWNLOAD_PATH;
    ocpn::replace(url, "@branch@", g_catalog_channel.ToStdString());
    return url;
}


catalog_status CatalogHandler::DownloadCatalog(std::ostream* stream)
{
    std::string path(g_catalog_custom_url.ToStdString());
    if (path == "") {
        path = std::string(DOWNLOAD_REPO) + DOWNLOAD_PATH;
        ocpn::replace(path, "@branch@", g_catalog_channel.ToStdString());
        wxLogMessage("Effective catalog path: %s", path.c_str());
    }
    Downloader downloader(path);
    bool ok = downloader.download(stream);
    if (ok) {
        return ServerStatus::OK;
    } 
    error_msg = downloader.last_error();
    return ServerStatus::CURL_ERROR;
}

catalog_status CatalogHandler::DownloadCatalog(std::ostream* stream, std::string url)
{
    Downloader downloader(url);
    bool ok = downloader.download(stream);
    if (ok) {
        return ServerStatus::OK;
    } 
    error_msg = downloader.last_error();
    return ServerStatus::CURL_ERROR;
}


catalog_status CatalogHandler::DownloadCatalog(std::string& filePath)
{
    if (filePath == "") {
        filePath = wxFileName::CreateTempFileName("ocpn_dl").ToStdString();
    }
    std::ofstream stream;
    stream.open(filePath.c_str(), std::ios::out | std::ios::trunc);
    if (!stream.is_open()) {
        wxLogMessage("CatalogHandler: Cannot open %s for write", filePath);
        error_msg = strerror(errno);
        return ServerStatus::OS_ERROR;
    }
    auto status = DownloadCatalog(&stream);
    stream.close();
    return status;
}

catalog_status CatalogHandler::DownloadCatalog(std::string& filePath, std::string url)
{
    if (filePath == "") {
        filePath = wxFileName::CreateTempFileName("ocpn_dl").ToStdString();
    }
    std::ofstream stream;
    stream.open(filePath.c_str(), std::ios::out | std::ios::trunc);
    if (!stream.is_open()) {
        wxLogMessage("CatalogHandler: Cannot open %s for write", filePath);
        error_msg = strerror(errno);
        return ServerStatus::OS_ERROR;
    }
    auto status = DownloadCatalog(&stream, url);
    stream.close();
    return status;
}


catalog_status CatalogHandler::ParseCatalog(const std::string xml, bool latest)
{
    catalog_ctx ctx;
    bool ok = ::ParseCatalog(xml, &ctx);
    if (ok && latest) {
        this->latest_data.version = ctx.version;
        this->latest_data.date = ctx.date;
        this->latest_data.undef = false;
        return ServerStatus::OK;
    }
    wxLogWarning("Cannot parse xml starting with: %s", xml.substr(0,60).c_str());
    return ok ? ServerStatus::OK : ServerStatus::XML_ERROR;

}


std::vector<std::string> CatalogHandler::GetChannels()
{
    return channels;
}


bool CatalogHandler::SetActiveChannel(const char* channel)
{
    for (auto c: channels) {
        if (c == channel) {
            g_catalog_channel = channel;
            return true;
        }
    }
    wxLogMessage("Attempt to set illegal active channel: %s", channel);
    return false;
}


std::string CatalogHandler::GetActiveChannel()
{
    return g_catalog_channel.ToStdString();
}


void CatalogHandler::SetCustomUrl(const char* url)
{
    g_catalog_custom_url = url;
}


CatalogData CatalogHandler::LatestCatalogData()
{
    if (latest_data.undef) {
        std::ostringstream os;
        if (DownloadCatalog(&os) == ServerStatus::OK) {
            ParseCatalog(os.str());
        }
    }
    return latest_data;
}


void 
CatalogHandler::LoadCatalogData(const std::string& path, CatalogData& data)
{
    if (!ocpn::exists(path)) {
        data.version= "?";
        data.date = "?";
        data.undef = false;
        return;
    }
    std::ifstream file;
    file.open(path, std::ios::in);
    if (file.is_open()) {
        std::string xml((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
        file.close();
        catalog_ctx ctx;
        if (::ParseCatalog(xml, &ctx)) {
            data.version = ctx.version;
            data.date = ctx.date;
            data.undef = false;
        }
    }
}


CatalogData CatalogHandler::UserCatalogData()
{
    if (user_data.undef) {
        auto plugin_handler = PluginHandler::getInstance();
        std::string path = g_Platform->GetPrivateDataDir().ToStdString();
        path += SEP;
        path += "ocpn-plugins.xml";
        LoadCatalogData(path, user_data);
    }
    return user_data;
}


CatalogData CatalogHandler::DefaultCatalogData()
{
    if (default_data.undef) {
        auto plugin_handler = PluginHandler::getInstance();
        std::string path = g_Platform->GetSharedDataDir().ToStdString();
        path += SEP;
        path += "ocpn-plugins.xml";
        LoadCatalogData(path, default_data);
    }
    return default_data;
}

void CatalogHandler::ClearCatalogData()
{
    default_data.undef = true;
    user_data.undef = true;
    latest_data.undef = true;
}

std::string CatalogHandler::GetCustomUrl()
{
     return g_catalog_custom_url.ToStdString();
}


std::string CatalogHandler::LastErrorMsg()
{
    return error_msg;
}


catalog_status CatalogHandler::LoadChannels(std::ostream* stream)
{
    Downloader downloader(std::string(API_ENDPOINT) + API_PATH);
    bool ok = downloader.download(stream);
    if (ok) {
        return ServerStatus::OK;
    } 
    error_msg = downloader.last_error();
    return ServerStatus::CURL_ERROR;
}


catalog_status CatalogHandler::LoadChannels(const std::string& json)
{
    wxJSONValue node;
    wxJSONReader parser;
    parser.Parse(json.c_str(),  &node);
    if (!node.IsArray()) {
        wxLogMessage("Cannot parse json (toplevel)");
        error_msg = parser.GetErrors().Item(0).ToStdString();
        return ServerStatus::JSON_ERROR;
    }
    auto branches = node.AsArray();
    wxLogMessage("Got %d branches", branches->Count());
    channels.clear();
    for (size_t i = 0; i < branches->Count(); i += 1) {
        auto branch = branches->Item(i);
        channels.push_back(branch["name"].AsString().ToStdString());
    }
    if (branches->Count() > 0) {
        wxLogMessage("First branch: %s", channels[0].c_str());
    }
    return ServerStatus::OK;
}
