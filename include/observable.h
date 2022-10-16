/*************************************************************************
 *
 * Project:  OpenCPN
 * Purpose: General observable implementation with several specializations.
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

#ifndef OBSERVABLE_H
#define OBSERVABLE_H

#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include <wx/event.h>

#include "observable_evt.h"

#ifndef DECL_EXP
#if defined(_MSC_VER) || defined(__CYGWIN__)
#define DECL_EXP __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
#define DECL_EXP __attribute__((visibility("default")))
#else
#define DECL_EXP
#endif
#endif    // DECL_EXP


/** Return address as printable string. */
std::string ptr_key(const void* ptr);


class ObservedVar;
class ObservedVarListener;

/**
 *  Private helper class. Basically a singleton map of listener lists
 *  where lists are managed by key, one for each key value.
 */
class ListenersByKey {
friend class ObservedVar;
friend ListenersByKey& getInstance(const std::string& key);

public:
  ListenersByKey() {}

private:
  static ListenersByKey& getInstance(const std::string& key);

  ListenersByKey(const ListenersByKey&) = delete;
  ListenersByKey& operator=(const ListenersByKey&) = default;

  std::vector<std::pair<wxEvtHandler*, wxEventType>> listeners;

};


/**  The observable notify/listen basic nuts and bolts.  */
class ObservedVar {
friend class ObservedVarListener;

public:
  ObservedVar(const std::string& _key)
      : key(_key), m_list(ListenersByKey::getInstance(_key)) {}

  /** Notify all listeners about variable change. */
  virtual const void Notify();

  /**
   * Remove window listening to ev from list of listeners.
   * @return true if such a listener existed, else false.
   */
  bool Unlisten(wxEvtHandler* listener, wxEventType ev);

  /** The key used to create and clone. */
  const std::string key;

protected:
  /**
   * Notify all listeners: send them a 'type' ObservedEvt message
   * as defined by listen() with optional data available using GetString()
   * and/or GetClientData().
   */
  const void Notify(std::shared_ptr<const void> ptr, const std::string& s,
                    int num, void* client_data);

  const void Notify(const std::string& s, void* client_data) {
      Notify(nullptr, s, 0, client_data);
  }

  const void Notify(std::shared_ptr<const void> p) { Notify(p, "", 0, 0); }

private:
  /** Set object to send ev_type to listener on variable changes. */
  void Listen(wxEvtHandler* listener, wxEventType ev_type);

  ListenersByKey& m_list;

  mutable std::mutex m_mutex;
};


/**
 *  Keeps listening over it's lifespan, removes itself on destruction.
 */
class DECL_EXP ObservedVarListener final {
public:
  /** Default constructor, does not listen to anything. */
  ObservedVarListener() : key(""), listener(0), ev_type(wxEVT_NULL) {}

  /** Set object to send wxEventType ev to handler on changes in key. */
  void Listen(const std::string& key, wxEvtHandler* listener, wxEventType evt);

  ObservedVarListener(const ObservedVarListener& other)
      : key(other.key), listener(other.listener), ev_type(other.ev_type) {
      Listen();
      }

  ~ObservedVarListener() { Unlisten(); };

private:
  void Listen();
  void Unlisten();

  std::string key;
  wxEvtHandler* listener;
  wxEventType ev_type;
};

/** Shorthand for accessing ObservedEvt.SharedPtr(). */
template <typename T>
std::shared_ptr<const T> UnpackEvtPointer(ObservedEvt ev) {
  return std::static_pointer_cast<const T>(ev.GetSharedPtr());
}


#endif  // OBSERVABLE_H
