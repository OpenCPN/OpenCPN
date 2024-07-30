/**************************************************************************
 *   Copyright (C) 2024 Alec Leamas                                        *
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

/** \file  mdns_cache.cpp Implement mdns_cache.h */

#include <algorithm>

#include <curl/curl.h>

#include "model/logger.h"
#include "model/mdns_cache.h"

/**
 * Check if we can connect to given host/port, does not
 * care if we cannot recieve data.
 */
static bool Ping(const std::string& url, long port = 8443L) {
  CURL* c = curl_easy_init();
  curl_easy_setopt(c, CURLOPT_URL, url.c_str());
  curl_easy_setopt(c, CURLOPT_PORT, port);
  curl_easy_setopt(c, CURLOPT_TIMEOUT_MS, 2000L);
  CURLcode result = curl_easy_perform(c);
  curl_easy_cleanup(c);
  bool ok = result == CURLE_RECV_ERROR || result == CURLE_OK;
  DEBUG_LOG << "Checked mdns host: " << url << ": "
            << (ok ? "ok" : curl_easy_strerror(result));
  return ok;
}

MdnsCache& MdnsCache::GetInstance() {
  static MdnsCache mdns_cache;
  return mdns_cache;
}

bool MdnsCache::Add(const Entry& entry) {
  std::unique_lock lock(m_mutex);
  auto found = std::find_if(m_cache.begin(), m_cache.end(),
                            [entry](Entry& e) { return e.ip == entry.ip; });
  DEBUG_LOG << "Added mdns cache entry, ip: " << entry.ip
            << ", status: " << (found == m_cache.end() ? "true" : "false");
  if (found != m_cache.end()) return false;
  m_cache.push_back(entry);
  return true;
}

bool MdnsCache::Add(const std::string& service, const std::string& host,
                    const std::string& _ip, const std::string& _port) {
  return Add(Entry(service, host, _ip, _port));
}

void MdnsCache::Validate() {
  std::unique_lock lock(m_mutex);
  for (auto it = m_cache.begin(); it != m_cache.end();) {
    if (!Ping(it->ip)) {
      m_cache.erase(it);
    } else {
      it++;
    }
  }
}
