/*************************************************************************
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
 * A common variable shared between producer and consumer which supports
 * Listen() and Notify().
 */


#ifndef OBSERVABLE___EVTVAR__H
#define OBSERVABLE___EVTVAR__H

#include <atomic>
#include <memory>
#include <string>

#include "observable.h"

/**
 * Generic event handling between MVC Model and Controller based on a
 * shared EventVar variable
 *
 *  Model usage:
 *  \code
 *
 *   class Model: ...
 *   public:
 *     EventVar change;
 *
 *     void some_method() {
 *       ...
 *       change.Notify("new value")
 *     }
 *  \endcode
 *
 *  Controller/GUI usage:
 *  \code
 *
 *    class Gui: public wxEvtHandler {
 *    public:
 *      Gui:Gui(Model& model) {
 *        auto action = [&](wxCommandEvent ev) {
 *          auto s = ev.GetString();    // s -> "new value"
 *          ... do something;
 *        });
 *        change_listener.Init(model.change, action);
 *      }
 *
 *    private:
 *      ObsListener change_listener;
 *    }
 *  \endcode
 */
class EventVar : public Observable {
public:
  EventVar() : Observable(Autokey()) {}

  /** Notify all listeners, no data supplied. */
  void Notify() override { Observable::Notify("", nullptr)
    ; }

  /** Notify all listeners about variable change with ClientData. */
  void Notify(void* data) { Observable::Notify("", data); }

  /** Notify all listeners about variable change with a string. */
  void Notify(const std::string& s) { Observable::Notify(s, nullptr); }

  /** Notify all listeners about variable change with a string and an int */
  void Notify(int n, const std::string& s) { Observable::Notify(nullptr, s, n, nullptr); }

  /** Notify all listeners about variable change with an int and ClientData */
  void Notify(int n, void* p) { Observable::Notify(nullptr, "", n, p); }

  /**
   * Notify all listeners about variable change with shared_ptr,
   * a string and an optional number.
   */
  void Notify(const std::shared_ptr<void>& p, const std::string& s, int n = 0) {
    Observable::Notify(p, s, n, nullptr);
  }

  /** Notify all listeners about variable change with const* shared_ptr, */
  void Notify(const std::shared_ptr<const void>& p) {
    Observable::Notify(p, "", 0, nullptr);
  }

private:
  std::string Autokey() {
    static std::atomic<unsigned long> last_ix(0);
    return std::string("!@%/+") + std::to_string(last_ix++);
  }
};

#endif  // OBSERVABLE___EVTVAR__H
