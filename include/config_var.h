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
#include <unordered_map>
#include <vector>

#include <wx/config.h>
#include <wx/event.h>

namespace ocpn {

/** Return address as printable string. */
std::string ptr_key(const void* ptr);


class ObservedVar;
class ObservedVarListener;


/**
 *  Private helper class. Basically a singleton map of listeners where
 *  singletons are managed by key, one for each key value.
 */
class SingletonVar {

private:
  static SingletonVar* getInstance(const std::string& key);
  std::unordered_map<wxEvtHandler*, wxEventType> listeners;

  SingletonVar() {}
  SingletonVar(const SingletonVar&);    // not implemented
  void operator=(const SingletonVar&);  // not implemented

  friend class ObservedVar;
};


/**  The observable notify/listen basic nuts and bolts.  */
class ObservedVar {
public:
  ObservedVar(const std::string& key)
      : singleton(SingletonVar::getInstance(key)) {}

  /** Notify all listeners about variable change. */
  virtual const void notify();

  /** Remove window from list of listeners, return true if listener exists */
  bool unlisten(wxEvtHandler* listener);


  /** Get listening object which unlisten() on destruction. */
  typedef std::unique_ptr<ObservedVarListener> Listener;
  Listener get_listener(wxEvtHandler* handler, wxEventType ev);


protected:
  /**
   * Notify all listeners: send them a 'type' wxCommandEvent message
   * as defined by listen() with optional data available using GetString()
   * and/or GetClientData().
   */
  const void notify(const std::string& s, void* client_data);

private:
  /** Set object to send ev_type to listener on variable changes. */
  void listen(wxEvtHandler* listener, wxEventType ev_type);

  SingletonVar* const singleton;

  friend class ObservedVarListener;
};


/**
 *  Keeps listening over it's lifespan, removes itself on destruction.
 *  Basically private, only available through ObservedVar::get_listener().
 */
class ObservedVarListener final {
public:
  ~ObservedVarListener() { var.unlisten(listener); };

private:
  /** Set object to send ev_type to window on variable changes. */
  ObservedVarListener(ObservedVar v, wxEvtHandler* w, wxEventType ev)
    : var(v), listener(w)  { var.listen(listener, ev); }

  ObservedVar var;
  wxEvtHandler* listener;

  friend class ObservedVar;
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
 *        change_listener = model.change.get_listener(this, EVT_FOO);
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
 *    ocpn::ConfigVar<bool> expert("/PlugIns", "CatalogExpert", &g_pConfig);
 *    bool old_value = expert.get(false);
 *    expert.set(false);
 *
 *  Client usage, listening to value changes.
 *
 *    class Foo: public wxEventHandler {
 *    public:
 *      Foo(...) {
 *        ocpn::ConfigVar<bool>
 *           expert("/PlugIns", "CatalogExpert", &g_pConfig);
 *
 *        // expert sends a wxCommandEvent of type EVT_FOO to this on changes:
 *        wxDEFINE_EVENT(EVT_FOO, wxCommandEvent);
 *        expert_listener = expert.get_listener(this, EVT_FOO);
 *
 *        // Handle  EVT_FOO as any event when it arrives, for example:
 *        Bind(EVT_FOO, [](wxCommandEvent&) { cout << "value has changed"; });
 *        ...
 *      }
 *    private:
 *      ocpn::ObservedVar::Listener expert_listener;
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
 *    ocpn::GlobalVar<wxString> compat_os(&g_compatOS);
 *    compat_os.set("ubuntu-gtk3-x86_64");
 *
 *  Client usage, modifying a value + notifying listeners:
 *
 *    ocpn::GlobalVar<wxString> plugin_array_var(&plugin_array);
 *    plugin_array.Add(new_pic);
 *    plugin_array_var.notify();
 *
 *  Client usage, listening to value changes:
 *
 *    class Foo: public wxEvtHandler {
 *    public:
 *      Foo(...) {
 *        ocpn::GlobalVar<wxString> compat_os(&g_compatOS);
 *
 *        // compat_os sends a wxCommandEvent type EVT_FOO to this on changes:
 *        wxDEFINE_EVENT(EVT_FOO, wxCommandEvent);
 *        compat_os_listener = compat_os.get_listener(this, EVT_FOO);)
 *
 *        // Handle  EVT_FOO as any event when it arrives, for example:
 *        Bind(EVT_FOO, [](wxCommandEvent&) { cout << "value has changed"; });
 *        ...
 *      }
 *    private:
 *      ocpn::ObservedVar::Listener compat_os_listener;
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

}  // namespace ocpn

#endif   // OBSERVABLE_H
