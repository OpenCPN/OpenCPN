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

#include <wx/filename.h>
#include <wx/tokenzr.h>

#include <math.h>

#include "TCDS_Ascii_Harmonic.h"

#ifndef M_PI
#define M_PI    ((2)*(acos(0.0)))
#endif

#define IFF_OPEN  0
#define IFF_CLOSE 1
#define IFF_SEEK  2
#define IFF_TELL  3
#define IFF_READ  4

typedef struct {
    void     *next;
    short int rec_start;
    char     *name;
} harmonic_file_entry;

/* Turn a time displacement of the form [-]HH:MM into the number of seconds. */
static int hhmm2seconds (char *hhmm)
{
    int h, m;
    char s;
    if (sscanf (hhmm, "%d:%d", &h, &m) != 2)
        return(0);
    if (sscanf (hhmm, "%c", &s) != 1)
        return(0);
    if (h < 0 || s == '-')
        m = -m;
    return h*3600 + m*60;
}

TCDS_Ascii_Harmonic::TCDS_Ascii_Harmonic()
{
    //  Initialize member variables
    m_IndexFile = NULL;

    m_cst_speeds = NULL;
    m_cst_nodes = NULL;
    m_cst_epochs = NULL;

    num_IDX = 0;
    num_nodes = 0;
    num_csts = 0;
    num_epochs = 0;
}

TCDS_Ascii_Harmonic::~TCDS_Ascii_Harmonic()
{
    free_data();

    m_msd_array.Clear();
}

TC_Error_Code TCDS_Ascii_Harmonic::LoadData(const wxString &data_file_path)
{
    if(m_IndexFile) IndexFileIO( IFF_CLOSE, 0 );

    m_indexfile_name = data_file_path;

    TC_Error_Code error_return = init_index_file();
    if(error_return != TC_NO_ERROR)
        return error_return;

    wxFileName f(data_file_path);
    m_harmfile_name = f.GetPath( wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME );
    m_harmfile_name += f.GetName();
    error_return = LoadHarmonicConstants(m_harmfile_name);

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

    return error_return;
}


IDX_entry *TCDS_Ascii_Harmonic::GetIndexEntry(int n_index)
{
    return &m_IDX_array.Item(n_index);
}


TC_Error_Code TCDS_Ascii_Harmonic::init_index_file()
{
    long int xref_start=0;
    int doing_xref=0;

    num_IDX=0;

    m_abbreviation_array.Clear();
    m_IDX_array.Clear();
    //   free_harmonic_file_list();
    int have_index = 0;
    int index_in_memory = 0;

    if (IndexFileIO(IFF_OPEN, 0)) {
        while (IndexFileIO(IFF_READ, 0)) {
            if ((index_line_buffer[0] == '#') || (index_line_buffer[0] <= ' '));  // Skip comment lines
            else if (!have_index && !xref_start) {
                if (!strncmp(index_line_buffer, "XREF", 4))
                    xref_start = IndexFileIO(IFF_TELL, 0);
            }
            else if (!have_index && !strncmp(index_line_buffer, "*END*", 5)) {
                if (m_abbreviation_array.GetCount() == 0) {
                    IndexFileIO(IFF_CLOSE, 0);
                    return(TC_INDEX_FILE_CORRUPT); // missing at least some data so no valid index
                }
                // We're done with abbreviation list (and no errors)
                else have_index = 1;
            } // found *END* of cross reference

            else if (!have_index && xref_start) {
                wxString line( index_line_buffer, wxConvUTF8 );

                abbr_entry *entry  = new abbr_entry;

                wxStringTokenizer tkz(line, _T(" "));
                wxString token = tkz.GetNextToken();
                if(token.IsSameAs(_T("REGION"), FALSE))
                    entry->type = REGION;
                else if(token.IsSameAs(_T("COUNTRY"), FALSE))
                    entry->type = COUNTRY;
                else if(token.IsSameAs(_T("STATE"), FALSE))
                    entry->type = STATE;

                token = tkz.GetNextToken();
                entry->short_s = token;

                entry->long_s = line.Mid(tkz.GetPosition()).Strip();

                m_abbreviation_array.Add(entry);

            }

            else if (have_index && (strchr("TtCcIUu", index_line_buffer[0]))) {
                // Load index file data .
                num_IDX++; // Keep counting entries for harmonic file stuff
                IDX_entry *pIDX = new IDX_entry;
                pIDX->source_data_type = SOURCE_TYPE_ASCII_HARMONIC;
                pIDX->pDataSource = NULL;

                index_in_memory   = TRUE;
                pIDX->Valid15 = 0;

                if(TC_NO_ERROR != build_IDX_entry(pIDX ) ) {
                }

                m_IDX_array.Add(pIDX);
            }

#if 0
            else if (have_index && (index_line_buffer[0] == 'H')) {
                // This is a new harmonic file name.
                sscanf(index_line, "Harmonic %s", s1);
                pHarmonic = harmonic_file_list;
                while (pHarmonic && pHarmonic->next)
                    pHarmonic = (harmonic_file_entry *)pHarmonic->next;
                pHarmonic_prev = pHarmonic;
                pHarmonic = (harmonic_file_entry *)malloc(sizeof(harmonic_file_entry));
                if (NULL == pHarmonic) {
                    //               no_mem_msg();
                    free_harmonic_file_list();
                }
                else {
                    if (!harmonic_file_list)
                        harmonic_file_list = pHarmonic;
                    else pHarmonic_prev->next = pHarmonic;
                    pHarmonic->next = NULL;
                    pHarmonic->rec_start = num_IDX;
                    if (allocate_copy_string(&pHarmonic->name,s1)) {
                        //                  no_mem_msg();
                        free_harmonic_file_list();
                    }
                }
            }
#endif
        } // while (more file)
        if (index_in_memory) IndexFileIO(IFF_CLOSE, 0); // All done with file
    } // index file can't be opened
    //   if (hwndBusy) DestroyWindow(hwndBusy);

//   max_IDX = num_IDX;
    return( TC_NO_ERROR );
}

// ----------------------------------
//   Decode an index data line into an IDX_entry
// ----------------------------------

TC_Error_Code TCDS_Ascii_Harmonic::build_IDX_entry(IDX_entry *pIDX )
{
    int TZHr, TZMin ;
    char stz[80];

    pIDX->pref_sta_data = NULL;                     // no reference data yet
    pIDX->IDX_Useable = 1;                          // but assume data is OK

    pIDX->IDX_tzname = NULL;
    stz[0] = 0;

    if (7 != sscanf( index_line_buffer, "%c%s%lf%lf%d:%d%*c%[^\r\n]",
                     &pIDX->IDX_type,&pIDX->IDX_zone[0],&pIDX->IDX_lon,&pIDX->IDX_lat,&TZHr,&TZMin,
                     &pIDX->IDX_station_name[0])) return(TC_INDEX_ENTRY_BAD);

    pIDX->IDX_time_zone = TZHr*60 + TZMin;

    if (strchr("tcUu",index_line_buffer[0])) { // Substation so get second line of info
        IndexFileIO(IFF_READ, 0);

        if(index_line_buffer[0] == '^')                  // Opencpn special
        {
            if (11 != sscanf(index_line_buffer, "%*c%d %f %f %d %f %f %d %d %d %d%*c%[^\r\n]",
                             &pIDX->IDX_ht_time_off, &pIDX->IDX_ht_mpy, &pIDX->IDX_ht_off,
                             &pIDX->IDX_lt_time_off, &pIDX->IDX_lt_mpy, &pIDX->IDX_lt_off,
                             &pIDX->IDX_sta_num, &pIDX->IDX_flood_dir, &pIDX->IDX_ebb_dir,
                             &pIDX->IDX_ref_file_num, pIDX->IDX_reference_name))
                return(TC_INDEX_ENTRY_BAD);

            if(abs(pIDX->IDX_ht_time_off) > 1000)           // useable?
                pIDX->IDX_Useable = 0;

            if(abs(pIDX->IDX_flood_dir) > 360)           // useable?
                pIDX->IDX_Useable = 0;
            if(abs(pIDX->IDX_ebb_dir) > 360)           // useable?
                pIDX->IDX_Useable = 0;

            //    Fix up the secondaries which are identical to masters
            if(pIDX->IDX_ht_mpy == 0.0)
                pIDX->IDX_ht_mpy = 1.0;
            if(pIDX->IDX_lt_mpy == 0.0)
                pIDX->IDX_lt_mpy = 1.0;

        }
        else
        {
            if (9 != sscanf(index_line_buffer, "%*c%d %f %f %d %f %f %d %d%*c%[^\r\n]",
                            &pIDX->IDX_ht_time_off, &pIDX->IDX_ht_mpy, &pIDX->IDX_ht_off,
                            &pIDX->IDX_lt_time_off, &pIDX->IDX_lt_mpy, &pIDX->IDX_lt_off,
                            &pIDX->IDX_sta_num, &pIDX->IDX_ref_file_num, pIDX->IDX_reference_name))
            {
                // Had an error so try alternate with timezone name before ref file number
                if (10 != sscanf(index_line_buffer, "%*c%d %f %f %d %f %f %d %s %d%*c%[^\r\n]",
                                 &pIDX->IDX_ht_time_off, &pIDX->IDX_ht_mpy, &pIDX->IDX_ht_off,
                                 &pIDX->IDX_lt_time_off, &pIDX->IDX_lt_mpy, &pIDX->IDX_lt_off,
                                 &pIDX->IDX_sta_num, stz, &pIDX->IDX_ref_file_num, pIDX->IDX_reference_name))
                    return(TC_INDEX_ENTRY_BAD);

                if (NULL!=(pIDX->IDX_tzname = (char *)malloc(strlen(stz)+1)))
                    strcpy(pIDX->IDX_tzname, stz);
            }


        }           // else


        //  We only consider 1 reference file per index file
        pIDX->IDX_ref_file_num = 0;
        /*
                if (pIDX->IDX_ref_file_num <= 0)
                {   // Find harmonic reference file number
                    pIDX->IDX_ref_file_num= 0;
                    // Find reference station in index, if no index, it had better be in the first one
                    pIDXh = pIDX_first;
                    while (pIDXh!=NULL && strcmp(pIDXh->IDX_reference_name,pIDX->IDX_reference_name))
                        pIDXh = (IDX_entry *)pIDXh->IDX_next;

                    // Copy reference station harmonic file number
                    if (pIDXh!=NULL)
                        pIDX->IDX_ref_file_num = pIDXh->IDX_ref_file_num;
                }
        */
    }

    else
    {   // Reference stations have no offsets
        pIDX->IDX_ht_time_off = pIDX->IDX_lt_time_off = 0;
        pIDX->IDX_ht_mpy      = pIDX->IDX_lt_mpy = 1.0;
        pIDX->IDX_ht_off      = pIDX->IDX_lt_off = 0.0;
        pIDX->IDX_sta_num     = 0;
        strcpy(pIDX->IDX_reference_name, pIDX->IDX_station_name);

    }

    if( pIDX->IDX_ht_time_off ||
            pIDX->IDX_ht_off != 0.0 ||
            pIDX->IDX_lt_off != 0.0 ||
            pIDX->IDX_ht_mpy != 1.0 ||
            pIDX->IDX_lt_mpy != 1.0)
        pIDX->have_offsets = 1;

    pIDX->station_tz_offset = 0;            // ASCII Harmonic data is (always??) corrected to Ref Station TZ

    return(TC_NO_ERROR);
}


//    Load the Harmonic Constant Invariants
TC_Error_Code TCDS_Ascii_Harmonic::LoadHarmonicConstants(const wxString &data_file_path)
{
    FILE *fp;
    char linrec[linelen];
    char junk[80];
    int a, b;

    free_data();

    fp = fopen (data_file_path.mb_str(), "r");
    if (NULL == fp)
        return TC_FILE_NOT_FOUND;

    read_next_line (fp, linrec, 0);
    sscanf (linrec, "%d", &num_csts);

    m_cst_speeds = (double *) malloc (num_csts * sizeof (double));
    m_work_buffer = (double *) malloc (num_csts * sizeof (double));

    /* Load constituent speeds */
    for (a=0; a<num_csts; a++) {
        read_next_line (fp, linrec, 0);
        sscanf (linrec, "%s %lf", junk, &(m_cst_speeds[a]));
        m_cst_speeds[a] *= M_PI / 648000; /* Convert to radians per second */
    }

    /* Get first year for nodes and epochs */
    read_next_line (fp, linrec, 0);
    sscanf (linrec, "%d", &m_first_year);

    /* Load epoch table */
    read_next_line (fp, linrec, 0);
    sscanf (linrec, "%d", &num_epochs);

    m_cst_epochs = (double **) malloc (num_csts * sizeof (double *));
    for (int i=0; i<num_csts; i++)
        m_cst_epochs[i] = (double *) malloc (num_epochs * sizeof (double));

    for (int i=0; i<num_csts; i++)
    {
        if(EOF == fscanf (fp, "%s", linrec))
            return TC_HARM_FILE_CORRUPT;
        for (int b=0; b<num_epochs; b++)
        {
            if(EOF == fscanf (fp, "%lf", &(m_cst_epochs[i][b])))
                return TC_HARM_FILE_CORRUPT;
            m_cst_epochs[i][b] *= M_PI / 180.0;
        }
    }


    /* Sanity check */
    if(EOF == fscanf (fp, "%s", linrec))
        return TC_HARM_FILE_CORRUPT;
    skipnl (fp);

    /* Load node factor table */
    read_next_line (fp, linrec, 0);
    sscanf (linrec, "%d", &num_nodes);

    m_cst_nodes = (double **) malloc (num_csts * sizeof (double *));
    for (int a=0; a<num_csts; a++)
        m_cst_nodes[a] = (double *) malloc (num_nodes * sizeof (double));

    for (int a=0; a<num_csts; a++) {
        int ignore = fscanf (fp, "%s", linrec);
        for (b=0; b<num_nodes; b++)
            ignore = fscanf (fp, "%lf", &(m_cst_nodes[a][b]));
    }

    fclose(fp);

    return TC_NO_ERROR;
}


TC_Error_Code TCDS_Ascii_Harmonic::LoadHarmonicData(IDX_entry *pIDX)
{
    Station_Data *psd = NULL;

    //    Look in the index first
    if(pIDX->pref_sta_data)
        return TC_NO_ERROR;         //easy


    // Try the member array of "already-looked-at" master stations
    for(unsigned int i=0 ; i < m_msd_array.GetCount() ; i++)
    {
        psd = &m_msd_array.Item(i);
        //    In the following comparison, it is allowed that the sub-station reference_name may be
        //          a pre-subset of the master station name.
        //          e.g  IDX_refence_name:  The Narrows midchannel New York
        //                            as found in HARMONIC.IDX
        //                 psd_station_name:      The Narrows, Midchannel, New York Harbor, New York Current
        //                            as found in HARMONIC
        if( (!slackcmp(psd->station_name, pIDX->IDX_reference_name)) && (toupper(pIDX->IDX_type) == psd->station_type) )
        {
            pIDX->pref_sta_data = psd;                // save for later
            return TC_NO_ERROR;
        }
    }


    //    OK, have to read and create from the raw file
    psd = NULL;

    //    If reference station was recently sought, and not found, don't bother
    //            if(!strcmp(pIDX->IDX_reference_name, plast_reference_not_found->mb_str()))
    if(m_last_reference_not_found.IsSameAs(wxString(pIDX->IDX_reference_name, wxConvUTF8)))
        return TC_MASTER_HARMONICS_NOT_FOUND;

    //    Clear for this looking
    m_last_reference_not_found.Clear();

    //    Find and load appropriate constituents
    FILE *fp;
    char linrec[linelen];
    fp = fopen (m_harmfile_name.mb_str(), "r");

    while (read_next_line (fp, linrec, 1))
    {
        nojunk (linrec);
        int curonly = 0;
        if (curonly)
            if (!strstr (linrec, "Current"))
                continue;
        //    See the note above about station names
        //                  if(!strncmp(linrec, "Rivi", 4))
        //                        int ggl = 4;

        if (slackcmp (linrec, pIDX->IDX_reference_name))
            continue;

        //    Got the right location, so load the data

        psd = new Station_Data;

        psd->amplitude = (double *)malloc(num_csts * sizeof(double));
        psd->epoch     = (double *)malloc(num_csts * sizeof(double));
        psd->station_name = (char *)malloc(strlen(linrec) +1);

        char junk[80];
        int a;
        strcpy (psd->station_name, linrec);

        //    Establish Station Type
        wxString caplin(linrec, wxConvUTF8);
        caplin.MakeUpper();
        if(caplin.Contains(_T("CURRENT")))
            psd->station_type = 'C';
        else
            psd->station_type = 'T';



        /* Get meridian */
        read_next_line (fp, linrec, 0);
        psd->meridian = hhmm2seconds (linrec);
        psd->zone_offset = 0;
        
        /* Get tzfile, if present */
        if (sscanf (nojunk(linrec), "%s %s", junk, psd->tzfile) < 2)
            strcpy (psd->tzfile, "UTC0");

        /* Get DATUM and units */
        read_next_line (fp, linrec, 0);
        if (sscanf (nojunk(linrec), "%lf %s", &(psd->DATUM), psd->unit) < 2)
            strcpy (psd->unit, "unknown");

        if ((a = findunit (psd->unit)) == -1)
        {
            // Nonsense....
            //                        strcpy (psd->units_abbrv, psd->unit);
            //                        strcpy (psd->units_conv, known_units[a].name);
        }

        psd->have_BOGUS = (findunit(psd->unit) != -1) && (known_units[findunit(psd->unit)].type == BOGUS);

        int unit_c;
        if (psd->have_BOGUS)
            unit_c = findunit("knots");
        else
            unit_c = findunit(psd->unit);

        if (unit_c != -1)
        {
            strcpy (psd->units_conv,       known_units[unit_c].name);
            strcpy (psd->units_abbrv,      known_units[unit_c].abbrv);
        }

        /* Get constituents */
        double loca, loce;
        for (a=0; a<num_csts; a++)
        {
            read_next_line (fp, linrec, 0);
            sscanf (linrec, "%s %lf %lf", junk, &loca, &loce);
            //          loc_epoch[a] *= M_PI / 180.0;
            psd->amplitude[a] = loca;
            psd->epoch[a] = loce * M_PI / 180.;
        }
        fclose (fp);

        break;
    }

    if(!psd) {
        m_last_reference_not_found = wxString(pIDX->IDX_reference_name, wxConvUTF8);
        return TC_MASTER_HARMONICS_NOT_FOUND;
    }
    else {
        m_msd_array.Add(psd);                     // add it to the member array
        pIDX->pref_sta_data = psd;
        return TC_NO_ERROR;
    }

}


/*---------------------------------
 * Low level Index file I/O
 *-------------------------------*/


long TCDS_Ascii_Harmonic::IndexFileIO(int func, long value) {
    char *str;

    switch ( func ) {
        // Close either/both if open
    case IFF_CLOSE :
        if (m_IndexFile) fclose(m_IndexFile);
        m_IndexFile = NULL;
        return(0);

        // Open
    case IFF_OPEN :
        m_IndexFile = fopen( m_indexfile_name.mb_str(), "rt");
        if (m_IndexFile == NULL) return(0);
        return(1);

        // Return file pointer only happens with master file
    case IFF_TELL :
        return(ftell(m_IndexFile));

        // Seek
    case IFF_SEEK :
        return(fseek(m_IndexFile,value,SEEK_SET));

        // Read until EOF .
    case IFF_READ :
        str = fgets( index_line_buffer, 1024, m_IndexFile);

        if (str != NULL)
            return(1);
        else return(0);

    }
    return(0);
}

/* Read a line from the harmonics file, skipping comment lines */
int TCDS_Ascii_Harmonic::read_next_line (FILE *fp, char linrec[linelen], int end_ok)
{
    do {
        if (!fgets (linrec, linelen, fp)) {
            if (end_ok)
                return 0;
            else {
                exit (-1);
            }
        }
    } while (linrec[0] == '#' || linrec[0] == '\r' || linrec[0] == '\n');
    return 1;
}

/* Remove lingering carriage return, but do nothing else */
int TCDS_Ascii_Harmonic::skipnl (FILE *fp)
{
    char linrec[linelen];
    if(NULL == fgets (linrec, linelen, fp))
        return 0;
    return 1;
}

/* Get rid of trailing garbage in buffer */
char * TCDS_Ascii_Harmonic::nojunk (char *line)
{
    char *a;
    a = &(line[strlen(line)]);
    while (a > line)
        if (*(a-1) == '\n' || *(a-1) == '\r' || *(a-1) == ' ')
            *(--a) = '\0';
        else
            break;
    return line;
}

/* Slackful strcmp; 0 = match.  It's case-insensitive and accepts a
 *   prefix instead of the entire string.  The second argument is the
 *   one that can be shorter. Second argument can contain '?' as wild
 *   card character.
 */
int TCDS_Ascii_Harmonic::slackcmp (char *a, char *b)
{
    int c, cmp, n;
    n = strlen (b);
    if ((int)(strlen (a)) < n)
        return 1;
    for (c=0; c<n; c++)
    {
        if(b[c] == '?')
            continue;

        cmp = ((a[c] >= 'A' && a[c] <= 'Z') ? a[c] - 'A' + 'a' : a[c])
              -
              ((b[c] >= 'A' && b[c] <= 'Z') ? b[c] - 'A' + 'a' : b[c]);
        if (cmp)
            return cmp;
    }
    return 0;
}

void TCDS_Ascii_Harmonic::free_cst()
{
    free(m_cst_speeds);
    m_cst_speeds = NULL;
}
void TCDS_Ascii_Harmonic::free_nodes()
{
    int a;
    if (num_csts && m_cst_nodes)
        for(a=0; a<num_csts; a++)
            free(m_cst_nodes[a]);
    free(m_cst_nodes);

    m_cst_nodes = NULL;
}

void TCDS_Ascii_Harmonic::free_epochs()
{
    int a;
    if (num_csts && m_cst_epochs)
        for(a=0; a<num_csts; a++)
            free(m_cst_epochs[a]);
    free(m_cst_epochs);

    m_cst_epochs = NULL;
}

/* free harmonics data */
void TCDS_Ascii_Harmonic::free_data ()
{
    free_nodes();
    free_epochs();
    free_cst();
}

