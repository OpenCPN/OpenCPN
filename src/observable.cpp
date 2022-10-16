/*************************************************************************
 *
 * Project: OpenCPN
 * Purpose: Implement observable.h
 *
 * Copyright (C) 2022 Alec Leamas
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

#include <algorithm>
#include <atomic>
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


ListenersByKey& ListenersByKey::getInstance(const std::string& key) {
  static std::unordered_map<std::string, ListenersByKey> instances;
  static std::mutex s_mutex;

  std::lock_guard<std::mutex> lock(s_mutex);
  if (instances.find(key) == instances.end()) {
    instances[key] = ListenersByKey();
  }
  return instances[key];
}


/* ObservedVar implementation. */

using ev_pair = std::pair<wxEvtHandler*, wxEventType>;

void ObservedVar::Listen(wxEvtHandler* listener, wxEventType ev_type) {
  std::lock_guard<std::mutex> lock(m_mutex);
  const auto& listeners = m_list.listeners;
  ev_pair key_pair(listener, ev_type);
  if (wxLog::GetLogLevel() <= wxLOG_Debug) {
    auto count = std::count(listeners.begin(), listeners.end(), key_pair);
    if (count > 2) {
        // There are two occurences when assigning, the source is assumed
        // to go away and remove one occurence.
        wxLogMessage("Duplicate listener, key: %s, listener: %s, ev_type: %d",
                     key, ptr_key(listener), ev_type);
    }
  }
  m_list.listeners.push_back(key_pair);
}

bool ObservedVar::Unlisten(wxEvtHandler* listener, wxEventType ev_type) {
  std::lock_guard<std::mutex> lock(m_mutex);
  auto& listeners = m_list.listeners;

  ev_pair key_pair(listener, ev_type);
  auto found = std::find(listeners.begin(), listeners.end(), key_pair);
  if (found == listeners.end()) return false;
  listeners.erase(found);
  if (wxLog::GetLogLevel() <= wxLOG_Debug) {
    auto count = std::count(listeners.begin(), listeners.end(), key_pair);
    if (count > 1) {
        wxLogMessage("Duplicate listener, key: %s, listener: %s, ev_type: %d",
                     key, ptr_key(listener), ev_type);
    }
  }
  return true;
}

const void ObservedVar::Notify(std::shared_ptr<const void> ptr,
                               const std::string& s, int num,
                               void* client_data) {
  std::lock_guard<std::mutex> lock(m_mutex);
  auto& listeners = m_list.listeners;
  for (auto l = listeners.begin(); l != listeners.end(); l++) {
    auto evt = new ObservedEvt(l->second);
    evt->SetSharedPtr(ptr);
    evt->SetClientData(client_data);
    evt->SetString(s.c_str());  // Better safe than sorry: force a deep copy
    evt->SetInt(num);
    wxQueueEvent(l->first, evt);
  }
}

const void ObservedVar::Notify() { Notify("", 0); }

using Listener = ObservedVarListener;

/* ObservedVarListener implementation. */

void ObservedVarListener::Listen(const std::string& k, wxEvtHandler* l,
                                 wxEventType e) {
  if (key != "") Unlisten();
  key = k;
  listener = l;
  ev_type = e;
  Listen();
}

void ObservedVarListener::Listen() {
  if (key != "") {
    assert(listener);
    ObservedVar var(key);
    var.Listen(listener, ev_type);
  }
}

void ObservedVarListener::Unlisten() {
  if (key != "") {
    assert(listener);
    ObservedVar var(key);
    var.Unlisten(listener, ev_type);
  }
}
