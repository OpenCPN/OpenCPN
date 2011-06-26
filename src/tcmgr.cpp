/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Tide and Current Manager
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   bdbcat@yahoo.com   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 *
 *
 */



#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include "dychart.h"


#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <wx/listimpl.cpp>
#include <wx/datetime.h>
#include <wx/textfile.h>
#include <wx/tokenzr.h>

#include "tcmgr.h"
#include "georef.h"



#ifndef PI
#define PI        3.1415926535897931160E0      /* pi */
#endif

//--------------------------------------------------------------------------------
//    Some Time Converters
//--------------------------------------------------------------------------------


/* Turn a time displacement of the form [-]HH:MM into the number of seconds. */
static int
hhmm2seconds (char *hhmm)
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

int TCMgr::yearoftimet (time_t t)
{
  return ((gmtime (&t))->tm_year) + 1900;
}

//--------------------------------------------------------------------------------
//    TCMgr Tide/Current Manager
//--------------------------------------------------------------------------------

TCMgr::TCMgr(const wxString &data_dir, const wxString &home_dir)
{
      bTCMReady = false;                        // Tide/Current Manager not ready yet

//  Build the units array
    known_units[0].name = (char *) malloc(strlen("feet") +1);
    strcpy(known_units[0].name, "feet");
    known_units[0].abbrv = (char *) malloc(strlen("ft") +1);
    strcpy(known_units[0].abbrv, "ft");
    known_units[0].type = LENGTH;
    known_units[0].conv_factor = 0.3048;

    known_units[1].name = (char *) malloc(strlen("meters") +1);
    strcpy(known_units[1].name, "meters");
    known_units[1].abbrv = (char *) malloc(strlen("m") +1);
    strcpy(known_units[1].abbrv, "m");
    known_units[1].type = LENGTH;
    known_units[1].conv_factor = 1.0;

    known_units[2].name = (char *) malloc(strlen("knots") +1);
    strcpy(known_units[2].name, "knots");
    known_units[2].abbrv = (char *) malloc(strlen("ky") +1);
    strcpy(known_units[2].abbrv, "kt");
    known_units[2].type = VELOCITY;
    known_units[2].conv_factor = 1.0;

    known_units[3].name = (char *) malloc(strlen("knots^2") +1);
    strcpy(known_units[3].name, "knots^2");
    known_units[3].abbrv = (char *) malloc(strlen("kt^2") +1);
    strcpy(known_units[3].abbrv, "kt^2");
    known_units[3].type = BOGUS;
    known_units[3].conv_factor = 1.0;


    abbreviation_list = NULL;
      harmonic_file_list = NULL;
      pIDX_first = NULL;
      max_IDX = 0;
      num_csts = 0;
      cst_nodes = NULL;
      cst_epochs = NULL;
      work = NULL;
      cst_speeds = NULL;

      index_in_memory=0;

      hfile_name = NULL;
      indexfile_name = NULL;
      userfile_name = NULL;
      IDX_reference_name = NULL;
      Izone = NULL;

      paIDX = NULL;

      pmru_next = NULL;
      pmru_head = NULL;
      pmru_last = NULL;

      looking_end = FALSE;
      rewound = 0;

      plast_reference_not_found = new wxString;
      plast_reference_not_found->Clear();


//    Set up default file names
      wxString hidx_file = data_dir;
      hidx_file.Append(_T("HARMONIC.IDX"));
      allocate_copy_string(&indexfile_name, hidx_file.mb_str());

      wxString harm_file = data_dir;
      harm_file.Append(_T("HARMONIC"));
      allocate_copy_string(&hfile_name, harm_file.mb_str());

      pmru_file_name = new wxString(home_dir);                    // in the current users home
#ifdef __WXMSW__
      pmru_file_name->Append(_T("station_mru.dat"));
#else
      pmru_file_name->Append(_T(".opencpn/station_mru.dat"));
#endif


//    Initialize and load the Index file structure
      init_index_file(1,0);


//    Build an array of pIDX for fast index access

      if(have_index)
      {
            paIDX = (IDX_entry **)malloc((max_IDX + 1) * sizeof(IDX_entry *));
            for(int i=0 ; i < max_IDX +1 ; i++)
            {
                  IDX_entry *pe = get_index_data( -i );           // Fetch next record pointer
                  paIDX[i] = pe;
            }
      }
      else
            return;                                         // No Index file found

//    Load the Harmonic Constant Invariants
      FILE *fp;
      char linrec[linelen];
      char junk[80];
      int a, b;
      fp = fopen (hfile_name, "r");
      if (NULL == fp)
            return;

      free_data();
      next_line (fp, linrec, 0);
      sscanf (linrec, "%d", &num_csts);
      allocate_cst ();

      /* Load constituent speeds */
      for (a=0;a<num_csts;a++)
      {
            next_line (fp, linrec, 0);
            sscanf (linrec, "%s %lf", junk, &(cst_speeds[a]));
            cst_speeds[a] *= M_PI / 648000; /* Convert to radians per second */
          /*   M_PI / 180 to get radians, / 3600 to get seconds */
      }

      /* Get first year for nodes and epochs */
      next_line (fp, linrec, 0);
      sscanf (linrec, "%d", &first_year);

      /* Load epoch table */
      next_line (fp, linrec, 0);
      sscanf (linrec, "%d", &num_epochs);
      allocate_epochs ();

      for (a=0;a<num_csts;a++)
      {
            if(EOF == fscanf (fp, "%s", linrec))
                  return;
            for (b=0;b<num_epochs;b++)
            {
                  if(EOF == fscanf (fp, "%lf", &(cst_epochs[a][b])))
                        return;
                  cst_epochs[a][b] *= M_PI / 180.0;
            }
      }

      /* Sanity check */
      if(EOF == fscanf (fp, "%s", linrec))
            return;
      skipnl (fp);

      /* Load node factor table */
      next_line (fp, linrec, 0);
      sscanf (linrec, "%d", &num_nodes);
      allocate_nodes ();

      for (a=0;a<num_csts;a++)
      {
            int ignore = fscanf (fp, "%s", linrec);
            for (b=0;b<num_nodes;b++)
                  ignore = fscanf (fp, "%lf", &(cst_nodes[a][b]));
      }

      fclose(fp);

//    Load the Master Station Data Cache file
      LoadMRU();

      bTCMReady = true;

}

TCMgr::~TCMgr()
{
   SaveMRU();

   FreeMRU();

   if(userfile_name)
      free(userfile_name);
   if(indexfile_name)
      free(indexfile_name);
   if(hfile_name)
      free(hfile_name);

   free_harmonic_file_list();
   free_abbreviation_list();
   free_station_index();

   if(paIDX)
      free(paIDX);

   free_data();

   delete plast_reference_not_found;
   delete pmru_file_name;

 // Free the known units array
    for (int iu=0 ; iu < NUMUNITS ; iu++)
    {
      free(known_units[iu].name);
      free(known_units[iu].abbrv);
    }

}


void TCMgr::LoadMRU(void)
{
      wxString str;

      mru_entry *pmru_entry;
      Station_Data *psd;
      pmru_head = NULL;

      wxTextFile mru_file(*pmru_file_name);

      if(!mru_file.Exists())
            return;

      if(mru_file.Open())
      {
            if(mru_file.GetLineCount())
                  str = mru_file.GetFirstLine();                  //Signature

            if(str != _T("Signature250"))
               return;

            while(!mru_file.Eof())
            {
                  str = mru_file.GetNextLine();
                  while(((str[0] == '#') || (str.IsEmpty())) && !mru_file.Eof())
                          str = mru_file.GetNextLine();

                  if(mru_file.Eof())
                      break;

                  pmru_entry = (mru_entry *)malloc(sizeof(mru_entry));

                  psd = new Station_Data;
                  pmru_entry->sta_data = psd;

//          Capture and store the data

//          Allocate and store station name
                  psd->station_name = (char *)malloc(str.Len() +1);
                  strcpy(psd->station_name, str.mb_str());

//          Station Type
                  str = mru_file.GetNextLine();
                  psd->station_type = *(str.mb_str());       // one character T or C

//          Meridian
                  str = mru_file.GetNextLine();
                  sscanf(str.mb_str(), "%d", &(psd->meridian));

//          Datum, units
                  char temp[40];
                  float tf, tfa, tfe;
                  str = mru_file.GetNextLine();

                  sscanf(str.mb_str(), "%f %s", &tf, &temp[0]);
                  psd->DATUM = tf;
                  strcpy( psd->unit, temp );
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

//          Amplitude/epoch
                  psd->amplitude = (double *)malloc(num_csts * sizeof(double));
                  psd->epoch = (double *)malloc(num_csts * sizeof(double));

                  for(int i=0 ; i<num_csts ; i++)
                  {
                        str = mru_file.GetNextLine();
                        sscanf(str.mb_str(), "%f %f", &tfa, &tfe);
                        psd->amplitude[i] = tfa;
                        psd->epoch[i] = tfe;
                  }
//          Link it up
                  pmru_entry->next = NULL;
                  if(pmru_head == NULL)
                  {
                        pmru_head = pmru_entry;
                        pmru_last = pmru_entry;
                        pmru_next = pmru_entry;             // for AddMRU
                  }
                  else
                  {
                        pmru_last->next = pmru_entry;
                        pmru_last = pmru_entry;
                        pmru_next = pmru_entry;             // for AddMRU
                  }
            }
      }
 }


void TCMgr::FreeMRU()
{
      Station_Data *psd;

      if(pmru_head)
      {
            mru_entry *pmru = pmru_head;

            while(pmru)
            {

                  psd = pmru->sta_data;
//    Station Name
                  free(psd->station_name);

//    Data
                  free(psd->amplitude);
                  free(psd->epoch);

                  delete psd;

                  mru_entry *pt = (mru_entry *)pmru->next;
                  free(pmru);

                  pmru = pt;
            }

      }
}


void TCMgr::AddMRU(Station_Data *psd)
{
            mru_entry *pmru_entry = (mru_entry *)malloc(sizeof(mru_entry));
            pmru_entry->sta_data = psd;


//    Link it in
            if(pmru_next == NULL)
            {
                  pmru_head = pmru_entry;
            }
            else
            {
                  pmru_next->next = pmru_entry;
            }

            pmru_entry->next = NULL;

            pmru_next = pmru_entry;
}



void TCMgr::SaveMRU(void)
{
      wxString str_sbuf;
      Station_Data *psd;

      if(pmru_head)
      {
            remove(pmru_file_name->mb_str());                // simply delete the existing file

            wxTextFile mru_file(*pmru_file_name);
            mru_file.Create();

            mru_file.AddLine(wxString(_T("Signature250")));

            mru_entry *pmru = pmru_head;

            while(pmru)
            {

                  psd = pmru->sta_data;
//    Station Name
                  mru_file.AddLine(wxString(psd->station_name,  wxConvUTF8));

//    Station Type
                  mru_file.AddLine(wxString(psd->station_type));

//    Meridian/tz
                  str_sbuf.Printf(_T("%d"), psd->meridian);
                  mru_file.AddLine(str_sbuf);
//    Datum, Units
                  wxString unit(psd->unit, wxConvUTF8);
                  str_sbuf.Printf(_T("%8.4f  "), psd->DATUM);
                  str_sbuf += unit;
                  mru_file.AddLine(str_sbuf);
//    Data
                  for(int i=0 ; i<num_csts ; i++)
                  {
                      str_sbuf.Printf(_T("%8.4f  %8.4f"), psd->amplitude[i], psd->epoch[i]);
                      mru_file.AddLine(str_sbuf);
                  }


                  mru_entry *pt = (mru_entry *)pmru->next;
                  pmru = pt;
            }

            mru_file.Write();
      }
}

int TCMgr::GetNextBigEvent (time_t *tm, int idx)
{
   float tcvalue[1]; float dir; bool ret;
  double p, q;
  int flags = 0, slope = 0;
      ret = GetTideOrCurrent(*tm, idx, tcvalue[0],  dir);
  p = tcvalue[0];
  *tm += 60;
      ret = GetTideOrCurrent(*tm, idx, tcvalue[0],  dir);
  q = tcvalue[0];
  *tm += 60;
  if (p < q)
    slope = 1;
  while (1) {
    if ((slope == 1 && q < p) || (slope == 0 && p < q)) {
      /* Tide event */
      flags |= (1 << slope);
    }
    if (flags) {
      *tm -= 60;
      if (flags < 4)
        *tm -= 60;
      return flags;
    }
    p = q;
      ret = GetTideOrCurrent(*tm, idx, tcvalue[0],  dir);
    q = tcvalue[0];
    *tm += 60;
  }
}


bool TCMgr::GetTideOrCurrent15(time_t t, int idx, float &tcvalue, float& dir, bool &bnew_val)
{
      int ret;
      IDX_entry *pIDX = paIDX[idx];             // point to the index entry


//    Figure out this computer timezone minute offset
      wxDateTime this_now = wxDateTime::Now();
      wxDateTime this_gmt = this_now.ToGMT();
      wxTimeSpan diff = this_gmt.Subtract(this_now);
      int diff_mins = diff.GetMinutes();

      int station_offset = pIDX->IDX_time_zone;
      if(this_now.IsDST())
            station_offset += 60;
      int corr_mins = station_offset - diff_mins;

      wxDateTime today_00 = wxDateTime::Today();
      int t_today_00 = today_00.GetTicks();
      int t_today_00_at_station = t_today_00 - (corr_mins * 60);

      int t_at_station = this_gmt.GetTicks() - (station_offset * 60) + (corr_mins * 60);

      int t_mins = (t_at_station - t_today_00_at_station) / 60;
      int t_15s = t_mins / 15;

      if(pIDX->Valid15)                               // valid data available
      {

            int tref1 = t_today_00_at_station + t_15s * 15 * 60;
            if(tref1 == pIDX->Valid15)
            {
                  tcvalue = pIDX->Value15;
                  dir = pIDX->Dir15;
                  bnew_val = false;
                  return pIDX->Ret15;
            }
            else
            {
                  int tref = t_today_00_at_station + t_15s * 15 * 60;
                  ret = GetTideOrCurrent(tref, idx, tcvalue, dir);

                  pIDX->Valid15 = tref;
                  pIDX->Value15 = tcvalue;
                  pIDX->Dir15 = dir;
                  pIDX->Ret15 = !(ret == 0);
                  bnew_val = true;

                  return !(ret == 0);
            }
      }

      else
      {


            int tref = t_today_00_at_station + t_15s * 15 * 60;
            ret = GetTideOrCurrent(tref, idx, tcvalue, dir);

            pIDX->Valid15 = tref;
            pIDX->Value15 = tcvalue;
            pIDX->Dir15 = dir;
            pIDX->Ret15 = !(ret == 0);
            bnew_val = true;

      }

      return !(ret == 0);

}

bool TCMgr::GetTideFlowSens(time_t t, int sch_step, int idx, float &tcvalue_now, float &tcvalue_prev, bool &w_t)
{

//    Return a sensible value of 0 by default
      tcvalue_now = 0;
	  tcvalue_prev = 0;
	  w_t = false;


//    Load up this location data

      IDX_entry *pIDX = paIDX[idx];             // point to the index entry
      if(   !pIDX->IDX_Useable )
            return false;                                        // no error, but unuseable

	  pmsd = find_or_load_harm_data(pIDX);
      if(!pmsd)                           // Master station not found
            return false;                      // Error

	  have_offsets = 0;
//    fudge_constituents(pmsd, pIDX);
	  if(       pIDX->IDX_ht_time_off ||
                pIDX->IDX_ht_off != 0.0 ||
                pIDX->IDX_lt_off != 0.0 ||
                pIDX->IDX_ht_mpy != 1.0 ||
                pIDX->IDX_lt_mpy != 1.0)
	  have_offsets = 1;

      amplitude = 0.0;                // Force multiplier re-compute
      time_t tt = time(NULL);
      int yott = ((gmtime (&tt))->tm_year) + 1900;

      happy_new_year (yott);//Force new multipliers

//    Finally, process the tide flow sens

	  tcvalue_now = time2asecondary (t , pIDX);
	  tcvalue_prev = time2asecondary (t + sch_step , pIDX);

	  w_t = tcvalue_now > tcvalue_prev;		// w_t = true --> flood , w_t = false --> ebb

	  return true;

}

void TCMgr::GetHightOrLowTide(time_t t, int sch_step_1, int sch_step_2, float tide_val ,bool w_t , int idx, float &tcvalue, time_t &tctime)
{

//    Return a sensible value of 0,0 by default
      tcvalue = 0;
	  tctime = t;


//    Load up this location data


      IDX_entry *pIDX = paIDX[idx];             // point to the index entry

      if(   !pIDX->IDX_Useable )
            return;                                        // no error, but unuseable

	  pmsd = find_or_load_harm_data(pIDX);

      if(!pmsd)                           // Master station not found
            return;                      // Error

	  have_offsets = 0;
//    fudge_constituents(pmsd, pIDX);
	  if(       pIDX->IDX_ht_time_off ||
                pIDX->IDX_ht_off != 0.0 ||
                pIDX->IDX_lt_off != 0.0 ||
                pIDX->IDX_ht_mpy != 1.0 ||
                pIDX->IDX_lt_mpy != 1.0)
	  have_offsets = 1;



      amplitude = 0.0;                // Force multiplier re-compute
      time_t tt = time(NULL);
      int yott = ((gmtime (&tt))->tm_year) + 1900;

      happy_new_year (yott);//Force new multipliers

// Finally, calculate the Hight and low tides
	  double newval = tide_val;
	  double oldval = ( w_t ) ? newval - 1: newval + 1 ;
	  int j = 0 ;
	  int k = 0 ;
	  int ttt = 0 ;
        while ( (newval > oldval) == w_t )			//searching each ten minute
	  {
		j++;
		oldval = newval;
		ttt = t + ( sch_step_1 * j );
		newval = time2asecondary (ttt, pIDX);
	  }
	  oldval = ( w_t ) ? newval - 1: newval + 1 ;
	  while ( (newval > oldval) == w_t )			// searching back each minute
	  {
		oldval = newval ;
		k++;
		ttt = t +  ( sch_step_1 * j ) - ( sch_step_2 * k ) ;
		newval = time2asecondary (ttt, pIDX);
	  }
        tcvalue = newval;
	  tctime = ttt + sch_step_2 ;
}

bool TCMgr::GetTideOrCurrent(time_t t, int idx, float &tcvalue, float& dir)
{

//    Return a sensible value of 0,0 by default
      dir = 0;
      tcvalue = 0;


//    Load up this location data


      IDX_entry *pIDX = paIDX[idx];             // point to the index entry

      if(   !pIDX->IDX_Useable )
      {
            dir = 0;
            tcvalue = 0;
            return(false);                                        // no error, but unuseable
      }

/*
   load_location_info( station_name, rec_num ))
         IDX_rec_num     = pIDX->IDX_rec_num;
         IDX_type        = pIDX->IDX_type;
         strcpy(IDX_zone, pIDX->IDX_zone);
         IDX_lon         = pIDX->IDX_lon;
         IDX_lat         = pIDX->IDX_lat;
         IDX_time_zone   = pIDX->IDX_time_zone;
         strcpy( IDX_station_name, pIDX->IDX_station_name);
         IDX_ht_time_off = pIDX->IDX_ht_time_off;
         IDX_ht_mpy      = pIDX->IDX_ht_mpy;
         IDX_ht_off      = pIDX->IDX_ht_off;
         IDX_lt_time_off = pIDX->IDX_lt_time_off;
         IDX_lt_mpy      = pIDX->IDX_lt_mpy;
         IDX_lt_off      = pIDX->IDX_lt_off;
         IDX_sta_num     = pIDX->IDX_sta_num;
         IDX_ref_file_num= pIDX->IDX_ref_file_num;
         strcpy(IDX_reference_name, pIDX->IDX_reference_name);
         if (pIDX->IDX_tzname != NULL)
              strcpy(IDX_tzname,pIDX->IDX_tzname);
         else strcpy(IDX_tzname,"");

*/

//          Subordinate Station
//         meridian = pIDX->IDX_time_zone;
//         have_user_offsets = 0;

/*
         if (labs(pIDX->IDX_ht_time_off) != 1111)
              httimeoff = (long int)pIDX->IDX_ht_time_off * 60;
         else if (labs(pIDX->IDX_lt_time_off) != 1111)
              httimeoff = (long int)pIDX->IDX_lt_time_off * 60;
         else httimeoff = 0;

         if (labs(pIDX->IDX_lt_time_off) != 1111)
              lttimeoff = (long int)pIDX->IDX_lt_time_off * 60;
         else lttimeoff = httimeoff;
*/
/*
         if ((pIDX->IDX_ht_mpy > 0.1) && (pIDX->IDX_ht_mpy < 10.0))
              hlevelmult = pIDX->IDX_ht_mpy;
         else if ((pIDX->IDX_lt_mpy > 0.1) && (pIDX->IDX_lt_mpy < 10.0))
              hlevelmult = pIDX->IDX_lt_mpy;
         else hlevelmult = 1.0;

         if ((pIDX->IDX_lt_mpy > 0.1) && (pIDX->IDX_lt_mpy < 10.0))
              llevelmult = pIDX->IDX_lt_mpy;
         else llevelmult = hlevelmult;
*/
/*
         if (fabs(pIDX->IDX_ht_off) < 100.0)
              htleveloff = pIDX->IDX_ht_off;
         else if (fabs(pIDX->IDX_lt_off) < 100.0)
              htleveloff = pIDX->IDX_lt_off;
         else htleveloff = 0;

         if (fabs(pIDX->IDX_lt_off) < 100.0)
              ltleveloff = pIDX->IDX_lt_off;
         else ltleveloff = htleveloff;
*/

      pmsd = find_or_load_harm_data(pIDX);

      if(!pmsd)                           // Master station not found
            return(false);                      // Error






      have_offsets = 0;
//    fudge_constituents(pmsd, pIDX);
  if(             pIDX->IDX_ht_time_off ||
                  pIDX->IDX_ht_off != 0.0 ||
                  pIDX->IDX_lt_off != 0.0 ||
                  pIDX->IDX_ht_mpy != 1.0 ||
                  pIDX->IDX_lt_mpy != 1.0)
            have_offsets = 1;



      amplitude = 0.0;                // Force multiplier re-compute
      time_t tt = time(NULL);
      int yott = ((gmtime (&tt))->tm_year) + 1900;

      happy_new_year (yott);//Force new multipliers

/*
      if (Usetadjust == 2) {
      change_time_zone (tadjust_tzname); // User forced tz
      utc = loctz = tadjust = 0;
   }
   else if (loctz) {                    // Use timezone of remote location
      if (is_ref) {
         change_time_zone (tzfile);     // Use reference station timezone
         utc = tadjust = 0;
      }
      else if (strlen(IDX_tzname) > 0) {// Does this entry have a timezone assigned?
         change_time_zone (IDX_tzname); // Use user station timezone
         utc = tadjust = 0;
      }
      else if (meridian % 60) {
         utc=1;
         tadjust=meridian*60;
      }
      else {
         sprintf(s,"Etc/GMT%+d",-meridian/60);
         change_time_zone (s);
         utc = tadjust = 0;
//      }
//      else {
//         utc=1;
//         tadjust=meridian;
      }
   }
   else if (!utc && !tadjust) {
      change_time_zone ("Local");
   }
   if (!have_user_offsets || !custom_name) {
      if (custom_name) free(custom_name);
      if (have_user_offsets)
           custom_name = stradoop(location);
      else custom_name = stradoop(IDX_station_name);
   }


//   add_mru( custom_name );
*/


//    Finally, calculate the tide/current

      double level = time2asecondary (t, pIDX);
      if(level >= 0)
            dir = pIDX->IDX_flood_dir;
      else
            dir = pIDX->IDX_ebb_dir;

      tcvalue = level;

      return(true); // Got it!
}

int TCMgr::GetStationTimeOffset(IDX_entry *pIDX)
{
      if(0/*pIDX->b_is_secondary*/)
      {
            IDX_entry *pIDXM = paIDX[pIDX->IDX_ref_dbIndex];
            return pIDXM->IDX_time_zone;
      }
      else
            return pIDX->IDX_time_zone;
}



Station_Data *TCMgr::find_or_load_harm_data(IDX_entry *pIDX)
{
      Station_Data *psd = NULL;

//    Look in the index first
      if(pIDX->pref_sta_data)
            return pIDX->pref_sta_data;         //easy

//    Try the MRU list
      mru_entry *pmru = pmru_head;

      while(pmru)
      {
            psd = pmru->sta_data;
//    In the following comparison, it is allowed that the sub-station reference_name may be
//          a pre-subset of the master station name.
//          e.g  IDX_refence_name:  The Narrows midchannel New York
//                            as found in HARMONIC.IDX
//                 psd_station_name:      The Narrows, Midchannel, New York Harbor, New York Current
//                            as found in HARMONIC
            if( (!slackcmp(psd->station_name, pIDX->IDX_reference_name)) && (toupper(pIDX->IDX_type) == psd->station_type) )
            {
                  pIDX->pref_sta_data = psd;                // save for later
                  return psd;
            }
            else
            {
                  mru_entry *pt = (mru_entry *)pmru->next;
                  pmru = pt;
            }
      }

//    OK, have to read and create from the raw file

      psd = NULL;
//      if(1/*!pIDX->IDX_tried_once*/)
      {
            pIDX->IDX_tried_once = 1;

//    If reference station was recently sought, and not found, don't bother
//            if(!strcmp(pIDX->IDX_reference_name, plast_reference_not_found->mb_str()))
            if(plast_reference_not_found->IsSameAs(wxString(pIDX->IDX_reference_name, wxConvUTF8)))

            {
                  return NULL;
            }
//    Clear for this looking
            plast_reference_not_found->Clear();

      //    Find and load appropriate constituents
            FILE *fp;
            char linrec[linelen];
            fp = fopen (hfile_name, "r");

            while (next_line (fp, linrec, 1))
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
              next_line (fp, linrec, 0);
              psd->meridian = hhmm2seconds (linrec);

              /* Get tzfile, if present */
              if (sscanf (nojunk(linrec), "%s %s", junk, psd->tzfile) < 2)
                  strcpy (psd->tzfile, "UTC0");

              /* Get DATUM and units */
              next_line (fp, linrec, 0);
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
              for (a=0;a<num_csts;a++)
              {
                  next_line (fp, linrec, 0);
                  sscanf (linrec, "%s %lf %lf", junk, &loca, &loce);
      //          loc_epoch[a] *= M_PI / 180.0;
                  psd->amplitude[a] = loca;
                  psd->epoch[a] = loce * M_PI / 180.;
              }
              fclose (fp);

              break;
            }

            if(!psd)
                plast_reference_not_found->Append(wxString(pIDX->IDX_reference_name, wxConvUTF8));

            if(psd)
                  AddMRU(psd);                                    // add it to the list

            pIDX->pref_sta_data = psd;                // save for later
            return psd;
      }
//      else                                                  // already tried
//            return NULL;

}



//----------------------------------------------------------------------------------
//          Adjust loaded constituents for any offsets
//----------------------------------------------------------------------------------
void TCMgr::fudge_constituents (Station_Data *psd, IDX_entry *pIDX)
{
  // Modify location name to show offsets
/*
  if (httimeoff)
    sprintf (location+strlen(location), " htoff=%dmin", httimeoff/60);
  if (lttimeoff)
    sprintf (location+strlen(location), " ltoff=%dmin", lttimeoff/60);
  if (htleveloff || hlevelmult != 1.0) {
    sprintf (location+strlen(location), " hloff=");
    if (hlevelmult != 1.0)
      sprintf (location+strlen(location), "*%.2f ", hlevelmult);
    if (htleveloff)
      sprintf (location+strlen(location), "%+.2f",  htleveloff);
  }
  if (ltleveloff || llevelmult != 1.0) {
    sprintf (location+strlen(location), " lloff=");
    if (llevelmult != 1.0)
      sprintf (location+strlen(location), "*%.2f ", llevelmult);
    if (ltleveloff)
      sprintf (location+strlen(location), "%+.2f",  ltleveloff);
  }
*/
  // Apply time offset
/*
  if (httimeoff == lttimeoff && httimeoff != 0) {
    int looper;
    for (looper=0; looper<num_csts; looper++)
      loc_epoch[looper] += cst_speeds[looper] * (double)httimeoff;
    httimeoff = lttimeoff = 0;
  }
*/
/*
  // Apply units conversions
  if (youwant) {
    int iyouhave, iyouwant;
    iyouhave = findunit (units);
    iyouwant = findunit (youwant);
    if (iyouhave != -1 && iyouwant != -1 && iyouhave != iyouwant) {
      if (known_units[iyouhave].type == known_units[iyouwant].type) {
        int looper;
        double mult = known_units[iyouhave].conv_factor /
                      known_units[iyouwant].conv_factor;
        for (looper=0; looper<num_csts; looper++)
          loc_amp[looper] *= mult;
        DATUM *= mult;
        strcpy (units, known_units[iyouwant].name);
        strcpy (units_abbrv, known_units[iyouwant].abbrv);
      }
    }
  }
*/
/*
  // Apply units conversions to offsets (in feet)
  if (!iscurrent && strcmp(units_abbrv,"ft")) {
    int iyouhave, iyouwant;
    iyouhave = findunit ("ft");
    iyouwant = findunit (units);
    if (iyouhave != -1 && iyouwant != -1 && iyouhave != iyouwant) {
      if (known_units[iyouhave].type == known_units[iyouwant].type) {
        double mult = known_units[iyouhave].conv_factor /
                      known_units[iyouwant].conv_factor;
        htleveloff *= mult; // Convert feet to meters
        ltleveloff *= mult;
      }
    }
  }
*/

  // Check for knots^2 and handle any of those conversions
//  psd->have_BOGUS = (findunit(psd->unit) != -1) && (known_units[findunit(psd->unit)].type == BOGUS);
/*
  if (psd->have_BOGUS)
  {
        pIDX->IDX_ht_off *= pIDX->IDX_ht_off;         // Square offset in kts to adjust for kts^2
        pIDX->IDX_lt_off *= pIDX->IDX_lt_off;

//     htleveloff *= htleveloff1; // Square offset in kts to adjust for kts^2
//     ltleveloff *= ltleveloff;

     if (convert_BOGUS) {
        int knots = findunit("knots");
        if (knots != -1) {
           strcpy (psd->units_conv,       known_units[knots].name);
           strcpy (psd->units_abbrv,      known_units[knots].abbrv);
        }
     }

  }

*/
  // Apply level offset
  /*
  if (htleveloff == ltleveloff && htleveloff != 0.0) {
     DATUM += htleveloff;
     htleveloff = ltleveloff = 0.0;
  }
  if (hlevelmult == llevelmult && hlevelmult != 1.0) {
     int looper;
     for (looper=0; looper<num_csts; looper++)
       loc_amp[looper] *= hlevelmult;
     DATUM *= hlevelmult;
     hlevelmult = llevelmult = 1.0;
  }
*/

//   Set flag to indicate whether offsets have been "handled"
/*
  if (httimeoff || lttimeoff || htleveloff != 0.0 || ltleveloff != 0.0 ||
                                hlevelmult != 1.0 || llevelmult != 1.0)
    have_offsets = 1;
*/

  if(       pIDX->IDX_ht_time_off ||
                  pIDX->IDX_ht_time_off ||
                  pIDX->IDX_ht_off != 0.0 ||
                  pIDX->IDX_lt_off != 0.0 ||
                  pIDX->IDX_ht_mpy != 1.0 ||
                  pIDX->IDX_lt_mpy != 1.0)
            have_offsets = 1;

}


/* Figure out max amplitude over all the years in the node factors table. */
/* This function by Geoffrey T. Dairiki */
void TCMgr::figure_amplitude ()
{
  int       i, a;

  if (amplitude == 0.0) {
      for (i = 0; i < num_nodes; i++) {
         double year_amp = 0.0;

         for (a=0; a < num_csts; a++)
               year_amp += pmsd->amplitude[a] * cst_nodes[a][i];
         if (year_amp > amplitude)
               amplitude = year_amp;
      }

/*
      // Figure other, related global values (DWF)
      absmax = DATUM + BOGUS_amplitude(1.0);
      absmin = DATUM - BOGUS_amplitude(1.0);

      absmax = htleveloff + hlevelmult * absmax;
      absmin = ltleveloff + llevelmult * absmin;
      if (absmax <= absmin)
      barf (MAXBELOWMIN);
      fakedatum = (absmax + absmin) / 2.0;
      fakeamplitude = (absmax - absmin) / 2.0;
      if (fabs (fakedatum) >= 100.0 || fakeamplitude >= 100.0)
        fprintf (stderr, "Tidelib warning:  tidal range out of normal bounds\n");
*/
    }
}

/* Figure out normalized multipliers for constituents for a particular
   year.  Save amplitude for drawing unit lines. */
void TCMgr::figure_multipliers ()
{
  int a;

  figure_amplitude();
  for (a = 0; a < num_csts; a++)
      work[a] = pmsd->amplitude[a] * cst_nodes[a][year-first_year] / amplitude;  // BOGUS_amplitude?
//  if (hincmagic)
//      pick_hinc ();
}

/* Re-initialize for a different year */
void TCMgr::happy_new_year (int new_year)
{
  year = new_year;
  figure_multipliers ();
  set_epoch (year, num_epochs, first_year);
}


/* Calculate time_t of the epoch. */
void TCMgr::set_epoch (int year, int num_epochs, int first_year)
{
  struct tm ht;

  ht.tm_year = year - 1900;
  ht.tm_sec = ht.tm_min = ht.tm_hour = ht.tm_mon = 0;
  ht.tm_mday = 1;
  epoch = tm2gmt (&ht);
}

/* This idiotic function is needed by the new tm2gmt. */
#define compare_int(a,b) (((int)(a))-((int)(b)))
int TCMgr::compare_tm (struct tm *a, struct tm *b) {
  int temp;
  /* printf ("A is %d:%d:%d:%d:%d:%d   B is %d:%d:%d:%d:%d:%d\n",
    a->tm_year+1900, a->tm_mon+1, a->tm_mday, a->tm_hour,
    a->tm_min, a->tm_sec,
    b->tm_year+1900, b->tm_mon+1, b->tm_mday, b->tm_hour,
    b->tm_min, b->tm_sec); */

  temp = compare_int (a->tm_year, b->tm_year);
  if (temp)
    return temp;
  temp = compare_int (a->tm_mon, b->tm_mon);
  if (temp)
    return temp;
  temp = compare_int (a->tm_mday, b->tm_mday);
  if (temp)
    return temp;
  temp = compare_int (a->tm_hour, b->tm_hour);
  if (temp)
    return temp;
  temp = compare_int (a->tm_min, b->tm_min);
  if (temp)
    return temp;
  return compare_int (a->tm_sec, b->tm_sec);
}

/* Convert a struct tm in GMT back to a time_t.  isdst is ignored, since
   it never should have been needed by mktime in the first place.

   Note that switching the global time zone to GMT, using mktime, and
   switching back either screws up or core dumps on certain popular
   platforms.

   I continue to assert that the Posix time API should be taken out and
   shot.  The status quo sucks, and Posix just standardized it without
   fixing it.  As a result, we have to use idiotic kludges and workarounds
   like this one.
*/
time_t TCMgr::tm2gmt (struct tm *ht)
{
  time_t guess, newguess, thebit;
  int loopcounter, compare;
  struct tm *gt;

  /*
      "A thing not worth doing at all is not worth doing well."

        -- Bruce W. Arden and Kenneth N. Astill, Numerical Algorithms:
           Origins and Applications, Addison-Wesley, 1970, Ch. 1.
  */

  guess = 0;
  loopcounter = (sizeof(time_t) * 8)-1;
  thebit = ((time_t)1) << (loopcounter-1);

  /* For simplicity, I'm going to insist that the time_t we want is
     positive.  If time_t is signed, skip the sign bit.
   */
  if ((signed long)thebit < (time_t)(0)) {
    /* You can't just shift thebit right because it propagates the sign bit. */
    loopcounter--;
    thebit = ((time_t)1) << (loopcounter-1);
  }

  for (; loopcounter; loopcounter--) {
    newguess = guess | thebit;
    gt = gmtime(&newguess);
    if(NULL != gt)
    {
      compare = compare_tm (gt, ht);
    /* printf ("thebit=%lu  guess=%lu  newguess=%lu  compare=%d\n",
      thebit, guess, newguess, compare); */
      if (compare <= 0)
       guess = newguess;
    }
    thebit >>= 1;
  }

/*
  if (compare_tm (gmtime(&guess), ht)) {
    fprintf (stderr, "XTide:  tm2gmt failed to converge on the following input\n");
    fprintf (stderr, "%d:%d:%d:%d:%d:%d\n",
      ht->tm_year+1900, ht->tm_mon+1, ht->tm_mday, ht->tm_hour,
      ht->tm_min, ht->tm_sec);
    barf (BADTIMESTAMP);
  }
*/
  return guess;
}



/* Allocate tide-prediction storage except for node factors and epochs. */
void TCMgr::allocate_cst ()
{
  cst_speeds = (double *) malloc (num_csts * sizeof (double));
//  loc_amp = (double *) malloc (num_csts * sizeof (double));
//  loc_epoch = (double *) malloc (num_csts * sizeof (double));
  work = (double *) malloc (num_csts * sizeof (double));
}


/* Find a unit; returns -1 if not found. */
int TCMgr::findunit (const char *unit) {
  int a;
  for (a=0; a<NUMUNITS; a++) {
    if (!strcmp (unit, known_units[a].name) ||
        !strcmp (unit, known_units[a].abbrv))
      return a;
  }
  return -1;
}

/* Allocate cst_nodes. */
void TCMgr::allocate_nodes ()
{
  int a;
  cst_nodes = (double **) malloc (num_csts * sizeof (double *));
  for (a=0;a<num_csts;a++)
    cst_nodes[a] = (double *) malloc (num_nodes * sizeof (double));
}

/* Allocate cst_epochs. */
void TCMgr::allocate_epochs ()
{
  int a;
  cst_epochs = (double **) malloc (num_csts * sizeof (double *));
  for (a=0;a<num_csts;a++)
    cst_epochs[a] = (double *) malloc (num_epochs * sizeof (double));
}

/* The following array de-allocates were added (per original win95 implementation) */
void TCMgr::free_cst()
 {
  free(cst_speeds);
//  free(loc_amp);
//  free(loc_epoch);
  free(work);
}
void TCMgr::free_nodes()
 {
  int a;
  if (num_csts && cst_nodes)
     for(a=0;a<num_csts;a++)
        free(cst_nodes[a]);
  free(cst_nodes);
}

void TCMgr::free_epochs()
 {
  int a;
  if (num_csts && cst_epochs)
     for(a=0;a<num_csts;a++)
       free(cst_epochs[a]);
  free(cst_epochs);
}

/* free harmonics data */
void TCMgr::free_data ()
 {
    free_nodes();
    free_epochs();
    free_cst();
}

/* Get rid of trailing garbage in linrec */
char * TCMgr::nojunk (char *linrec)
{
  char *a;
  a = &(linrec[strlen(linrec)]);
  while (a > linrec)
    if (*(a-1) == '\n' || *(a-1) == '\r' || *(a-1) == ' ')
      *(--a) = '\0';
    else
      break;
  return linrec;
}

/* Slackful strcmp; 0 = match.  It's case-insensitive and accepts a
   prefix instead of the entire string.  The second argument is the
   one that can be shorter. Second argument can contain '?' as wild
   card character.
*/
int TCMgr::slackcmp (char *a, char *b)
{
  int c, cmp, n;
  n = strlen (b);
  if ((int)(strlen (a)) < n)
    return 1;
  for (c=0;c<n;c++)
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

/* Read a line from the harmonics file, skipping comment lines */
int TCMgr::next_line (FILE *fp, char linrec[linelen], int end_ok)
{
  do {
    if (!fgets (linrec, linelen, fp)) {
      if (end_ok)
        return 0;
      else {
        fprintf (stderr, "Unexpected end of harmonics file '%s'\n",
        hfile_name);
        exit (-1);
      }
    }
  } while (linrec[0] == '#' || linrec[0] == '\r' || linrec[0] == '\n');
  return 1;
}

/* Remove lingering carriage return, but do nothing else */
int TCMgr::skipnl (FILE *fp)
 {
  char linrec[linelen];
  if(NULL == fgets (linrec, linelen, fp))
      return 0;
  return 1;
}






/* -----------------10/13/97 2:48PM------------------
   clear string of leading and trailing white space
 --------------------------------------------------*/
void TCMgr::clean_string(char *str) {
   while ((str[0] <= ' ') && (str[0] > 0))
         memmove(str, str+1, strlen(str)); // Strip leading blanks
   while (strlen(str) && str[strlen(str)-1] == ' ')
         str[strlen(str)-1] = '\0';  // Strip trailing blanks and controls
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

   Allocate space and copy string

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int TCMgr::allocate_copy_string(char **dst, const char *string) {
   char *cp=(char *)malloc( (int)(strlen(string)) +1);

   *dst = cp;
   if (dst) {
      strcpy(*dst,string);
      return(0);
   } else return (-1);
}





/* -----------------------------------
   Free arrays allocated for harmonic file list
 --------------------------------------------------*/
void TCMgr::free_harmonic_file_list() {
harmonic_file_entry *pHarmonic, *pHarmonic_next;

   pHarmonic=harmonic_file_list;
   if (NULL != pHarmonic)
   {
      while (pHarmonic->next) {
         pHarmonic_next = (harmonic_file_entry *)pHarmonic->next;
         free(pHarmonic->name);
         free(pHarmonic);
         pHarmonic=pHarmonic_next;
      }

      free(pHarmonic->name);
      free(pHarmonic);
      harmonic_file_list = NULL;
   }
}

/* -----------------------------------
   Free arrays allocated for abbreviation list
 --------------------------------------------------*/
void TCMgr::free_abbreviation_list() {
int i, done;

   if (abbreviation_list) {
      done = FALSE;
      for (i=0; abbreviation_list[i] && !done; i++) {
         if (abbreviation_list[i]->type) {
            free( abbreviation_list[i]->short_s);
            free( abbreviation_list[i]->long_s);
         }
         else done = TRUE;
         free( abbreviation_list[i] );
      }
      free(abbreviation_list); // and free master pointer
      abbreviation_list = NULL;
   }
   have_index = FALSE;
}

/* -----------------------------------
   Free arrays allocated for station index
 --------------------------------------------------*/
void TCMgr::free_station_index() {
IDX_entry *pIDX, *pIDX_next;

      int in = 1;

      if (pIDX_first) {
      pIDX = pIDX_first;
      pIDX_next=pIDX_first;

      while (NULL != pIDX_next)
      {
         pIDX_next=(IDX_entry *)pIDX->IDX_next;

         if (pIDX->IDX_tzname != NULL)
             free(pIDX->IDX_tzname);
         free(pIDX);

         pIDX = pIDX_next;
       in++;
      }

//      free( pIDX);
      pIDX_first = NULL;
   }
   index_in_memory = FALSE;
}


/*---------------------------------
 * Low level Index file I/O
 * These routines are used to make operations on the
 * Master index file and User index file orthagonal.
 * --------------------------------------------------*/
#define IFF_OPEN  0
#define IFF_CLOSE 1
#define IFF_SEEK  2
#define IFF_TELL  3
#define IFF_READ  4

 long TCMgr::IndexFileIO(int func, long value) {
char *str;

   switch ( func ) {
// Close either/both if open
     case IFF_CLOSE :
       if (IndexFile) fclose(IndexFile);
       IndexFile = NULL;
//       if (UserFile) fclose(UserFile);
//       UserFile = NULL;
       return(0);
// Open both, short circuit if master index file is missing
     case IFF_OPEN :
       IndexFile = fopen( indexfile_name, "rt");
       if (IndexFile == NULL) return(0);
//       UserFile = fopen( userfile_name, "rt");
       return(1);
// Return file pointer only happens with master file
     case IFF_TELL :
       return(ftell(IndexFile));
// Seek happens with both but always rewinds user file
     case IFF_SEEK :
 //      if (UserFile) fseek(UserFile,0L,SEEK_SET);
       return(fseek(IndexFile,value,SEEK_SET));
// Read master until EOF then read user file.
     case IFF_READ :
       str = fgets( index_line, 1024, IndexFile);

/*
       if (str != NULL)
       {
// Scrub the index_line[] for invalid characters
            int i_scrub = 0;
            while(index_line[i_scrub])
            {
                if((signed char)index_line[i_scrub] < 0)
                    index_line[i_scrub] = '?';
                i_scrub++;
            }

            return(1);
       }
*/
 //      if (UserFile)
 //        str = fgets( index_line, 1024, UserFile);
       if (str != NULL)
            return(1);
       else return(0);
   }
   return(0);
}

/*---------------------------------
 * User Station File Functions
 *  USF_REMOVE 1 - remove named entry from USF
 *  USF_UPDATE 2 - update/add named entry
 *  USF_WRITE  3 - closing, write file if changed
 * --------------------------------------------------*/
void TCMgr::UserStationFuncs(int func, char *custom_name) {
/*dsr
int was_open, d;
char bakname[65], sign, s[256], stz[256];
IDX_entry *pIDX, *pIDX_prev;

  switch ( func ) {
// Remove - find user station entry and unlink it from the index.
    case USF_REMOVE :
      pIDX_prev = NULL;
      pIDX      = pIDX_first;
      while (pIDX != NULL && strcmp(custom_name, pIDX->IDX_station_name)) {
        pIDX_prev = pIDX;
        pIDX      = (IDX_entry *)pIDX->IDX_next;
      }
      if (pIDX != NULL && pIDX_prev != NULL) {
        pIDX_prev->IDX_next = pIDX->IDX_next;
        free(pIDX);
        UserFileChanged = TRUE;
      }
    break;

    case USF_UPDATE :
      if (!index_in_memory)
        init_index_file(TRUE, NULL); // Load full index
      pIDX_prev = NULL;
      pIDX      = pIDX_first;
      while (pIDX != NULL && strcmp(custom_name, pIDX->IDX_station_name)) {
        pIDX_prev = pIDX;
        pIDX      = (IDX_entry *)pIDX->IDX_next;
      }
      if (pIDX==NULL && pIDX_prev!=NULL && NULL!=(pIDX = (IDX_entry *)malloc(sizeof(IDX_entry)))) {
        pIDX_prev->IDX_next = pIDX;          // New entry so link it into the list
        pIDX->IDX_rec_num   = pIDX_prev->IDX_rec_num+1;
        pIDX->IDX_next      = NULL;
        pIDX->IDX_tzname    = NULL;
      }
      if (pIDX != NULL) {
        strcpy(pIDX->IDX_station_name,   custom_name);
        strcpy(pIDX->IDX_reference_name, IDX_reference_name);
        strcpy(pIDX->IDX_zone,           Izone);
        pIDX->IDX_type        = iscurrent? 'u' : 'U';
        pIDX->IDX_lon         = Ilon;
        pIDX->IDX_lat         = Ilat;
        pIDX->IDX_time_zone   = IDX_time_zone; // this is reference station meridian
        pIDX->IDX_ht_time_off = Ihttimeoff/60;
        pIDX->IDX_ht_mpy      = Ihlevelmult;
        pIDX->IDX_ht_off      = Ihtleveloff;
        pIDX->IDX_lt_time_off = Ilttimeoff/60;
        pIDX->IDX_lt_mpy      = Illevelmult;
        pIDX->IDX_lt_off      = Iltleveloff;
        pIDX->IDX_sta_num     = IDX_sta_num;
        pIDX->IDX_ref_file_num= IDX_ref_file_num;
        if (pIDX->IDX_tzname != NULL) free(pIDX->IDX_tzname);
        if (strlen(tadjust_last) > 1 && isalpha(tadjust_last[0]) &&
           (NULL!=(pIDX->IDX_tzname = malloc(strlen(tadjust_last)+1)))) {
          strcpy(pIDX->IDX_tzname, tadjust_last);
          strcpy(IDX_tzname, pIDX->IDX_tzname);
        }
        else {
          pIDX->IDX_tzname = NULL;
          strcpy(IDX_tzname, "");
        }

        strcpy(IDX_station_name,   pIDX->IDX_station_name);
        strcpy(IDX_reference_name, pIDX->IDX_reference_name);
        strcpy(IDX_zone,           pIDX->IDX_zone);
        IDX_rec_num     = pIDX->IDX_rec_num;
        IDX_type        = pIDX->IDX_type;
        IDX_lon         = pIDX->IDX_lon;
        IDX_lat         = pIDX->IDX_lat;
        IDX_time_zone   = pIDX->IDX_time_zone;
        IDX_ht_time_off = pIDX->IDX_ht_time_off;
        IDX_ht_mpy      = pIDX->IDX_ht_mpy;
        IDX_ht_off      = pIDX->IDX_ht_off;
        IDX_lt_time_off = pIDX->IDX_lt_time_off;
        IDX_lt_mpy      = pIDX->IDX_lt_mpy;
        IDX_lt_off      = pIDX->IDX_lt_off;
        IDX_sta_num     = pIDX->IDX_sta_num;
        IDX_ref_file_num= pIDX->IDX_ref_file_num;

        UserFileChanged = TRUE;
      }
    break;

    case USF_WRITE :
      if (UserFileChanged) {
        was_open = (UserFile != NULL);
        if (was_open) fclose(UserFile);

        strcpy(bakname, userfile_name);
        if (     bakname[strlen(bakname)-4] == '.')
             strcpy(bakname+strlen(bakname)-4, ".bak");
        else strcpy(bakname+strlen(bakname)  , ".bak");
        CopyFile(userfile_name, bakname, FALSE);

        UserFile = fopen( userfile_name, "wt");
        pIDX = pIDX_first;
        while (pIDX != NULL) {
          if (strchr("Uu", pIDX->IDX_type)) {
//uNAG:US:FL: -82.6833 27.9833 -5:0 Hopper's test case
//&98 1.22 0 115 1.28 0 3551 [tzname] 1 St. Petersburg, Florida
            sign = (pIDX->IDX_time_zone < 0)? '-' : ' ';
            if (pIDX->IDX_tzname!=NULL)
                 strcpy(stz, pIDX->IDX_tzname);
            else strcpy(stz, "");
            sprintf(s,"%c%s %.4lf %.4lf %c%d:%d %s\n",
              pIDX->IDX_type,
              pIDX->IDX_zone,
              pIDX->IDX_lon,
              pIDX->IDX_lat,
              sign,
              abs(pIDX->IDX_time_zone / 60),
              abs(pIDX->IDX_time_zone % 60),
              pIDX->IDX_station_name);
            fwrite(s, strlen(s),1, UserFile);
            sprintf(s,"&%d %.2f %.1f %d %.2f %.1f %d %s %d %s\n",
              pIDX->IDX_ht_time_off,
              pIDX->IDX_ht_mpy,
              pIDX->IDX_ht_off,
              pIDX->IDX_lt_time_off,
              pIDX->IDX_lt_mpy,
              pIDX->IDX_lt_off,
              pIDX->IDX_sta_num,
              stz,
              0,//pIDX->IDX_ref_file_num,
              pIDX->IDX_reference_name
            );
            fwrite(s, strlen(s),1, UserFile);
          }
          pIDX=pIDX->IDX_next;
        }
        if (UserFile) fclose(UserFile);
        if (was_open)
          UserFile = fopen( userfile_name, "rt");
      }
    break;
  }
*/
}

/* ----------------------------------
   Decode an index data line into an IDX_entry structure.
 --------------------------------------------------*/

int TCMgr::build_IDX_entry(IDX_entry *pIDX )
{
int TZHr, TZMin ;
harmonic_file_entry *pHarmonic;
IDX_entry *pIDXh;
char stz[80];

#ifdef __WXMSW__
//#error Added extra \r to format strings... check it, or convert to wxTextFile
#endif
      pIDX->pref_sta_data = NULL;                     // no reference data yet
      pIDX->IDX_Useable = 1;                          // but assume data is OK

      pIDX->IDX_tzname = NULL;
      if (7 != sscanf( index_line, "%c%s%lf%lf%d:%d%*c%[^\r\n]",
            &pIDX->IDX_type,&pIDX->IDX_zone[0],&pIDX->IDX_lon,&pIDX->IDX_lat,&TZHr,&TZMin,
            &pIDX->IDX_station_name[0])) return(1);

//      if(!strncmp(pIDX->IDX_station_name, "Rivi", 4))
//            int hhk = 4;

      pIDX->IDX_time_zone = TZHr*60 + TZMin;

      if (strchr("tcUu",index_line[0])) { // Substation so get second line of info
                  IndexFileIO(IFF_READ, 0);

                  /*
                  cNAA:XX:XX: -122.5500  47.7167 -8:0 Agate Passage, north end
                  ^-88   0.80 0 -18   0.70 0   1301 230 32 1 ADMIRALTY INLET (off Bush Point), WASH.
                  */

            if(index_line[0] == '^')                  // Opencpn special
            {
                  if (11 != sscanf(index_line, "%*c%d %f %f %d %f %f %d %d %d %d%*c%[^\r\n]",
                        &pIDX->IDX_ht_time_off, &pIDX->IDX_ht_mpy, &pIDX->IDX_ht_off,
                        &pIDX->IDX_lt_time_off, &pIDX->IDX_lt_mpy, &pIDX->IDX_lt_off,
                        &pIDX->IDX_sta_num, &pIDX->IDX_flood_dir, &pIDX->IDX_ebb_dir,
                        &pIDX->IDX_ref_file_num, pIDX->IDX_reference_name))
                  return(1);

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
            /*
            cNAP:US:CA: -123.3667 48.3167 -08:00 045 235 Trial Island, 5.2 miles SSW of
            & 0:39 *0.7  0:55 *0.5 0 0 Admiralty Inlet
            */

            else
            {
                        if (9 != sscanf(index_line, "%*c%d %f %f %d %f %f %d %d%*c%[^\r\n]",
                        &pIDX->IDX_ht_time_off, &pIDX->IDX_ht_mpy, &pIDX->IDX_ht_off,
                        &pIDX->IDX_lt_time_off, &pIDX->IDX_lt_mpy, &pIDX->IDX_lt_off,
                        &pIDX->IDX_sta_num, &pIDX->IDX_ref_file_num, pIDX->IDX_reference_name))
                  {
// Had an error so try alternate with timezone name before ref file number
                        if (10 != sscanf(index_line, "%*c%d %f %f %d %f %f %d %s %d%*c%[^\r\n]",
                              &pIDX->IDX_ht_time_off, &pIDX->IDX_ht_mpy, &pIDX->IDX_ht_off,
                              &pIDX->IDX_lt_time_off, &pIDX->IDX_lt_mpy, &pIDX->IDX_lt_off,
                              &pIDX->IDX_sta_num, stz, &pIDX->IDX_ref_file_num, pIDX->IDX_reference_name))
                              return(1);
                  }


                  if (NULL!=(pIDX->IDX_tzname = (char *)malloc(strlen(stz)+1)))
                        strcpy(pIDX->IDX_tzname, stz);
                  }           // else


                  if (pIDX->IDX_ref_file_num <= 0)
            { // Find harmonic reference file number
                  pIDX->IDX_ref_file_num= 0;
// Find reference station in index, if no index, it had better be in the first one
                  pIDXh = pIDX_first;
                  while (pIDXh!=NULL && strcmp(pIDXh->IDX_reference_name,pIDX->IDX_reference_name))
                        pIDXh = (IDX_entry *)pIDXh->IDX_next;

// Copy reference station harmonic file number
                  if (pIDXh!=NULL)
                        pIDX->IDX_ref_file_num = pIDXh->IDX_ref_file_num;
                  }
      }

      else
      { // Reference stations have no offsets
                  pIDX->IDX_ht_time_off = pIDX->IDX_lt_time_off = 0;
                  pIDX->IDX_ht_mpy      = pIDX->IDX_lt_mpy = 1.0;
                  pIDX->IDX_ht_off      = pIDX->IDX_lt_off = 0.0;
                  pIDX->IDX_sta_num     = 0;
                  strcpy(pIDX->IDX_reference_name, pIDX->IDX_station_name);

                  pIDX->IDX_ref_file_num= 0;
                  pHarmonic = harmonic_file_list;
                  while (pHarmonic && (pHarmonic->rec_start <= pIDX->IDX_rec_num))
            {
                  pHarmonic = (harmonic_file_entry *)pHarmonic->next;
                  pIDX->IDX_ref_file_num++;
                  }
      }


      return(0);
}

/* -----------------------------------
   Initialize index file.
   Generate local copy of abbreviation list then make
   a linked list of decoded station entries.
 --------------------------------------------------*/
int TCMgr::init_index_file(int load_index, int hwnd)
{
char s1[80], s2[80], s3[80];
long int xref_start=0;
int doing_xref=0, num_abv=0, num_IDX=0, i;
IDX_entry *pIDX, *pIDX_prev;
harmonic_file_entry *pHarmonic, *pHarmonic_prev;

      free_harmonic_file_list();
      free_abbreviation_list();
      free_station_index();
      have_index = 0;
      index_in_memory = 0;
      pIDX_first = pIDX_prev = NULL;
      if (IndexFileIO(IFF_OPEN, 0)) {
        while (IndexFileIO(IFF_READ, 0)) {
         if ((index_line[0] == '#') || (index_line[0] <= ' '));  // Skip comment lines
         else if (!have_index && !xref_start) {
            if (!strncmp(index_line, "XREF", 4))
               xref_start = IndexFileIO(IFF_TELL, 0);
         }
         else if (!have_index && !strncmp(index_line, "*END*", 5)) {
            if (num_abv == 0) {
               IndexFileIO(IFF_CLOSE, 0);
               return(FALSE); // missing at least some data so no valid index
            }
            if (doing_xref++ == 0) { // First pass through, flag for second pass
               IndexFileIO(IFF_SEEK, xref_start); // Position back to start of index
               // Allocate memory for the array
               if (NULL==(abbreviation_list=
                  (abbreviation_entry **)malloc((num_abv+1)*sizeof(abbreviation_entry *)))) {
                  IndexFileIO(IFF_CLOSE, 0);
//                  no_mem_msg();
                  return(FALSE);
               }
               else {
                  for (i=0; i<=num_abv; i++)
                     if (NULL==(abbreviation_list[i] = (abbreviation_entry *)
                              malloc(sizeof(abbreviation_entry)))) { // If we can't allocate..
                        free_abbreviation_list();
                        IndexFileIO(IFF_CLOSE, 0);
//                        no_mem_msg();
                        return(FALSE);
                     }
                  abbreviation_list[num_abv]->type    = 0;   // Flag final entry
                  abbreviation_list[num_abv]->short_s = NULL;
                  abbreviation_list[num_abv]->long_s  = NULL;
                  num_abv = 0;
               }
            }
              // We're done (and no errors)
            else have_index = 1;
         } // found *END* of cross reference

         else if (!have_index && xref_start) {
            sscanf( index_line, "%s%s%[^\n]", s1, s2, s3 );
            clean_string( s3 );
            if (  (!strncmp(s1,"REGION",6))  ||
                  (!strncmp(s1,"COUNTRY",7)) ||
                  (!strncmp(s1,"STATE",5))) {
               if (doing_xref) {
                  if (allocate_copy_string(&abbreviation_list[num_abv]->short_s, s2) ||
                     (allocate_copy_string(&abbreviation_list[num_abv]->long_s, s3))) {
                     free_abbreviation_list();
                     IndexFileIO(IFF_CLOSE, 0);
//                     no_mem_msg();
                     return(FALSE);
                  }
                  if      (!strncmp(s1,"REGION",6))  abbreviation_list[num_abv]->type = REGION;
                  else if (!strncmp(s1,"COUNTRY",7)) abbreviation_list[num_abv]->type = COUNTRY;
                  else                               abbreviation_list[num_abv]->type = STATE;
               }
               num_abv++;
            }
         }
         else if (have_index && (strchr("TtCcIUu", index_line[0]))) {
// All done with abbreviation list.
// Load index file data into memory (assuming we can).
            num_IDX++; // Keep counting entries for harmonic file stuff
            if (load_index) {
               if (NULL!=(pIDX = (IDX_entry *)malloc(sizeof(IDX_entry)))) {
                  index_in_memory   = TRUE;
                  pIDX->IDX_next    = NULL;
                  pIDX->IDX_rec_num = num_IDX;
                  pIDX->IDX_tried_once = 0;               // master station search control
                  pIDX->Valid15 = 0;

                  if (build_IDX_entry(pIDX))
                     printf("Index file error at entry %d!\n", num_IDX);
                  if (pIDX_first == NULL)
                     pIDX_first = pIDX;
                  else
                     pIDX_prev->IDX_next = pIDX;
                  pIDX_prev = pIDX;
               }
               else {  // Could not allocate memory for index, do long way
// We couldn't allocate memory somewhere along the line, so free all we have so far.
//                  no_mem_msg();
                  free_station_index(); // Free any we have allocated so far
               }
            }
         }
         else if (have_index && (index_line[0] == 'H')) {
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
      } // while (more file)
      if (index_in_memory) IndexFileIO(IFF_CLOSE, 0); // All done with file
   } // index file can't be opened
//   if (hwndBusy) DestroyWindow(hwndBusy);

      max_IDX = num_IDX;
   return( have_index );
} // init_index_file()

/* -----------------10/13/97 3:18PM------------------
   get data from index file
   If the data has been loaded into memory (by init_index) then
   all we have to do is pass the next pointer.
   Otherwise, we have to read the next line(s) from the index file.

   rec_num parameter takes the following values:
   <0: Next record
   =0: rewind to first record
   >0: return absolute record number
 --------------------------------------------------*/
 IDX_entry *TCMgr::get_index_data( short int rec_num ) {

   if (rec_num >= 0) {
      if (index_in_memory) {
         if (rec_num == 0) rewound = TRUE;
         else {
            rewound = FALSE;
            pIDXs = pIDX_first;
            while ((--rec_num > 0) && (pIDXs != NULL)) pIDXs=(IDX_entry *)pIDXs->IDX_next;
            return (pIDXs );
         }
      }
      else {
         IDX_rec_num = 0;
         IndexFileIO(IFF_SEEK, 0l); // Position back to start of index
         looking_end = TRUE;
         while (looking_end && IndexFileIO(IFF_READ, 0)) {
            if (!strncmp(index_line, "*END*", 5)) // Stop looking when *END* found
               looking_end = FALSE;
         }
         if (!looking_end) {
            while (rec_num > 0 && IndexFileIO(IFF_READ, 0))
               if (strchr("TtCcIUu",index_line[0])) {
                  rec_num--;
                  IDX_rec_num++;
               }
            if (rec_num < 0) {
               pIDXs = &IDX;
               if (build_IDX_entry(pIDXs))
                  printf("Index file error at entry %d!\n", IDX_rec_num);
               pIDXs->IDX_rec_num = IDX_rec_num;
               return (pIDXs);
            }
         }
      }
   }
   else {
      if (index_in_memory) {
         if (rewound) {
            rewound = FALSE;
            return( pIDXs = pIDX_first);
         }
         if (pIDXs != NULL) return(pIDXs = (IDX_entry *)pIDXs->IDX_next);
      }
      else {
         while (IndexFileIO(IFF_READ, 0)) {
            if (strchr("TtCcIUu",index_line[0])) {
               IDX_rec_num++;
               pIDXs = &IDX;
               build_IDX_entry(pIDXs);
               pIDXs->IDX_rec_num = IDX_rec_num;
               return (pIDXs);
            }
         }
      }
   }
   return( NULL );
}



#if 0
/* -----------------10/13/97 3:16PM------------------
   Find an entry for a given station and load it's info.
   If rec_num is a valid index record number, that info is
   loaded, otherwise a station name search is performed.
 --------------------------------------------------*/
int load_location_info(char *station_name, int rec_num) {
char type, temp_station[MAXNAMELEN];
int s, found, sloppy=TRUE;
IDX_entry *pIDX;

// First extract (then remove) any station type ("TtCcUu") appended to the station name.
   type = '\0';
   strncpy( temp_station, station_name, sizeof(temp_station)-1 );
   temp_station[sizeof(temp_station)-1] = '\0'; // If station name is LOOOONNNGGGG, clip it
   if (strlen(temp_station) > 4) {
      s = strlen(temp_station) - 3;
      if ((temp_station[s] == '(') && (temp_station[s+2] == ')')) {
         type = temp_station[s+1];
         if (strchr("TtCcUu", type)) {
            sloppy = FALSE;
            temp_station[s-1] = '\0';
         }
      }
   }
// Now search for that station name and type code
   clean_string( temp_station );
   if (have_index) {
      if ((rec_num <= 0) || ((rec_num>0) && (!(pIDX=get_index_data( rec_num ))))) {
         get_index_data( 0 ); // Have to search for it
         found = FALSE;
         while (!found && (NULL!=(pIDX=(get_index_data( -1 ))))) {
            if (((type == '\0') || (type == pIDX->IDX_type)) &&
               (             !strcmp(pIDX->IDX_station_name,temp_station) ||
                (sloppy && !slackcmp(pIDX->IDX_station_name,temp_station))) )
               found = TRUE;
         }
      }
      if (pIDX) {
// We found the entry!
//printf("IDX_type=%c, IDX_lon=%f, IDX_lat=%f \n",pIDX->IDX_type, pIDX->IDX_lon, pIDX->IDX_lat);
//printf("IDX_station_name=[%s]\n",pIDX->IDX_station_name);
//printf("IDX_ht_time_off=%d, IDX_ht_mpy=%f, IDX_ht_off=%f\n",pIDX->IDX_ht_time_off,pIDX->IDX_ht_mpy,pIDX->IDX_ht_off);
//printf("IDX_lt_time_off=%d, IDX_lt_mpy=%f, IDX_lt_off=%f\n",pIDX->IDX_lt_time_off,pIDX->IDX_lt_mpy,pIDX->IDX_lt_off);
//printf("IDX_sta_num=%d,IDX_ref_file_num=%d,IDX_reference_name=%s\n",pIDX->IDX_sta_num,pIDX->IDX_ref_file_num,pIDX->IDX_station_name);
         IDX_rec_num     = pIDX->IDX_rec_num;
         IDX_type        = pIDX->IDX_type;
         strcpy(IDX_zone, pIDX->IDX_zone);
         IDX_lon         = pIDX->IDX_lon;
         IDX_lat         = pIDX->IDX_lat;
         IDX_time_zone   = pIDX->IDX_time_zone;
         strcpy( IDX_station_name, pIDX->IDX_station_name);
         IDX_ht_time_off = pIDX->IDX_ht_time_off;
         IDX_ht_mpy      = pIDX->IDX_ht_mpy;
         IDX_ht_off      = pIDX->IDX_ht_off;
         IDX_lt_time_off = pIDX->IDX_lt_time_off;
         IDX_lt_mpy      = pIDX->IDX_lt_mpy;
         IDX_lt_off      = pIDX->IDX_lt_off;
         IDX_sta_num     = pIDX->IDX_sta_num;
         IDX_ref_file_num= pIDX->IDX_ref_file_num;
         strcpy(IDX_reference_name, pIDX->IDX_reference_name);
         if (pIDX->IDX_tzname != NULL)
              strcpy(IDX_tzname,pIDX->IDX_tzname);
         else strcpy(IDX_tzname,"");

         return( TRUE ); //  "We found it"
      }
   }
   else {  // We don't have an index so fake it
      strcpy( IDX_station_name,   temp_station );
      strcpy( IDX_reference_name, temp_station );
      if (strstr(station_name, "Current"))
           IDX_type = 'C';
      else IDX_type = 'T';
      strcpy(IDX_zone, ":::");
      IDX_rec_num     = 0;
      IDX_lon         = IDX_lat = 0.0;
      IDX_ht_mpy      = IDX_lt_mpy = 1.0;
      IDX_ht_off      = IDX_lt_off = 0.0;
      IDX_ht_time_off = IDX_lt_time_off = 0;
      IDX_time_zone   = 0;
      IDX_sta_num     = 0;
      strcpy(IDX_tzname,"");
      IDX_ref_file_num= 0;
      return ( TRUE ); // "We found it"
   }
   return( FALSE ); // "We didn't find it"
}


/* Load up harmonics data once location is found (called by load_data). */
static void
load_this_data (FILE *fp, char linrec[linelen])
{
  char junk[80];
  int a;
  strcpy (location, linrec);
  /* Is it a current? */
  if (strstr (location, "Current"))
    iscurrent = 1;
  else
    iscurrent = 0; //added mgh
  /* Get meridian */
  next_line (fp, linrec, 0);
  meridian = hhmm2seconds (linrec);
  /* Get tzfile, if present */
  if (sscanf (nojunk(linrec), "%s %s", junk, tzfile) < 2)
    strcpy (tzfile, "UTC0");
  /* Get DATUM and units */
  next_line (fp, linrec, 0);
  if (sscanf (nojunk(linrec), "%lf %s", &DATUM, units) < 2)
    strcpy (units, "unknown");
  if ((a = findunit (units)) == -1)
    strcpy (units_abbrv, units);
  else {
    if (!strcmp (units, known_units[a].name))
      strcpy (units_abbrv, known_units[a].abbrv);
    else {
      strcpy (units_abbrv, units);
      strcpy (units, known_units[a].name);
    }
  }
  /* Get constituents */
  for (a=0;a<num_csts;a++) {
    next_line (fp, linrec, 0);
    win_assert (sscanf (linrec, "%s %lf %lf", junk, &(loc_amp[a]),
      &(loc_epoch[a])) == 3);
    /* win_assert (loc_amp[a] >= 0.0); */
    if (loc_amp[a] < 0.0) {
      fprintf (stderr, "Location = %s\n", location);
      win_assert (loc_amp[a] >= 0.0);
    }
    loc_epoch[a] *= M_PI / 180.0;
  }
  fclose (fp);
}

/* Load harmonics data */
void
load_data ()
{
  FILE *fp;
  char linrec[linelen], junk[80];
  int a, b;
  if (!(fp = fopen (hfile_name, "r"))) {
    fprintf (stderr, "Could not open harmonics file '%s'\n", hfile_name);
    barf (CANTOPENFILE);
  }
  free_data();
  next_line (fp, linrec, 0);
  win_assert (sscanf (linrec, "%d", &num_csts) == 1);
  allocate_cst ();
  /* Load constituent speeds */
  for (a=0;a<num_csts;a++) {
    next_line (fp, linrec, 0);
    win_assert (sscanf (linrec, "%s %lf", junk, &(cst_speeds[a])) == 2);
    cst_speeds[a] *= M_PI / 648000; /* Convert to radians per second */
          /*   M_PI / 180 to get radians, / 3600 to get seconds */
  }
  /* Get first year for nodes and epochs */
  next_line (fp, linrec, 0);
  win_assert (sscanf (linrec, "%d", &first_year) == 1);

  /* Load epoch table */
  next_line (fp, linrec, 0);
  win_assert (sscanf (linrec, "%d", &num_epochs) == 1);
  allocate_epochs ();
  for (a=0;a<num_csts;a++) {
    win_assert (fscanf (fp, "%s", linrec) == 1);
    for (b=0;b<num_epochs;b++) {
      win_assert (fscanf (fp, "%lf", &(cst_epochs[a][b])) == 1);
      cst_epochs[a][b] *= M_PI / 180.0;
    }
  }
  /* Sanity check */
  win_assert (fscanf (fp, "%s", linrec) == 1);
  win_assert (!strcmp (linrec, "*END*"));
  skipnl (fp);

  /* Load node factor table */
  next_line (fp, linrec, 0);
  win_assert (sscanf (linrec, "%d", &num_nodes) == 1);
  allocate_nodes ();
  for (a=0;a<num_csts;a++) {
    win_assert (fscanf (fp, "%s", linrec) == 1);
    for (b=0;b<num_nodes;b++)
      win_assert (fscanf (fp, "%lf", &(cst_nodes[a][b])) == 1);
  }
  /* Sanity check */
  win_assert (fscanf (fp, "%s", linrec) == 1);
  win_assert (!strcmp (linrec, "*END*"));
  skipnl (fp);



  /* Load harmonic constants for desired location */
  while (next_line (fp, linrec, 1)) {
    nojunk (linrec);
    if (curonly)
      if (!strstr (linrec, "Current"))
        continue;
    if (slackcmp (linrec, location))
      continue;
    load_this_data (fp, linrec);
    return;
  }
  if (curonly)
    fprintf (stderr, "Could not find current set '%s' in harmonics file %s\n",
      location, hfile_name);
  else
    fprintf (stderr, "Could not find location '%s' in harmonics file %s\n",
      location, hfile_name);
//  exit (-1);
}

#endif

//-----------------------------------------------------------------------------------
//    TIDELIB
//-----------------------------------------------------------------------------------


double TCMgr::time2tide (time_t t)
{
  return time2dt_tide(t, 0);
}



/** BOGUS amplitude stuff - Added mgh
 * For knots^2 current stations, returns square root of (value * amplitude),
 * For normal stations, returns value * amplitude */

double TCMgr::BOGUS_amplitude(double mpy, IDX_entry *pIDX)
{
       Station_Data *pmsd = pIDX->pref_sta_data;

      if (!pmsd->have_BOGUS)                                // || !convert_BOGUS)   // Added mgh
        return(mpy * amplitude);
  else {
     if (mpy >= 0.0)
        return( sqrt( mpy * amplitude));
     else
        return(-sqrt(-mpy * amplitude));
  }
}

/* Calculate the denormalized tide. */
double TCMgr::time2atide (time_t t, IDX_entry *pIDX)
{
  return BOGUS_amplitude(time2tide(t), pIDX) + pmsd->DATUM;
}



/* Next high tide, low tide, transition of the mark level, or some
   combination.
       Bit      Meaning
        0       low tide
        1       high tide
        2       falling transition
        3       rising transition
*/
int TCMgr::next_big_event (time_t *tm, IDX_entry *pIDX)
{
  double p, q;
  int flags = 0, slope = 0;
  p = time2atide (*tm, pIDX);
  *tm += 60;
  q = time2atide (*tm, pIDX);
  *tm += 60;
  if (p < q)
    slope = 1;
  while (1) {
    if ((slope == 1 && q < p) || (slope == 0 && p < q)) {
      /* Tide event */
      flags |= (1 << slope);
    }
    /* Modes in which to return mark transitions: */
    /*    -text (no -graph)   */
    /*    -graph (no -text)   */
    /*    -ppm                */
    /*    -gif                */
    /*    -ps                 */


//    if (mark && ((text && !graphmode) || (!text && graphmode)
//    || ppm || gif || ps))
//      int marklev = 0;
#if (0)
      if(0)
      if ((p > marklev && q <= marklev) || (p < marklev && q >= marklev)) {
        /* Transition event */
        if (p < q)
          flags |= 8;
        else
          flags |= 4;
        if (!(flags & 3)) {
          /* If we're incredibly unlucky, we could miss a tide event if we
             don't check for it here:

                             . <----   Value that would be returned
                        -----------    Mark level
                      .           .
          */
          p = q;
          q = time2atide (*tm, pIDX);
          if ((slope == 1 && q < p) || (slope == 0 && p < q)) {
            /* Tide event */
            flags |= (1 << slope);
          }
        }
      }
#endif

    if (flags) {
      *tm -= 60;
      /* Don't back up over a transition event, but do back up to where the
         tide changed if possible.  If they happen at the same time, then
         we're off by a minute on the tide, but if we back it up it will
         get snagged on the transition event over and over. */
      if (flags < 4)
        *tm -= 60;
      return flags;
    }
    p = q;
    q = time2atide (*tm, pIDX);
    *tm += 60;
  }
}



/* Estimate the normalized mean tide level around a particular time by
   summing only the long-term constituents. */
/* Does not do any blending around year's end. */
/* This is used only by time2asecondary for finding the mean tide level */
double TCMgr::time2mean (time_t t)
{
  double tide = 0.0;
  int a, new_year = yearoftimet (t);
  if (new_year != year)
    happy_new_year (new_year);
  for (a=0;a<num_csts;a++) {
    if (cst_speeds[a] < 6e-6)
      tide += work[a] *
        cos (cst_speeds[a] * ((long)(t - epoch) + pmsd->meridian) +
        cst_epochs[a][year-first_year] - pmsd->epoch[a]);
  }
  return tide;
}



/* If offsets are in effect, interpolate the 'corrected' denormalized
tide.  The normalized is derived from this, instead of the other way
around, because the application of height offsets requires the
denormalized tide. */
double TCMgr::time2asecondary (time_t t, IDX_entry *pIDX) {

  /* Get rid of the normals. */
  if (!(have_offsets))
    return time2atide (t, pIDX);

  {
/* Intervalwidth of 14 (was originally 13) failed on this input:
-location Dublon -hloff +0.0001 -gstart 1997:09:10:00:00 -raw 1997:09:15:00:00
*/
#define intervalwidth 15
#define stretchfactor 3

    static time_t lowtime=0, hightime=0;
    static double lowlvl, highlvl; /* Normalized tide levels for MIN, MAX */
    time_t T;  /* Adjusted t */
    double S, Z, HI, HS, magicnum;
    time_t interval = 3600 * intervalwidth;
    long difflow, diffhigh;
    int badlowflag=0, badhighflag=0;


    /* Algorithm by Jean-Pierre Lapointe (scipur@collegenotre-dame.qc.ca) */
    /* as interpreted, munged, and implemented by DWF */

    /* This is the initial guess (average of time offsets) */
//    T = t - (httimeoff + lttimeoff) / 2;
      T = t - (pIDX->IDX_ht_time_off * 60 + pIDX->IDX_lt_time_off * 60) / 2;
    /* The usage of an estimate of mean tide level here is to correct
       for seasonal changes in tide level.  Previously I had simply used
       the zero of the tide function as the mean, but this gave bad
       results around summer and winter for locations with large seasonal
       variations. */
    Z = time2mean(T);
    S = time2tide(T) - Z;

    /* Find MAX and MIN.  I use the highest high tide and the lowest
       low tide over a 26 hour period, but I allow the interval to stretch
       a lot if necessary to avoid creating discontinuities.  The
       heuristic used is not perfect but will hopefully be good enough.

       It is an assumption in the algorithm that the tide level will
       be above the mean tide level for MAX and below it for MIN.  A
       changeover occurs at mean tide level.  It would be nice to
       always use the two tides that immediately bracket T and to put
       the changeover at mid tide instead of always at mean tide
       level, since this would eliminate much of the inaccuracy.
       Unfortunately if you change the location of the changeover it
       causes the tide function to become discontinuous.

       Now that I'm using time2mean, the changeover does move, but so
       slowly that it makes no difference.
    */

    if (lowtime < T)
      difflow = T - lowtime;
    else
      difflow = lowtime - T;
    if (hightime < T)
      diffhigh = T - hightime;
    else
      diffhigh = hightime - T;

    /* Update MIN? */
    if (difflow > interval * stretchfactor)
      badlowflag = 1;
    if (badlowflag || (difflow > interval && S > 0)) {
      time_t tt;
      double tl;
      tt = T - interval;
      next_big_event (&tt, pIDX);
      lowlvl = time2tide (tt);
      lowtime = tt;
      while (tt < T + interval) {
        next_big_event (&tt, pIDX);
        tl = time2tide (tt);
        if (tl < lowlvl && tt < T + interval) {
          lowlvl = tl;
          lowtime = tt;
        }
      }
    }
    /* Update MAX? */
    if (diffhigh > interval * stretchfactor)
      badhighflag = 1;
    if (badhighflag || (diffhigh > interval && S < 0)) {
      time_t tt;
      double tl;
      tt = T - interval;
      next_big_event (&tt, pIDX);
      highlvl = time2tide (tt);
      hightime = tt;
      while (tt < T + interval) {
        next_big_event (&tt, pIDX);
        tl = time2tide (tt);
        if (tl > highlvl && tt < T + interval) {
          highlvl = tl;
          hightime = tt;
        }
      }
    }

#if 0
    /* UNFORTUNATELY there are times when the tide level NEVER CROSSES
       THE MEAN for extended periods of time.  ARRRGH!  */
    if (lowlvl >= 0.0)
      lowlvl = -1.0;
    if (highlvl <= 0.0)
      highlvl = 1.0;
#endif
    /* Now that I'm using time2mean, I should be guaranteed to get
       an appropriate low and high. */


    /* Improve the initial guess. */
    if (S > 0)
      magicnum = 0.5 * S / fabs(highlvl - Z);
    else
      magicnum = 0.5 * S / fabs(lowlvl - Z);
//    T = T - magicnum * (httimeoff - lttimeoff);
    T = T - (time_t)(magicnum * ((pIDX->IDX_ht_time_off * 60) - (pIDX->IDX_lt_time_off * 60)));
      HI = time2tide(T);

//    Correct the amplitude offsets for BOGUS knot^2 units
      double ht_off, lt_off;
      if (pmsd->have_BOGUS)
      {
            ht_off = pIDX->IDX_ht_off * pIDX->IDX_ht_off;         // Square offset in kts to adjust for kts^2
            lt_off = pIDX->IDX_lt_off * pIDX->IDX_lt_off;
      }
      else
      {
            ht_off = pIDX->IDX_ht_off;
            lt_off = pIDX->IDX_lt_off;
      }


    /* Denormalize and apply the height offsets. */
    HI = BOGUS_amplitude(HI, pIDX) + pmsd->DATUM;
    {
      double RH=1.0, RL=1.0, HH=0.0, HL=0.0;
      RH = pIDX->IDX_ht_mpy;
      HH = ht_off;
      RL = pIDX->IDX_lt_mpy;
      HL = lt_off;

      /* I patched the usage of RH and RL to avoid big ugly
      discontinuities when they are not equal.  -- DWF */

      HS =  HI * ((RH+RL)/2 + (RH-RL)*magicnum)
                + (HH+HL)/2 + (HH-HL)*magicnum;
    }

    return HS;
  }
}






/*
 * We will need a function for tidal height as a function of time
 * which is continuous (and has continuous first and second derivatives)
 * for all times.
 *
 * Since the epochs & multipliers for the tidal constituents change
 * with the year, the regular time2tide(t) function has small
 * discontinuities at new years.  These discontinuities really
 * fry the fast root-finders.
 *
 * We will eliminate the new-years discontinuities by smoothly
 * interpolating (or "blending") between the tides calculated with one
 * year's coefficients, and the tides calculated with the next year's
 * coefficients.
 *
 * i.e. for times near a new years, we will "blend" a tide
 * as follows:
 *
 * tide(t) = tide(year-1, t)
 *                  + w((t - t0) / Tblend) * (tide(year,t) - tide(year-1,t))
 *
 * Here:  t0 is the time of the nearest new-year.
 *        tide(year-1, t) is the tide calculated using the coefficients
 *           for the year just preceding t0.
 *        tide(year, t) is the tide calculated using the coefficients
 *           for the year which starts at t0.
 *        Tblend is the "blending" time scale.  This is set by
 *           the macro TIDE_BLEND_TIME, currently one hour.
 *        w(x) is the "blending function", whice varies smoothly
 *           from 0, for x < -1 to 1 for x > 1.
 *
 * Derivatives of the blended tide can be evaluated in terms of derivatives
 * of w(x), tide(year-1, t), and tide(year, t).  The blended tide is
 * guaranteed to have as many continuous derivatives as w(x).  */

/* time2dt_tide(time_t t, int n)
 *
 *   Calculate nth time derivative the normalized tide.
 *
 * Notes: This function does not check for changes in year.
 *  This is important to our algorithm, since for times near
 *  new years, we interpolate between the tides calculated
 *  using one years coefficients, and the next years coefficients.
 *
 *  Except for this detail, time2dt_tide(t,0) should return a value
 *  identical to time2tide(t).
 */
 double TCMgr::_time2dt_tide (time_t t, int deriv)
{
  double dt_tide = 0.0;
  int a, b;
  double term, tempd;

  tempd = M_PI / 2.0 * deriv;
  for (a=0;a<num_csts;a++)
    {
      term = work[a] *
          cos(tempd +
              cst_speeds[a] * ((long)(t - epoch) + pmsd->meridian) +
              cst_epochs[a][year-first_year] - pmsd->epoch[a]);
      for (b = deriv; b > 0; b--)
          term *= cst_speeds[a];
      dt_tide += term;
    }
  return dt_tide;
}


/* blend_weight (double x, int deriv)
 *
 * Returns the value nth derivative of the "blending function" w(x):
 *
 *   w(x) =  0,     for x <= -1
 *
 *   w(x) =  1/2 + (15/16) x - (5/8) x^3 + (3/16) x^5,
 *                  for  -1 < x < 1
 *
 *   w(x) =  1,     for x >= 1
 *
 * This function has the following desirable properties:
 *
 *    w(x) is exactly either 0 or 1 for |x| > 1
 *
 *    w(x), as well as its first two derivatives are continuous for all x.
 */
static double
blend_weight (double x, int deriv)
{
  double x2 = x * x;

  if (x2 >= 1.0)
      return deriv == 0 && x > 0.0 ? 1.0 : 0.0;

  switch (deriv) {
  case 0:
      return ((3.0 * x2 -10.0) * x2 + 15.0) * x / 16.0 + 0.5;
  case 1:
      return ((x2 - 2.0) * x2 + 1.0) * (15.0/16.0);
  case 2:
      return (x2 - 1.0) * x * (15.0/4.0);
  }
  return(0); // mgh+ to get rid of compiler warning
}

/*
 * This function does the actual "blending" of the tide
 * and its derivatives.
 */
double TCMgr::blend_tide (time_t t, unsigned int deriv, int first_year, double blend)
{
  double        fl[TIDE_MAX_DERIV + 1];
  double        fr[TIDE_MAX_DERIV + 1];
  double *      fp      = fl;
  double        w[TIDE_MAX_DERIV + 1];
  double        fact = 1.0;
  double        f;
  unsigned int  n;


  /*
   * If we are already happy_new_year()ed into one of the two years
   * of interest, compute that years tide values first.
   */
  if (year == first_year + 1)
      fp = fr;
  else if (year != first_year)
      happy_new_year(first_year);
  for (n = 0; n <= deriv; n++)
      fp[n] = _time2dt_tide(t, n);

  /*
   * Compute tide values for the other year of interest,
   *  and the needed values of w(x) and its derivatives.
   */
  if (fp == fl)
    {
      happy_new_year(first_year + 1);
      fp = fr;
    }
  else
    {
      happy_new_year(first_year);
      fp = fl;
    }
  for (n = 0; n <= deriv; n++)
    {
      fp[n] = _time2dt_tide(t, n);
      w[n] = blend_weight(blend, n);
    }

  /*
   * Do the blending.
   */
  f = fl[deriv];
  for (n = 0; n <= deriv; n++)
    {
      f += fact * w[n] * (fr[deriv-n] - fl[deriv-n]);
      fact *= (double)(deriv - n)/(n+1) * (1.0/TIDE_BLEND_TIME);
    }
  return f;
}

double TCMgr::time2dt_tide (time_t t, int deriv)
{
  static time_t next_epoch      = TIDE_BAD_TIME; /* next years newyears */
  static time_t this_epoch      = TIDE_BAD_TIME; /* this years newyears */
  static int    this_year       = -1;
  int           new_year;
      time_t tt = time(NULL);
      int yott = ((gmtime (&tt))->tm_year) + 1900;
      new_year = yott;                    //= yearoftimet(t);

  /* Make sure our values of next_epoch and epoch are up to date. */
  if (new_year != this_year)
    {
      if (new_year + 1 < first_year + num_epochs)
        {
          set_epoch(new_year + 1, num_epochs, first_year);
          next_epoch = epoch;
        }
      else
          next_epoch = TIDE_BAD_TIME;

      happy_new_year(this_year = new_year);
      this_epoch = epoch;
    }


  /*
   * If we're close to either the previous or the next
   * new years we must blend the two years tides.
   */
  if (t - this_epoch <= TIDE_BLEND_TIME && this_year > first_year)
      return blend_tide(t, deriv,
                        this_year - 1,
                        (double)(t - this_epoch)/TIDE_BLEND_TIME);
  else if (next_epoch - t <= TIDE_BLEND_TIME
           && this_year + 1 < first_year + num_epochs)
      return blend_tide(t, deriv,
                        this_year,
                        -(double)(next_epoch - t)/TIDE_BLEND_TIME);

  /*
   * Else, we're far enough from newyears to ignore the blending.
   */
  if (this_year != year)
      happy_new_year(this_year);
  return _time2dt_tide(t, deriv);
}

int TCMgr::GetStationIDXbyName(wxString prefix, double xlat, double xlon, TCMgr *ptcmgr)
	{
		  IDX_entry *lpIDX;
		  int jx = 0;
		  wxString locn;
		  double distx = 100000.;

	     // if (prp->m_MarkName.Find(_T("@~~")) != wxNOT_FOUND) {
		    //tide_form = prp->m_MarkName.Mid(prp->m_MarkName.Find(_T("@~~"))+3);

			for ( int j=1 ; j<ptcmgr->Get_max_IDX() +1 ; j++ ) {
					lpIDX = ptcmgr->GetIDX_entry ( j );
					char type = lpIDX->IDX_type;             // Entry "TCtcIUu" identifier
					wxString locnx ( lpIDX->IDX_station_name, wxConvUTF8 );

					if ( (( type == 't' ) ||  ( type == 'T' ) )   // only Tides
						&& (locnx.StartsWith(prefix))) {
							double brg, dist;
							DistanceBearingMercator(xlat, xlon, lpIDX->IDX_lat, lpIDX->IDX_lon, &brg, &dist);
							if (dist < distx){
								distx = dist;
								//locn = locnx.Prepend(_T(" @~~"));
								jx = j;
							}
						}
			} // end for loop
		  //} // end if @~~ found in WP
			return(jx);
		}
