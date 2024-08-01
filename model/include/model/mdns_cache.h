
/***************************************************************************
 *   Copyright (C) 2024  Alec Leamas                                       *
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

/**
 * Singleton cache for hosts looked up using mdns. A critical region accessed
 * both by timer routines and the main thread.
 *
 * Entries are added by the various Add() signatures. The Validate() method
 * removes all entries where to host does not respond to a http request on
 * on port 8443.
 */

#ifndef MDNS_CACHE_H
#define MDNS_CACHE_H

#include <mutex>
#include <string>
#include <vector>

class MdnsCache {
public:
  struct Entry {
    std::string service_instance;
    std::string hostname;
    std::string ip;
    std::string port;
    Entry(const std::string& service, const std::string host,
          const std::string& _ip, const std::string _port)
        : service_instance(service), hostname(host), ip(_ip), port(_port) {}
  };

  static MdnsCache& GetInstance();

  MdnsCache& operator=(MdnsCache&) = delete;
  MdnsCache(const MdnsCache&) = delete;

  /** Check that all entries are accessible, remove stale ones. */
  void Validate();

  /**
   * Add new entry to the cache
   * @return true if entry was added, false if entry with same ip
   * address already exists.
   */
  bool Add(const Entry& entry);

  /**
   * Add new entry to the cache
   * @return true if entry was added, false if entry with same ip
   * address already exists.
   */
  bool Add(const std::string& service, const std::string& host,
           const std::string& _ip, const std::string& _port);

  /**
   * Add a manual entry where only IP and port is known.
   * @return true if entry was added, false if entry with same ip
   * address already exists.
   */
  bool Add(const std::string& _ip,  const std::string& _port);

  /** Return read-only cached entries reference. */
  const std::vector<Entry>& GetCache() const { return m_cache; }

private:
  mutable std::mutex m_mutex;
  std::vector<Entry> m_cache;

  MdnsCache() = default;
};

#endif  // MDNS_CACHE_H
