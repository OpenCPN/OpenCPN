/*************************************************************************
 *
 *
 * Copyright (C) 2022 - 2025 Alec Leamas
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.
 **************************************************************************/

/**
 *\file
 *
 * Implement observable.h
 */


#include <algorithm>
#include <mutex>
#include <sstream>
#include <unordered_map>

#include <wx/log.h>

#include "observable.h"

std::string ptr_key(const void* ptr) {
  std::ostringstream oss;
  oss << ptr;
  return oss.str();
}

/* ListenersByKey implementation. */

ListenersByKey& ListenersByKey::GetInstance(const std::string& key) {
  static std::unordered_map<std::string, ListenersByKey> instances;
  static std::mutex s_mutex;

  std::lock_guard<std::mutex> lock(s_mutex);
  if (instances.find(key) == instances.end()) {
    instances[key] = ListenersByKey();
  }
  return instances[key];
}

/* Observable implementation. */

using ev_pair = std::pair<wxEvtHandler*, wxEventType>;

void Observable::Listen(wxEvtHandler* listener, wxEventType ev_type) {
  std::lock_guard<std::mutex> lock(m_mutex);
  const auto& listeners = m_list.listeners;

  ev_pair key_pair(listener, ev_type);
  auto found = std::find(listeners.begin(), listeners.end(), key_pair);
  assert((found == listeners.end()) && "Duplicate listener");
  m_list.listeners.push_back(key_pair);
}

bool Observable::Unlisten(wxEvtHandler* listener, wxEventType ev_type) {
  std::lock_guard<std::mutex> lock(m_mutex);
  auto& listeners = m_list.listeners;

  ev_pair key_pair(listener, ev_type);
  auto found = std::find(listeners.begin(), listeners.end(), key_pair);
  if (found == listeners.end()) return false;
  listeners.erase(found);
  return true;
}

void Observable::Notify(const std::shared_ptr<const void>& ptr,
                              const std::string& s, int num,
                              void* client_data) {
  std::lock_guard<std::mutex> lock(m_mutex);
  auto& listeners = m_list.listeners;

  for (const auto& l : listeners) {
    auto evt = new ObservedEvt(l.second);
    evt->SetSharedPtr(ptr);
    evt->SetClientData(client_data);
    evt->SetString(s.c_str());  // Better safe than sorry: force a deep copy
    evt->SetInt(num);
    wxQueueEvent(l.first, evt);
  }
}

void Observable::Notify() { Notify("", nullptr); }

/* ObservableListener implementation. */

void ObservableListener::Listen(const std::string& k, wxEvtHandler* l,
                                wxEventType e) {
  if (!key.empty()) Unlisten();
  key = k;
  listener = l;
  ev_type = e;
  Listen();
}

void ObservableListener::Listen() {
  if (!key.empty()) {
    assert(listener);
    Observable(key).Listen(listener, ev_type);
  }
}

void ObservableListener::Unlisten() {
  if (!key.empty()) {
    assert(listener);
    Observable(key).Unlisten(listener, ev_type);
    key = "";
  }
}
