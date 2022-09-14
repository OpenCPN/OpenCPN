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
 *  Private helper class. Basically a singleton map of listeners where
 *  singletons are managed by key, one for each key value.
 */
class ListenersByKey {
public:
  ListenersByKey() {}

private:
  static ListenersByKey& getInstance(const std::string& key);

  ListenersByKey(const ListenersByKey&) = delete;
  ListenersByKey& operator=(const ListenersByKey&) = default;

  std::vector<std::pair<wxEvtHandler*, wxEventType>> listeners;

friend class ObservedVar;
friend ListenersByKey& getInstance(const std::string& key);
};


/**  The observable notify/listen basic nuts and bolts.  */
class ObservedVar {
public:
  ObservedVar(const std::string& _key)
      : key(_key), singleton(ListenersByKey::getInstance(_key)) {}

  /** Notify all listeners about variable change. */
  virtual const void notify();

  /**
   * Remove window listening to ev from list of listeners.
   * @return true if such a listener existed, else false.
   */
  bool unlisten(wxEvtHandler* listener, wxEventType ev);

  /** The key used to create and clone. */
  const std::string key;

  /** Shorthand for ObservedVarListener(this, handler, event_type) CTOR: */
  ObservedVarListener GetListener(wxEvtHandler* handler, wxEventType ev);

protected:
  /**
   * Notify all listeners: send them a 'type' ObservedEvt message
   * as defined by listen() with optional data available using GetString()
   * and/or GetClientData().
   */
  const void notify(std::shared_ptr<const void> ptr, const std::string& s,
                    int num, void* client_data);

  const void notify(const std::string& s, void* client_data) {
      notify(nullptr, s, 0, client_data);
  }
  const void notify(std::shared_ptr<const void> p) { notify(p, "", 0, 0); }

private:
  /** Set object to send ev_type to listener on variable changes. */
  void listen(wxEvtHandler* listener, wxEventType ev_type);

  ListenersByKey& singleton;

  friend class ObservedVarListener;
};


/**
 *  Keeps listening over it's lifespan, removes itself on destruction.
 */
class DECL_EXP ObservedVarListener final {
public:
  /** Default constructor, does not listen to anything. */
  ObservedVarListener() : key(""), listener(0), ev_type(wxEVT_NULL) {}

  /** Set object to send wxEventType ev to handler on variable changes. */
  ObservedVarListener(ObservedVar* v, wxEvtHandler* w, wxEventType ev)
    : key(v->key), listener(w), ev_type(ev) { listen(); }

  ObservedVarListener(const ObservedVarListener& other) { copy(other); }

  ~ObservedVarListener() { unlisten(); };

  void operator=(const ObservedVarListener& other) {
    unlisten();
    copy(other);
  }

private:
  void listen();
  void unlisten();
  void copy(const ObservedVarListener& other);

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
