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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 **************************************************************************/

 /**
  * \file
  *
  * General observable pattern implementation built on top of wxWidgets
  * event handling.
  *
  * Based on Notify() / Listen() with several specializations with a
  * general EventVar and more specific solutions for configuration variables
  * and globals. Easily extended for other use cases.
  */

#ifndef OBSERVABLE_H
#define OBSERVABLE_H

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include <wx/event.h>

#include "event.h"

#ifndef DECL_EXP
#if defined(_MSC_VER) || defined(__CYGWIN__)
#define DECL_EXP __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
#define DECL_EXP __attribute__((visibility("default")))
#else
#define DECL_EXP
#endif
#endif  // DECL_EXP

class ObservableListener;   // forward

namespace  obs {
class Listener;   // forward

/** Return address as printable string. */
std::string PtrKey(const void* ptr);

class Observable;   // forward

/**
 * Interface implemented by classes which listens. The string returned must
 * be stable and unique.
 *
 * @interface KeyProvider observable.h "observable.h"
 */
class KeyProvider {
public:
  virtual ~KeyProvider() = default;

  /** Return string used by Notify() and Listen(). */
  [[nodiscard]] virtual std::string GetKey() const = 0;
};

/**
 *  Private helper class. Basically a singleton map of listener lists
 *  where lists are managed by key, one for each key value.
 */
class ListenersByKey {
  friend class Observable;
  friend ListenersByKey& GetInstance(const std::string& key);

public:
  ListenersByKey() = default;
  ListenersByKey(const ListenersByKey&) = delete;

private:
  static ListenersByKey& GetInstance(const std::string& key);

  ListenersByKey& operator=(const ListenersByKey&) = default;

  std::vector<std::pair<wxEvtHandler*, wxEventType>> listeners;
};

/**  The observable notify/listen basic nuts and bolts.  */
class Observable : public KeyProvider {
  friend class ::ObservableListener;

public:
  /** Initiate an object which can apply Notify() and Listen() to given key */
  explicit Observable(const std::string& _key)
      : key(_key), m_list(ListenersByKey::GetInstance(_key)) {}

  /** Initiate an object which can apply Notify() and Listen() to given key */
  explicit Observable(const KeyProvider& kp) : Observable(kp.GetKey()) {}

  ~Observable() override = default;

  /** Notify all listeners to configured key about variable change. */
  virtual void Notify();

  /**
  * Notify all listeners to configured key about variable change. Send them
  * a 'type'  ObservedEvt message as defined by Listen() with a
  * shared_ptr<const whatever> available to listeners using
  * event.GetSharedPtr().
  */
  virtual void Notify(const std::shared_ptr<const void>& p) {
    Notify(p, "", 0, nullptr);
  }

  /**
   * Remove window listening to ev from list of listeners.
   * @return true if such a listener existed, else false.
   */
  bool Unlisten(wxEvtHandler* listener, wxEventType ev);

  /** Return the key  given to the constructor. */
  std::string GetKey() const override { return key; }

  /** The key used to create and clone. */
  const std::string key;

protected:
  /**
   * Notify all listeners: send them a 'type' ObservedEvt message
   * as defined by Listen() with optional data available using GetString(),
   * GetClientData(), GetInt() and/or GetSharedPtr()
   */
  void Notify(const std::shared_ptr<const void>& ptr, const std::string& s,
                    int num, void* client_data);

  /**
   * Notify all listeners: send them a 'type' ObservedEvt message
   * as defined by Listen() with optional data available using GetString(),
   * and/or GetClientData().
   */
  void Notify(const std::string& s, void* client_data){
    Notify(nullptr, s, 0, client_data);
  }

private:
  /** Set object to send ev_type to listener on variable changes. */
  void Listen(wxEvtHandler* listener, wxEventType ev_type);

  ListenersByKey& m_list;

  mutable std::mutex m_mutex;
};

}   // namespace

/**
 *  Manages listening to an Observable instance.  Keeps listening over its
 *  lifespan, removes itself on destruction. Old legacy code kept in place
 *  because it is visible in the plugin interface. For the same reason it
 *  is not in the obs namespace.
 *
 *  Use obs::Listener in new code.
 */
class DECL_EXP ObservableListener final {
  friend class obs::Listener;

public:
  /** Default constructor, does not listen to anything. */
  ObservableListener() : m_listener(nullptr), m_ev_type(wxEVT_NULL) {}

  /**
   * Construct a listening object.
   * @param  k  String used to coordinate Notify() and Listen().
   * @param  l  EventHandler which will be receiving messages on Notify()
   * @param  e  EventType forwarded by Notify() to listeners.
   */
  ObservableListener (std::string k, wxEvtHandler* l, wxEventType e)
      : m_key(std::move(k)), m_listener(l), m_ev_type(e) {
    Listen();
  }

  /**
   * Construct a listening object.
   * @param  kp  Provides string used to coordinate Notify() and Listen().
   * @param  l  EventHandler which will be receiving messages on Notify()
   * @param  e  EventType forwarded by Notify()  to listeners.
   */
  ObservableListener(const obs::KeyProvider& kp, wxEvtHandler* l, wxEventType e)
      : ObservableListener(kp.GetKey(), l, e) {}

  ~ObservableListener() { Unlisten(); }

  /** std::move() support. */
  ObservableListener(ObservableListener&& other) noexcept {
    m_key = other.m_key;
    m_listener = other.m_listener;
    m_ev_type = other.m_ev_type;
    other.Unlisten();
    Listen();
  }

  /** std::move() support. */
  ObservableListener& operator=(ObservableListener&& other) noexcept {
    m_key = other.m_key;
    m_listener = other.m_listener;
    m_ev_type = other.m_ev_type;
    other.Unlisten();
    Listen();
    return *this;
  }

  /** A listener is non copyable, can only be transferred using std::move(). */
  ObservableListener(const ObservableListener& other) = delete;

  /** A listener is non copyable, can only be transferred using std::move(). */
  ObservableListener& operator=(ObservableListener&) = delete;

  /** Set object to send wxEventType ev to listeners on changes in key. */
  void Listen(const std::string& key, wxEvtHandler* listener, wxEventType evt);

  /** Set object to send wxEventType ev to listeners on changes in key. */
  void Listen(const obs::KeyProvider& kp, wxEvtHandler* l, wxEventType evt) {
    Listen(kp.GetKey(), l, evt);
  }

private:
  void Listen();
  void Unlisten();

  std::string m_key;
  wxEvtHandler* m_listener;
  wxEventType m_ev_type;
};

namespace obs {

/**
 * Define an action to be performed when a KeyProvider is notified.
 * Convenience container hiding the Bind(), wxEVENT_TYPE and listening details.
 * The action function is usually a lambda expression.
 *
 * Controller/GUI example usage, listening to the EventVar model.change:
 * \code
 *
 *       class Gui: public SomeBaseClass {
 *       public:
 *         Gui:Gui(Model& model):
 *           change_listener(model.change, [&](ObservableEvt& ev) {
 *               auto s = ev.GetString();
 *               .... do something
 *           })
 *         {}
 *
 *       private:
 *         obs::Listener change_listener;
 *       }
 * \endcode
 *
 * Or, using Init():
 * \code
 *
 *       Gui:Gui(Model& model)  {
 *         auto action = [&](ObservableEvt& ev) {
 *             auto s = ev.GetString();
 *             .... do something
 *         });
 *         change_listener.Init(model.change, action);
 *       }
 *
 * \endcode
 *
 * Listener is non-copyable, but can be created and assigned using
 * std::move like in
 * \code
 *
 *     std::vector<ObsListener> v;
 *     ObsListener l;
 *     v[0] = std::move(l);
 *
 * \endcode
 */
class Listener : public wxEvtHandler {
public:
  /** Create an object which does not listen until Init(); */
  Listener() : m_obs_evt(wxNewEventType()) {}

  /** std::move support */
  Listener(Listener&& other) noexcept: m_obs_evt(wxNewEventType()) {
    m_listener.Unlisten();
    Unbind(other.m_obs_evt, other.m_action);
    m_action = other.m_action;
    Bind(m_obs_evt, m_action);
    m_listener.Listen(other.m_listener.m_key, this, m_obs_evt);
  }

  /** std::move support */
  Listener& operator=(Listener&& other) noexcept {
    m_listener.Unlisten();
    Unbind(other.m_obs_evt, other.m_action);
    m_action = other.m_action;
    Bind(m_obs_evt, m_action);
    m_listener.Listen(other.m_listener.m_key, this, m_obs_evt);
    return *this;
  }

  /** ObsListener is non-copyable and can only be assigned using std::move */
  Listener(const Listener&) = delete;

  /** ObsListener is non-copyable and can only be assigned using std::move */
  Listener& operator=(Listener&) = delete;

  /** Create object which invokes action when the kp key is notified. */
  Listener(const KeyProvider& kp,
           const std::function<void(ObservedEvt& ev)>& action)
      : m_obs_evt(wxNewEventType()) {
    Init(kp, action);
  }

  /** Create object which invokes action when the kp key is notified. */
  Listener(const KeyProvider& kp, const std::function<void()>& action)
      : Listener(kp, [&](ObservedEvt&) { action(); }) {}

  /**
   * Initiate an object yet not listening so it invokes action when the kp
   * key is notified.
   */
  void Init(const KeyProvider& kp,
            const std::function<void(ObservedEvt& ev)>& action) {
    m_action = action;
    const wxEventTypeTag<ObservedEvt> EvtObs(wxNewEventType());
    // i. e. wxDEFINE_EVENT(), avoiding the evil macro.
    m_listener.Listen(kp, this, EvtObs);
    Bind(EvtObs, action);
  }

private:
  ::ObservableListener m_listener;
  std::function<void(ObservedEvt& ev)> m_action;
  const wxEventTypeTag<ObservedEvt> m_obs_evt;
};

/** Shorthand for accessing ObservedEvt.SharedPtr(). */
template <typename T>
std::shared_ptr<const T> UnpackEvtPointer(const ObservedEvt& ev) {
  return std::static_pointer_cast<const T>(ev.GetSharedPtr());
}

}  // namespace
#endif  // OBSERVABLE_H
