/*************************************************************************
 *
 * Project: OpenCPN
 * Purpose: Implement observable.h
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

#include <sstream>
#include <string>

#include <wx/log.h>
#include <wx/string.h>
#include <wx/config.h>

#include "observable_confvar.h"


/**
 * Add >> support for wxString, for some reason missing in wxWidgets 3.0,
 * required by ConfigVar::get()
 */
std::istream& operator>>(std::istream& input, wxString& ws) {
  std::string s;
  input >> s;
  ws.Append(s);
  return input;
}

/* ConfigVar implementation. */

template <typename T>
ConfigVar<T>::ConfigVar(const std::string& section_, const std::string& key_,
                        wxConfigBase* cb)
    : Observable(section_ + "/" + key_),
      section(section_),
      key(key_),
      config(cb) {}

template <typename T>
const T ConfigVar<T>::Get(const T& default_val) {
  std::istringstream iss;
  config->SetPath(section);
  auto value = config->Read(key, "").ToStdString();
  iss.str(value);
  T r;
  iss >> r;
  return iss.fail() ? default_val : r;
}

template <typename T>
void ConfigVar<T>::Set(const T& arg) {
  std::ostringstream oss;
  oss << arg;
  if (oss.fail()) {
    wxLogWarning("Cannot dump failed buffer for key %s:%s", section.c_str(),
                 key.c_str());
    return;
  }
  config->SetPath(section);
  if (!config->Write(key.c_str(), oss.str().c_str())) {
    wxLogWarning("Error writing buffer to key %s:%s", section.c_str(),
                 key.c_str());
  }
  Observable::Notify();
}

/* Explicitly instantiate the ConfigVar types supported. */
template class ConfigVar<bool>;
template class ConfigVar<double>;
template class ConfigVar<int>;
template class ConfigVar<std::string>;
template class ConfigVar<wxString>;
