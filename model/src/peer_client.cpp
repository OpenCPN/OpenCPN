/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Peer-peer data sharing.
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David Register                                  *
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
 **************************************************************************/

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>

#include <curl/curl.h>

#include <wx/fileconf.h>
#include <wx/json_defs.h>
#include <wx/jsonreader.h>
#include <wx/log.h>
#include <wx/string.h>

#include "model/config_vars.h"
#include "model/nav_object_database.h"
#include "model/peer_client.h"
#include "model/ocpn_utils.h"
#include "model/rest_server.h"
#include "model/semantic_vers.h"
#include "observable_confvar.h"

struct MemoryStruct {
  char* memory;
  size_t size;
  MemoryStruct() {
    memory = (char*)malloc(1);
    size = 0;
  }
  ~MemoryStruct() { free(memory); }
};

using PeerDlgPair = std::pair<PeerDlgResult, std::string>;

PeerData::PeerData(EventVar& p)
    : overwrite(false),
      activate(false),
      progress(p),
      run_status_dlg([](PeerDlg, int) { return PeerDlgResult::Cancel; }),
      run_pincode_dlg([] { return PeerDlgPair(PeerDlgResult::Cancel, ""); }) {}

static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb,
                                  void* userp) {
  size_t realsize = size * nmemb;
  struct MemoryStruct* mem = (struct MemoryStruct*)userp;

  char* ptr = (char*)realloc(mem->memory, mem->size + realsize + 1);
  if (!ptr) {
    /* out of memory! */
    std::cerr << "not enough memory (realloc returned NULL)\n";
    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

static int xfer_callback(void* clientp, [[maybe_unused]] curl_off_t dltotal,
                         [[maybe_unused]] curl_off_t dlnow, curl_off_t ultotal,
                         curl_off_t ulnow) {
  auto peer_data = static_cast<PeerData*>(clientp);
  if (ultotal == 0) {
    peer_data->progress.Notify(0, "");
  } else {
    peer_data->progress.Notify(100 * ulnow / ultotal, "");
  }
// FIXME (leamas) dirty fix for outdated, bundled curl
// returning 0 is undocumented, but worked for  5.8
#ifdef CURL_PROGRESSFUNC_CONTINUE
  return CURL_PROGRESSFUNC_CONTINUE;
#else
  return 0;
#endif
}

/**
 *  Perform a POST operation on server, store possible reply in response.
 *  @return positive http status or negated CURLcode error
 */
static long ApiPost(const std::string& url, const std::string& body,
                    PeerData& peer_data, MemoryStruct* response) {
  long response_code = -1;
  peer_data.progress.Notify(0, "");

  CURL* c = curl_easy_init();
  // No encoding, plain ASCII
  curl_easy_setopt(c, CURLOPT_ENCODING, "identity");  // Plain ASCII
  curl_easy_setopt(c, CURLOPT_URL, url.c_str());
  curl_easy_setopt(c, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(c, CURLOPT_SSL_VERIFYHOST, 0L);

  curl_easy_setopt(c, CURLOPT_POSTFIELDSIZE, body.size());
  curl_easy_setopt(c, CURLOPT_COPYPOSTFIELDS, body.c_str());
  curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(c, CURLOPT_WRITEDATA, (void*)response);
  curl_easy_setopt(c, CURLOPT_NOPROGRESS, 0);
  curl_easy_setopt(c, CURLOPT_XFERINFODATA, &peer_data);
  curl_easy_setopt(c, CURLOPT_XFERINFOFUNCTION, xfer_callback);
  curl_easy_setopt(c, CURLOPT_TIMEOUT, 20);
  // FIXME (leamas) always logs
  curl_easy_setopt(c, CURLOPT_VERBOSE,
                   wxLog::GetLogLevel() >= wxLOG_Debug ? 1 : 0);

  CURLcode result = curl_easy_perform(c);
  peer_data.progress.Notify(0, "");
  if (result == CURLE_OK)
    curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &response_code);

  curl_easy_cleanup(c);
  return response_code == -1 ? -static_cast<long>(result) : response_code;
}

/**
 * Perform a GET operation on server, store possible reply in chunk.
 * @return positive http status or negated CURLcode error
 */
static int ApiGet(const std::string& url, const MemoryStruct* chunk,
                  int timeout = 0) {
  int response_code = -1;

  CURL* c = curl_easy_init();
  curl_easy_setopt(c, CURLOPT_ENCODING, "identity");  // Encoding: plain ASCII
  curl_easy_setopt(c, CURLOPT_URL, url.c_str());
  curl_easy_setopt(c, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(c, CURLOPT_SSL_VERIFYHOST, 0L);
  curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(c, CURLOPT_WRITEDATA, (void*)chunk);
  curl_easy_setopt(c, CURLOPT_NOPROGRESS, 1);
  if (timeout != 0) curl_easy_setopt(c, CURLOPT_TIMEOUT, timeout);
  CURLcode result = curl_easy_perform(c);
  if (result == CURLE_OK)
    curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &response_code);
  curl_easy_cleanup(c);
  return response_code == -1 ? -static_cast<long>(result) : response_code;
}

static std::string GetClientKey(std::string& server_name) {
  ConfigVar<std::string> server_keys("/Settings/RESTClient", "ServerKeys",
                                     TheBaseConfig());
  auto key_string = server_keys.Get("");
  auto entries = ocpn::split(key_string.c_str(), ";");
  for (const auto& entry : entries) {
    auto server_key = ocpn::split(entry.c_str(), ":");
    if (server_key.size() != 2) continue;
    if (server_key[0] == server_name) return server_key[1];
  }
  return "1";
}

static void SaveClientKey(std::string& server_name, std::string key) {
  ConfigVar<std::string> server_keys("/Settings/RESTClient", "ServerKeys",
                                     TheBaseConfig());
  auto config_server_keys = server_keys.Get("");

  auto server_keys_list = ocpn::split(config_server_keys.c_str(), ";");
  std::unordered_map<std::string, std::string> key_by_server;
  for (const auto& item : server_keys_list) {
    auto server_and_key = ocpn::split(item.c_str(), ":");
    if (server_and_key.size() != 2) continue;
    key_by_server[server_and_key[0]] = server_and_key[1];
  }
  key_by_server[server_name] = key;

  config_server_keys = "";
  for (const auto& it : key_by_server) {
    config_server_keys += it.first + ":" + it.second + ";";
  }
  server_keys.Set(config_server_keys);
  wxLog::FlushActive();
}
static RestServerResult ParseServerJson(const MemoryStruct& reply,
                                        PeerData& peer_data) {
  wxString body(reply.memory);
  wxJSONValue root;
  wxJSONReader reader;
  int num_errors = reader.Parse(body, &root);
  if (num_errors != 0) {
    for (const auto& error : reader.GetErrors()) {
      wxLogMessage("Json server reply parse error: %s",
                   error.ToStdString().c_str());
    }
    peer_data.run_status_dlg(PeerDlg::JsonParseError, num_errors);
    peer_data.api_version = SemanticVersion(-1, -1);
    return RestServerResult::Void;
  }
  if (root.HasMember("version")) {
    auto s = root["version"].AsString().ToStdString();
    peer_data.api_version = SemanticVersion::parse(s);
  }
  if (root.HasMember("result")) {
    return static_cast<RestServerResult>(root["result"].AsInt());
  } else {
    return RestServerResult::Void;
  }
}

bool CheckKey(const std::string& key, PeerData peer_data) {
  std::stringstream url;
  url << "https://" << peer_data.dest_ip_address << "/api/ping"
      << "?source=" << g_hostname << "&apikey=" << key;
  MemoryStruct reply;
  long status = ApiGet(url.str(), &reply, 5);
  if (status != 200) {
    peer_data.run_status_dlg(PeerDlg::InvalidHttpResponse, status);
    return false;
  }
  auto result = ParseServerJson(reply, peer_data);
  return result != RestServerResult::NewPinRequested;
}

void GetApiVersion(PeerData& peer_data) {
  if (peer_data.api_version > SemanticVersion(5, 0)) return;
  std::stringstream url;
  url << "https://" << peer_data.dest_ip_address << "/api/get-version";

  struct MemoryStruct chunk;
  std::string buf;
  long response_code = ApiGet(url.str(), &chunk, 2);

  if (response_code == 200) {
    ParseServerJson(chunk, peer_data);
  } else {
    // Return "old" version without /api/writable support
    peer_data.api_version = SemanticVersion(5, 8);
  }
}

/** Return a usable api key, possibly after user dialogs. */
static bool GetApiKey(PeerData& peer_data, std::string& key) {
  std::string api_key;
  if (peer_data.api_version == SemanticVersion(0, 0)) GetApiVersion(peer_data);

  while (true) {
    api_key = GetClientKey(peer_data.server_name);
    std::stringstream url;
    url << "https://" << peer_data.dest_ip_address << "/api/ping"
        << "?source=" << g_hostname << "&apikey=" << api_key;
    MemoryStruct chunk;
    int status = ApiGet(url.str(), &chunk, 3);
    if (status != 200) {
      auto r = peer_data.run_status_dlg(PeerDlg::InvalidHttpResponse, status);
      if (r == PeerDlgResult::Ok) continue;
      return false;
    }
    auto result = ParseServerJson(chunk, peer_data);
    switch (result) {
      case RestServerResult::NewPinRequested: {
        auto pin_result = peer_data.run_pincode_dlg();
        if (pin_result.first == PeerDlgResult::HasPincode) {
          std::string tentative_pin = ocpn::trim(pin_result.second);
          unsigned int_pin = atoi(tentative_pin.c_str());
          Pincode pincode(int_pin);
          api_key = pincode.Hash();
          GetApiVersion(peer_data);
          if (peer_data.api_version < SemanticVersion(5, 9)) {
            api_key = pincode.CompatHash();
          }
          if (!CheckKey(api_key, peer_data)) {
            auto r = peer_data.run_status_dlg(PeerDlg::BadPincode, 0);
            if (r == PeerDlgResult::Ok) continue;
            return false;
          }
          SaveClientKey(peer_data.server_name, api_key);
        } else if (pin_result.first == PeerDlgResult::Cancel) {
          return false;
        } else {
          auto r = peer_data.run_status_dlg(PeerDlg::ErrorReturn,
                                            static_cast<int>(result));
          if (r == PeerDlgResult::Ok) continue;
          return false;
        }
      } break;
      case RestServerResult::GenericError:
        // 5.8 returns GenericError for a valid key (!)
        [[fallthrough]];
      case RestServerResult::NoError:
        break;
      default:
        auto r = peer_data.run_status_dlg(PeerDlg::ErrorReturn,
                                          static_cast<int>(result));
        if (r == PeerDlgResult::Ok) continue;
        return false;
    }
    break;
  }
  key = api_key;
  return true;
}

/** Convert PeerData routes, tracks and waypoints to GPX XML format. */
static std::string PeerDataToXml(PeerData& peer_data) {
  NavObjectCollection1 gpx;
  std::ostringstream stream;
  int total = peer_data.routes.size() + peer_data.tracks.size() +
              peer_data.routepoints.size();
  int gpxgen = 0;
  for (auto r : peer_data.routes) {
    gpxgen++;
    gpx.AddGPXRoute(r);
    peer_data.progress.Notify(100 * gpxgen / total, "");
    wxYield();
  }
  for (auto r : peer_data.routepoints) {
    gpxgen++;
    gpx.AddGPXWaypoint(r);
    peer_data.progress.Notify(100 * gpxgen / total, "");
    wxYield();
  }
  for (auto r : peer_data.tracks) {
    gpxgen++;
    gpx.AddGPXTrack(r);
    peer_data.progress.Notify(100 * gpxgen / total, "");
    wxYield();
  }
  gpx.save(stream, PUGIXML_TEXT(" "));
  return stream.str();
}

/** Actually transfer body. */
static void SendObjects(std::string& body, const std::string& api_key,
                        PeerData& peer_data) {
  bool cancel = false;
  while (!cancel) {
    std::stringstream url;
    url << "https://" << peer_data.dest_ip_address << "/api/rx_object"
        << "?source=" << g_hostname << "&apikey=" << api_key;
    if (peer_data.overwrite) url << "&force=1";
    if (peer_data.activate) url << "&activate=1";

    struct MemoryStruct chunk;
    long response_code = ApiPost(url.str(), body, peer_data, &chunk);
    if (response_code == 200) {
      wxString json(chunk.memory);
      wxJSONValue root;
      wxJSONReader reader;

      int num_errors = reader.Parse(json, &root);
      if (num_errors > 0)
        wxLogDebug("SendObjects, parse errors: %d", num_errors);
      // Capture the result
      int result = root["result"].AsInt();
      if (result > 0) {
        peer_data.run_status_dlg(PeerDlg::ErrorReturn, result);
      } else {
        peer_data.run_status_dlg(PeerDlg::TransferOk, 0);
      }
      cancel = true;
    } else {
      peer_data.run_status_dlg(PeerDlg::InvalidHttpResponse, response_code);
      cancel = true;
    }
  }
}

/** Parse json message in chunk, return "result" from server. */
static int CheckChunk(struct MemoryStruct& chunk, const std::string& guid) {
  wxString body(chunk.memory);
  wxJSONValue root;
  wxJSONReader reader;
  int num_errors = reader.Parse(body, &root);
  if (num_errors > 0)
    wxLogDebug("CheckChunk: parsing errors found: %d", num_errors);
  int result = root["result"].AsInt();
  if (result != 0) {
    wxLogDebug("Server rejected guid %s, status: %d", guid.c_str(), result);
    return result;
  }
  return 0;
}

/** Return true if server accepts overwriting all peer_data objects. */
static bool CheckObjects(const std::string& api_key, PeerData& peer_data) {
  std::stringstream url;
  url << "https://" << peer_data.dest_ip_address << "/api/writable"
      << "?source=" << g_hostname << "&apikey=" << api_key << "&guid=";
  for (const auto& r : peer_data.routes) {
    std::string guid = r->GetGUID().ToStdString();
    std::string full_url = url.str() + guid;
    struct MemoryStruct chunk;
    if (ApiGet(full_url, &chunk) != 200) {
      wxLogMessage("Cannot check /api/writable for route %s", guid.c_str());
      return false;
    }
    int result = CheckChunk(chunk, guid);
    if (result != 0) return false;
  }
  for (const auto& t : peer_data.tracks) {
    std::string guid = t->m_GUID.ToStdString();
    std::string full_url = url.str() + guid;
    struct MemoryStruct chunk;
    if (ApiGet(full_url, &chunk) != 200) {
      wxLogMessage("Cannot check /api/writable for track %s", guid.c_str());
      return false;
    }
    int result = CheckChunk(chunk, guid);
    if (result != 0) return false;
  }
  for (const auto& rp : peer_data.routepoints) {
    std::string guid = rp->m_GUID.ToStdString();
    std::string full_url = url.str() + guid;
    struct MemoryStruct chunk;
    if (ApiGet(full_url, &chunk) != 200) {
      wxLogMessage("Cannot check /api/writable for waypoint %s", guid.c_str());
      return false;
    }
    int result = CheckChunk(chunk, guid);
    if (result != 0) return false;
  }
  return true;
}

bool SendNavobjects(PeerData& peer_data) {
  if (peer_data.routes.empty() && peer_data.routepoints.empty() &&
      peer_data.tracks.empty()) {
    return true;
  }
  std::string api_key;
  bool apikey_ok = GetApiKey(peer_data, api_key);
  if (!apikey_ok) return false;
  if (peer_data.api_version < SemanticVersion(5, 9) && peer_data.activate) {
    peer_data.run_status_dlg(PeerDlg::ActivateUnsupported, 0);
    return false;
  }
  std::string body = PeerDataToXml(peer_data);
  SendObjects(body, api_key, peer_data);
  return true;
}

bool CheckNavObjects(PeerData& peer_data) {
  if (peer_data.routes.empty() && peer_data.routepoints.empty() &&
      peer_data.tracks.empty()) {
    return true;  // the server will not object to null transfers.
  }
  std::string apikey;
  bool apikey_ok = GetApiKey(peer_data, apikey);
  if (!apikey_ok) return false;
  return CheckObjects(apikey, peer_data);
}
