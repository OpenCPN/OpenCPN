/***************************************************************************
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
 **************************************************************************/

#include "TCDS_Binary_Harmonic.h"
#include "tcmgr.h"

/* Declarations for zoneinfo compatibility */

/* Most of these entries are loaded from the tzdata.h include file. That
 *   file was generated from tzdata200c.                                  */

static const char *tz_names[][2] = {
#include "tzdata.h"

    /* Terminator */
    {NULL, NULL}
};

/*  Timelib  Time services.
 *    Original XTide source code date: 1997-08-15
 *    Last modified 1998-09-07 by Mike Hopper for WXTide32
 *
 *    Copyright (C) 1997  David Flater.
 *    Also starring:  Geoff Kuenning; Rob Miracle; Dean Pentcheff;
 *    Eric Rosen.
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *    Changes by Mike Hopper for WXTide32:
 *    Changed do_timestamp to shorten NT's LONG time zone names just the CAPS
 *    Changed _hpux selector to WIN32 to select HP timezone values.
 *    Added a whole set of remote TZ handler routines, all starting with "tz".
 */

#ifndef __WXMSW__
typedef unsigned short WORD;
typedef long LONG;
typedef char WCHAR;

typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME;


typedef struct _TIME_ZONE_INFORMATION {
    LONG       Bias;
    WCHAR      StandardName[32];
    SYSTEMTIME StandardDate;
    LONG       StandardBias;
    WCHAR      DaylightName[32];
    SYSTEMTIME DaylightDate;
    LONG       DaylightBias;
} TIME_ZONE_INFORMATION, *PTIME_ZONE_INFORMATION;
#else
#include <Windows.h>
#endif

/*-----------------9/24/2002 4:30PM-----------------
 * An attempt to get Windoz to work with non-US timezones...
 * --------------------------------------------------*/
typedef struct {
    TIME_ZONE_INFORMATION tzi;
    time_t year_beg;
    time_t year_end;
    time_t enter_std;
    time_t enter_dst;
    int    isdst;
} tz_info_entry;

tz_info_entry tz_info_local, tz_info_remote, *tz_info = &tz_info_local;


/*-----------------9/24/2002 8:12AM-----------------
 * Parse time string in the form [-][hh][:mm][:ss] into seconds.
 * Returns updated string pointer and signed seconds.
 * --------------------------------------------------*/
char *tz_time2sec( char *psrc, long *timesec ) {
    int neg;
    long temp, mpy;
    *timesec = 0;
    mpy      = 3600;
    while (*psrc == ' ') psrc++; /* Skip leading blanks */
    if (*psrc == '+') psrc++;    /* Gobble leading + */
    if (*psrc == '-') {
        neg = TRUE;
        psrc++;
    }
    else neg = FALSE;

    do {
        temp = 0;
        while (isdigit(*psrc))
            temp = temp * 10 + (*(psrc++) - '0');

        *timesec = *timesec + temp * mpy;

        if (*psrc == ':') {
            mpy /= 60;
            psrc++;
        }
    } while ( isdigit(*psrc) );

    if (neg) *timesec = 0 - *timesec;

    return( psrc );
}


/*-----------------9/24/2002 8:16AM-----------------
 * Parse timezone name string.
 * Returns string at psrc, updated psrc, and chars copied.
 * --------------------------------------------------*/
static char *tz_parse_name( char *psrc, char *pdst, int maxlen ) {
    int nReturn;

    nReturn = 0;
    while (*psrc == ' ') psrc++; /* Skip leading blanks */

    while (isalpha(*psrc) && nReturn < maxlen) {
        *(pdst++) = *(psrc++);
        nReturn++;
    }

    *pdst = 0;
    return( psrc );
}

/*-----------------9/24/2002 8:38AM-----------------
 * Parse tz rule string into SYSTEMTIME structure.
 * --------------------------------------------------*/
static char *tz_parse_rule( char *psrc, SYSTEMTIME *st ) {
    int mol[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    long temp, mo;
    if    (*psrc == ',') psrc++; /* Gobble leading comma */

    while (*psrc == ' ') psrc++; /* Skip leading blanks */

    st->wYear       = 0;
    st->wMonth      = 0;
    st->wDay        = 0;
    st->wDayOfWeek  = 0;
    st->wHour       = 0;
    st->wMinute     = 0;
    st->wSecond     = 0;
    st->wMilliseconds= 0;

    if (*psrc == 'J') {          /* Julian day (1 <= n <= 365) no leap */
        psrc++; /* Gobble 'J' */
        temp = 0;
        while (isdigit(*psrc))
            temp = temp * 10 + (*(psrc++) - '0');

        if (temp < 1 || temp > 365) return(0);
        temp--;
        for (mo=0; temp >= mol[mo]; mo++) temp -= mol[mo];
        st->wMonth = mo + 1;
        st->wDay   = temp + 1;
        st->wYear  = 1;
    }

    else if (*psrc == 'M') {
        psrc++; /* Gobble 'M' */

        temp = 0;
        while (isdigit(*psrc))
            temp = temp * 10 + (*(psrc++) - '0'); /* Get month */
        if (temp < 1 || temp > 12 || *psrc != '.') return(0);
        st->wMonth = (unsigned short)temp;

        psrc++; /* Gobble '.' */
        temp = 0;
        while (isdigit(*psrc))
            temp = temp * 10 + (*(psrc++) - '0'); /* Get week number */
        if (temp < 1 || temp > 5 || *psrc != '.') return(0);
        st->wDay = (unsigned short)temp;

        psrc++; /* Gobble '.' */
        temp = 0;
        while (isdigit(*psrc))
            temp = temp * 10 + (*(psrc++) - '0'); /* Get day of week number */
        if (temp < 0 || temp > 6) return(0);
        st->wDayOfWeek = (unsigned short)temp;
    }

    if (*psrc == '/') {          /* time is specified */
        psrc++; /* Gobble '/' */
        psrc = tz_time2sec( psrc, &temp );
        if (temp < 0 || temp >= 86400) return(0);
        st->wHour = temp / 3600;
        temp = temp % 3600;
        st->wMinute = temp / 60;
        st->wSecond = temp % 60;
    }
    return( psrc );
}


/*-----------------9/24/2002 3:38PM-----------------
 * Load tz rule into timezone info data block.
 * --------------------------------------------------*/
static void tz_load_rule( char *prule, tz_info_entry *tz_info_remote ) {

    prule = tz_parse_name( prule, (char *)tz_info_remote->tzi.StandardName, 30 );
    prule = tz_time2sec( prule, &tz_info_remote->tzi.Bias );
    tz_info_remote->tzi.Bias /= 60;
    tz_info_remote->tzi.StandardBias = 0;

    prule = tz_parse_name( prule, (char *)tz_info_remote->tzi.DaylightName, 30 );
    if ( *(char *)tz_info_remote->tzi.DaylightName != '\0' ) {
        prule = tz_time2sec( prule, &tz_info_remote->tzi.DaylightBias );
        tz_info_remote->tzi.DaylightBias /= 60;
        if ( tz_info_remote->tzi.DaylightBias == 0 )
            tz_info_remote->tzi.DaylightBias = -60;
        else tz_info_remote->tzi.DaylightBias -= tz_info_remote->tzi.Bias;

        if (*prule == ',') {
            prule = tz_parse_rule( prule, &tz_info_remote->tzi.DaylightDate );
            if (prule && *prule == ',')
                tz_parse_rule( prule, &tz_info_remote->tzi.StandardDate );
            else
                tz_parse_rule( (char *)"M10.5.0/02:00:00", &tz_info_remote->tzi.StandardDate );
        }
        else {   /* Default is US style tz change */
            tz_parse_rule( (char *)"M4.1.0/02:00:00" , &tz_info_remote->tzi.DaylightDate );
            tz_parse_rule( (char *)"M10.5.0/02:00:00", &tz_info_remote->tzi.StandardDate );
        }
    }
    else { /* No DST */
        tz_info_remote->tzi.DaylightDate.wMonth = 0;
        tz_info_remote->isdst = 0;
    }
}

/* Attempt to load up the local time zone of the location.  Moof! */
static void change_time_zone(const char *tz)
{
    //  static char env_string[MAXARGLEN+1];
    int index;

    if (*tz == ':') tz++; /* Gobble lead-in char */
    /* Find the translation for the timezone string */
    index = 0;
    while (1) {
        if (tz_names[index][0] == NULL) {
            tz_info = &tz_info_local;
            /* Not found. */
            break;
        }
        if (!strcmp (tz_names[index][0], (tz))) {
            char tz[40];
            strncpy(tz, tz_names[index][1], 39);
            tz_load_rule( tz, &tz_info_remote );
            tz_info = &tz_info_remote;
            /* Force compute next time this data is used */
            tz_info->year_beg = 0;      // Begin date/time is Jan 1, 1970
            tz_info->year_end = 0;      // End date/time is Jan 1, 1970
            //      sprintf (env_string, "TZ=%s", tz_names[index][1]);
            break;
        }
        index++;
    }
}

TCDS_Binary_Harmonic::TCDS_Binary_Harmonic()
{
    m_cst_speeds = NULL;
    m_cst_nodes = NULL;
    m_cst_epochs = NULL;

    num_IDX = 0;
    num_nodes = 0;
    num_csts = 0;
    num_epochs = 0;

    //  Build the units array

}

TCDS_Binary_Harmonic::~TCDS_Binary_Harmonic()
{
}

TC_Error_Code TCDS_Binary_Harmonic::LoadData(const wxString &data_file_path)
{
    if(!open_tide_db (data_file_path.mb_str())) return TC_TCD_FILE_CORRUPT;

    //Build the tables of constituent data

    DB_HEADER_PUBLIC hdr = get_tide_db_header ();

    source_ident = wxString( hdr.version, wxConvUTF8 );

    num_csts = hdr.constituents;
    if(0 == num_csts)
        return TC_GENERIC_ERROR;

    num_nodes = hdr.number_of_years;
    if(0 == num_nodes)
        return TC_GENERIC_ERROR;
    
    //  Allocate a working buffer
    m_work_buffer = (double *) malloc (num_csts * sizeof (double));

    //  Constituent speeds
    m_cst_speeds = (double *) malloc (num_csts * sizeof (double));

    for (int a=0; a<num_csts; a++) {
        m_cst_speeds[a] = get_speed (a);
        m_cst_speeds[a] *= M_PI / 648000; /* Convert to radians per second */
    }

    //  Equilibrium tables by year
    m_first_year = hdr.start_year;
    num_epochs = hdr.number_of_years;

    m_cst_epochs = (double **) malloc (num_csts * sizeof (double *));
    for (int i=0; i<num_csts; i++)
        m_cst_epochs[i] = (double *) malloc (num_epochs * sizeof (double));

    for (int i=0; i<num_csts; i++)
    {
        for (int year=0; year<num_epochs; year++)
        {
            m_cst_epochs[i][year] = get_equilibrium (i, year);
            m_cst_epochs[i][year] *= M_PI / 180.0;
        }
    }

    //  Node factors

    m_cst_nodes = (double **) malloc (num_csts * sizeof (double *));
    for (int a=0; a<num_csts; a++)
        m_cst_nodes[a] = (double *) malloc (num_nodes * sizeof (double));

    for (int a=0; a<num_csts; a++) {
        for (int year=0; year<num_nodes; year++)
            m_cst_nodes[a][year] = get_node_factor (a, year);
    }


    // now load and create the index

    TIDE_RECORD *ptiderec = (TIDE_RECORD *)calloc(sizeof(TIDE_RECORD), 1);
    for(unsigned int i=0 ; i < hdr.number_of_records ; i++) {
        read_tide_record (i, ptiderec);

        num_IDX++; // Keep counting entries for harmonic file stuff
        IDX_entry *pIDX = new IDX_entry;
        pIDX->source_data_type = SOURCE_TYPE_BINARY_HARMONIC;
        pIDX->pDataSource = NULL;

        pIDX->Valid15 = 0;

        pIDX->pref_sta_data = NULL;                     // no reference data yet
        pIDX->IDX_Useable = 1;                          // but assume data is OK
        pIDX->IDX_tzname = NULL;

        pIDX->IDX_lon = ptiderec->header.longitude;
        pIDX->IDX_lat = ptiderec->header.latitude;

        const char *tz = get_tzfile (ptiderec->header.tzfile);
        change_time_zone ((char *)tz);
        if(tz_info)
            pIDX->IDX_time_zone = -tz_info->tzi.Bias;


        strncpy(pIDX->IDX_station_name, ptiderec->header.name, MAXNAMELEN);
//        if(strstr(ptiderec->header.name, "Beaufort") != NULL)
//            int yyp = 4;

        pIDX->IDX_flood_dir = ptiderec->max_direction;
        pIDX->IDX_ebb_dir = ptiderec->min_direction;

        if(REFERENCE_STATION == ptiderec->header.record_type) {
            //    Establish Station Type
            wxString caplin(pIDX->IDX_station_name, wxConvUTF8);
            caplin.MakeUpper();
            if(caplin.Contains(_T("CURRENT")))
                pIDX->IDX_type = 'C';
            else
                pIDX->IDX_type = 'T';

            int t1 = ptiderec->zone_offset;
            double zone_offset = (double)(t1 / 100) + ((double)(t1 % 100))/60.;
//            pIDX->IDX_time_zone = t1a;

            pIDX->IDX_ht_time_off = pIDX->IDX_lt_time_off = 0;
            pIDX->IDX_ht_mpy      = pIDX->IDX_lt_mpy = 1.0;
            pIDX->IDX_ht_off      = pIDX->IDX_lt_off = 0.0;
            pIDX->IDX_ref_dbIndex = ptiderec->header.reference_station;         // will be -1

            //  build a Station_Data class, and add to member array

            Station_Data *psd = new Station_Data;

            psd->amplitude = (double *)malloc(num_csts * sizeof(double));
            psd->epoch     = (double *)malloc(num_csts * sizeof(double));
            psd->station_name = (char *)malloc(ONELINER_LENGTH);

            strncpy(psd->station_name, ptiderec->header.name, MAXNAMELEN);
            psd->station_type = pIDX->IDX_type;


            // Get meridian, which is seconds difference from UTC, not figuring DST, so that New York is always (-300 * 60)
            psd->meridian =  -(tz_info->tzi.Bias * 60);
            psd->zone_offset = zone_offset;

            // Get units
            strncpy (psd->unit, get_level_units (ptiderec->level_units), 40);

            psd->have_BOGUS = (findunit(psd->unit) != -1) && (known_units[findunit(psd->unit)].type == BOGUS);

            int unit_c;
            if (psd->have_BOGUS)
                unit_c = findunit("knots");
            else
                unit_c = findunit(psd->unit);

            if(unit_c != -1) {
                strncpy (psd->units_conv, known_units[unit_c].name, sizeof(psd->units_conv)-1);
                strncpy (psd->units_abbrv, known_units[unit_c].abbrv, sizeof(psd->units_abbrv)-1);
            }
            else {
                strncpy (psd->units_conv, psd->unit, sizeof(psd->units_conv)-1);
                strncpy (psd->units_abbrv, psd->unit, sizeof(psd->units_abbrv)-1);
            }


            // Get constituents
            for (int a=0; a<num_csts; a++)
            {
                psd->amplitude[a] = ptiderec->amplitude[a];
                psd->epoch[a] = ptiderec->epoch[a] * M_PI / 180.;
            }

            psd->DATUM = ptiderec->datum_offset;

            m_msd_array.Add(psd);                     // add it to the member array
            pIDX->pref_sta_data = psd;
            pIDX->IDX_ref_dbIndex = i;
            pIDX->have_offsets = 0;
        }
        else if(SUBORDINATE_STATION == ptiderec->header.record_type) {
            //    Establish Station Type
            wxString caplin(pIDX->IDX_station_name, wxConvUTF8);
            caplin.MakeUpper();
            if(caplin.Contains(_T("CURRENT")))
                pIDX->IDX_type = 'c';
            else
                pIDX->IDX_type = 't';

            int t1 = ptiderec->max_time_add;
            double t1a = (double)(t1 / 100) + ((double)(t1 % 100))/60.;
            t1a *= 60;                  // Minutes
            pIDX->IDX_ht_time_off = t1a;
            pIDX->IDX_ht_mpy = ptiderec->max_level_multiply;
            if(0. == pIDX->IDX_ht_mpy) pIDX->IDX_ht_mpy = 1.0;
            pIDX->IDX_ht_off = ptiderec->max_level_add;


            t1 = ptiderec->min_time_add;
            t1a = (double)(t1 / 100) + ((double)(t1 % 100))/60.;
            t1a *= 60;                  // Minutes
            pIDX->IDX_lt_time_off = t1a;
            pIDX->IDX_lt_mpy = ptiderec->min_level_multiply;
            if(0. == pIDX->IDX_lt_mpy) pIDX->IDX_lt_mpy = 1.0;
            pIDX->IDX_lt_off = ptiderec->min_level_add;

            pIDX->IDX_ref_dbIndex = ptiderec->header.reference_station;
//           strncpy(pIDX->IDX_reference_name, ptiderec->header.name, MAXNAMELEN);

            if( pIDX->IDX_ht_time_off ||
                    pIDX->IDX_ht_off != 0.0 ||
                    pIDX->IDX_lt_off != 0.0 ||
                    pIDX->IDX_ht_mpy != 1.0 ||
                    pIDX->IDX_lt_mpy != 1.0)
                pIDX->have_offsets = 1;
        }

        m_IDX_array.Add(pIDX);
    }



    //  Mark the index entries individually with invariant harmonic constants
    unsigned int max_index = GetMaxIndex();
    for(unsigned int i=0 ; i < max_index ; i++) {
        IDX_entry *pIDX = GetIndexEntry( i );
        if(pIDX) {
            pIDX->num_nodes = num_nodes;
            pIDX->num_csts = num_csts;
            pIDX->num_epochs = num_epochs;
            pIDX->m_cst_speeds = m_cst_speeds;
            pIDX->m_cst_nodes = m_cst_nodes;
            pIDX->m_cst_epochs = m_cst_epochs;
            pIDX->first_year = m_first_year;
            pIDX->m_work_buffer = m_work_buffer;
        }
    }
    free( ptiderec );

    return TC_NO_ERROR;
}


IDX_entry *TCDS_Binary_Harmonic::GetIndexEntry(int n_index)
{
    return &m_IDX_array.Item(n_index);
}


TC_Error_Code TCDS_Binary_Harmonic::LoadHarmonicData(IDX_entry *pIDX)
{
    // Find the indicated Master station
    if(!strlen(pIDX->IDX_reference_name)) {
        strncpy(pIDX->IDX_reference_name, get_station (pIDX->IDX_ref_dbIndex), MAXNAMELEN );

//        TIDE_RECORD *ptiderec = (TIDE_RECORD *)calloc(sizeof(TIDE_RECORD), 1);
//        read_tide_record (pIDX->IDX_ref_dbIndex, ptiderec);
//        free( ptiderec );

        IDX_entry *pIDX_Ref = &m_IDX_array.Item(pIDX->IDX_ref_dbIndex);
        Station_Data *pRefSta = pIDX_Ref->pref_sta_data;
        pIDX->pref_sta_data = pRefSta;
        pIDX->station_tz_offset = -pRefSta->meridian + (pRefSta->zone_offset * 3600);
    }
    return TC_NO_ERROR;
}


