/***************************************************************************
 *   Copyright (C) 2025 Alec Leamas                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Dialogs handing user defined filters.
 */

#ifndef FILTER_DLG_H__
#define FILTER_DLG_H__

#include <functional>

#include <wx/window.h>
#include "observable_evtvar.h"

class FilterEvents {
public:
  static FilterEvents& GetInstance() {
    static FilterEvents the_instance;
    return the_instance;
  }

  FilterEvents(const FilterEvents&) = delete;
  FilterEvents& operator=(const FilterEvents&) = delete;

  /** Notified without data when user creates or removes a filter. */
  EventVar filter_list_change;

  /** Notified with filter name when filter is updated on disk. */
  EventVar filter_update;

  /** Notified with filter name when applied by user. */
  EventVar filter_apply;

private:
  FilterEvents() = default;
};

void CreateFilterDlg(wxWindow* parent);

void RemoveFilterDlg(wxWindow* parent);

void EditFilterDlg(wxWindow* parent);

void RenameFilterDlg(wxWindow* parent);

void EditOneFilterDlg(wxWindow* parent, const std::string& filter);

#endif  // FILTER_DLG_H__
