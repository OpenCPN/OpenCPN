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
  * \file
  * General observable implementation with several specializations.
  *
  * A Notify() / Listen() library built on top of wxWidget's event handling.
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

#include "observable_evt.h"

#ifndef DECL_EXP
#if defined(_MSC_VER) || defined(__CYGWIN__)
#define DECL_EXP __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
#define DECL_EXP __attribute__((visibility("default")))
#else
#define DECL_EXP
#endif
#endif  // DECL_EXP

/** Return address as printable string. */
std::string ptr_key(const void* ptr);

class Observable;
class ObservableListener;

/**
 * Interface implemented by classes which listens.
 *
 * @interface KeyProvider observable.h "observable.h"
 */
class KeyProvider {
public:
  /**
   * Destroy the Key Provider object.
   */
  virtual ~KeyProvider() = default;

  /**
   * Get the Key object from the Key Provider.
   *
   * @return std::string Key Object.
   */
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
  friend class ObservableListener;

public:
  explicit Observable(const std::string& _key)
      : key(_key), m_list(ListenersByKey::GetInstance(_key)) {}

  explicit Observable(const KeyProvider& kp) : Observable(kp.GetKey()) {}

  /**
   * Destroy the Observable object.
   */
  ~Observable() override = default;

  /** Notify all listeners about variable change. */
  virtual void Notify();

  void Notify(const std::shared_ptr<const void>& p) { Notify(p, "", 0, nullptr); }

  /**
   * Remove window listening to ev from list of listeners.
   * @return true if such a listener existed, else false.
   */
  bool Unlisten(wxEvtHandler* listener, wxEventType ev);

  std::string GetKey() const override { return key; }

  /** The key used to create and clone. */
  const std::string key;

protected:
  /**
   * Notify all listeners: send them a 'type' ObservedEvt message
   * as defined by listen() with optional data available using GetString()
   * and/or GetClientData().
   */
  void Notify(const std::shared_ptr<const void>& ptr, const std::string& s,
                    int num, void* client_data);

  void Notify(const std::string& s, void* client_data) {
    Notify(nullptr, s, 0, client_data);
  }

private:
  /** Set object to send ev_type to listener on variable changes. */
  void Listen(wxEvtHandler* listener, wxEventType ev_type);

  ListenersByKey& m_list;

  mutable std::mutex m_mutex;
};

/**
 *  Keeps listening over its lifespan, removes itself on destruction.
 */
class DECL_EXP ObservableListener final {
  friend class ObsListener;

public:
  /** Default constructor, does not listen to anything. */
  ObservableListener() : listener(nullptr), ev_type(wxEVT_NULL) {}

  /** Construct a listening object. */
  ObservableListener (std::string k, wxEvtHandler* l, wxEventType e)
      : key(std::move(k)), listener(l), ev_type(e) {
    Listen();
  }

  ObservableListener(const KeyProvider& kp, wxEvtHandler* l, wxEventType e)
      : ObservableListener(kp.GetKey(), l, e) {}

  /** A listener can only be transferred using std::move(). */
  ObservableListener(ObservableListener&& other) noexcept {
    key = other.key;
    listener = other.listener;
    ev_type = other.ev_type;
    other.Unlisten();
    Listen();
  }

  /** A listener can only be transferred using std::move(). */
  ObservableListener& operator=(ObservableListener&& other) noexcept {
    key = other.key;
    listener = other.listener;
    ev_type = other.ev_type;
    other.Unlisten();
    Listen();
    return *this;
  }

  ObservableListener(const ObservableListener& other) = delete;
  ObservableListener& operator=(ObservableListener&) = delete;

  ~ObservableListener() { Unlisten(); }

  /** Set object to send wxEventType ev to listener on changes in key. */
  void Listen(const std::string& key, wxEvtHandler* listener, wxEventType evt);

  /** Set object to send wxEventType ev to listener on changes in key. */
  void Listen(const KeyProvider& kp, wxEvtHandler* l, wxEventType evt) {
    Listen(kp.GetKey(), l, evt);
  }

private:
  void Listen();
  void Unlisten();

  std::string key;
  wxEvtHandler* listener;
  wxEventType ev_type;
};

/**
 * Define an action to be performed when a KeyProvider is notified.
 * Convenience container hiding the Bind(), wxEVENT_TYPE and listening details.
 * The action function is in most use cases a lambda expression.
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
 *         EvtVarListener change_listener;
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
 * ObsListener is non-copyable, but can be created and assigned using
 * std::move like in
 * \code
 *
 *     std::vector<ObsListener> v;
 *     ObsListener l;
 *     v[0] = std::move(l);
 *
 * \endcode
 */
class ObsListener : public wxEvtHandler {
public:
  /** Create an object which does not listen until Init(); */
  ObsListener() : m_obs_evt(wxNewEventType()) {}

  /** ObsListener can only be assigned using std::move */
  ObsListener(ObsListener&& other) noexcept: m_obs_evt(wxNewEventType()) {
    m_listener.Unlisten();
    Unbind(other.m_obs_evt, other.m_action);
    m_action = other.m_action;
    Bind(m_obs_evt, m_action);
    m_listener.Listen(other.m_listener.key, this, m_obs_evt);
  }

  ObsListener& operator=(ObsListener&& other) noexcept {
    m_listener.Unlisten();
    Unbind(other.m_obs_evt, other.m_action);
    m_action = other.m_action;
    Bind(m_obs_evt, m_action);
    m_listener.Listen(other.m_listener.key, this, m_obs_evt);
    return *this;
  }

  ObsListener(const ObsListener&) = delete;
  ObsListener& operator=(ObsListener&) = delete;

  /** Create object which invokes action when kp is notified. */
  ObsListener(const KeyProvider& kp,
             const  std::function<void(ObservedEvt& ev)>& action)
      : m_obs_evt(wxNewEventType()) {
    Init(kp, action);
  }

  /** Create object which invokes action when kp is notified. */
  ObsListener(const KeyProvider& kp, const std::function<void()>& action)
      : ObsListener(kp, [&](ObservedEvt&) { action(); }) {}

  /** Initiate an object yet not listening. */
  void Init(const KeyProvider& kp,
            const std::function<void(ObservedEvt& ev)>& action) {
    m_action = action;
    const wxEventTypeTag<ObservedEvt> EvtObs(wxNewEventType());
    // i. e. wxDEFINE_EVENT(), avoiding the evil macro.
    m_listener.Listen(kp, this, EvtObs);
    Bind(EvtObs, action);
  }

private:
  ObservableListener m_listener;
  std::function<void(ObservedEvt& ev)> m_action;
  const wxEventTypeTag<ObservedEvt> m_obs_evt;
};

/** Shorthand for accessing ObservedEvt.SharedPtr(). */
template <typename T>
std::shared_ptr<const T> UnpackEvtPointer(const ObservedEvt& ev) {
  return std::static_pointer_cast<const T>(ev.GetSharedPtr());
}

#endif  // OBSERVABLE_H
