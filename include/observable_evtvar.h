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
  std::string autokey() {
    static  std::atomic<unsigned long> last_ix(0);
    return std::string("!@%/+") + std::to_string(last_ix++);
  }
};

#endif  // _OBSERVABLE_EVTVAR_H
