#ifndef CONFIG_VAR_H
#define CONFIG_VAR_H

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <wx/fileconf.h>
#include <wx/window.h>

namespace ocpn {

/** Return address as printable string. */
std::string ptr_key(const void* ptr);

/**
 *  Helper class, not for public consumption. Basically a singleton map of
 *  listeners where singletons are managed by key, one for each key value.
 */
class SingletonVar {
public:
  static SingletonVar* getInstance(const std::string& key);
  std::map<wxWindow*, wxEventType> listeners;

private:
  SingletonVar() {}
  SingletonVar(const SingletonVar&);    // not implemented
  void operator=(const SingletonVar&);  // not implemented
};

/**  The observable notify/listen basic nuts and bolts.  */
class ObservedVar {
public:
  ObservedVar(const std::string& key)
      : singleton(SingletonVar::getInstance(key)) {}

  /** Set object to send ev_type to listener on variable changes. */
  void listen(wxWindow* listener, wxEventType ev_type);

  /** Notify all listeners about variable change. */
  const void notify();

  /** Remove window from list of listeners, return true if listener exists */
  bool unlisten(wxWindow* listener);

private:
  SingletonVar* const singleton;
};

/**
 *  Wrapper for configuration variables which lives in the global wxFileConfig
 *  object. Supports int, bool, double, std::string and wxString. Besides
 *  basic set()/get() also provides notification events when value changes.
 *
 *  Client usage, reading and setting a value:
 *
 *     ocpn::ConfigVar<bool> expert("/PlugIns", "CatalogExpert", &g_pConfig);
 *     bool old_value = expert.get(false);
 *     expert.set(false);
 *
 *  Client usage, listening to value changes:
 *
 *     ocpn::ConfigVar<bool> expert("/PlugIns", "CatalogExpert", &g_pConfig);
 *
 *     // expert sends a wxCommandEVent of type EVT_FOO to this on changes:
 *     wxDEFINE_EVENT(EVT_FOO, wxCommandEvent);
 *     expert.listen(this, EVT_FOO)
 *
 *     // Handle  EVT_FOO as any event when it arrives, for example:
 *     Bind(EVT_FOO, [](wxCommandEvent&) { cout << "value has changed"; });
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
 *     ocpn::GlobalVar<wxString> compat_os(&g_compatOS);
 *     compat_os.set("ubuntu-gtk3-x86_64");
 *
 *  Client usage, modifying a value + notifying listeners:
 *
 *     ocpn::GlobalVar<wxString> plugin_array_var(&plugin__array);
 *     plugin_array.Add(new_pic);
 *     plugin_array_var.notify();
 *
 *  Client usage, listening to value changes:
 *
 *     ocpn::GlobalVar<wxString> compat_os(&g_compatOS);
 *
 *     // compat_os sends a wxCommandEVent of type EVT_FOO to this on changes:
 *     wxDEFINE_EVENT(EVT_FOO, wxCommandEvent);
 *     compat_os.listen(this, EVT_FOO)
 *
 *     // Handle  EVT_FOO as any event when it arrives, for example:
 *     Bind(EVT_FOO, [](wxCommandEvent&) { cout << "value has changed"; });
 *
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

#endif
