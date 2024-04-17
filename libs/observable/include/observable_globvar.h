/*************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  global variables listen/notify wrapper.
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

#ifndef _OBSERVABLE_GLOBVAR_H
#define _OBSERVABLE_GLOBVAR_H

#include <memory>
#include <string>
#include <vector>

#include <wx/event.h>

#include "observable.h"

/**
 *  Wrapper for global variable, supports notification events when value
 *  changes.
 *
 *  Client usage, writing a value + notifying listeners
 *  \code
 *
 *    GlobalVar<wxString> compat_os(&g_compatOS);
 *    compat_os.Set("ubuntu-gtk3-x86_64");
 *  \endcode
 *
 *  Client usage, modifying a value + notifying listeners:
 *  \code
 *
 *    GlobalVar<wxString> plugin_array_var(&plugin_array);
 *    plugin_array.Add(new_pic);
 *    plugin_array_var.Notify();
 *  \endcode
 *
 *  Client usage, listening to value changes:
 *  \code
 *
 *    class Foo: public wxEvtHandler {
 *    public:
 *      Foo(...) {
 *        GlobalVar<wxString> compat_os(&g_compatOS);
 *        auto action = [](wxCommandEvent&) { cout << "value has changed"; };
 *        compat_os_listener.Init(compat_os, action);
 *      }
 *    private:
 *      ObsListener compat_os_listener;
 *      ...
 *    }
 *  \endcode
 */
template <typename T>
class GlobalVar : public Observable {
public:
  GlobalVar(T* ptr) : Observable(ptr_key(ptr)), variable(ptr) {}

  void Set(const T& arg) {
    *variable = arg;
    Observable::Notify();
  }

  const T Get() { return *variable; }

private:
  GlobalVar();  // not implemented

  T* const variable;
};

#endif  // _OBSERVABLE_GLOBVAR_H
