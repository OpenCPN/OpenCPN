/**************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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
 ***************************************************************************/

/**
 * \file
 *
 * Implement idx_entry.h -- tidal data container
 */

#include "idx_entry.h"

#include <string.h>
#include <stdlib.h>
#include <wx/arrimpl.cpp>

WX_DEFINE_OBJARRAY(ArrayOfIDXEntry);

IDX_entry::IDX_entry()
    : source_data_type(SOURCE_TYPE_UNKNOWN),
      pDataSource(nullptr),
      IDX_rec_num(0),
      IDX_type(0),
      IDX_lon(0.0),
      IDX_lat(0.0),
      IDX_ht_time_off(0),
      IDX_ht_mpy(0.0f),
      IDX_ht_off(0.0f),
      IDX_lt_time_off(0),
      IDX_lt_mpy(0.0f),
      IDX_lt_off(0.0f),
      IDX_sta_num(0),
      IDX_flood_dir(0),
      IDX_ebb_dir(0),
      IDX_Useable(0),
      Valid15(0),
      Value15(0.0f),
      Dir15(0.0f),
      Ret15(false),
      IDX_tzname(nullptr),
      IDX_ref_file_num(0),
      IDX_ref_dbIndex(0),
      max_amplitude(0.0),
      have_offsets(0),
      station_tz_offset(0),
      IDX_time_zone(0),
      pref_sta_data(nullptr),
      num_nodes(0),
      num_csts(0),
      num_epochs(0),
      m_cst_speeds(nullptr),
      m_cst_nodes(nullptr),
      m_cst_epochs(nullptr),
      m_work_buffer(nullptr),
      first_year(0),
      epoch(0),
      epoch_year(0),
      current_depth(0),
      b_skipTooDeep(false),
      recent_highlow_calc_time(0),
      recent_high_level(0.0f),
      recent_high_time(0),
      recent_low_level(0.0f),
      recent_low_time(0) {
  memset(source_ident, 0, sizeof(source_ident));
  memset(IDX_zone, 0, sizeof(IDX_zone));
  memset(IDX_station_name, 0, sizeof(IDX_station_name));
  memset(IDX_reference_name, 0, sizeof(IDX_reference_name));
  memset(IDX_datum, 0, sizeof(IDX_datum));
}

IDX_entry::~IDX_entry() { free(IDX_tzname); }
