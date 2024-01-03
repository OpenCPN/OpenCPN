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

#include <algorithm>
#include <fstream>
#include <sstream>

#include <wx/filename.h>
#include <wx/jsonreader.h>
#include <wx/log.h>

#include "model/catalog_handler.h"
#include "model/catalog_parser.h"
#include "model/downloader.h"
#include "observable_evtvar.h"
#include "observable_globvar.h"
#include "model/ocpn_utils.h"
#include "model/base_platform.h"
#include "model/plugin_handler.h"

#ifdef _WIN32
static const std::string SEP("\\");
#else
static const std::string SEP("/");
#endif

extern wxString g_catalog_custom_url;
extern wxString g_catalog_channel;
extern BasePlatform* g_BasePlatform;

static const char* const DOWNLOAD_REPO =
    "https://raw.githubusercontent.com/OpenCPN/plugins";

static const char* const DOWNLOAD_PATH = "/@branch@/ocpn-plugins.xml";

static const char* const API_ENDPOINT = "https://api.github.com/repos";
// static const char* const API_PATH = "/leamas/plugins/branches";
static const char* const API_PATH = "/OpenCPN/plugins/branches";

CatalogHandler::CatalogHandler() :
     status(ServerStatus::UNKNOWN),
     m_catalog_status(ServerStatus::UNKNOWN){
  if (g_catalog_channel == "") {
    g_catalog_channel = DEFAULT_CHANNEL;
  }
}

CatalogHandler* CatalogHandler::getInstance() {
  static CatalogHandler* instance = 0;
  if (!instance) {
    instance = new (CatalogHandler);
  }
  return instance;
}

std::string CatalogHandler::GetDefaultUrl() {
  std::string url = std::string(DOWNLOAD_REPO) + DOWNLOAD_PATH;
  ocpn::replace(url, "@branch@", g_catalog_channel.ToStdString());
  return url;
}

catalog_status CatalogHandler::GetCatalogStatus(){
    return m_catalog_status;
}

CatalogCtx *CatalogHandler::GetActiveCatalogContext() {

  if (m_catalog_status == ServerStatus::OK){
    return &m_catalogctx;
  }

  auto path = PluginHandler::getInstance()->getMetadataPath();

  if (!ocpn::exists(path)) {
    m_catalog_status = ServerStatus::FILE_ERROR;
  }
  std::ifstream file;
  file.open(path, std::ios::in);
  if (file.is_open()) {
    std::string xml((std::istreambuf_iterator<char>(file)),
                    std::istreambuf_iterator<char>());
    file.close();
    auto status = DoParseCatalog(xml, &m_catalogctx);
    m_catalog_status = status;
    return &m_catalogctx;
  }

  return &m_catalogctx;
}

bool CatalogHandler::AddMetadataToActiveContext(PluginMetadata metadata) {
  if (m_catalog_status == ServerStatus::OK) {
    m_catalogctx.plugins.push_back(metadata);
    return true;
  }
  else return false;
}


catalog_status CatalogHandler::DownloadCatalog(std::ostream* stream) {
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

catalog_status CatalogHandler::DownloadCatalog(std::ostream* stream,
                                               std::string url) {
  Downloader downloader(url);
  bool ok = downloader.download(stream);
  if (ok) {
    return ServerStatus::OK;
  }
  error_msg = downloader.last_error();
  return ServerStatus::CURL_ERROR;
}

catalog_status CatalogHandler::DownloadCatalog(std::string& filePath) {
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

catalog_status CatalogHandler::DownloadCatalog(std::string& filePath,
                                               std::string url) {
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

catalog_status CatalogHandler::DoParseCatalog(const std::string xml,
                                              CatalogCtx* ctx) {
  std::string url;

  bool ok = ::ParseCatalog(xml, ctx);
  for (auto path : PluginHandler::getInstance()->GetImportPaths()) {
    std::ifstream plugin_xml(path);
    std::stringstream ss;
    ss << plugin_xml.rdbuf();
    PluginMetadata metadata;
    if (ss.str().size() == 0) {
      continue;
    }
    ::ParsePlugin(ss.str().c_str(), metadata);
    metadata.is_imported = true;
    ctx->plugins.push_back(metadata);
  }
  while (ctx->meta_urls.size() > 0) {
    std::ostringstream xml;
    url = ctx->meta_urls.back();
    ctx->meta_urls.pop_back();

    // already parsed this meta file?
    auto match = [url](const std::string& s) { return url == s; };
    const auto& haystack = ctx->parsed_metas;
    auto found = std::find_if(haystack.begin(), haystack.end(), match);
    if (found != haystack.end()) {
        continue;
    }
    ctx->parsed_metas.push_back(url);
    if (DownloadCatalog(&xml, url) != ServerStatus::OK) {
      wxLogMessage("CatalogHandler: Cannot download meta-url: %s",
                   url.c_str());
    } else {
      ok = DoParseCatalog(xml.str(), ctx) == ServerStatus::OK;
      if (!ok) break;
    }
  }
  if (!ok) {
    wxLogWarning("Cannot parse xml starting with: %s",
                 xml.substr(0, 60).c_str());
  }
  return ok ? ServerStatus::OK : ServerStatus::XML_ERROR;
}

catalog_status CatalogHandler::ParseCatalog(const std::string xml,
                                            bool latest) {
  CatalogCtx ctx;
  auto status = DoParseCatalog(xml, &ctx);
  if (status == ServerStatus::OK && latest) {
    this->latest_data.version = ctx.version;
    this->latest_data.date = ctx.date;
    this->latest_data.undef = false;
  }
  return status;
}

std::vector<std::string> CatalogHandler::GetChannels() { return channels; }

bool CatalogHandler::SetActiveChannel(const char* channel) {
  for (auto c : channels) {
    if (c == channel) {
      GlobalVar<wxString> catalog_channel(&g_catalog_channel);
      catalog_channel.Set(channel);
      return true;
    }
  }
  wxLogMessage("Attempt to set illegal active channel: %s", channel);
  return false;
}

std::string CatalogHandler::GetActiveChannel() {
  return g_catalog_channel.ToStdString();
}

void CatalogHandler::SetCustomUrl(const char* url) {
  g_catalog_custom_url = url;
}

CatalogData CatalogHandler::LatestCatalogData() {
  if (latest_data.undef) {
    std::ostringstream os;
    if (DownloadCatalog(&os) == ServerStatus::OK) {
      ParseCatalog(os.str());
    }
  }
  return latest_data;
}

void CatalogHandler::LoadCatalogData(const std::string& path,
                                     CatalogData& data) {
  if (!ocpn::exists(path)) {
    data.version = "?";
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
    CatalogCtx ctx;
    auto status = DoParseCatalog(xml, &ctx);
    if (status == ServerStatus::OK) {
      data.version = ctx.version;
      data.date = ctx.date;
      data.undef = false;
    }
  }
}

CatalogData CatalogHandler::UserCatalogData() {
  if (user_data.undef) {
    auto plugin_handler = PluginHandler::getInstance();
    std::string path = g_BasePlatform->GetPrivateDataDir().ToStdString();
    path += SEP;
    path += "ocpn-plugins.xml";
    LoadCatalogData(path, user_data);
  }
  return user_data;
}

CatalogData CatalogHandler::DefaultCatalogData() {
  if (default_data.undef) {
    auto plugin_handler = PluginHandler::getInstance();
    std::string path = g_BasePlatform->GetSharedDataDir().ToStdString();
    path += SEP;
    path += "ocpn-plugins.xml";
    LoadCatalogData(path, default_data);
  }
  return default_data;
}

void CatalogHandler::ClearCatalogData() {
  default_data.undef = true;
  user_data.undef = true;
  latest_data.undef = true;
  m_catalog_status = ServerStatus::UNKNOWN;

  m_catalogctx.plugins.clear();
  m_catalogctx.meta_urls.clear();
  m_catalogctx.parsed_metas.clear();
  m_catalogctx.version.clear();
  m_catalogctx.date.clear();


}

std::string CatalogHandler::GetCustomUrl() {
  return g_catalog_custom_url.ToStdString();
}

std::string CatalogHandler::LastErrorMsg() { return error_msg; }

catalog_status CatalogHandler::LoadChannels(std::ostream* stream) {
  Downloader downloader(std::string(API_ENDPOINT) + API_PATH);
  bool ok = downloader.download(stream);
  if (ok) {
    return ServerStatus::OK;
  }
  error_msg = downloader.last_error();
  return ServerStatus::CURL_ERROR;
}

catalog_status CatalogHandler::LoadChannels(const std::string& json) {
  wxJSONValue node;
  wxJSONReader parser;
  parser.Parse(json.c_str(), &node);
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
