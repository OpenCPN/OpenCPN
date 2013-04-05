/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */

#ifndef __IDX_ENTRY_H__
#define __IDX_ENTRY_H__

#include <wx/dynarray.h>

#define MAXNAMELEN 90

class TCDataSource;
class Station_Data;

typedef enum {
    SOURCE_TYPE_UNKNOWN,
    SOURCE_TYPE_ASCII_HARMONIC,
    SOURCE_TYPE_BINARY_HARMONIC,
} source_data_t;

class IDX_entry
{
public:
    IDX_entry();
    ~IDX_entry();

    source_data_t       source_data_type;
    TCDataSource        *pDataSource;
    char                source_ident[MAXNAMELEN];       // actually, the file name

    int       IDX_rec_num;                   // Keeps track of multiple entries w/same name
    char      IDX_type;                      // Entry "TCtcIUu" identifier
    char      IDX_zone[40];                  // Alpha region/country/state ID
    char      IDX_station_name[MAXNAMELEN];  // Name of station
    double    IDX_lon;                       // Longitude (+East)
    double    IDX_lat;                       // Latitude (+North)
    int       IDX_ht_time_off;               // High tide offset in minutes
    float     IDX_ht_mpy;                    // High tide multiplier (nom 1.0)
    float     IDX_ht_off;                    // High tide level offset (feet?)
    int       IDX_lt_time_off;               // Low tide offset in minutes
    float     IDX_lt_mpy;                    // Low tide multiplier (nom 1.0)
    float     IDX_lt_off;                    // Low tide level offset (feet?)
    int       IDX_sta_num;                   // Subordinate station number, **UNUSED**
    int       IDX_flood_dir;                 // Added DSR opencpn
    int       IDX_ebb_dir;
    int       IDX_Useable;
    int       Valid15;
    float     Value15;
    float     Dir15;
    bool      Ret15;
    char     *IDX_tzname;                    // Timezone name
    int       IDX_ref_file_num;              // # of reference file where reference station is
    char      IDX_reference_name[MAXNAMELEN];// Name of reference station
    int       IDX_ref_dbIndex;               // tcd index of reference station
    double    max_amplitude;
    int       have_offsets;
    int       station_tz_offset;             // Offset in seconds to convert from harmonic data (epochs) to
                                             // the station time zone.  Depends on Master Station reference only.
                                             // For ASCII data, typically 0
                                             // For Binary data, probably -(IDX_time_zone * 60)-(tiderec->zone_offset * 3600)
    int       IDX_time_zone;                 // Station location minutes offset from UTC


    Station_Data   *pref_sta_data;           // Pointer to the Reference Station Data

    int         num_nodes;                   // These are copies of relevant data pointers
    int         num_csts;                    // allocated during invariant harmonic loading
    int         num_epochs;                  // and owned by the DataSource
    double      *m_cst_speeds;
    double      **m_cst_nodes;
    double      **m_cst_epochs;
    double      *m_work_buffer;
    int         first_year;
    time_t      epoch;
    int         epoch_year;

};

WX_DECLARE_OBJARRAY(IDX_entry, ArrayOfIDXEntry);

#endif
