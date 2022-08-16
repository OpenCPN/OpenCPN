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
#include <vector>

#include <wx/config.h>
#include <wx/event.h>

/** Return address as printable string. */
std::string ptr_key(const void* ptr);

/* The event use by  notify/listen. */
class ObservedEvt;

wxDECLARE_EVENT(obsNOTIFY, ObservedEvt);

/** Adds a std::shared<void> element to wxCommandEvent. */
class ObservedEvt : public wxCommandEvent {
public:
  ObservedEvt(wxEventType commandType = obsNOTIFY, int id = 0)
    : wxCommandEvent(commandType, id) {}
  ObservedEvt(const ObservedEvt& event)
    : wxCommandEvent(event) {this->m_shared_ptr = event.m_shared_ptr; }

  wxEvent* Clone() const { return new ObservedEvt(*this); }

  std::shared_ptr<void> GetSharedPtr() const { return m_shared_ptr; }

  void SetSharedPtr(std::shared_ptr<void> p) { m_shared_ptr = p; }

private:
  std::shared_ptr<void> m_shared_ptr;
};


class ObservedVar;
class ObservedVarListener;


/**
 *  Private helper class. Basically a singleton map of listeners where
 *  singletons are managed by key, one for each key value.
 */
class ListenersByKey {

private:
  static ListenersByKey* getInstance(const std::string& key);
  std::vector<std::pair<wxEvtHandler*, wxEventType>>  listeners;

  ListenersByKey() {}
  ListenersByKey(const ListenersByKey&) = delete;
  void operator=(const ListenersByKey&) = delete;

  friend class ObservedVar;
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
  const void notify(std::shared_ptr<void> ptr, const std::string& s,
                    int num, void* client_data);

  const void notify(const std::string& s, void* client_data) {
      notify(nullptr, s, 0, client_data);
  }

private:
  /** Set object to send ev_type to listener on variable changes. */
  void listen(wxEvtHandler* listener, wxEventType ev_type);

  ListenersByKey* const singleton;

  friend class ObservedVarListener;
};


/**
 *  Keeps listening over it's lifespan, removes itself on destruction.
 */
class ObservedVarListener final {
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
  void listen() {
    if (key != "") {
      assert(listener);
      ObservedVar var(key);
      var.listen(listener, ev_type);
    }
  }

  void unlisten() {
    if (key != "") {
      assert(listener);
      ObservedVar var(key);
      var.unlisten(listener, ev_type);
    }
  }

  void copy(const ObservedVarListener& other) {
    listener = other.listener;
    key = other.key;
    ev_type = other.ev_type;
    listen();
  }

  std::string key;
  wxEvtHandler* listener;
  wxEventType ev_type;
};


/**
 * Generic event handling between MVC Model and Controller based on a
 * shared EventVar variable
 *
 *  Model usage:
 *
 *   class Model: ...
 *   public:
 *     EventVar change;
 *
 *     void some_method() {
 *       ...
 *       change.notify("new value")
 *     }
 *
 *  Controller/GUI usage:
 *
 *    class Gui: public wxEvtHandler {
 *    public:
 *      Gui:Gui(Model& model) {
 *        wxDEFINE_EVENT(EVT_FOO, wxCommandEvent);
 *        change_listener = model.change.GetListener(this, EVT_FOO);
 *        Bind(EVT_FOO, [&](wxCommandEvent ev) {
 *          auto s = ev.GetString();    s -> "new value"
 *          ... do something;
 *        });
 *      }
 *
 *    private:
 *      ObservedVarListener change_listener;
 *    }
 */
class EventVar : public ObservedVar {
public:
  EventVar() : ObservedVar(autokey()) {}

  /** Notify all listeners, no data supplied. */
  const void notify() { ObservedVar::notify("", 0); }

  /** Notify all listeners about variable change with ClientData. */
  const void notify(void* data) { ObservedVar::notify("", data); }

  /** Notify all listeners about variable change with a string. */
  const void notify(const std::string& s) { ObservedVar::notify(s, 0); }

private:
  std::string autokey();
};


/**
 *  Wrapper for configuration variables which lives in a wxBaseConfig
 *  object. Supports int, bool, double, std::string and wxString. Besides
 *  basic set()/get() also provides notification events when value changes.
 *
 *  Client usage when reading, setting a value and notifying listeners:
 *
 *    ConfigVar<bool> expert("/PlugIns", "CatalogExpert", &g_pConfig);
 *    bool old_value = expert.get(false);
 *    expert.set(false);
 *
 *  Client usage, listening to value changes.
 *
 *    class Foo: public wxEventHandler {
 *    public:
 *      Foo(...) {
 *        ConfigVar<bool> expert("/PlugIns", "CatalogExpert", &g_pConfig);
 *
 *        // expert sends a wxCommandEvent of type EVT_FOO to this on changes:
 *        wxDEFINE_EVENT(EVT_FOO, wxCommandEvent);
 *        expert_listener = expert.GetListener(this, EVT_FOO);
 *
 *        // Handle  EVT_FOO as any event when it arrives, for example:
 *        Bind(EVT_FOO, [](wxCommandEvent&) { cout << "value has changed"; });
 *        ...
 *      }
 *    private:
 *      ObservedVarListener expert_listener;
 *      ...
 *    }
 *
 */
template <typename T = std::string>
class ConfigVar : public ObservedVar {
public:
  ConfigVar(const std::string& section_, const std::string& key_,
            wxConfigBase* cb);

  void set(const T& arg);

  const T get(const T& default_val);

private:
  ConfigVar();  // not implemented

  const std::string section;
  const std::string key;
  wxConfigBase* const config;
};


/**
 *  Wrapper for global variable, supports notification events when value
 *  changes.
 *
 *  Client usage, writing a value + notifying listeners:
 *
 *    GlobalVar<wxString> compat_os(&g_compatOS);
 *    compat_os.set("ubuntu-gtk3-x86_64");
 *
 *  Client usage, modifying a value + notifying listeners:
 *
 *    GlobalVar<wxString> plugin_array_var(&plugin_array);
 *    plugin_array.Add(new_pic);
 *    plugin_array_var.notify();
 *
 *  Client usage, listening to value changes:
 *
 *    class Foo: public wxEvtHandler {
 *    public:
 *      Foo(...) {
 *        GlobalVar<wxString> compat_os(&g_compatOS);
 *
 *        // compat_os sends a wxCommandEvent type EVT_FOO to this on changes:
 *        wxDEFINE_EVENT(EVT_FOO, wxCommandEvent);
 *        compat_os_listener = compat_os.GetListener(this, EVT_FOO);)
 *
 *        // Handle  EVT_FOO as any event when it arrives, for example:
 *        Bind(EVT_FOO, [](wxCommandEvent&) { cout << "value has changed"; });
 *        ...
 *      }
 *    private:
 *      ObservedVarListener compat_os_listener;
 *      ...
 *    }
 */
template <typename T>
class GlobalVar : public ObservedVar {
public:
  GlobalVar(T* ptr) : ObservedVar(ptr_key(ptr)), variable(ptr) {}

  void set(const T& arg) {
    *variable = arg;
    ObservedVar::notify();
  }

  const T get() { return *variable; }

private:
  GlobalVar();  // not implemented

  T* const variable;
};

#endif  // OBSERVABLE_H
