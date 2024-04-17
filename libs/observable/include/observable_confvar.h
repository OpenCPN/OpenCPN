/*************************************************************************
 *
 * Project:  OpenCPN
 * Purpose: Notify/listen config var wrapper
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

#ifndef OBSERVABLE_CONFVAR_H
#define OBSERVABLE_CONFVAR_H

#include <memory>
#include <string>
#include <vector>

#include <wx/config.h>

#include "observable.h"

/**
 *  Wrapper for configuration variables which lives in a wxBaseConfig
 *  object. Supports int, bool, double, std::string and wxString. Besides
 *  basic set()/get() also provides notification events when value changes.
 *
 *  Client usage when reading, setting a value and notifying listeners:
 *  \code
 *
 *    ConfigVar<bool> expert("/PlugIns", "CatalogExpert", &g_pConfig);
 *    bool old_value = expert.Get(false);
 *    expert.Set(false);
 *  \endcode
 *
 *  Client usage, listening to value changes.
 *  \code
 *
 *    class Foo: public wxEventHandler {
 *    public:
 *      Foo(...) {
 *        ConfigVar<bool> expert("/PlugIns", "CatalogExpert", &g_pConfig);
 *        auto action = [](wxCommandEvent&) { cout << "value has changed"; });
 *        expert_listener.Init(expert, action);
 *        ...
 *      }
 *    private:
 *      ObsListener expert_listener;
 *      ...
 *    }
 *
 *  \endcode
 */
template <typename T = std::string>
class ConfigVar : public Observable {
public:
  ConfigVar(const std::string& section_, const std::string& key_,
            wxConfigBase* cb);

  void Set(const T& arg);

  const T Get(const T& default_val);

private:
  ConfigVar();  // not implemented

  const std::string section;
  const std::string key;
  wxConfigBase* const config;
};

#endif  // OBSERVABLE_CONFVAR_H
