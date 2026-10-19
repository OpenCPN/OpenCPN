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
 * Tidal data container
 */

#ifndef __IDX_ENTRY_H__
#define __IDX_ENTRY_H__

#include <wx/dynarray.h>

#define MAXNAMELEN 90
#define MAXDATUMLEN 100

class TCDataSource;
class Station_Data;

/**
 * Enumeration of source data types.
 */
typedef enum {
  SOURCE_TYPE_UNKNOWN,          ///< Unknown source type
  SOURCE_TYPE_ASCII_HARMONIC,   ///< ASCII harmonic source type
  SOURCE_TYPE_BINARY_HARMONIC,  ///< Binary harmonic source type
} source_data_t;

/**
 * Represents an index entry for tidal and current data. Stores various
 * attributes and data related to tidal and current stations, including
 * location, offsets, and harmonic constituents.
 */
class IDX_entry {
public:
  IDX_entry();
  ~IDX_entry();

  source_data_t
      source_data_type;       ///< Format of the source data (ASCII or binary)
  TCDataSource *pDataSource;  ///< Pointer to the associated data source
  char source_ident[MAXNAMELEN];  ///< Identifier of the source (typically file
                                  ///< name)

  int IDX_rec_num;    ///< Record number for multiple entries with same name
  char IDX_type;      ///< Entry type identifier "TCtcIUu"
  char IDX_zone[40];  ///< Geographic zone identifier
  char IDX_station_name[MAXNAMELEN];  ///< Name of the tidal or current station
  char IDX_datum[MAXDATUMLEN];        ///< Datum used for this record
  double IDX_lon;        ///< Longitude of the station (in degrees, +East)
  double IDX_lat;        ///< Latitude of the station (in degrees, +North)
  int IDX_ht_time_off;   ///< High tide time offset (in minutes)
  float IDX_ht_mpy;      ///< High tide height multiplier
  float IDX_ht_off;      ///< High tide height offset
  int IDX_lt_time_off;   ///< Low tide time offset (in minutes)
  float IDX_lt_mpy;      ///< Low tide height multiplier
  float IDX_lt_off;      ///< Low tide height offset
  int IDX_sta_num;       ///< Subordinate station number (UNUSED)
  int IDX_flood_dir;     ///< Flood current direction (in degrees)
  int IDX_ebb_dir;       ///< Ebb current direction (in degrees)
  int IDX_Useable;       ///< Flag indicating if the entry is usable
  int Valid15;           ///< Validity flag for 15-minute interval data
  float Value15;         ///< Value for 15-minute interval data
  float Dir15;           ///< Direction for 15-minute interval data
  bool Ret15;            ///< Return flag for 15-minute interval data
  char *IDX_tzname;      ///< Timezone name (dynamically allocated)
  int IDX_ref_file_num;  ///< Reference file number
  char IDX_reference_name[MAXNAMELEN];  ///< Name of the reference station
  int IDX_ref_dbIndex;   ///< Database index of the reference station
  double max_amplitude;  ///< Maximum tidal amplitude
  int have_offsets;      ///< Flag indicating presence of time/height offsets
  /**
   * @brief Offset in seconds to convert from harmonic data (epochs) to the
   * station time zone
   *
   * This offset depends on the Master Station reference only. For ASCII data,
   * it's typically 0. For Binary data, it's probably calculated as:
   * -(IDX_time_zone * 60) - (tiderec->zone_offset * 3600)
   */
  int station_tz_offset;
  int IDX_time_zone;  ///< Station timezone offset from UTC (in minutes)

  Station_Data *pref_sta_data;  ///< Pointer to the reference station data

  int num_nodes;          ///< Number of nodes in harmonic analysis
  int num_csts;           ///< Number of harmonic constituents
  int num_epochs;         ///< Number of epochs in harmonic data
  double *m_cst_speeds;   ///< Array of constituent speeds
  double **m_cst_nodes;   ///< 2D array of constituent nodes
  double **m_cst_epochs;  ///< 2D array of constituent epochs
  double *m_work_buffer;  ///< Work buffer for calculations
  int first_year;         ///< First year of valid data
  time_t epoch;           ///< Epoch time for the station
  int epoch_year;         ///< Year of the epoch
  int current_depth;      ///< Depth for current stations
  bool b_skipTooDeep;     ///< Flag to skip processing if depth exceeds limit

  // Cached values for performance
  time_t recent_highlow_calc_time;  ///< Timestamp of the most recent high/low
                                    ///< calculation
  float recent_high_level;  ///< Most recently calculated high tide level
  time_t recent_high_time;  ///< Time of the most recent high tide
  float recent_low_level;   ///< Most recently calculated low tide level
  time_t recent_low_time;   ///< Time of the most recent low tide
};

WX_DECLARE_OBJARRAY(IDX_entry, ArrayOfIDXEntry);

#endif
