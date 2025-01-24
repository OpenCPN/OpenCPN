/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Tide and Current Manager
 * Author:   David Register
 * Todo add original author
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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
 **************************************************************************/

#ifndef __TCMGR_H__
#define __TCMGR_H__

#include <map>
#include <vector>
#include <string>

#include <wx/datetime.h>

#include "Station_Data.h"
#include "IDX_entry.h"
#include "TC_Error_Code.h"
#include "TCDataSource.h"

// ----------------------------------------------------------------------------
// external C linkages
// ----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------

#define USF_REMOVE 1
#define USF_UPDATE 2
#define USF_WRITE 3

#ifndef M_PI
#define M_PI 3.141592654
#endif

#define TIDE_MAX_DERIV (2)  // Maximum derivative supported
/* TIDE_TIME_PREC
 *   Precision (in seconds) to which we will find roots
 */
#define TIDE_TIME_PREC (15)

/* TIDE_TIME_BLEND
 *   Half the number of seconds over which to blend the tides from
 *   one epoch to the next.
 */
#define TIDE_BLEND_TIME (3600)

/* TIDE_TIME_STEP
 *   We are guaranteed to find all high and low tides as long as their
 * spacing is greater than this value (in seconds).
 */
#define TIDE_TIME_STEP (TIDE_TIME_PREC)
#define TIDE_BAD_TIME ((time_t) - 1)

//----------------------------------------------------------------------------
//   Reference Station Data
//----------------------------------------------------------------------------

typedef struct {
  Station_Data *sta_data;
  void *next;
} mru_entry;

// Define tide event.
class TideEvent {
private:
  int m_station_id;
  double m_lat;
  double m_lon;

  // type of tide
  int m_event;
  wxDateTime m_ref_dt;
  wxDateTime m_event_dt;

  std::wstring GetEventStr();
  wxDateTime GetLocalTime();

public:
  int m_offset;
  std::wstring m_station_name;

  TideEvent();
  TideEvent(int station_id, wxDateTime ref_dt, double lat, double lon);

  std::wstring GetEventStr(std::wstring dt_type, const char *dt_format);
  std::wstring GetLocalTimeStr(const char *dt_format);
  std::string GetLocalOffsetStr(std::wstring dt_type);

  void SetStationName(const std::wstring &station_name);
};

//----------------------------------------------------------------------------
//   TCMgr
//----------------------------------------------------------------------------

class TCMgr {
public:
  TCMgr();
  ~TCMgr();

  TC_Error_Code LoadDataSources(std::vector<std::string> &sources);
  std::vector<std::string> GetDataSet(void) { return m_sourcefile_array; }

  bool IsReady(void) { return bTCMReady; }

  bool GetTideOrCurrent(time_t t, int idx, float &value, float &dir);
  bool GetTideOrCurrent15(time_t t, int idx, float &tcvalue, float &dir,
                          bool &bnew_val);
  bool GetTideFlowSens(time_t t, int sch_step, int idx, float &tcvalue_now,
                       float &tcvalue_prev, bool &w_t);
  void GetHightOrLowTide(time_t t, int sch_step_1, int sch_step_2,
                         float tide_val, bool w_t, int idx, float &tcvalue,
                         time_t &tctime);

  int GetStationTimeOffset(IDX_entry *pIDX);
  int GetNextBigEvent(time_t *tm, int idx);
  double GetStationLat(IDX_entry *pIDX);
  double GetStationLon(IDX_entry *pIDX);

  const IDX_entry *GetIDX_entry(int index) const;

  int Get_max_IDX() const { return m_Combined_IDX_array.size() - 1; }

  std::map<double, const IDX_entry *> GetStationsForLL(double xlat,
                                                       double xlon) const;

  int GetStationIDXbyName(const wxString &prefix, double xlat,
                          double xlon) const;
  int GetStationIDXbyNameType(const wxString &prefix, double xlat, double xlon,
                              char type) const;
  void ScrubCurrentDepths();

  TideEvent GetTideEvent(std::wstring station_name, wxDateTime ref_dt,
                         double xlat, double xlon);

private:
  void PurgeData();

  void LoadMRU(void);
  void SaveMRU(void);
  void AddMRU(Station_Data *psd);
  void FreeMRU(void);

  bool bTCMReady;
  wxString pmru_file_name;

  ArrayOfTCDSources m_source_array;
  std::vector<std::string> m_sourcefile_array;

  std::vector<IDX_entry *> m_Combined_IDX_array;
};

/* $Id: tcd.h.in 3744 2010-08-17 22:34:46Z flaterco $ */
/* tcd.h.  Generated from tcd.h.in by configure. */

#ifndef __OAML_TIDES_H__
#define __OAML_TIDES_H__

/*****************************************************************************
 *
 *                            DISTRIBUTION STATEMENT
 *
 *    This source file is unclassified, distribution unlimited, public
 *    domain.  It is distributed in the hope that it will be useful, but
 *    WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 ******************************************************************************/

/*
 *  Data types for libtcd API
 *  Formerly nvtypes.h, NAVO Standard Data Type Definitions
 *
 *  This section may appear slightly different from one platform to the
 *  next.  The build process for libtcd generates #includes and data
 *  type definitions as needed to provide integer types of specific
 *  sizes.
 */

/* Generated includes */
#include <sys/types.h>
#ifndef __WXMSW__
#include <inttypes.h>

/* Generated data types */
#define NV_BYTE int8_t
#define NV_INT16 int16_t
#define NV_INT32 int32_t
#define NV_INT64 int64_t
#define NV_U_BYTE uint8_t
#define NV_U_INT16 uint16_t
#define NV_U_INT32 uint32_t
#define NV_U_INT64 uint64_t
#else
#define NV_BYTE unsigned char
#define NV_INT16 short
#define NV_INT32 int
#define NV_INT64 long
#define NV_U_BYTE unsigned char
#define NV_U_INT16 unsigned short
#define NV_U_INT32 unsigned int
#define NV_U_INT64 unsigned long

#endif

/* Hard-coded data types */
#define NV_BOOL unsigned char
#define NV_CHAR char
#define NV_U_CHAR unsigned char
#define NV_FLOAT32 float
#define NV_FLOAT64 double

/* Defined values */
#define NVFalse 0
#define NVTrue 1
#define NV_U_INT32_MAX 4294967295
#define NV_INT32_MAX 2147483647
#define NV_U_INT16_MAX 65535
#define NV_INT16_MAX 32767

#define LIBTCD_VERSION "PFM Software - libtcd v2.2.5 - 2010-08-17"
#define LIBTCD_MAJOR_REV 2
#define LIBTCD_MINOR_REV 2

/*
 *  COMPAT114 is defined here if and only if libtcd was compiled with
 *  --enable-COMPAT114.  COMPAT114 forces API changes to enable
 *  applications written against the v1 API to build with more recent
 *  libraries and successfully interact with v2 TCD files.  This will
 *  BREAK applications expecting the v2 API.  It is for localized use
 *  only (libraries built with this option should not be distributed).
 */
#undef COMPAT114

/* Select #warning versus #pragma message (whichever works) */
#undef USE_PRAGMA_MESSAGE

#ifdef COMPAT114
#ifdef USE_PRAGMA_MESSAGE
#pragma message("WARNING:  COMPAT114 is enabled!  See libtcd.html.")
#else
#warning COMPAT114 is enabled!  See libtcd.html.
#endif
#endif

/*  Maximum values.  */

/* One-line character strings */
#define ONELINER_LENGTH 90
/* Verbose character strings */
#define MONOLOGUE_LENGTH 10000
#define MAX_CONSTITUENTS 255

#ifdef __cplusplus
extern "C" {
#endif

/*  Public portion of the database header.  */
/* See libtcd.html for documentation */

typedef struct {
  NV_CHAR version[ONELINER_LENGTH];
  NV_U_INT32 major_rev;
  NV_U_INT32 minor_rev;
  NV_CHAR last_modified[ONELINER_LENGTH];
  NV_U_INT32 number_of_records;
  NV_INT32 start_year;
  NV_U_INT32 number_of_years;
  NV_U_INT32 constituents;
  NV_U_INT32 level_unit_types;
  NV_U_INT32 dir_unit_types;
  NV_U_INT32 restriction_types;
  NV_U_INT32 datum_types;
  NV_U_INT32 countries;
  NV_U_INT32 tzfiles;
  NV_U_INT32 legaleses;

  /* Need this to read V1 files. */
  NV_U_INT32 pedigree_types;
} DB_HEADER_PUBLIC;

/*  Header portion of each station record.  */
/* See libtcd.html for documentation */

enum TIDE_RECORD_TYPE { REFERENCE_STATION = 1, SUBORDINATE_STATION = 2 };

typedef struct {
  NV_INT32 record_number;
  NV_U_INT32 record_size;
  NV_U_BYTE record_type;
  NV_FLOAT64 latitude;
  NV_FLOAT64 longitude;
  NV_INT32 reference_station;
  NV_INT16 tzfile;
  NV_CHAR name[ONELINER_LENGTH];
} TIDE_STATION_HEADER;

/*  Tide station record.  */
/* See libtcd.html for documentation */

typedef struct {
  /* Common */
  TIDE_STATION_HEADER header;
  NV_INT16 country;
  NV_CHAR source[ONELINER_LENGTH];
  NV_U_BYTE restriction;
  NV_CHAR comments[MONOLOGUE_LENGTH];
  NV_CHAR notes[MONOLOGUE_LENGTH];
  NV_U_BYTE legalese;
  NV_CHAR station_id_context[ONELINER_LENGTH];
  NV_CHAR station_id[ONELINER_LENGTH];
  NV_U_INT32 date_imported;
  NV_CHAR xfields[MONOLOGUE_LENGTH];
  NV_U_BYTE direction_units;
  NV_INT32 min_direction;
  NV_INT32 max_direction;
  NV_U_BYTE level_units;

  /* Type 1 */
  NV_FLOAT32 datum_offset;
  NV_INT16 datum;
  NV_INT32 zone_offset;
  NV_U_INT32 expiration_date;
  NV_U_INT16 months_on_station;
  NV_U_INT32 last_date_on_station;
  NV_U_BYTE confidence;
  NV_FLOAT32 amplitude[MAX_CONSTITUENTS];
  NV_FLOAT32 epoch[MAX_CONSTITUENTS];

  /* Type 2 */
  NV_INT32 min_time_add;
  NV_FLOAT32 min_level_add;
  NV_FLOAT32 min_level_multiply;
  NV_INT32 max_time_add;
  NV_FLOAT32 max_level_add;
  NV_FLOAT32 max_level_multiply;
  NV_INT32 flood_begins;
  NV_INT32 ebb_begins;

#ifdef COMPAT114
  /* Deprecated */
  NV_INT16 pedigree;
  NV_U_BYTE units;
  NV_U_BYTE avg_level_units;
  NV_FLOAT32 min_avg_level;
  NV_FLOAT32 max_avg_level;
#endif
} TIDE_RECORD;

/* DWF: This value signifies "null" or "omitted" slack offsets
 *   (flood_begins, ebb_begins).  Zero is *not* the same. */
/* Time offsets are represented as hours * 100 plus minutes.
 *   0xA00 = 2560
 *   It turns out that offsets do exceed 24 hours (long story), but we
 *   should still be safe with the 60.
 */
#define NULLSLACKOFFSET 0xA00

/* This is the level below which an amplitude rounds to zero. */
/* It should be exactly (0.5 / DEFAULT_AMPLITUDE_SCALE). */
#define AMPLITUDE_EPSILON 0.00005

/* Rounding function. */
#define NINT(a) ((a) < 0.0 ? (NV_INT32)((a) - 0.5) : (NV_INT32)((a) + 0.5))

/*  Public function prototypes.  */

/* Prints a low-level dump of the tide record to stderr. */
void dump_tide_record(const TIDE_RECORD *rec);

/* For fields in the tide record that are indices into tables of
 *   character string values, these functions are used to retrieve the
 *   character string value corresponding to a particular index.  The
 *   value "Unknown" is returned when no translation exists.  The return
 *   value is a pointer into static memory. */
const NV_CHAR *get_country(NV_INT32 num);
const NV_CHAR *get_tzfile(NV_INT32 num);
const NV_CHAR *get_level_units(NV_INT32 num);
const NV_CHAR *get_dir_units(NV_INT32 num);
const NV_CHAR *get_restriction(NV_INT32 num);
const NV_CHAR *get_datum(NV_INT32 num);
const NV_CHAR *get_legalese(NV_INT32 num);

/* Get the name of the constituent corresponding to index num
 *   [0,constituents-1].  The return value is a pointer into static
 *   memory. */
const NV_CHAR *get_constituent(NV_INT32 num);

/* Get the name of the station whose record_number is num
 *   [0,number_of_records-1].  The return value is a pointer into static
 *   memory. */
const NV_CHAR *get_station(NV_INT32 num);

/* Returns the speed of the constituent indicated by num
 *   [0,constituents-1]. */
NV_FLOAT64 get_speed(NV_INT32 num);

/* Get the equilibrium argument and node factor for the constituent
 *   indicated by num [0,constituents-1], for the year
 *   start_year+year. */
NV_FLOAT32 get_equilibrium(NV_INT32 num, NV_INT32 year);
NV_FLOAT32 get_node_factor(NV_INT32 num, NV_INT32 year);

/* Get all available equilibrium arguments and node factors for the
 *   constituent indicated by num [0,constituents-1].  The return value
 *   is a pointer into static memory which is an array of
 *   number_of_years floats, corresponding to the years start_year
 *   through start_year+number_of_years-1. */
NV_FLOAT32 *get_equilibriums(NV_INT32 num);
NV_FLOAT32 *get_node_factors(NV_INT32 num);

/* Convert between character strings of the form "[+-]HH:MM" and the
 *   encoding Hours * 100 + Minutes.  ret_time pads the hours with a
 *   leading zero when less than 10; ret_time_neat omits the leading
 *   zero and omits the sign when the value is 0:00.  Returned pointers
 *   point into static memory. */
NV_INT32 get_time(const NV_CHAR *string);
NV_CHAR *ret_time(NV_INT32 time);
NV_CHAR *ret_time_neat(NV_INT32 time);

/* Convert the encoding Year * 10000 + Month [1, 12] * 100 + Day [1,
 *   31] to a character string of the form "YYYY-MM-DD", or "NULL" if
 *   the value is zero.  The returned pointer points into static memory.
 *   (The compact form, without hyphens, is obtainable just by printing
 *   the integer.) */
NV_CHAR *ret_date(NV_U_INT32 date);

/* When invoked multiple times with the same string, returns record
 *   numbers of all stations that have that string anywhere in the
 *   station name.  This search is case insensitive.  When no more
 *   records are found it returns -1. */
NV_INT32 search_station(const NV_CHAR *string);

/* Inverses of the corresponding get_ operations.  Return -1 for not
 *   found. */
NV_INT32 find_station(const NV_CHAR *name);
NV_INT32 find_tzfile(const NV_CHAR *name);
NV_INT32 find_country(const NV_CHAR *name);
NV_INT32 find_level_units(const NV_CHAR *name);
NV_INT32 find_dir_units(const NV_CHAR *name);
NV_INT32 find_restriction(const NV_CHAR *name);
NV_INT32 find_datum(const NV_CHAR *name);
NV_INT32 find_constituent(const NV_CHAR *name);
NV_INT32 find_legalese(const NV_CHAR *name);

/* Add the value of name to the corresponding lookup table and return
 *   the index of the new value.  If db is not NULL, the database header
 *   struct pointed to will be updated to reflect the changes. */
NV_INT32 add_restriction(const NV_CHAR *name, DB_HEADER_PUBLIC *db);
NV_INT32 add_tzfile(const NV_CHAR *name, DB_HEADER_PUBLIC *db);
NV_INT32 add_country(const NV_CHAR *name, DB_HEADER_PUBLIC *db);
NV_INT32 add_datum(const NV_CHAR *name, DB_HEADER_PUBLIC *db);
NV_INT32 add_legalese(const NV_CHAR *name, DB_HEADER_PUBLIC *db);

/* Add the value of name to the corresponding lookup table if and
 *   only if it is not already present.  Return the index of the value.
 *   If db is not NULL, the database header struct pointed to will be
 *   updated to reflect the changes. */
NV_INT32 find_or_add_restriction(const NV_CHAR *name, DB_HEADER_PUBLIC *db);
NV_INT32 find_or_add_tzfile(const NV_CHAR *name, DB_HEADER_PUBLIC *db);
NV_INT32 find_or_add_country(const NV_CHAR *name, DB_HEADER_PUBLIC *db);
NV_INT32 find_or_add_datum(const NV_CHAR *name, DB_HEADER_PUBLIC *db);
NV_INT32 find_or_add_legalese(const NV_CHAR *name, DB_HEADER_PUBLIC *db);

/* Set the speed for the constituent corresponding to index num
 *   [0,constituents-1]. */
void set_speed(NV_INT32 num, NV_FLOAT64 value);

/* Set the equilibrium argument and node factor for the constituent
 *   corresponding to index num [0,constituents-1], for the year
 *   start_year+year. */
void set_equilibrium(NV_INT32 num, NV_INT32 year, NV_FLOAT32 value);
void set_node_factor(NV_INT32 num, NV_INT32 year, NV_FLOAT32 value);

/* Opens the specified TCD file.  If a different database is already
 *   open, it will be closed.  libtcd maintains considerable internal
 *   state and can only handle one open database at a time.  Returns
 *   false if the open failed. */
NV_BOOL open_tide_db(const NV_CHAR *file);

/* Closes the open database. */
void close_tide_db();

/* Creates a TCD file with the supplied constituents and no tide
 *   stations.  Returns false if creation failed.  The database is left
 *   in an open state. */
NV_BOOL create_tide_db(const NV_CHAR *file, NV_U_INT32 constituents,
                       NV_CHAR const *const constituent[],
                       const NV_FLOAT64 *speed, NV_INT32 start_year,
                       NV_U_INT32 num_years,
                       NV_FLOAT32 const *const equilibrium[],
                       NV_FLOAT32 const *const node_factor[]);

/* Returns a copy of the database header for the open database. */
DB_HEADER_PUBLIC get_tide_db_header();

/* Gets "header" portion of tide record for the station whose
 *   record_number is num [0,number_of_records-1] and writes it into
 *   rec.  Returns false if num is out of range.  num is preserved in
 *   the static variable current_index. */
NV_BOOL get_partial_tide_record(NV_INT32 num, TIDE_STATION_HEADER *rec);

/* Invokes get_partial_tide_record for current_index+1.  Returns the
 *   record number or -1 for failure. */
NV_INT32 get_next_partial_tide_record(TIDE_STATION_HEADER *rec);

/* Invokes get_partial_tide_record for a station that appears closest
 *   to the specified lat and lon in the Cylindrical Equidistant
 *   projection.  Returns the record number or -1 for failure. */
NV_INT32 get_nearest_partial_tide_record(NV_FLOAT64 lat, NV_FLOAT64 lon,
                                         TIDE_STATION_HEADER *rec);

/* Gets tide record for the station whose record_number is num
 *   [0,number_of_records-1] and writes it into rec.  num is preserved
 *   in the static variable current_record.  Returns num, or -1 if num is
 *   out of range. */
NV_INT32 read_tide_record(NV_INT32 num, TIDE_RECORD *rec);

/* Invokes read_tide_record for current_record+1.  Returns the record
 *   number or -1 for failure. */
NV_INT32 read_next_tide_record(TIDE_RECORD *rec);

/* Add a new record, update an existing record, or delete an existing
 *   record.  If the deleted record is a reference station, all
 *   dependent subordinate stations will also be deleted.  Add and
 *   update return false if the new record is invalid; delete and update
 *   return false if the specified num is invalid.  If db is not NULL,
 *   the database header struct pointed to will be updated to reflect
 *   the changes. */
NV_BOOL add_tide_record(TIDE_RECORD *rec, DB_HEADER_PUBLIC *db);
#ifdef COMPAT114
/* Omission of db parameter was a bug. */
NV_BOOL update_tide_record(NV_INT32 num, TIDE_RECORD *rec);
#else
NV_BOOL update_tide_record(NV_INT32 num, TIDE_RECORD *rec,
                           DB_HEADER_PUBLIC *db);
#endif
NV_BOOL delete_tide_record(NV_INT32 num, DB_HEADER_PUBLIC *db);

/* Computes inferred constituents when M2, S2, K1, and O1 are given
 *   and fills in the remaining unfilled constituents.  The inferred
 *   constituents are developed or decided based on Article 230 of
 *   "Manual of Harmonic Analysis and Prediction of Tides," Paul
 *   Schureman, C&GS Special Publication No. 98, October 1971.  Returns
 *   false if M2, S2, K1, or O1 is missing. */
NV_BOOL infer_constituents(TIDE_RECORD *rec);

#ifdef COMPAT114
/* Deprecated stuff. */
#define NAME_LENGTH ONELINER_LENGTH
#define SOURCE_LENGTH ONELINER_LENGTH
#define COMMENTS_LENGTH MONOLOGUE_LENGTH
NV_CHAR *get_pedigree(NV_INT32 num);
NV_INT32 find_pedigree(const NV_CHAR *name);
NV_INT32 add_pedigree(const NV_CHAR *name, const DB_HEADER_PUBLIC *db);
NV_BOOL check_simple(TIDE_RECORD rec);
#endif

#ifdef __cplusplus
}
#endif

#endif

/* $Id: tide_db_header.h 1092 2006-11-16 03:02:42Z flaterco $ */

/*****************************************************************************\
 *
 *                            DISTRIBUTION STATEMENT
 *
 *    This source file is unclassified, distribution unlimited, public
 *    domain.  It is distributed in the hope that it will be useful, but
 *    WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * \*****************************************************************************/

/***************************************************************************\
 *
 *    Module Name:        TIDE DB option header file
 *
 *    Programmer(s):      Jan C. Depner
 *
 *    Date Written:       July 2002
 *
 *    Purpose:            This header file defines all of the format options
 *                        that are stored in the TIDE DB file header.  To add a
 *                        new option you simply add it to the header_data
 *                        structure, then put a definition in the keys
 structure.
 *                        The keys structure contains a keyphrase such as
 *                        "[DATE]", a format like "%s", a datatype (see the
 *                        union in the TIDE_HEADER_DATA structure, a pointer to
 *                        the location at which to store the value(s), a count
 *                        if it is a repeating field (array), and a version
 *                        dependency flag (major.minor * 10).
 *
 *
 *
 *****************************************************************************

 This header file uses the NAVO standard data types.  You must set one of
 the following options on the compile command line:

 NVDOS, NVHPUX, NVIRIX, NVCLIX, NVLinux, NVSUNOS, NVWINNT, or NVWIN3X

 The only one referenced in libtcd is NVWIN3X. -- DWF, 2004-09-29

 \***************************************************************************/

#ifndef __TIDE_HEADER__
#define __TIDE_HEADER__

// #include "tcd.h"

typedef struct {
  const NV_CHAR *keyphrase;
  const NV_CHAR *datatype;
  union {
    const NV_CHAR *cstr;
    NV_INT32 *i32;
    NV_U_INT32 *ui32;
  } address;
} KEY;

typedef struct {
  DB_HEADER_PUBLIC pub;
  NV_CHAR **constituent;
  NV_FLOAT64 *speed;
  NV_FLOAT32 **equilibrium;
  NV_FLOAT32 **node_factor;
  NV_CHAR **level_unit;
  NV_CHAR **dir_unit;
  NV_CHAR **restriction;
  NV_CHAR **tzfile;
  NV_CHAR **country;
  NV_CHAR **datum;
  NV_CHAR **legalese;
  NV_U_INT32 header_size;
  NV_U_INT32 speed_bits;
  NV_U_INT32 speed_scale;
  NV_INT32 speed_offset;
  NV_U_INT32 equilibrium_bits;
  NV_U_INT32 equilibrium_scale;
  NV_INT32 equilibrium_offset;
  NV_U_INT32 node_bits;
  NV_U_INT32 node_scale;
  NV_INT32 node_offset;
  NV_U_INT32 amplitude_bits;
  NV_U_INT32 amplitude_scale;
  NV_U_INT32 epoch_bits;
  NV_U_INT32 epoch_scale;
  NV_U_INT32 constituent_bits;
  NV_U_INT32 record_type_bits;
  NV_U_INT32 latitude_scale;
  NV_U_INT32 latitude_bits;
  NV_U_INT32 longitude_scale;
  NV_U_INT32 longitude_bits;
  NV_U_INT32 record_size_bits;
  NV_U_INT32 station_bits;
  NV_U_INT32 level_unit_bits;
  NV_U_INT32 dir_unit_bits;
  NV_U_INT32 restriction_bits;
  NV_U_INT32 max_restriction_types;
  NV_U_INT32 tzfile_bits;
  NV_U_INT32 max_tzfiles;
  NV_U_INT32 country_bits;
  NV_U_INT32 max_countries;
  NV_U_INT32 datum_bits;
  NV_U_INT32 max_datum_types;
  NV_U_INT32 legalese_bits;
  NV_U_INT32 max_legaleses;
  NV_U_INT32 datum_offset_bits;
  NV_U_INT32 datum_offset_scale;
  NV_U_INT32 date_bits;
  NV_U_INT32 months_on_station_bits;
  NV_U_INT32 confidence_value_bits;
  NV_U_INT32 time_bits;
  NV_U_INT32 level_add_bits;
  NV_U_INT32 level_add_scale;
  NV_U_INT32 level_multiply_bits;
  NV_U_INT32 level_multiply_scale;
  NV_U_INT32 direction_bits;
  NV_U_INT32 constituent_size;
  NV_U_INT32 level_unit_size;
  NV_U_INT32 dir_unit_size;
  NV_U_INT32 restriction_size;
  NV_U_INT32 tzfile_size;
  NV_U_INT32 country_size;
  NV_U_INT32 datum_size;
  NV_U_INT32 legalese_size;
  NV_U_INT32 end_of_file;

  /* Need these to read V1 files. */
  NV_U_INT32 pedigree_bits;
  NV_U_INT32 pedigree_size;
} TIDE_HEADER_DATA;

static TIDE_HEADER_DATA hd;

/* "The initializer for a union with no constructor is either a single
 *   expression of the same type, or a brace-enclosed initializer for
 *   the first member of the union." -- Ellis & Stroustrup, The Annotated
 *   C++ Reference Manual, 1990, Section 8.4.1. */

static KEY keys[] = {{
                         "[VERSION]",
                         "cstr",
                         {(NV_CHAR *)hd.pub.version},
                     },
                     {
                         "[MAJOR REV]",
                         "ui32",
                         {(NV_CHAR *)&hd.pub.major_rev},
                     },
                     {
                         "[MINOR REV]",
                         "ui32",
                         {(NV_CHAR *)&hd.pub.minor_rev},
                     },
                     {
                         "[LAST MODIFIED]",
                         "cstr",
                         {(NV_CHAR *)hd.pub.last_modified},
                     },
                     {
                         "[NUMBER OF RECORDS]",
                         "ui32",
                         {(NV_CHAR *)&hd.pub.number_of_records},
                     },
                     {
                         "[START YEAR]",
                         "i32",
                         {(NV_CHAR *)&hd.pub.start_year},
                     },
                     {
                         "[NUMBER OF YEARS]",
                         "ui32",
                         {(NV_CHAR *)&hd.pub.number_of_years},
                     },
                     {
                         "[CONSTITUENTS]",
                         "ui32",
                         {(NV_CHAR *)&hd.pub.constituents},
                     },
                     {
                         "[LEVEL UNIT TYPES]",
                         "ui32",
                         {(NV_CHAR *)&hd.pub.level_unit_types},
                     },
                     {
                         "[DIRECTION UNIT TYPES]",
                         "ui32",
                         {(NV_CHAR *)&hd.pub.dir_unit_types},
                     },
                     {
                         "[RESTRICTION TYPES]",
                         "ui32",
                         {(NV_CHAR *)&hd.pub.restriction_types},
                     },
                     {
                         "[PEDIGREE TYPES]",
                         "ui32",
                         {(NV_CHAR *)&hd.pub.pedigree_types},
                     },
                     {
                         "[TZFILES]",
                         "ui32",
                         {(NV_CHAR *)&hd.pub.tzfiles},
                     },
                     {
                         "[COUNTRIES]",
                         "ui32",
                         {(NV_CHAR *)&hd.pub.countries},
                     },
                     {
                         "[DATUM TYPES]",
                         "ui32",
                         {(NV_CHAR *)&hd.pub.datum_types},
                     },
                     {
                         "[LEGALESES]",
                         "ui32",
                         {(NV_CHAR *)&hd.pub.legaleses},
                     },
                     {
                         "[HEADER SIZE]",
                         "ui32",
                         {(NV_CHAR *)&hd.header_size},
                     },
                     {
                         "[SPEED BITS]",
                         "ui32",
                         {(NV_CHAR *)&hd.speed_bits},
                     },
                     {
                         "[SPEED SCALE]",
                         "ui32",
                         {(NV_CHAR *)&hd.speed_scale},
                     },
                     {
                         "[SPEED OFFSET]",
                         "i32",
                         {(NV_CHAR *)&hd.speed_offset},
                     },
                     {
                         "[EQUILIBRIUM BITS]",
                         "ui32",
                         {(NV_CHAR *)&hd.equilibrium_bits},
                     },
                     {
                         "[EQUILIBRIUM SCALE]",
                         "ui32",
                         {(NV_CHAR *)&hd.equilibrium_scale},
                     },
                     {
                         "[EQUILIBRIUM OFFSET]",
                         "i32",
                         {(NV_CHAR *)&hd.equilibrium_offset},
                     },
                     {
                         "[NODE BITS]",
                         "ui32",
                         {(NV_CHAR *)&hd.node_bits},
                     },
                     {
                         "[NODE SCALE]",
                         "ui32",
                         {(NV_CHAR *)&hd.node_scale},
                     },
                     {
                         "[NODE OFFSET]",
                         "i32",
                         {(NV_CHAR *)&hd.node_offset},
                     },
                     {
                         "[AMPLITUDE BITS]",
                         "ui32",
                         {(NV_CHAR *)&hd.amplitude_bits},
                     },
                     {
                         "[AMPLITUDE SCALE]",
                         "ui32",
                         {(NV_CHAR *)&hd.amplitude_scale},
                     },
                     {
                         "[EPOCH BITS]",
                         "ui32",
                         {(NV_CHAR *)&hd.epoch_bits},
                     },
                     {
                         "[EPOCH SCALE]",
                         "ui32",
                         {(NV_CHAR *)&hd.epoch_scale},
                     },
                     {
                         "[CONSTITUENT BITS]",
                         "ui32",
                         {(NV_CHAR *)&hd.constituent_bits},
                     },
                     {
                         "[LEVEL UNIT BITS]",
                         "ui32",
                         {(NV_CHAR *)&hd.level_unit_bits},
                     },
                     {
                         "[DIRECTION UNIT BITS]",
                         "ui32",
                         {(NV_CHAR *)&hd.dir_unit_bits},
                     },
                     {
                         "[RESTRICTION BITS]",
                         "ui32",
                         {(NV_CHAR *)&hd.restriction_bits},
                     },
                     {
                         "[PEDIGREE BITS]",
                         "ui32",
                         {(NV_CHAR *)&hd.pedigree_bits},
                     },
                     {
                         "[TZFILE BITS]",
                         "ui32",
                         {(NV_CHAR *)&hd.tzfile_bits},
                     },
                     {
                         "[COUNTRY BITS]",
                         "ui32",
                         {(NV_CHAR *)&hd.country_bits},
                     },
                     {
                         "[DATUM BITS]",
                         "ui32",
                         {(NV_CHAR *)&hd.datum_bits},
                     },
                     {
                         "[LEGALESE BITS]",
                         "ui32",
                         {(NV_CHAR *)&hd.legalese_bits},
                     },
                     {
                         "[RECORD TYPE BITS]",
                         "ui32",
                         {(NV_CHAR *)&hd.record_type_bits},
                     },
                     {
                         "[LATITUDE SCALE]",
                         "ui32",
                         {(NV_CHAR *)&hd.latitude_scale},
                     },
                     {
                         "[LATITUDE BITS]",
                         "ui32",
                         {(NV_CHAR *)&hd.latitude_bits},
                     },
                     {
                         "[LONGITUDE SCALE]",
                         "ui32",
                         {(NV_CHAR *)&hd.longitude_scale},
                     },
                     {
                         "[LONGITUDE BITS]",
                         "ui32",
                         {(NV_CHAR *)&hd.longitude_bits},
                     },
                     {
                         "[RECORD SIZE BITS]",
                         "ui32",
                         {(NV_CHAR *)&hd.record_size_bits},
                     },
                     {
                         "[STATION BITS]",
                         "ui32",
                         {(NV_CHAR *)&hd.station_bits},
                     },
                     {
                         "[DATUM OFFSET BITS]",
                         "ui32",
                         {(NV_CHAR *)&hd.datum_offset_bits},
                     },
                     {
                         "[DATUM OFFSET SCALE]",
                         "ui32",
                         {(NV_CHAR *)&hd.datum_offset_scale},
                     },
                     {
                         "[DATE BITS]",
                         "ui32",
                         {(NV_CHAR *)&hd.date_bits},
                     },
                     {
                         "[MONTHS ON STATION BITS]",
                         "ui32",
                         {(NV_CHAR *)&hd.months_on_station_bits},
                     },
                     {
                         "[CONFIDENCE VALUE BITS]",
                         "ui32",
                         {(NV_CHAR *)&hd.confidence_value_bits},
                     },
                     {
                         "[TIME BITS]",
                         "ui32",
                         {(NV_CHAR *)&hd.time_bits},
                     },
                     {
                         "[LEVEL ADD BITS]",
                         "ui32",
                         {(NV_CHAR *)&hd.level_add_bits},
                     },
                     {
                         "[LEVEL ADD SCALE]",
                         "ui32",
                         {(NV_CHAR *)&hd.level_add_scale},
                     },
                     {
                         "[LEVEL MULTIPLY BITS]",
                         "ui32",
                         {(NV_CHAR *)&hd.level_multiply_bits},
                     },
                     {
                         "[LEVEL MULTIPLY SCALE]",
                         "ui32",
                         {(NV_CHAR *)&hd.level_multiply_scale},
                     },
                     {
                         "[DIRECTION BITS]",
                         "ui32",
                         {(NV_CHAR *)&hd.direction_bits},
                     },
                     {
                         "[CONSTITUENT SIZE]",
                         "ui32",
                         {(NV_CHAR *)&hd.constituent_size},
                     },
                     {
                         "[LEVEL UNIT SIZE]",
                         "ui32",
                         {(NV_CHAR *)&hd.level_unit_size},
                     },
                     {
                         "[DIRECTION UNIT SIZE]",
                         "ui32",
                         {(NV_CHAR *)&hd.dir_unit_size},
                     },
                     {
                         "[RESTRICTION SIZE]",
                         "ui32",
                         {(NV_CHAR *)&hd.restriction_size},
                     },
                     {
                         "[PEDIGREE SIZE]",
                         "ui32",
                         {(NV_CHAR *)&hd.pedigree_size},
                     },
                     {
                         "[TZFILE SIZE]",
                         "ui32",
                         {(NV_CHAR *)&hd.tzfile_size},
                     },
                     {
                         "[COUNTRY SIZE]",
                         "ui32",
                         {(NV_CHAR *)&hd.country_size},
                     },
                     {
                         "[DATUM SIZE]",
                         "ui32",
                         {(NV_CHAR *)&hd.datum_size},
                     },
                     {
                         "[LEGALESE SIZE]",
                         "ui32",
                         {(NV_CHAR *)&hd.legalese_size},
                     },
                     {
                         "[END OF FILE]",
                         "ui32",
                         {(NV_CHAR *)&hd.end_of_file},
                     }};
#endif

#endif
