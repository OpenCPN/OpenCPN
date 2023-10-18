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

#ifndef _OBSERVABLE_EVTVAR_H
#define _OBSERVABLE_EVTVAR_H

#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include <wx/event.h>

#include "observable.h"

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
 *       change.Notify("new value")
 *     }
 *
 *  Controller/GUI usage:
 *
 *    class Gui: public wxEvtHandler {
 *    public:
 *      Gui:Gui(Model& model) {
 *        wxDEFINE_EVENT(EVT_FOO, wxCommandEvent);
 *        change_listener.Listen(model.change, this, EVT_FOO);
 *        Bind(EVT_FOO, [&](wxCommandEvent ev) {
 *          auto s = ev.GetString();    s -> "new value"
 *          ... do something;
 *        });
 *      }
 *
 *    private:
 *      ObservableListener change_listener;
 *    }
 */
class EventVar : public Observable {
public:
  EventVar() : Observable(Autokey()) {}

  /** Notify all listeners, no data supplied. */
  const void Notify() { Observable::Notify("", 0); }

  /** Notify all listeners about variable change with ClientData. */
  const void Notify(void* data) { Observable::Notify("", data); }

  /** Notify all listeners about variable change with a string. */
  const void Notify(const std::string& s) { Observable::Notify(s, 0); }

  /** Notify all listeners about variable change with a string and an int */
  const void Notify(int n, const std::string& s) { Observable::Notify(0, s, n, 0); }
  /**
   * Notify all listeners about variable change with shared_ptr,
   * a string and an optional number.
   */
  const void Notify(std::shared_ptr<void> p, const std::string& s, int n = 0) {
    Observable::Notify(p, s, n, 0);
  }

private:
  std::string Autokey() {
    static std::atomic<unsigned long> last_ix(0);
    return std::string("!@%/+") + std::to_string(last_ix++);
  }
};

#endif  // _OBSERVABLE_EVTVAR_H
