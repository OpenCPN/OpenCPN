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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 ************************************************************************/

/**
 * \file
 *
 * Notify()/Listen() configuration variable wrapper
 */

#ifndef OBSERVABLE_CONFVAR_H
#define OBSERVABLE_CONFVAR_H

#include <string>

#include <wx/config.h>

#include "observable.h"

namespace obs {
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
 *      Foo(...)  {
 *        auto action = [](wxCommandEvent&) { cout << "value has changed"; });
 *        m_expert_listener.Init(expert, action);
 *        ...
 *      }
 *    private:
 *      obs::Listener m_expert_listener;
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
  ConfigVar() = delete;

  void Set(const T& arg);

  T Get(const T& default_val);

private:
  const std::string m_section;
  const std::string m_key;
  wxConfigBase* const m_config;
};

}  // namespace obs

#endif  // OBSERVABLE_CONFVAR_H
