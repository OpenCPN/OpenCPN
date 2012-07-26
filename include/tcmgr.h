/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Tide and Current Manager
 * Author:   David Register
 * Todo add original author
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
 *
 **
 *
 */



#ifndef __TCMGR_H__
#define __TCMGR_H__


// ----------------------------------------------------------------------------
// external C linkages
// ----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------
typedef enum {LENGTH, VELOCITY, BOGUS} unit_type;
typedef struct {
    char *name;
    char *abbrv;
    unit_type type;
    double conv_factor;
} unit;

#define NUMUNITS 4

#define MAXNAMELEN 90
#define USF_REMOVE 1
#define USF_UPDATE 2
#define USF_WRITE  3

#ifndef M_PI
#define M_PI 3.141592654
#endif

#define linelen 300

#define TIDE_MAX_DERIV (2)      // Maximum derivative supported
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
#define TIDE_BAD_TIME   ((time_t) -1)


//    class/struct declarations

class Station_Data;

typedef struct {
    int   type;
    char *short_s;
    char *long_s;
} abbreviation_entry;

#define REGION 1
#define COUNTRY 2
#define STATE 3

typedef struct {
    void     *next;
    short int rec_start;
    char     *name;
} harmonic_file_entry;


//----------------------------------------------------------------------------
//   Reference Station Data
//----------------------------------------------------------------------------

class Station_Data
{
public:
    Station_Data();
    ~Station_Data();
    char        *station_name;
    wxChar      station_type;            // T or C
    double      *amplitude;
    double      *epoch;
    double      DATUM;
    int         meridian;
    char        tzfile[40];
    char        unit[40];
    char        units_conv[40];         // printable converted units
    char        units_abbrv[20];        // and abbreviation
    int         have_BOGUS;
};


typedef struct {
    Station_Data      *sta_data;
    void        *next;
} mru_entry;



class TCDataFactory;
class TCDataSource;
class TCDS_Ascii_Harmonic;
class TCDS_Binary_Harmonic;

class abbr_entry
{
public:
    int         type;
    wxString    short_s;
    wxString    long_s;
};




typedef enum {
    TC_NO_ERROR,
    TC_GENERIC_ERROR,
    TC_FILE_NOT_FOUND,
    TC_INDEX_FILE_CORRUPT,
    TC_INDEX_ENTRY_BAD,
    TC_HARM_FILE_CORRUPT,
    TC_MASTER_HARMONICS_NOT_FOUND
} TC_Error_Code;

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

    int       IDX_rec_num;                   // Keeps track of multiple entries w/same name
    char      IDX_type;                      // Entry "TCtcIUu" identifier
    char      IDX_zone[40];                  // Alpha region/country/state ID
    char      IDX_station_name[MAXNAMELEN];  // Name of station
    double    IDX_lon;                       // Longitude (+East)
    double    IDX_lat;                       // Latitude (+North)
    int       IDX_time_zone;                 // Minutes offset from UTC
    int       IDX_ht_time_off;               // High tide offset in minutes
    float     IDX_ht_mpy;                    // High tide multiplier (nom 1.0)
    float     IDX_ht_off;                    // High tide level offset (feet?)
    int       IDX_lt_time_off;               // Low tide offset in minutes
    float     IDX_lt_mpy;                    // Low tide multiplier (nom 1.0)
    float     IDX_lt_off;                    // Low tide level offset (feet?)
    int       IDX_sta_num;                   // Subordinate station number
    int       IDX_flood_dir;                 // Added DSR opencpn
    int       IDX_ebb_dir;
    int       IDX_Useable;
    int       Valid15;
    float     Value15;
    float     Dir15;
    bool      Ret15;
    bool      b_is_secondary;
    char     *IDX_tzname;                    // Timezone name
    int       IDX_ref_file_num;              // # of reference file where reference station is
    char      IDX_reference_name[MAXNAMELEN];// Name of reference station
    int       IDX_ref_dbIndex;               // tcd index of reference station
    double    max_amplitude;
    int       have_offsets;


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

};




WX_DECLARE_OBJARRAY( abbr_entry, ArrayOfAbbrEntry);
WX_DECLARE_OBJARRAY( IDX_entry, ArrayOfIDXEntry);
WX_DECLARE_OBJARRAY( TCDataSource, ArrayOfTCDSources);
WX_DECLARE_OBJARRAY( Station_Data, ArrayOfStationData);



//----------------------------------------------------------------------------
//   TCMgr
//----------------------------------------------------------------------------

class TCMgr
{
public:
    TCMgr();
    ~TCMgr();

    TC_Error_Code LoadDataSources(wxArrayString &sources);
    wxArrayString GetDataSet( void ) {
        return m_sourcefile_array;
    }

    bool IsReady(void) {
        return bTCMReady;
    }

    bool GetTideOrCurrent(time_t t, int idx, float &value, float& dir);
    bool GetTideOrCurrent15(time_t t, int idx, float &tcvalue, float& dir, bool &bnew_val);
    bool GetTideFlowSens(time_t t, int sch_step, int idx, float &tcvalue_now, float &tcvalue_prev, bool &w_t);
    void GetHightOrLowTide(time_t t, int sch_step_1, int sch_step_2, float tide_val ,bool w_t , int idx, float &tcvalue, time_t &tctime);

    int GetStationTimeOffset(IDX_entry *pIDX);
    int GetNextBigEvent(time_t *tm, int idx);

    IDX_entry *GetIDX_entry(int index);

    int Get_max_IDX() {
        return m_Combined_IDX_array.GetCount()-1;
    }
    int GetStationIDXbyName(wxString prefix, double xlat, double xlon);
    int GetStationIDXbyNameType(wxString prefix, double xlat, double xlon, char type);

private:
    void PurgeData();
    
    void LoadMRU(void);
    void SaveMRU(void);
    void AddMRU(Station_Data *psd);
    void FreeMRU(void);

    bool bTCMReady;
    wxString pmru_file_name;

    ArrayOfTCDSources   m_source_array;
    wxArrayString       m_sourcefile_array;

    ArrayOfIDXEntry     m_Combined_IDX_array;

};


//      TCDataSource Definition
class TCDataSource
{
public:
    TCDataSource();
    ~TCDataSource();

    TC_Error_Code LoadData(wxString &data_file_path);

    int GetMaxIndex(void);
    IDX_entry *GetIndexEntry(int n_index);
    TC_Error_Code LoadHarmonicData(IDX_entry *pIDX);

private:

    wxString             m_data_source_path;
    TCDataFactory        *m_pfactory;
    TCDS_Ascii_Harmonic  *pTCDS_Ascii_Harmonic;
    TCDS_Binary_Harmonic *pTCDS_Binary_Harmonic;

};

//      TCDataFactory Definition
class TCDataFactory
{
public:
    TCDataFactory();
    virtual ~TCDataFactory();

    virtual TC_Error_Code LoadData(wxString &data_file_path) = 0;

    virtual int GetMaxIndex(void) = 0;
    virtual IDX_entry *GetIndexEntry(int n_index) = 0;


private:

};


//      TCDS_Ascii_Harmonic Definition
class TCDS_Ascii_Harmonic : public TCDataFactory
{
public:
    TCDS_Ascii_Harmonic();
    ~TCDS_Ascii_Harmonic();

    TC_Error_Code LoadData(wxString &data_file_path);

    int GetMaxIndex(void) {
        return num_IDX;
    };
    IDX_entry *GetIndexEntry(int n_index);
    TC_Error_Code LoadHarmonicData(IDX_entry *pIDX);

private:
    long IndexFileIO(int func, long value);
    TC_Error_Code init_index_file();
    TC_Error_Code build_IDX_entry(IDX_entry *pIDX );
    TC_Error_Code LoadHarmonicConstants(wxString &data_file_path);
    int read_next_line (FILE *fp, char linrec[linelen], int end_ok);
    int skipnl (FILE *fp);
    char *nojunk (char *line);
    int slackcmp (char *a, char *b);
    int findunit (const char *unit);

    void free_cst();
    void free_nodes();
    void free_epochs();
    void free_data();

    ArrayOfStationData  m_msd_array;

    wxString            m_indexfile_name;
    wxString            m_harmfile_name;
    wxString            m_last_reference_not_found;

    char                index_line_buffer[1024];
    FILE                *m_IndexFile;
    ArrayOfAbbrEntry    m_abbreviation_array;
    ArrayOfIDXEntry     m_IDX_array;

    unit              known_units[NUMUNITS];

    int         num_IDX;
    int         num_nodes;
    int         num_csts;
    int         num_epochs;
    double      *m_cst_speeds;
    double      **m_cst_nodes;
    double      **m_cst_epochs;
    double      *m_work_buffer;
    int         m_first_year;


};

//      TCDS_Binary_Harmonic Definition
class TCDS_Binary_Harmonic : public TCDataFactory
{
public:
    TCDS_Binary_Harmonic();
    ~TCDS_Binary_Harmonic();

    TC_Error_Code LoadData(wxString &data_file_path);
   
    int GetMaxIndex(void);
    IDX_entry *GetIndexEntry(int n_index);
    TC_Error_Code LoadHarmonicData(IDX_entry *pIDX);

private:

};



#endif
