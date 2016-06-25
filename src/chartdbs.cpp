/**************************************************************************
*
* Project:  ChartManager
* Purpose:  Basic Chart Info Storage
* Author:   David Register, Mark A Sikes
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

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/arrimpl.cpp>
#include <wx/encconv.h>
#include <wx/regex.h>
#include <wx/progdlg.h>
#include "wx/tokenzr.h"
#include "wx/dir.h"

#include "chartdbs.h"
#include "chartbase.h"
#include "pluginmanager.h"
#include "mygeom.h"                     // For DouglasPeucker();
#ifndef UINT32
#define UINT32 unsigned int
#endif

extern PlugInManager    *g_pi_manager;

int s_dbVersion;                                //    Database version currently in use at runtime
                                                //  Needed for ChartTableEntry::GetChartType() only
                                                //  TODO This can go away at opencpn Version 1.3.8 and above....
///////////////////////////////////////////////////////////////////////

bool FindMatchingFile(const wxString &theDir, const wxChar *theRegEx, int nameLength, wxString &theMatch) {
    wxDir dir(theDir);
    wxRegEx rePattern(theRegEx);
    for (bool fileFound = dir.GetFirst(&theMatch); fileFound; fileFound = dir.GetNext(&theMatch))
        if (theMatch.length() == (unsigned int)nameLength && rePattern.Matches(theMatch))
            return true;
    return false;
}


ChartFamilyEnum GetChartFamily(int charttype)
{
      ChartFamilyEnum cf;

      switch( charttype)
      {
            case        CHART_TYPE_KAP:      cf = CHART_FAMILY_RASTER; break;
            case        CHART_TYPE_GEO:      cf = CHART_FAMILY_RASTER; break;
            case        CHART_TYPE_S57:      cf = CHART_FAMILY_VECTOR; break;
            case        CHART_TYPE_CM93:     cf = CHART_FAMILY_VECTOR; break;
            case        CHART_TYPE_CM93COMP: cf = CHART_FAMILY_VECTOR; break;
            case        CHART_TYPE_DUMMY:    cf = CHART_FAMILY_RASTER; break;
            case        CHART_TYPE_UNKNOWN:  cf = CHART_FAMILY_UNKNOWN; break;
            default:                         cf = CHART_FAMILY_UNKNOWN; break;
      }
      return cf;
}


///////////////////////////////////////////////////////////////////////
// ChartTableHeader
///////////////////////////////////////////////////////////////////////

void ChartTableHeader::Read(wxInputStream &is)
{
    is.Read(this, sizeof(ChartTableHeader));
}

void ChartTableHeader::Write(wxOutputStream &os)
{
    char vb[5];
    sprintf(vb, "V%03d", DB_VERSION_CURRENT);

    memcpy(dbVersion, vb, 4);
    os.Write(this, sizeof(ChartTableHeader));
}

bool ChartTableHeader::CheckValid()
{
    char vb[5];
    sprintf(vb, "V%03d", DB_VERSION_CURRENT);
    if (strncmp(vb, dbVersion, sizeof(dbVersion)))
    {
          wxString msg;
          char vbo[5];
          memcpy(vbo, dbVersion, 4);
          vbo[4] = 0;
          msg.Append(wxString(vbo, wxConvUTF8));
          msg.Prepend(wxT("   Warning: found incorrect chart db version: "));
          wxLogMessage(msg);

//          return false;       // no match....


          // Try previous version....
          sprintf(vb, "V%03d", DB_VERSION_PREVIOUS);
          if (strncmp(vb, dbVersion, sizeof(dbVersion)))
                return false;
          else
          {
                wxLogMessage(_T("   Scheduling db upgrade to current db version on Options->Charts page visit..."));
                return true;
          }


    }
    else
    {
          wxString msg;
          char vbo[5];
          memcpy(vbo, dbVersion, 4);
          vbo[4] = 0;
          msg.Append(wxString(vbo, wxConvUTF8));
          msg.Prepend(wxT("Loading chart db version: "));
          wxLogMessage(msg);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////
// ChartTableEntry
///////////////////////////////////////////////////////////////////////

ChartTableEntry::ChartTableEntry(ChartBase &theChart)
{
    Clear();

    char *pt = (char *)malloc(strlen(theChart.GetFullPath().mb_str(wxConvUTF8)) + 1);
    strcpy(pt, theChart.GetFullPath().mb_str(wxConvUTF8));
    pFullPath = pt;



    ChartType = theChart.GetChartType();
    ChartFamily = theChart.GetChartFamily();
    Scale = theChart.GetNativeScale();

    Skew = theChart.GetChartSkew();
    ProjectionType = theChart.GetChartProjectionType();

    wxDateTime ed = theChart.GetEditionDate();
    if(theChart.GetEditionDate().IsValid())
          edition_date = theChart.GetEditionDate().GetTicks();

    wxFileName fn(theChart.GetFullPath());
    if(fn.GetModificationTime().IsValid())
          file_date = fn.GetModificationTime().GetTicks();

    m_pfilename = new wxString;           // create and populate helper members
    *m_pfilename = fn.GetFullName();
    m_psFullPath = new wxString;
    *m_psFullPath = theChart.GetFullPath();
    
    Extent ext;
    theChart.GetChartExtent(&ext);
    LatMax = ext.NLAT;
    LatMin = ext.SLAT;
    LonMin = ext.WLON;
    LonMax = ext.ELON;

    m_bbox.SetMax(LonMax, LatMax);
    m_bbox.SetMin(LonMin, LatMin);
    
    // Fill in the PLY information
    //  LOD calculation
    int LOD_pixels = 1;
    double scale_max_zoom = Scale / 4;
    
    double display_ppm = 1 / .00025;     // nominal for most LCD displays
    double meters_per_pixel_max_scale = scale_max_zoom / display_ppm;
    double LOD_meters = meters_per_pixel_max_scale * LOD_pixels;
    
//    double LOD_meters = 5;
    
    // If  COVR table has only one entry, us it for the primary Ply Table
    if (theChart.GetCOVREntries() == 1) {
          nPlyEntries = theChart.GetCOVRTablePoints(0);
          
          if(nPlyEntries > 5 && (LOD_meters > .01)){
              wxArrayInt index_keep;
              
              index_keep.Clear();
              index_keep.Add(0);
              index_keep.Add(nPlyEntries-1);
              index_keep.Add(1);
              index_keep.Add(nPlyEntries-2);

              double *DPbuffer = (double *)malloc(2 * nPlyEntries * sizeof(double));
              
              double *pfed = DPbuffer;
              Plypoint *ppp = (Plypoint *)theChart.GetCOVRTableHead(0);
              
              for (int i = 0; i < nPlyEntries; i++) {
                  *pfed++ = ppp->ltp;
                  *pfed++ = ppp->lnp;
                  ppp++;
              }
              
              DouglasPeucker(DPbuffer, 1, nPlyEntries-2, LOD_meters/(1852 * 60), &index_keep);
//              printf("DB DP Reduction: %d/%d\n", index_keep.GetCount(), nPlyEntries);

              // Mark the keepers by adding a simple constant to ltp
              for(unsigned int i=0 ; i < index_keep.GetCount() ; i++){
                  int k = index_keep.Item(i);
                  DPbuffer[2*k] += 2000.;
              }
              
              
              float *pf = (float *)malloc(2 * index_keep.GetCount() * sizeof(float));
              float *pfe = pf;
              
              for (int i = 0; i < nPlyEntries; i++) {
                  if(DPbuffer[2 * i] > 1000.){
                      *pfe++ = DPbuffer[2*i] - 2000.;
                      *pfe++ = DPbuffer[(2*i) + 1];
                  }
              }

              pPlyTable = pf;
              nPlyEntries = index_keep.GetCount();
              free( DPbuffer );
          }
          else {
            float *pf = (float *)malloc(2 * nPlyEntries * sizeof(float));
            pPlyTable = pf;
            float *pfe = pf;
            Plypoint *ppp = (Plypoint *)theChart.GetCOVRTableHead(0);

            for (int i = 0; i < nPlyEntries; i++) {
                *pfe++ = ppp->ltp;
                *pfe++ = ppp->lnp;
                ppp++;
            }
          }
    }
    // Else create a rectangular primary Ply Table from the chart extents
    // and create AuxPly table from the COVR tables
    else {
        // Create new artificial Ply table from chart extents
          nPlyEntries = 4;
          float *pf1 = (float *)malloc(2 * 4 * sizeof(float));
          pPlyTable = pf1;
          float *pfe = pf1;
          Extent fext;
          theChart.GetChartExtent(&fext);

          *pfe++ = fext.NLAT; //LatMax;
          *pfe++ = fext.WLON; //LonMin;

          *pfe++ = fext.NLAT; //LatMax;
          *pfe++ = fext.ELON; //LonMax;

          *pfe++ = fext.SLAT; //LatMin;
          *pfe++ = fext.ELON; //LonMax;

          *pfe++ = fext.SLAT; //LatMin;
          *pfe++ = fext.WLON; //LonMin;

        // Fill in the structure for pAuxPlyTable

          nAuxPlyEntries = theChart.GetCOVREntries();
          wxASSERT(nAuxPlyEntries);
          float **pfp = (float **)malloc(nAuxPlyEntries * sizeof(float *));
          float **pft0 = pfp;
          int *pip = (int *)malloc(nAuxPlyEntries * sizeof(int));

          for (int j = 0 ; j < nAuxPlyEntries; j++) {
              int nPE = theChart.GetCOVRTablePoints(j);
              
              if(nPE > 5 && (LOD_meters > .01)){
                  wxArrayInt index_keep;
                  
                  index_keep.Clear();
                  index_keep.Add(0);
                  index_keep.Add(nPE-1);
                  index_keep.Add(1);
                  index_keep.Add(nPE-2);
                  
                  double *DPbuffer = (double *)malloc(2 * nPE * sizeof(double));
                  
                  double *pfed = DPbuffer;
                  Plypoint *ppp = (Plypoint *)theChart.GetCOVRTableHead(j);
                  
                  for (int i = 0; i < nPE; i++) {
                      *pfed++ = ppp->ltp;
                      *pfed++ = ppp->lnp;
                      ppp++;
                  }
                  
                  DouglasPeucker(DPbuffer, 1, nPE-2, LOD_meters/(1852 * 60), &index_keep);
 //                 printf("DBa DP Reduction: %d/%d\n", index_keep.GetCount(), nPE);
                  
                  // Mark the keepers by adding a simple constant to ltp
                  for(unsigned int i=0 ; i < index_keep.GetCount() ; i++){
                      int k = index_keep.Item(i);
                      DPbuffer[2*k] += 2000.;
                  }
                  
                  
                  float *pf = (float *)malloc(2 * index_keep.GetCount() * sizeof(float));
                  float *pfe = pf;
                  
                  for (int i = 0; i < nPE; i++) {
                      if(DPbuffer[2 * i] > 1000.){
                          *pfe++ = DPbuffer[2*i] - 2000.;
                          *pfe++ = DPbuffer[(2*i) + 1];
                      }
                  }
                  
                  pft0[j] = pf;
                  pip[j] = index_keep.GetCount();
                  free( DPbuffer );
              }
              else {
                float *pf_entry = (float *)malloc(theChart.GetCOVRTablePoints(j) * 2 * sizeof(float));
                memcpy(pf_entry, theChart.GetCOVRTableHead(j), theChart.GetCOVRTablePoints(j) * 2 * sizeof(float));
                pft0[j] = pf_entry;
                pip[j] = theChart.GetCOVRTablePoints(j);
              }
          }

          pAuxPlyTable = pfp;
          pAuxCntTable = pip;

    }
    
    //  Get and populate the NoCovr tables
    
    nNoCovrPlyEntries = theChart.GetNoCOVREntries();
    float **pfpnc = (float **)malloc(nNoCovrPlyEntries * sizeof(float *));
    float **pft0nc = pfpnc;
    int *pipnc = (int *)malloc(nNoCovrPlyEntries * sizeof(int));
    
    for (int j = 0 ; j < nNoCovrPlyEntries; j++) {
        float *pf_entry = (float *)malloc(theChart.GetNoCOVRTablePoints(j) * 2 * sizeof(float));
        memcpy(pf_entry, theChart.GetNoCOVRTableHead(j), theChart.GetNoCOVRTablePoints(j) * 2 * sizeof(float));
        pft0nc[j] = pf_entry;
        pipnc[j] = theChart.GetNoCOVRTablePoints(j);
    }

    pNoCovrPlyTable = pfpnc;
    pNoCovrCntTable = pipnc;
    
}

///////////////////////////////////////////////////////////////////////

ChartTableEntry::~ChartTableEntry()
{
    free(pFullPath);
    free(pPlyTable);
    
    for (int i = 0; i < nAuxPlyEntries; i++)
        free(pAuxPlyTable[i]);
    free(pAuxPlyTable);
    free(pAuxCntTable);

    if (nNoCovrPlyEntries) {
        for (int i = 0; i < nNoCovrPlyEntries; i++) 
            free( pNoCovrPlyTable[i] );
        free( pNoCovrPlyTable );
        free( pNoCovrCntTable );
    }
    
    delete m_pfilename;
    delete m_psFullPath;
}

///////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////

bool ChartTableEntry::IsEarlierThan(const ChartTableEntry &cte) const {
    wxDateTime mine(edition_date);
    wxDateTime theirs(cte.edition_date);
    return ( mine.IsEarlierThan(theirs) );
}

bool ChartTableEntry::IsEqualTo(const ChartTableEntry &cte) const {
      wxDateTime mine(edition_date);
      wxDateTime theirs(cte.edition_date);
      return ( mine.IsEqualTo(theirs) );
}

///////////////////////////////////////////////////////////////////////

int ChartTableEntry::GetChartType() const
{
      //    Hackeroo here....
      //    dB version 14 had different ChartType Enum, patch it here
      if(s_dbVersion == 14)
      {
            switch(ChartType)
            {
                  case 0: return CHART_TYPE_KAP;
                  case 1: return CHART_TYPE_GEO;
                  case 2: return CHART_TYPE_S57;
                  case 3: return CHART_TYPE_CM93;
                  case 4: return CHART_TYPE_CM93COMP;
                  case 5: return CHART_TYPE_UNKNOWN;
                  case 6: return CHART_TYPE_DONTCARE;
                  case 7: return CHART_TYPE_DUMMY;
                  default: return CHART_TYPE_UNKNOWN;
            }
      }
      else
       return ChartType;
}

int ChartTableEntry::GetChartFamily() const
{
    if(s_dbVersion < 18)
    {
        switch(ChartType)
        {
            case CHART_TYPE_KAP:
            case CHART_TYPE_GEO:
                  return CHART_FAMILY_RASTER;

            case CHART_TYPE_S57:
            case CHART_TYPE_CM93:
            case CHART_TYPE_CM93COMP:
                  return CHART_FAMILY_VECTOR;

            default:
                  return CHART_FAMILY_UNKNOWN;
      }
    }
    else
        return ChartFamily;
}




bool ChartTableEntry::Read(const ChartDatabase *pDb, wxInputStream &is)
{
    char path[4096], *cp;

    Clear();

    //      Allow reading of current db format, and maybe others
    ChartDatabase *pD = (ChartDatabase *)pDb;
    int db_version = pD->GetVersion();

    if(db_version == 18)
    {
        // Read the path first
        for (cp = path; (*cp = (char)is.GetC()) != 0; cp++);
        pFullPath = (char *)malloc(cp - path + 1);
        strncpy(pFullPath, path, cp - path + 1);
        wxLogVerbose(_T("  Chart %s"), pFullPath);
        
        //  Create and populate the helper members
        m_pfilename = new wxString;
        wxString fullfilename(pFullPath, wxConvUTF8);
        wxFileName fn(fullfilename);
        *m_pfilename = fn.GetFullName();
        m_psFullPath = new wxString;
        *m_psFullPath = fullfilename;
        
        // Read the table entry
        ChartTableEntry_onDisk_18 cte;
        is.Read(&cte, sizeof(ChartTableEntry_onDisk_18));
        
        //    Transcribe the elements....
        EntryOffset = cte.EntryOffset;
        ChartType = cte.ChartType;
        ChartFamily = cte.ChartFamily;
        LatMax = cte.LatMax;
        LatMin = cte.LatMin;
        LonMax = cte.LonMax;
        LonMin = cte.LonMin;

        m_bbox.SetMax(LonMax, LatMax);
        m_bbox.SetMin(LonMin, LatMin);
        
        Skew = cte.skew;
        ProjectionType = cte.ProjectionType;
        
        Scale = cte.Scale;
        edition_date = cte.edition_date;
        file_date = cte.file_date;
        
        nPlyEntries = cte.nPlyEntries;
        nAuxPlyEntries = cte.nAuxPlyEntries;
        
        nNoCovrPlyEntries = cte.nNoCovrPlyEntries;
        
        bValid = cte.bValid;
        
        if (nPlyEntries) {
            int npeSize = nPlyEntries * 2 * sizeof(float);
            pPlyTable = (float *)malloc(npeSize);
            is.Read(pPlyTable, npeSize);
        }
        
        if (nAuxPlyEntries) {
            int napeSize = nAuxPlyEntries * sizeof(int);
            pAuxPlyTable = (float **)malloc(nAuxPlyEntries * sizeof(float *));
            pAuxCntTable = (int *)malloc(napeSize);
            is.Read(pAuxCntTable, napeSize);
            
            for (int nAuxPlyEntry = 0; nAuxPlyEntry < nAuxPlyEntries; nAuxPlyEntry++) {
                int nfSize = pAuxCntTable[nAuxPlyEntry] * 2 * sizeof(float);
                pAuxPlyTable[nAuxPlyEntry] = (float *)malloc(nfSize);
                is.Read(pAuxPlyTable[nAuxPlyEntry], nfSize);
            }
        }
        
        if (nNoCovrPlyEntries) {
            int napeSize = nNoCovrPlyEntries * sizeof(int);
            pNoCovrCntTable = (int *)malloc(napeSize);
            is.Read(pNoCovrCntTable, napeSize);
            
            pNoCovrPlyTable = (float **)malloc(nNoCovrPlyEntries * sizeof(float *));
            for (int i = 0; i < nNoCovrPlyEntries; i++) {
                int nfSize = pNoCovrCntTable[i] * 2 * sizeof(float);
                pNoCovrPlyTable[i] = (float *)malloc(nfSize);
                is.Read(pNoCovrPlyTable[i], nfSize);
            }
        }
    }
    
    else if(db_version == 17)
    {
        // Read the path first
        for (cp = path; (*cp = (char)is.GetC()) != 0; cp++);
        pFullPath = (char *)malloc(cp - path + 1);
        strncpy(pFullPath, path, cp - path + 1);
        wxLogVerbose(_T("  Chart %s"), pFullPath);
        
        //  Create and populate the helper members
        m_pfilename = new wxString;
        wxString fullfilename(pFullPath, wxConvUTF8);
        wxFileName fn(fullfilename);
        *m_pfilename = fn.GetFullName();
        m_psFullPath = new wxString;
        *m_psFullPath = fullfilename;
        
        // Read the table entry
        ChartTableEntry_onDisk_17 cte;
        is.Read(&cte, sizeof(ChartTableEntry_onDisk_17));
        
        //    Transcribe the elements....
        EntryOffset = cte.EntryOffset;
        ChartType = cte.ChartType;
        LatMax = cte.LatMax;
        LatMin = cte.LatMin;
        LonMax = cte.LonMax;
        LonMin = cte.LonMin;

        m_bbox.SetMax(LonMax, LatMax);
        m_bbox.SetMin(LonMin, LatMin);
        
        Skew = cte.skew;
        ProjectionType = cte.ProjectionType;
        
        Scale = cte.Scale;
        edition_date = cte.edition_date;
        file_date = cte.file_date;
        
        nPlyEntries = cte.nPlyEntries;
        nAuxPlyEntries = cte.nAuxPlyEntries;
        
        nNoCovrPlyEntries = cte.nNoCovrPlyEntries;
        
        bValid = cte.bValid;
        
        if (nPlyEntries) {
            int npeSize = nPlyEntries * 2 * sizeof(float);
            pPlyTable = (float *)malloc(npeSize);
            is.Read(pPlyTable, npeSize);
        }
 
        if (nAuxPlyEntries) {
            int napeSize = nAuxPlyEntries * sizeof(int);
            pAuxPlyTable = (float **)malloc(nAuxPlyEntries * sizeof(float *));
            pAuxCntTable = (int *)malloc(napeSize);
            is.Read(pAuxCntTable, napeSize);
            
            for (int nAuxPlyEntry = 0; nAuxPlyEntry < nAuxPlyEntries; nAuxPlyEntry++) {
                int nfSize = pAuxCntTable[nAuxPlyEntry] * 2 * sizeof(float);
                pAuxPlyTable[nAuxPlyEntry] = (float *)malloc(nfSize);
                is.Read(pAuxPlyTable[nAuxPlyEntry], nfSize);
            }
        }

        if (nNoCovrPlyEntries) {
            int napeSize = nNoCovrPlyEntries * sizeof(int);
            pNoCovrCntTable = (int *)malloc(napeSize);
            is.Read(pNoCovrCntTable, napeSize);
            
            pNoCovrPlyTable = (float **)malloc(nNoCovrPlyEntries * sizeof(float *));
            for (int i = 0; i < nNoCovrPlyEntries; i++) {
                int nfSize = pNoCovrCntTable[i] * 2 * sizeof(float);
                pNoCovrPlyTable[i] = (float *)malloc(nfSize);
                is.Read(pNoCovrPlyTable[i], nfSize);
            }
        }
    }
    
    else if(db_version == 16)
    {
      // Read the path first
          for (cp = path; (*cp = (char)is.GetC()) != 0; cp++);
     // TODO: optimize prepended dir
          pFullPath = (char *)malloc(cp - path + 1);
          strncpy(pFullPath, path, cp - path + 1);
          wxLogVerbose(_T("  Chart %s"), pFullPath);

     //  Create and populate the helper members
          m_pfilename = new wxString;
          wxString fullfilename(pFullPath, wxConvUTF8);
          wxFileName fn(fullfilename);
          *m_pfilename = fn.GetFullName();
          m_psFullPath = new wxString;
          *m_psFullPath = fullfilename;
          
      // Read the table entry
          ChartTableEntry_onDisk_16 cte;
          is.Read(&cte, sizeof(ChartTableEntry_onDisk_16));

      //    Transcribe the elements....
          EntryOffset = cte.EntryOffset;
          ChartType = cte.ChartType;
          LatMax = cte.LatMax;
          LatMin = cte.LatMin;
          LonMax = cte.LonMax;
          LonMin = cte.LonMin;

          m_bbox.SetMax(LonMax, LatMax);
          m_bbox.SetMin(LonMin, LatMin);
          
          Skew = cte.skew;
          ProjectionType = cte.ProjectionType;

          Scale = cte.Scale;
          edition_date = cte.edition_date;
          file_date = cte.file_date;

          nPlyEntries = cte.nPlyEntries;
          nAuxPlyEntries = cte.nAuxPlyEntries;

          bValid = cte.bValid;

          if (nPlyEntries) {
                int npeSize = nPlyEntries * 2 * sizeof(float);
                pPlyTable = (float *)malloc(npeSize);
                is.Read(pPlyTable, npeSize);
          }

          if (nAuxPlyEntries) {
                int napeSize = nAuxPlyEntries * sizeof(int);
                pAuxPlyTable = (float **)malloc(nAuxPlyEntries * sizeof(float *));
                pAuxCntTable = (int *)malloc(napeSize);
                is.Read(pAuxCntTable, napeSize);

                for (int nAuxPlyEntry = 0; nAuxPlyEntry < nAuxPlyEntries; nAuxPlyEntry++) {
                      int nfSize = pAuxCntTable[nAuxPlyEntry] * 2 * sizeof(float);
                      pAuxPlyTable[nAuxPlyEntry] = (float *)malloc(nfSize);
                      is.Read(pAuxPlyTable[nAuxPlyEntry], nfSize);
                }
          }
    }
    
    else if(db_version == 15)
    {
      // Read the path first
      for (cp = path; (*cp = (char)is.GetC()) != 0; cp++);
     // TODO: optimize prepended dir
      pFullPath = (char *)malloc(cp - path + 1);
      strncpy(pFullPath, path, cp - path + 1);
      wxLogVerbose(_T("  Chart %s"), pFullPath);

      // Read the table entry
      ChartTableEntry_onDisk_15 cte;
      is.Read(&cte, sizeof(ChartTableEntry_onDisk_15));

      //    Transcribe the elements....
      EntryOffset = cte.EntryOffset;
      ChartType = cte.ChartType;
      LatMax = cte.LatMax;
      LatMin = cte.LatMin;
      LonMax = cte.LonMax;
      LonMin = cte.LonMin;

      m_bbox.SetMax(LonMax, LatMax);
      m_bbox.SetMin(LonMin, LatMin);
      
      Scale = cte.Scale;
      edition_date = cte.edition_date;
      file_date = cte.file_date;

      nPlyEntries = cte.nPlyEntries;
      nAuxPlyEntries = cte.nAuxPlyEntries;

      bValid = cte.bValid;

      if (nPlyEntries) {
            int npeSize = nPlyEntries * 2 * sizeof(float);
            pPlyTable = (float *)malloc(npeSize);
            is.Read(pPlyTable, npeSize);
      }

      if (nAuxPlyEntries) {
            int napeSize = nAuxPlyEntries * sizeof(int);
            pAuxPlyTable = (float **)malloc(nAuxPlyEntries * sizeof(float *));
            pAuxCntTable = (int *)malloc(napeSize);
            is.Read(pAuxCntTable, napeSize);

            for (int nAuxPlyEntry = 0; nAuxPlyEntry < nAuxPlyEntries; nAuxPlyEntry++) {
                  int nfSize = pAuxCntTable[nAuxPlyEntry] * 2 * sizeof(float);
                  pAuxPlyTable[nAuxPlyEntry] = (float *)malloc(nfSize);
                  is.Read(pAuxPlyTable[nAuxPlyEntry], nfSize);
            }
      }
    }
    else if(db_version == 14)
    {
     // Read the path first
          for (cp = path; (*cp = (char)is.GetC()) != 0; cp++);
          pFullPath = (char *)malloc(cp - path + 1);
          strncpy(pFullPath, path, cp - path + 1);
          wxLogVerbose(_T("  Chart %s"), pFullPath);

      // Read the table entry
          ChartTableEntry_onDisk_14 cte;
          is.Read(&cte, sizeof(ChartTableEntry_onDisk_14));

      //    Transcribe the elements....
          EntryOffset = cte.EntryOffset;
          ChartType = cte.ChartType;
          LatMax = cte.LatMax;
          LatMin = cte.LatMin;
          LonMax = cte.LonMax;
          LonMin = cte.LonMin;
          m_bbox.SetMax(LonMax, LatMax);
          m_bbox.SetMin(LonMin, LatMin);
          
          Scale = cte.Scale;
          edition_date = cte.edition_date;
          file_date = 0;                        //  file_date does not exist in V14;
          nPlyEntries = cte.nPlyEntries;
          nAuxPlyEntries = cte.nAuxPlyEntries;
          bValid = cte.bValid;

          if (nPlyEntries) {
                int npeSize = nPlyEntries * 2 * sizeof(float);
                pPlyTable = (float *)malloc(npeSize);
                is.Read(pPlyTable, npeSize);
          }

          if (nAuxPlyEntries) {
                int napeSize = nAuxPlyEntries * sizeof(int);
                pAuxPlyTable = (float **)malloc(nAuxPlyEntries * sizeof(float *));
                pAuxCntTable = (int *)malloc(napeSize);
                is.Read(pAuxCntTable, napeSize);

                for (int nAuxPlyEntry = 0; nAuxPlyEntry < nAuxPlyEntries; nAuxPlyEntry++) {
                      int nfSize = pAuxCntTable[nAuxPlyEntry] * 2 * sizeof(float);
                      pAuxPlyTable[nAuxPlyEntry] = (float *)malloc(nfSize);
                      is.Read(pAuxPlyTable[nAuxPlyEntry], nfSize);
                }
          }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////

bool ChartTableEntry::Write(const ChartDatabase *pDb, wxOutputStream &os)
{
    os.Write(pFullPath, strlen(pFullPath) + 1);

    //      Write the current version type only
    //      Create an on_disk table entry
    ChartTableEntry_onDisk_18 cte;

      //    Transcribe the elements....
    cte.EntryOffset = EntryOffset;
    cte.ChartType = ChartType;
    cte.ChartFamily = ChartFamily;
    cte.LatMax = LatMax;
    cte.LatMin = LatMin;
    cte.LonMax = LonMax;
    cte.LonMin = LonMin;

    cte.Scale = Scale;
    cte.edition_date = edition_date;
    cte.file_date = file_date;

    cte.nPlyEntries = nPlyEntries;
    cte.nAuxPlyEntries = nAuxPlyEntries;

    cte.skew = Skew;
    cte.ProjectionType = ProjectionType;

    cte.bValid = bValid;

    cte.nNoCovrPlyEntries = nNoCovrPlyEntries;
    
    os.Write(&cte, sizeof(ChartTableEntry_onDisk_18));
    wxLogVerbose(_T("  Wrote Chart %s"), pFullPath);

    //      Write out the tables
    if (nPlyEntries) {
        int npeSize = nPlyEntries * 2 * sizeof(float);
        os.Write(pPlyTable, npeSize);
    }
        
    if (nAuxPlyEntries) {
        int napeSize = nAuxPlyEntries * sizeof(int);
        os.Write(pAuxCntTable, napeSize);

        for (int nAuxPlyEntry = 0; nAuxPlyEntry < nAuxPlyEntries; nAuxPlyEntry++) {
            int nfSize = pAuxCntTable[nAuxPlyEntry] * 2 * sizeof(float);
            os.Write(pAuxPlyTable[nAuxPlyEntry], nfSize);
        }
    }
    
    if (nNoCovrPlyEntries ) {
        int ncSize = nNoCovrPlyEntries * sizeof(int);
        os.Write(pNoCovrCntTable, ncSize);
        
        for (int i = 0; i < nNoCovrPlyEntries; i++) {
            int nctSize = pNoCovrCntTable[i] * 2 * sizeof(float);
            os.Write(pNoCovrPlyTable[i], nctSize);
        }
    }
    

    return true;
}

///////////////////////////////////////////////////////////////////////

void ChartTableEntry::Clear()
{
//    memset(this, 0, sizeof(ChartTableEntry));
    
    pFullPath = NULL;
    pPlyTable = NULL;
    pAuxPlyTable = NULL;
    pAuxCntTable = NULL;
    bValid = false;;
    pNoCovrCntTable = NULL;
    pNoCovrPlyTable = NULL;
    
    nNoCovrPlyEntries = 0;
    nAuxPlyEntries = 0;
    
    m_pfilename = NULL;             // a helper member, not on disk
    m_psFullPath = NULL;
    
}

///////////////////////////////////////////////////////////////////////

void ChartTableEntry::Disable()
{
    // Mark this chart in the database, so that it will not be seen during this run
    // How?  By setting the chart bounding box to an absurd value
    // TODO... Fix this heinous hack
    LatMax = (float) 100.;
    LatMin = (float)91.;
}

///////////////////////////////////////////////////////////////////////
// ChartDatabase
///////////////////////////////////////////////////////////////////////

WX_DEFINE_OBJARRAY(ChartTable);
WX_DEFINE_OBJARRAY(ArrayOfChartClassDescriptor);

ChartDatabase::ChartDatabase()
{
      m_ChartTableEntryDummy.Clear();

      UpdateChartClassDescriptorArray();
}

void ChartDatabase::UpdateChartClassDescriptorArray(void)
{
      m_ChartClassDescriptorArray.Clear();

            //    Create and add the descriptors for the default chart types recognized
      ChartClassDescriptor *pcd;

      pcd = new ChartClassDescriptor(_T("ChartKAP"), _T("*.kap"), BUILTIN_DESCRIPTOR);
      m_ChartClassDescriptorArray.Add(pcd);
      pcd = new ChartClassDescriptor(_T("ChartGEO"), _T("*.geo"), BUILTIN_DESCRIPTOR);
      m_ChartClassDescriptorArray.Add(pcd);
      pcd = new ChartClassDescriptor(_T("s57chart"), _T("*.000"), BUILTIN_DESCRIPTOR);
      m_ChartClassDescriptorArray.Add(pcd);
      pcd = new ChartClassDescriptor(_T("s57chart"), _T("*.s57"), BUILTIN_DESCRIPTOR);
      m_ChartClassDescriptorArray.Add(pcd);
      pcd = new ChartClassDescriptor(_T("cm93compchart"), _T("00300000.a"), BUILTIN_DESCRIPTOR);
      m_ChartClassDescriptorArray.Add(pcd);

      //    If the PlugIn Manager exists, get the array of dynamically loadable chart class names
      if(g_pi_manager)
      {
            wxArrayString array = g_pi_manager->GetPlugInChartClassNameArray();
            for(unsigned int j = 0 ; j < array.GetCount() ; j++)
            {
                  //    Instantiate a blank chart to retrieve the directory search mask for this chart type
                  wxString class_name = array.Item(j);
                  ChartPlugInWrapper *cpiw = new ChartPlugInWrapper(class_name);
                  if(cpiw)
                  {
                        wxString mask = cpiw->GetFileSearchMask();

            //    Create a new descriptor
                        ChartClassDescriptor *picd = new ChartClassDescriptor(class_name, mask, PLUGIN_DESCRIPTOR);

            //    Add descriptor to the database array member
                        m_ChartClassDescriptorArray.Add(picd);

                        delete cpiw;
                  }
            }
      }

}


const ChartTableEntry &ChartDatabase::GetChartTableEntry(int index) const
{
      if(index < m_nentries)
          return active_chartTable[index];
      else
            return m_ChartTableEntryDummy;
}

ChartTableEntry *ChartDatabase::GetpChartTableEntry(int index) const
{
    if(index < m_nentries)
          return &active_chartTable[index];
      else
            return (ChartTableEntry *)&m_ChartTableEntryDummy;
}

bool ChartDatabase::CompareChartDirArray( ArrayOfCDI& test_array )
{
    //  Compare the parameter "test_array" with this.m_dir_array
    //    Return true if functionally identical (order does not signify).
    
    if(test_array.GetCount() != m_dir_array.GetCount())
        return false;
    
    bool bfound_inner;
    unsigned int nfound_outer = 0;
    
    for(unsigned int i = 0 ; i < test_array.GetCount() ; i++){
        ChartDirInfo p = test_array.Item(i);
        bfound_inner = false;
        for(unsigned int j = 0 ; j < m_dir_array.GetCount() ; j++){
            ChartDirInfo q = m_dir_array.Item(j);
            
            if(p.fullpath.IsSameAs(q.fullpath)){
                bfound_inner = true;
                break;
            }
        }
        if(bfound_inner)
            nfound_outer++;
    }
    
    return (nfound_outer == test_array.GetCount());
    
}

wxString ChartDatabase::GetMagicNumberCached(wxString dir)
{
    for(unsigned int j = 0 ; j < m_dir_array.GetCount() ; j++){
        ChartDirInfo q = m_dir_array.Item(j);
        if(dir.IsSameAs(q.fullpath))
            return q.magic_number;
    }
    
    return _T("");
            
}



bool ChartDatabase::Read(const wxString &filePath)
{
    ChartTableEntry entry;
    int entries;

    bValid = false;

    wxFileName file(filePath);
    if (!file.FileExists()) return false;

    m_DBFileName = filePath;

    wxFFileInputStream ifs(filePath);
    if(!ifs.Ok()) return false;

    ChartTableHeader cth;
    cth.Read(ifs);
    if (!cth.CheckValid()) return false;

    //      Capture the version number
    char vbo[5];
    memcpy(vbo, cth.GetDBVersionString(), 4);
    vbo[4] = 0;
    m_dbversion = atoi(&vbo[1]);
    s_dbVersion = m_dbversion;                  // save the static copy

    wxLogVerbose(wxT("Chartdb:Reading %d directory entries, %d table entries"), cth.GetDirEntries(), cth.GetTableEntries());
    wxLogMessage(_T("Chartdb: Chart directory list follows"));
    if(0 == cth.GetDirEntries())
          wxLogMessage(_T("  Nil"));

    for (int iDir = 0; iDir < cth.GetDirEntries(); iDir++) {
        wxString dir;
        int dirlen;
        ifs.Read(&dirlen, sizeof(int));
        while (dirlen > 0) {
            char dirbuf[1024];
            int alen = dirlen > 1023 ? 1023 : dirlen;
            if(ifs.Read(&dirbuf, alen).Eof())
                goto read_error;
            dirbuf[alen] = 0;
            dirlen -= alen;
            dir.Append(wxString(dirbuf, wxConvUTF8));
        }
        wxString msg;
        msg.Printf(wxT("  Chart directory #%d: "), iDir);
        msg.Append(dir);
        wxLogMessage(msg);
        m_chartDirs.Add(dir);
    }

    entries = cth.GetTableEntries();
    active_chartTable.Alloc(entries);
    while (entries-- && entry.Read(this, ifs))
        active_chartTable.Add(entry);

    entry.Clear();
    bValid = true;
    entry.SetAvailable(true);
    
    m_nentries = active_chartTable.GetCount();
    return true;

read_error:
    bValid = false;
    m_nentries = active_chartTable.GetCount();
    return false;
}

///////////////////////////////////////////////////////////////////////

bool ChartDatabase::Write(const wxString &filePath)
{
    wxFileName file(filePath);
    wxFileName dir(file.GetPath(wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME, wxPATH_NATIVE));

    if (!dir.DirExists() && !dir.Mkdir()) return false;

    wxFFileOutputStream ofs(filePath);
    if(!ofs.Ok()) return false;

    ChartTableHeader cth(m_chartDirs.GetCount(), active_chartTable.GetCount());
    cth.Write(ofs);

    for (int iDir = 0; iDir < cth.GetDirEntries(); iDir++) {
        wxString &dir = m_chartDirs[iDir];
        int dirlen = dir.length();
        char s[200];
        strncpy(s, dir.mb_str(wxConvUTF8), 199);
        s[199] = 0;
        dirlen = strlen(s);
        ofs.Write(&dirlen, sizeof(int));
//        ofs.Write(dir.fn_str(), dirlen);
        ofs.Write(s, dirlen);
    }

    for (UINT32 iTable = 0; iTable < active_chartTable.size(); iTable++)
        active_chartTable[iTable].Write(this, ofs);

    //      Explicitly set the version
    m_dbversion = DB_VERSION_CURRENT;

    return true;
}

///////////////////////////////////////////////////////////////////////
wxString SplitPath(wxString s, wxString tkd, int nchar, int offset, int *pn_split)
{
      wxString r;
      int ncr = 0;

      int rlen = offset;
      wxStringTokenizer tkz(s, tkd);
      while ( tkz.HasMoreTokens() )
      {
            wxString token = tkz.GetNextToken();
            if((rlen + (int)token.Len() + 1) < nchar)
            {
                  r += token;
                  r += tkd[0];
                  rlen += token.Len() + 1;
            }
            else
            {
                  r += _T("\n");
                  ncr ++;
                  for(int i=0 ; i< offset ; i++){ r += _T(" "); }
                  r += token;
                  r += tkd[0];
                  rlen = offset + token.Len() + 1;
            }
      }

      if(pn_split)
            *pn_split = ncr;

      return r.Mid(0, r.Len()-1);             // strip the last separator char


}


wxString ChartDatabase::GetFullChartInfo(ChartBase *pc, int dbIndex, int *char_width, int *line_count)
{
      wxString r;
      int lc = 0;
      unsigned int max_width = 0;
      int ncr;
      unsigned int target_width = 60;

      const ChartTableEntry &cte = GetChartTableEntry(dbIndex);
      if(1)       //TODO why can't this be cte.GetbValid()?
      {
            wxString line;
            line = _(" ChartFile:  ");
            wxString longline(cte.GetpFullPath(), wxConvUTF8);
/*
            if(CHART_TYPE_CM93COMP == cte.GetChartType())
            {
                  //    Remove filename and two trailing directories
                  //    to make it pretty....
                  wxString path;
                  const wxString full_path(cte.GetpFullPath(), wxConvUTF8);
                  wxFileName::SplitPath(full_path, &path, NULL, NULL);

                  wxFileName fn(path);
                  fn.RemoveLastDir();
                  longline = fn.GetPath();
                  longline += _T("...");
            }
*/
            if(longline.Len() > target_width)
            {
                  line += SplitPath(wxString(cte.GetpFullPath(), wxConvUTF8), _T("/,\\"), target_width, 15, &ncr);
                  max_width = wxMax(max_width, target_width+4);
                  lc += ncr;
            }
            else
            {
                  line += longline;
                  max_width = wxMax(max_width, line.Len()+4);
            }


            r += line;
            r += _T("\n");
            lc++;

            line.Empty();
            if(pc)
            {
                  line = _(" Name:  ");
                  wxString longline = pc->GetName();

                  wxString tkz;
                  if(longline.Find(' ') != wxNOT_FOUND)     // assume a proper name
                        tkz = _T(" ");
                  else
                        tkz = _T("/,\\");                   // else a file name

                  if(longline.Len() > target_width)
                  {
                        line += SplitPath(pc->GetName(), tkz, target_width, 12, &ncr);
                        max_width = wxMax(max_width, target_width+4);
                        lc += ncr;
                  }
                  else
                  {
                        line += longline;
                        max_width = wxMax(max_width, line.Len()+4);
                  }

            }

            line += _T("\n");
            r += line;
            lc++;

            if(pc)      // chart is loaded and available
                  line.Printf(_(" Scale:  1:%d"), pc->GetNativeScale() );
            else
                  line.Printf(_(" Scale:  1:%d"), cte.GetScale() );

            line += _T("\n");
            max_width = wxMax(max_width, line.Len());
            r += line;
            lc++;

            if(pc)
            {
                  line.Empty();
                  line = _(" ID:  ");
                  line += pc->GetID();
                  line += _T("\n");
                  max_width = wxMax(max_width, line.Len());
                  r += line;
                  lc++;

                  line.Empty();
                  line = _(" Depth Units:  ");
                  line += pc->GetDepthUnits();
                  line += _T("\n");
                  max_width = wxMax(max_width, line.Len());
                  r += line;
                  lc++;

                  line.Empty();
                  line = _(" Soundings:  ");
                  line += pc->GetSoundingsDatum();
                  line += _T("\n");
                  max_width = wxMax(max_width, line.Len());
                  r += line;
                  lc++;

                  line.Empty();
                  line = _(" Datum:  ");
                  line += pc->GetDatumString();
                  line += _T("\n");
                  max_width = wxMax(max_width, line.Len());
                  r += line;
                  lc++;
            }

            line = _(" Projection:  ");
            if(PROJECTION_UNKNOWN == cte.GetChartProjectionType())
                  line += _("Unknown");
            else if(PROJECTION_MERCATOR == cte.GetChartProjectionType())
                  line += _("Mercator");
            else if(PROJECTION_TRANSVERSE_MERCATOR == cte.GetChartProjectionType())
                  line += _("Transverse Mercator");
            else if(PROJECTION_POLYCONIC == cte.GetChartProjectionType())
                  line += _("Polyconic");
            line += _T("\n");
            max_width = wxMax(max_width, line.Len());
            r += line;
            lc++;

            line.Empty();
            if(pc)
            {
                  line = _(" Source Edition:  ");
                  line += pc->GetSE();
                  line += _T("\n");
                  max_width = wxMax(max_width, line.Len());
                  r += line;
                  lc++;

                  line.Empty();
                  line = _(" Updated:  ");
                  wxDateTime ed = pc->GetEditionDate();
                  line += ed.FormatISODate();
                  line += _T("\n");
                  max_width = wxMax(max_width, line.Len());
                  r += line;
                  lc++;
            }
            
            line.Empty();
            if(pc && pc->GetExtraInfo().Len())
            {
                  line += pc->GetExtraInfo();
                  line += _T("\n");
                  max_width = wxMax(max_width, line.Len());
                  r += line;
                  lc++;

            }
      }

      if(line_count)
            *line_count = lc;

      if(char_width)
            *char_width = max_width;

      return r;
}

// ----------------------------------------------------------------------------
// Create Chart Table Database by directory search
//    resulting in valid pChartTable in (this)
// ----------------------------------------------------------------------------
bool ChartDatabase::Create(ArrayOfCDI &dir_array, wxProgressDialog *pprog)
{
      m_dir_array = dir_array;

      bValid = false;

      m_chartDirs.Clear();
      active_chartTable.Clear();
      Update(dir_array, true, pprog);                   // force the update the reload everything

      bValid = true;

      //      Explicitly set the version
      m_dbversion = DB_VERSION_CURRENT;

      return true;
}





// ----------------------------------------------------------------------------
// Update existing ChartTable Database by directory search
//    resulting in valid pChartTable in (this)
// ----------------------------------------------------------------------------
bool ChartDatabase::Update(ArrayOfCDI& dir_array, bool bForce, wxProgressDialog *pprog)
{
      m_dir_array = dir_array;

      bValid = false;               // database is not useable right now...

      //  Mark all charts provisionally invalid
      for(unsigned int i=0 ; i<active_chartTable.GetCount() ; i++)
          active_chartTable[i].SetValid(false);

      m_chartDirs.Clear();

      if(bForce)
          active_chartTable.Clear();

      bool lbForce = bForce;

      //    Do a dB Version upgrade if the current one is obsolete
      if(s_dbVersion != DB_VERSION_CURRENT)
      {

          active_chartTable.Clear();
            lbForce = true;
            s_dbVersion = DB_VERSION_CURRENT;         // Update the static indicator
            m_dbversion = DB_VERSION_CURRENT;         // and the member

      }

    //  Get the new charts

      for(unsigned int j=0 ; j<dir_array.GetCount() ; j++)
      {
            ChartDirInfo dir_info = dir_array.Item(j);

            wxString dir_magic;
            TraverseDirAndAddCharts(dir_info, pprog, dir_magic, lbForce);

        //  Update the dir_list entry, even if the magic values are the same

            dir_info.magic_number = dir_magic;
            dir_array.RemoveAt(j);
            dir_array.Insert(dir_info, j);

            m_chartDirs.Add(dir_info.fullpath);
      }           //for


      for(unsigned int i=0 ; i<active_chartTable.GetCount() ; i++)
      {
          if(!active_chartTable[i].GetbValid())
            {
                active_chartTable.RemoveAt(i);
                  i--;                 // entry is gone, recheck this index for next entry
            }
      }

      //    And once more, setting the Entry index field
      for(unsigned int i=0 ; i<active_chartTable.GetCount() ; i++)
          active_chartTable[i].SetEntryOffset( i );


      m_nentries = active_chartTable.GetCount();
      
      bValid = true;
      return true;
}

//-------------------------------------------------------------------
//    Find Chart dbIndex
//-------------------------------------------------------------------

int ChartDatabase::FinddbIndex(wxString PathToFind)
{
      //    Find the chart
      for(unsigned int i=0 ; i<active_chartTable.GetCount() ; i++)
      {
          if(active_chartTable[i].GetpsFullPath()->IsSameAs(PathToFind))
            {
                  return i;
            }
      }

      return -1;
}



//-------------------------------------------------------------------
//    Disable Chart
//-------------------------------------------------------------------

int ChartDatabase::DisableChart(wxString& PathToDisable)
{
      //    Find the chart
      for(unsigned int i=0 ; i<active_chartTable.GetCount() ; i++)
      {
          if(PathToDisable.IsSameAs(wxString(active_chartTable[i].GetpFullPath(), wxConvUTF8)))
            {
                ChartTableEntry *pentry = &active_chartTable[i];
                  pentry->Disable();

                  return 1;
            }
      }

      return 0;
}

// ----------------------------------------------------------------------------
//  Traverse the given directory looking for charts
//  If bupdate is true, also search the existing database for a name match.
//  If target chart is already in database, mark the entry valid and skip additional processing
// ----------------------------------------------------------------------------

int ChartDatabase::TraverseDirAndAddCharts(ChartDirInfo& dir_info, wxProgressDialog *pprog, wxString &dir_magic, bool bForce)
{
      //    Extract the true dir name and magic number from the compound string
      wxString dir_path = dir_info.fullpath;
      wxString old_magic = dir_info.magic_number;
      wxString new_magic = old_magic;
      dir_magic = old_magic;              // provisionally the same

      int nAdd = 0;

      bool b_skipDetectDirChange = false;
      bool b_dirchange = false;

      // Does this directory actually exist?
      if(!wxDir::Exists(dir_path))
            return 0;

      // Check to see if this is a cm93 directory root
      // If so, skip the DetectDirChange since it may be very slow
      // and give no information
      // Assume a change has happened, and process accordingly
      bool b_cm93 = Check_CM93_Structure(dir_path);
      if(b_cm93)
      {
            b_skipDetectDirChange = true;
            b_dirchange = true;
      }

      //    Quick scan the directory to see if it has changed
      //    If not, there is no need to scan again.....
      if(!b_skipDetectDirChange)
            b_dirchange = DetectDirChange(dir_path, old_magic, new_magic, pprog);

      if( !bForce && !b_dirchange)
      {
            wxString msg(_T("   No change detected on directory "));
            msg.Append(dir_path);
            wxLogMessage(msg);

            //    Traverse the database, and mark as valid all charts coming from this dir,
            //    or anywhere in its tree

            wxFileName fn_dir(dir_path, _T("stuff"));
            unsigned int dir_path_count = fn_dir.GetDirCount();

            if(pprog)
                  pprog->SetTitle(_("OpenCPN Chart Scan...."));

            int nEntries = active_chartTable.GetCount();

            for(int ic=0 ; ic<nEntries ; ic++)
            {

                wxFileName fn(wxString(active_chartTable[ic].GetpFullPath(), wxConvUTF8));
                  wxString t = fn.GetPath();


                  while(fn.GetDirCount() >= dir_path_count)
                  {
                        t = fn.GetPath();
                        if(fn.GetPath() == dir_path)
                        {
                            active_chartTable[ic].SetValid(true);
 //                             if(pprog)
 //                                  pprog->Update((ic * 100) /nEntries, fn.GetFullPath());

                              break;
                        }
                        fn.RemoveLastDir();
                  }
            }

            return 0;
      }

      //    There presumably was a change in the directory contents.  Return the new magic number
      dir_magic = new_magic;

      //    Look for all possible defined chart classes
      for(unsigned int i = 0 ; i < m_ChartClassDescriptorArray.GetCount() ; i++)
      {
            nAdd += SearchDirAndAddCharts(dir_path, m_ChartClassDescriptorArray.Item(i), pprog);
      }

      return nAdd;
}

bool ChartDatabase::DetectDirChange(const wxString & dir_path, const wxString & magic, wxString &new_magic, wxProgressDialog *pprog)
{
      if(pprog)
            pprog->SetTitle(_("OpenCPN Directory Scan...."));

      //    parse the magic number
      long long unsigned int nmagic;
      wxULongLong nacc = 0;

      magic.ToULongLong(&nmagic, 10);

      //    Get an arraystring of all files
      wxArrayString FileList;
      wxDir dir(dir_path);
      int n_files = dir.GetAllFiles(dir_path, &FileList);

      //Traverse the list of files, getting their interesting stuff to add to accumulator
      for(int ifile=0 ; ifile < n_files ; ifile++)
      {
            if(pprog && (ifile % (n_files / 60 + 1)) == 0)
                  pprog->Update(wxMin((ifile * 100) /n_files, 100), dir_path);

            wxFileName file(FileList.Item(ifile));

            //    File Size;
            wxULongLong size = file.GetSize();
            if(wxInvalidSize != size)
                  nacc = nacc + size;

            //    Mod time, in ticks
            wxDateTime t = file.GetModificationTime();
            nacc += t.GetTicks();

            //    File name
            wxString n = file.GetFullName();
            for(unsigned int in=0 ; in < n.Len() ; in++)
            {
                  nacc += (unsigned char)n[in];
            }

      }

      //    Return the calculated magic number
      new_magic = nacc.ToString();

      //    And do the test
      if(new_magic != magic)
            return true;
      else
            return false;
}




bool ChartDatabase::IsChartDirUsed(const wxString &theDir)
{
    wxString dir(theDir);
    if (dir.Last() == '/' || dir.Last() == wxFileName::GetPathSeparator())
        dir.RemoveLast();

    dir.Append(wxT("*"));
    for (UINT32 i = 0; i < active_chartTable.GetCount(); i++) {
        if (active_chartTable[i].GetpsFullPath()->Matches(dir))
            return true;
    }
    return false;
}

//-----------------------------------------------------------------------------
// Validate a given directory as a cm93 root database
// If it appears to be a cm93 database, then return true
//-----------------------------------------------------------------------------
bool ChartDatabase::Check_CM93_Structure(wxString dir_name)
{
      wxString filespec;

      wxRegEx test(_T("[0-9]+"));

      wxDir dirt(dir_name);
      wxString candidate;

      bool b_maybe_found_cm93 = false;
      bool b_cont = dirt.GetFirst(&candidate);

      while(b_cont)
      {
            if(test.Matches(candidate)&& (candidate.Len() == 8))
            {
                  b_maybe_found_cm93 = true;
                  break;
            }

            b_cont = dirt.GetNext(&candidate);

      }

      if(b_maybe_found_cm93)
      {
            wxString dir_next = dir_name;
            dir_next += _T("/");
            dir_next += candidate;
            if(wxDir::Exists(dir_next))
            {
                  wxDir dir_n(dir_next);
                  wxString candidate_n;

                  wxRegEx test_n(_T("^[A-Ga-g]"));
                  bool b_probably_found_cm93 = false;
                  bool b_cont_n = dir_n.GetFirst(&candidate_n);
                  while(b_cont_n)
                  {
                        if(test_n.Matches(candidate_n) && (candidate_n.Len() == 1))
                        {
                              b_probably_found_cm93 = true;
                              break;
                        }
                        b_cont_n = dir_n.GetNext(&candidate_n);
                  }

                  if(b_probably_found_cm93)           // found a directory that looks
                                                      //like {dir_name}/12345678/A
                                                      //probably cm93
                  {
                        // make sure the dir exists
                        wxString dir_luk = dir_next;
                        dir_luk += _T("/");
                        dir_luk += candidate_n;
                        if(wxDir::Exists(dir_luk))
                              return true;

                  }
            }
      }

      return false;
}


/*
//-----------------------------------------------------------------------------
// Validate a given directory as a cm93 root database
// If it appears to be a cm93 database, then return the name of an existing cell file
// File name will be unique with respect to member element m_cm93_filename_array
// If not cm93, return empty string
//-----------------------------------------------------------------------------
wxString ChartDatabase::Get_CM93_FileName(wxString dir_name)
{
      wxString filespec;

      wxRegEx test(_T("[0-9]+"));

      wxDir dirt(dir_name);
      wxString candidate;

      bool b_maybe_found_cm93 = false;
      bool b_cont = dirt.GetFirst(&candidate);

      while(b_cont)
      {
            if(test.Matches(candidate)&& (candidate.Len() == 8))
            {
                  b_maybe_found_cm93 = true;
                  break;
            }

            b_cont = dirt.GetNext(&candidate);

      }

      if(b_maybe_found_cm93)
      {
            wxString dir_next = dir_name;
            dir_next += _T("/");
            dir_next += candidate;
            if(wxDir::Exists(dir_next))
            {
                  wxDir dir_n(dir_next);
                  wxString candidate_n;

                  wxRegEx test_n(_T("^[A-Ga-g]"));
                  bool b_probably_found_cm93 = false;
                  bool b_cont_n = dir_n.GetFirst(&candidate_n);
                  while(b_cont_n)
                  {
                        if(test_n.Matches(candidate_n) && (candidate_n.Len() == 1))
                        {
                              b_probably_found_cm93 = true;
                              break;
                        }
                        b_cont_n = dir_n.GetNext(&candidate_n);
                  }

                  if(b_probably_found_cm93)           // found a directory that looks like {dir_name}/12345678/A   probably cm93
                  {                                   // and we want to try and shorten the recursive search
                        // make sure the dir exists
                        wxString dir_luk = dir_next;
                        dir_luk += _T("/");
                        dir_luk += candidate_n;
                        if(wxDir::Exists(dir_luk))
                        {
                              wxString msg(_T("Found probable CM93 database in "));
                              msg += dir_name;
                              wxLogMessage(msg);

                              wxString dir_name_plus = dir_luk;                 // be very specific about the dir_name,

                              wxDir dir_get(dir_name_plus);
                              wxString one_file;
                              dir_get.GetFirst(&one_file);

                              //    We must return a unique file name, i.e. one that has not bee seen
                              //    before in this invocation of chart dir scans.
                              bool find_unique = false;
                              while(!find_unique)
                              {
                                    find_unique = true;
                                    for(unsigned int ifile=0; ifile < m_cm93_filename_array.GetCount(); ifile++)
                                    {
                                          if(m_cm93_filename_array.Item(ifile) == one_file)
                                                find_unique = false;
                                    }
                                    if(!find_unique)
                                          dir_get.GetNext(&one_file);
                              }

                              m_cm93_filename_array.Add(one_file);

                              filespec = one_file;
                        }

                  }
            }
      }

      return filespec;
}
*/



// ----------------------------------------------------------------------------
// Populate Chart Table by directory search for specified file type
// If bupdate flag is true, search the Chart Table for matching chart.
//  if target chart is already in table, mark it valid and skip chart processing
// ----------------------------------------------------------------------------

WX_DECLARE_STRING_HASH_MAP( int, ChartCollisionsHashMap );

int ChartDatabase::SearchDirAndAddCharts(wxString& dir_name_base,
                                         ChartClassDescriptor &chart_desc,
                                         wxProgressDialog *pprog)
{
      wxString msg(_T("Searching directory: "));
      msg += dir_name_base;
      msg += _T(" for ");
      msg += chart_desc.m_search_mask;
      wxLogMessage(msg);

      if(!wxDir::Exists(dir_name_base))
            return 0;

      wxString dir_name = dir_name_base;
      wxString filespec = chart_desc.m_search_mask.Upper();
      wxString lowerFileSpec = chart_desc.m_search_mask.Lower();
      wxString filename;

//    Count the files
      wxArrayString FileList;
      int gaf_flags = wxDIR_DEFAULT;                  // as default, recurse into subdirs


      //    Here is an optimization for MSW/cm93 especially
      //    If this directory seems to be a cm93, and we are not explicitely looking for cm93, then abort
      //    Otherwise, we will be looking thru entire cm93 tree for non-existent .KAP files, etc.

      bool b_found_cm93 = false;
      bool b_cm93 = Check_CM93_Structure(dir_name);
      if(b_cm93)
      {
            if (filespec != _T("00300000.A"))
                  return false;
            else {
                  filespec = dir_name;
                  b_found_cm93 = true;
            }
      }


      if(!b_found_cm93)
      {
            wxDir dir(dir_name);
            dir.GetAllFiles(dir_name, &FileList, filespec, gaf_flags);
#ifndef __WXMSW__
            if (filespec != lowerFileSpec)
            {
            // add lowercase filespec files too
            wxArrayString lowerFileList;
            dir.GetAllFiles(dir_name, &lowerFileList, lowerFileSpec, gaf_flags);
            for (wxArrayString::const_iterator item = lowerFileList.begin(); item != lowerFileList.end(); item++)
                  FileList.Add(*item);
            }
#endif
      }
      else {                            // This is a cm93 dataset, specified as yada/yada/cm93
            wxString dir_plus = dir_name;
            dir_plus += wxFileName::GetPathSeparator();
            FileList .Add(dir_plus);
      }

      int nFile = FileList.GetCount();


      if(!nFile)
            return false;


      int nDirEntry = 0;

      //    Check to see if there are any charts in the DB which refer to this directory
      //    If none at all, there is no need to scan the DB for fullpath match of each potential addition
      //    and bthis_dir_in_dB is false.
      bool bthis_dir_in_dB = IsChartDirUsed(dir_name);

      if(pprog)
            pprog->SetTitle(_("OpenCPN Chart Add...."));

      // build a hash table based on filename (without directory prefix) of
      // the chart to fast to detect identical charts
      ChartCollisionsHashMap collision_map; 
      int nEntry = active_chartTable.GetCount();
      for(int i=0 ; i<nEntry ; i++) {
          wxString table_file_name(active_chartTable[i].GetpFullPath(), wxConvUTF8);
          wxFileName table_file(table_file_name);
          collision_map[table_file.GetFullName()] = i;
      }

      for(int ifile=0 ; ifile < nFile ; ifile++)
      {
            wxFileName file(FileList.Item(ifile));
            wxString full_name = file.GetFullPath();
            wxString file_name = file.GetFullName();
            //    Validate the file name again, considering MSW's semi-random treatment of case....
            // TODO...something fishy here - may need to normalize saved name?
            if(!file_name.Matches(lowerFileSpec) && !file_name.Matches(filespec) && !b_found_cm93)
                continue;

            if(pprog && (ifile % (nFile / 60 + 1)) == 0)
                  pprog->Update(wxMin((ifile * 100) / nFile, 100), full_name);

            ChartTableEntry *pnewChart = NULL;
            bool bAddFinal = true;
            int b_add_msg = 0;

            pnewChart = CreateChartTableEntry(full_name, chart_desc);
            if(!pnewChart)
            {
                  bAddFinal = false;
                  wxString msg = _T("   CreateChartTableEntry() failed for file: ");
                  msg.Append(full_name);
                  wxLogMessage(msg);
            }
            else         // check the collisions map looking for duplicates, and choosing the right one
            {
                if(collision_map.find(file_name) != collision_map.end()) {
                    int isearch = collision_map[file_name];
                    wxString table_file_name(active_chartTable[isearch].GetpFullPath(), wxConvUTF8);

                    //    If the chart full file paths are exactly the same, select the newer one
                    if(bthis_dir_in_dB && full_name.IsSameAs(table_file_name))
                    {
                        b_add_msg++;

                        //    Check the file modification time
                        time_t t_oldFile = active_chartTable[isearch].GetFileTime();
                        time_t t_newFile = file.GetModificationTime().GetTicks();
                        
                        if( t_newFile <= t_oldFile )
                        {
                            bAddFinal = false;
                            active_chartTable[isearch].SetValid(true);
                        }
                        else
                        {
                            bAddFinal = true;
                            active_chartTable[isearch].SetValid(false);
                            wxString msg = _T("   Replacing older chart file of same path: ");
                            msg.Append(full_name);
                            wxLogMessage(msg);
                        }
                    } else {
                    
                        //  Look at the chart file name (without directory prefix) for a further check for duplicates
                        //  This catches the case in which the "same" chart is in different locations,
                        //  and one may be newer than the other.
                        b_add_msg++;
                        
                        if(pnewChart->IsEarlierThan(active_chartTable[isearch]))
                        {
                            wxFileName table_file(table_file_name);
                            //    Make sure the compare file actually exists
                            if(table_file.IsFileReadable())
                            {
                                active_chartTable[isearch].SetValid(true);
                                bAddFinal = false;
                                wxString msg = _T("   Retaining newer chart file of same name: ");
                                msg.Append(full_name);
                                wxLogMessage(msg);

                            }
                        }
                        else if(pnewChart->IsEqualTo(active_chartTable[isearch]))
                        {
                            //    The file names (without dir prefix) are identical,
                            //    and the mod times are identical
                            //    Prsume that this is intentional, in order to facilitate
                            //    having the same chart in multiple groups.
                            //    So, add this chart.
                            bAddFinal = true;
                        }

                        else
                        {
                            active_chartTable[isearch].SetValid(false);
                            bAddFinal = true;
                            wxString msg = _T("   Replacing older chart file of same name: ");
                            msg.Append(full_name);
                            wxLogMessage(msg);
                        }
                    }
                }
            }

            if(bAddFinal)
            {
                  if(0 == b_add_msg)
                  {
                        wxString msg = _T("   Adding chart file: ");
                        msg.Append(full_name);
                        wxLogMessage(msg);
                  }
                  collision_map[file_name] = active_chartTable.GetCount();
                  active_chartTable.Add(pnewChart);
                  nDirEntry++;
            }
            else
            {
                delete pnewChart;
//                  wxString msg = _T("   Not adding chart file: ");
//                  msg.Append(full_name);
//                  wxLogMessage(msg);
            }
      }

      m_nentries = active_chartTable.GetCount();
      
      return nDirEntry;
}


bool ChartDatabase::AddChart( wxString &chartfilename, ChartClassDescriptor &chart_desc, wxProgressDialog *pprog,
    int isearch, bool bthis_dir_in_dB)
{
    bool rv = false;
    wxFileName file(chartfilename);
    wxString full_name = file.GetFullPath();
    wxString file_name = file.GetFullName();
    
    //    Validate the file name again, considering MSW's semi-random treatment of case....
    // TODO...something fishy here - may need to normalize saved name?
//    if(!file_name.Matches(lowerFileSpec) && !file_name.Matches(filespec) && !b_found_cm93)
//        continue;
    
    if(pprog)
        pprog->Update(wxMin((m_pdifile * 100) /m_pdnFile, 100), full_name);
    
    
    ChartTableEntry *pnewChart = NULL;
    bool bAddFinal = true;
    int b_add_msg = 0;
    
    pnewChart = CreateChartTableEntry(full_name, chart_desc);
    if(!pnewChart)
    {
        bAddFinal = false;
        wxString msg = _T("   CreateChartTableEntry() failed for file: ");
        msg.Append(full_name);
        wxLogMessage(msg);
        return false;
    }
    else         // traverse the existing database looking for duplicates, and choosing the right one
            {
                int nEntry = active_chartTable.GetCount();
                for(int i=0 ; i<nEntry ; i++)
                {
                    wxString *ptable_file_name = active_chartTable[isearch].GetpsFullPath();
                    
                    //    If the chart full file paths are exactly the same, select the newer one
                    if(bthis_dir_in_dB && full_name.IsSameAs(*ptable_file_name))
                    {
                        b_add_msg++;
                        
                        //    Check the file modification time
                        time_t t_oldFile = active_chartTable[isearch].GetFileTime();
                        time_t t_newFile = file.GetModificationTime().GetTicks();
                        
                        if( t_newFile <= t_oldFile )
                        {
                            bAddFinal = false;
                            active_chartTable[isearch].SetValid(true);
                        }
                        else
                        {
                            bAddFinal = true;
                            active_chartTable[isearch].SetValid(false);
                            wxString msg = _T("   Replacing older chart file of same path: ");
                            msg.Append(full_name);
                            wxLogMessage(msg);
                        }
                        
                        break;
                    }
                    
                    
                    //  Look at the chart file name (without directory prefix) for a further check for duplicates
                    //  This catches the case in which the "same" chart is in different locations,
                    //  and one may be newer than the other.
                    wxFileName table_file(*ptable_file_name);
                    
                    if( table_file.GetFullName() == file_name )
                    {
                        b_add_msg++;
                        
                        if(pnewChart->IsEarlierThan(active_chartTable[isearch]))
                        {
                            //    Make sure the compare file actually exists
                            if(table_file.IsFileReadable())
                            {
                                active_chartTable[isearch].SetValid(true);
                                bAddFinal = false;
                                wxString msg = _T("   Retaining newer chart file of same name: ");
                                msg.Append(full_name);
                                wxLogMessage(msg);
                                
                            }
                        }
                        else if(pnewChart->IsEqualTo(active_chartTable[isearch]))
                        {
                            //    The file names (without dir prefix) are identical,
                            //    and the mod times are identical
                            //    Prsume that this is intentional, in order to facilitate
                            //    having the same chart in multiple groups.
                            //    So, add this chart.
                            bAddFinal = true;
                        }
                        
                        else
                        {
                            active_chartTable[isearch].SetValid(false);
                            bAddFinal = true;
                            wxString msg = _T("   Replacing older chart file of same name: ");
                            msg.Append(full_name);
                            wxLogMessage(msg);
                        }
                        
                        break;
                    }
                    
                    //TODO    Look at the chart ID as a further check against duplicates
                    
                    
                    isearch++;
                    if(nEntry == isearch)
                        isearch = 0;
                }     // for
            }
            
            
            if(bAddFinal)
            {
                if(0 == b_add_msg)
                {
                    wxString msg = _T("   Adding chart file: ");
                    msg.Append(full_name);
                    wxLogMessage(msg);
                }

                active_chartTable.Add(pnewChart);
                
                rv = true;
            }
            else
            {
                delete pnewChart;
                //                  wxString msg = _T("   Not adding chart file: ");
                //                  msg.Append(full_name);
                //                  wxLogMessage(msg);
                rv = false;
            }
            
    m_nentries = active_chartTable.GetCount();
            
    return rv;
}
            
bool ChartDatabase::AddSingleChart( wxString &ChartFullPath, bool b_force_full_search )
{
    //  Find a relevant chart class descriptor
    wxFileName fn(ChartFullPath);
    wxString ext = fn.GetExt();
    ext.Prepend(_T("*."));
    wxString ext_upper = ext.MakeUpper();
    wxString ext_lower = ext.MakeLower();
    wxString dir_name = fn.GetPath();

    //    Search the array of chart class descriptors to find a match
    //    bewteen the search mask and the the chart file extension

    ChartClassDescriptor desc;
    for(unsigned int i=0 ; i < m_ChartClassDescriptorArray.GetCount() ; i++)
    {
        if(m_ChartClassDescriptorArray.Item(i).m_descriptor_type == PLUGIN_DESCRIPTOR)
        {
            if(m_ChartClassDescriptorArray.Item(i).m_search_mask == ext_upper)
            {
                desc = m_ChartClassDescriptorArray.Item(i);
                break;
            }
            if(m_ChartClassDescriptorArray.Item(i).m_search_mask == ext_lower)
            {
                desc = m_ChartClassDescriptorArray.Item(i);
                break;
            }
        }
    }
    
    //  If we know that we need to do a full recursive search of the db,
    //  then there is no need to verify it by doing a directory match
    bool b_recurse = true;
    if(!b_force_full_search)
        b_recurse = IsChartDirUsed(dir_name);
    
    bool rv = AddChart( ChartFullPath, desc, NULL, 0,  b_recurse );

    //  remove duplicates marked in AddChart()

    for(unsigned int i=0 ; i<active_chartTable.GetCount() ; i++)
    {
        if(!active_chartTable[i].GetbValid())
        {
            active_chartTable.RemoveAt(i);
            i--;                 // entry is gone, recheck this index for next entry
        }
    }
    
    //    Update the Entry index fields
    for(unsigned int i=0 ; i<active_chartTable.GetCount() ; i++)
        active_chartTable[i].SetEntryOffset( i );
 
    //  Get a new magic number
    wxString new_magic;
    DetectDirChange(dir_name, _T(""), new_magic, 0);
        
    
    //    Update (clone) the CDI array
    bool bcfound = false;
    ArrayOfCDI NewChartDirArray;
    
    ArrayOfCDI ChartDirArray = GetChartDirArray();
    for(unsigned int i=0 ; i < ChartDirArray.GetCount(); i++)
    {
        ChartDirInfo cdi = ChartDirArray.Item(i);
        
        ChartDirInfo newcdi = cdi;
        
        //      If entry is found that matches this cell, clear the magic number.
        if( newcdi.fullpath == dir_name ){
            newcdi.magic_number = new_magic;
            bcfound = true;
        }
        
        NewChartDirArray.Add(newcdi);
    }
    
    if( !bcfound ){
        ChartDirInfo cdi;
        cdi.fullpath = dir_name;
        cdi.magic_number = new_magic;
        NewChartDirArray.Add ( cdi );
    }
    
    
    // Update the database master copy of the CDI array
    SetChartDirArray( NewChartDirArray );
    
    //  Update the list of chart dirs.
    m_chartDirs.Clear();
    
    for(unsigned int i=0 ; i < GetChartDirArray().GetCount(); i++)
    {
        ChartDirInfo cdi = GetChartDirArray().Item(i);
        m_chartDirs.Add( cdi.fullpath );
    }
    
    m_nentries = active_chartTable.GetCount();
    
    return rv;
    
}


bool ChartDatabase::RemoveSingleChart( wxString &ChartFullPath )
{
    bool rv = false;
 
    //  Walk the chart table, looking for the target
    for(unsigned int i=0 ; i<active_chartTable.GetCount() ; i++) {
        if( !strcmp( ChartFullPath.mb_str(), GetChartTableEntry(i).GetpFullPath() ) ){
            active_chartTable.RemoveAt(i);
            break;
        }
    }

    
    //    Update the EntryOffset fields for the array
    ChartTableEntry *pcte;
    
    for(unsigned int i=0 ; i<active_chartTable.GetCount() ; i++){
        pcte = GetpChartTableEntry(i);
        pcte->SetEntryOffset( i );
    }
 
 //  Check and update the dir array
    wxFileName fn(ChartFullPath);
    wxString fd = fn.GetPath();
    if( !IsChartDirUsed(fd) ){
        
        //      Clone a new array, removing the unused directory, 
        ArrayOfCDI NewChartDirArray;
        
        ArrayOfCDI ChartDirArray = GetChartDirArray();
        for(unsigned int i=0 ; i < ChartDirArray.GetCount(); i++){
            ChartDirInfo cdi = ChartDirArray.Item(i);
            
            ChartDirInfo newcdi = cdi;
            
            if( newcdi.fullpath != fd )
                NewChartDirArray.Add(newcdi);
        }
        
        SetChartDirArray( NewChartDirArray );
    }

    //  Update the list of chart dirs.
    m_chartDirs.Clear();
    for(unsigned int i=0 ; i < GetChartDirArray().GetCount(); i++)
    {
        ChartDirInfo cdi = GetChartDirArray().Item(i);
        m_chartDirs.Add( cdi.fullpath );
    }
    
    m_nentries = active_chartTable.GetCount();
    
    return rv;
}
    


///////////////////////////////////////////////////////////////////////
// Create a Chart object
///////////////////////////////////////////////////////////////////////

ChartBase *ChartDatabase::GetChart(const wxChar *theFilePath, ChartClassDescriptor &chart_desc) const
{
    // TODO: support non-UI chart factory
    return NULL;
}

///////////////////////////////////////////////////////////////////////
// Create Chart Table entry by reading chart header info, etc.
///////////////////////////////////////////////////////////////////////

ChartTableEntry *ChartDatabase::CreateChartTableEntry(const wxString &filePath, ChartClassDescriptor &chart_desc)
{
      wxString msg = wxT("Loading chart data for ");
      msg.Append(filePath);
      wxLogMessage(msg);

      ChartBase *pch = GetChart(filePath, chart_desc);
      if (pch == NULL) {
            wxString msg = wxT("   ...creation failed for ");
            msg.Append(filePath);
            wxLogMessage(msg);
            return NULL;
      }

      InitReturn rc = pch->Init(filePath, HEADER_ONLY);
      if (rc != INIT_OK) {
            delete pch;
            wxString msg = wxT("   ...initialization failed for ");
            msg.Append(filePath);
            wxLogMessage(msg);
            return NULL;
      }


      ChartTableEntry *ret_val = new ChartTableEntry(*pch);
      ret_val->SetValid(true);

      delete pch;

      return ret_val;
}

bool ChartDatabase::GetCentroidOfLargestScaleChart(double *clat, double *clon, ChartFamilyEnum family)
{
      int cur_max_i = -1;
      int cur_max_scale = 0;

      int nEntry = active_chartTable.GetCount();

      for(int i=0 ; i<nEntry ; i++)
      {
          if(GetChartFamily(active_chartTable[i].GetChartType()) == family)
            {
                if(active_chartTable[i].GetScale() > cur_max_scale)
                  {
                      cur_max_scale = active_chartTable[i].GetScale();
                        cur_max_i = i;
                  }
            }
      }

      if(cur_max_i == -1)
            return false;                             // nothing found
      else
      {
          *clat = (active_chartTable[cur_max_i].GetLatMax() + active_chartTable[cur_max_i].GetLatMin()) / 2.;
          *clon = (active_chartTable[cur_max_i].GetLonMin() + active_chartTable[cur_max_i].GetLonMax()) /2.;
      }
      return true;
}

//-------------------------------------------------------------------
//    Get DBChart Projection
//-------------------------------------------------------------------
int ChartDatabase::GetDBChartProj(int dbIndex)
{
    if((bValid) && (dbIndex >= 0) && (dbIndex < (int)active_chartTable.size()))
        return active_chartTable[dbIndex].GetChartProjectionType();
      else
            return PROJECTION_UNKNOWN;
}

//-------------------------------------------------------------------
//    Get DBChart Family
//-------------------------------------------------------------------
int ChartDatabase::GetDBChartFamily(int dbIndex)
{
    if((bValid) && (dbIndex >= 0) && (dbIndex < (int)active_chartTable.size()))
          return active_chartTable[dbIndex].GetChartFamily();
      else
            return CHART_FAMILY_UNKNOWN;
}

//-------------------------------------------------------------------
//    Get DBChart FullFileName
//-------------------------------------------------------------------
wxString ChartDatabase::GetDBChartFileName(int dbIndex)
{
    if((bValid) && (dbIndex >= 0) && (dbIndex < (int)active_chartTable.size()))
      {
          return wxString(active_chartTable[dbIndex].GetpFullPath(), wxConvUTF8);
      }
      else
            return _T("");
}


//-------------------------------------------------------------------
//    Get DBChart Type
//-------------------------------------------------------------------
int ChartDatabase::GetDBChartType(int dbIndex)
{
    if((bValid) && (dbIndex >= 0) && (dbIndex < (int)active_chartTable.size()))
        return active_chartTable[dbIndex].GetChartType();
      else
            return 0;
}

//-------------------------------------------------------------------
//    Get DBChart Skew
//-------------------------------------------------------------------
float ChartDatabase::GetDBChartSkew(int dbIndex)
{
    if((bValid) && (dbIndex >= 0) && (dbIndex < (int)active_chartTable.size()))
        return active_chartTable[dbIndex].GetChartSkew();
      else
            return 0.;
}

//-------------------------------------------------------------------
//    Get DBChart Scale
//-------------------------------------------------------------------
int ChartDatabase::GetDBChartScale(int dbIndex)
{
    if((bValid) && (dbIndex >= 0) && (dbIndex < (int)active_chartTable.size()))
        return active_chartTable[dbIndex].GetScale();
      else
            return 1;
}

//-------------------------------------------------------------------
//    Get Lat/Lon Bounding Box from db
//-------------------------------------------------------------------
bool ChartDatabase::GetDBBoundingBox(int dbIndex, wxBoundingBox *box)
{
    if((bValid) && (dbIndex >= 0) && (dbIndex < (int)active_chartTable.size()))
      {
            const ChartTableEntry &entry = GetChartTableEntry(dbIndex);
            box->SetMax(entry.GetLonMax(), entry.GetLatMax());
            box->SetMin(entry.GetLonMin(), entry.GetLatMin());
      }

      return true;
}

const wxBoundingBox &ChartDatabase::GetDBBoundingBox(int dbIndex)
{
    if((bValid) && (dbIndex >= 0) )
    {
        const ChartTableEntry &entry = GetChartTableEntry(dbIndex);
        return entry.GetBBox();
    }
    else
    {
        return m_dummy_bbox;
    }
}


//-------------------------------------------------------------------
//    Get PlyPoint from Database
//-------------------------------------------------------------------
int ChartDatabase::GetDBPlyPoint(int dbIndex, int plyindex, float *lat, float *lon)
{
    if((bValid) && (dbIndex >= 0) && (dbIndex < (int)active_chartTable.size()))
      {
            const ChartTableEntry &entry = GetChartTableEntry(dbIndex);
            if(entry.GetnPlyEntries())
            {
                  float *fp = entry.GetpPlyTable();
                  fp += plyindex*2;
                  if(lat)
                     *lat = *fp;
                  fp++;
                  if(lon)
                     *lon = *fp;
            }
            return entry.GetnPlyEntries();
      }
      else
            return 0;
}

//-------------------------------------------------------------------
//    Get AuxPlyPoint from Database
//-------------------------------------------------------------------
int ChartDatabase::GetDBAuxPlyPoint(int dbIndex, int plyindex, int ply, float *lat, float *lon)
{
    if((bValid) && (dbIndex >= 0) && (dbIndex < (int)active_chartTable.size()))
      {
            const ChartTableEntry &entry = GetChartTableEntry(dbIndex);
            if(entry.GetnAuxPlyEntries())
            {
                  float *fp = entry.GetpAuxPlyTableEntry(ply);

                  fp += plyindex*2;
                  if(lat)
                     *lat = *fp;
                  fp++;
                  if(lon)
                     *lon = *fp;
            }

            return entry.GetAuxCntTableEntry(ply);
      }
      else
            return 0;
}

int  ChartDatabase::GetnAuxPlyEntries(int dbIndex)
{
    if((bValid) && (dbIndex >= 0) && (dbIndex < (int)active_chartTable.size()))
      {
            const ChartTableEntry &entry = GetChartTableEntry(dbIndex);
            return entry.GetnAuxPlyEntries();
      }
      else
            return 0;
}

bool  ChartDatabase::IsChartAvailable(int dbIndex)
{
    if((bValid) && (dbIndex >= 0) && (dbIndex < (int)active_chartTable.size()))
    {
        ChartTableEntry *pentry = GetpChartTableEntry(dbIndex);
        
        //      If not PLugIn chart, assume always available
        if(pentry->GetChartType() != CHART_TYPE_PLUGIN)
            return true;
        
        wxString *path = pentry->GetpsFullPath();
        wxFileName fn(*path);
        wxString ext = fn.GetExt();
        ext.Prepend(_T("*."));
        wxString ext_upper = ext.MakeUpper();
        wxString ext_lower = ext.MakeLower();
        
        //    Search the array of chart class descriptors to find a match
        //    between the search mask and the the chart file extension
        
        for(unsigned int i=0 ; i < m_ChartClassDescriptorArray.GetCount() ; i++)
        {
            if(m_ChartClassDescriptorArray.Item(i).m_descriptor_type == PLUGIN_DESCRIPTOR)
            {
                if(m_ChartClassDescriptorArray.Item(i).m_search_mask == ext_upper) {
                    return true;
                }
                if(m_ChartClassDescriptorArray.Item(i).m_search_mask == ext_lower) {
                    return true;
                }
            }
        }
    }
        
    return false;
}

void ChartDatabase::ApplyGroupArray(ChartGroupArray *pGroupArray)
{
    wxString separator(wxFileName::GetPathSeparator());

    for(unsigned int ic=0 ; ic < active_chartTable.GetCount(); ic++)
      {
            ChartTableEntry *pcte = &active_chartTable[ic];

            pcte->ClearGroupArray();

            wxString *chart_full_path = pcte->GetpsFullPath();

            for(unsigned int igroup = 0; igroup < pGroupArray->GetCount(); igroup++)
            {
                  ChartGroup *pGroup = pGroupArray->Item(igroup);
                  for(unsigned int j=0; j < pGroup->m_element_array.GetCount(); j++)
                  {
                        wxString element_root = pGroup->m_element_array.Item(j)->m_element_name;
                        
                        //  The element may be a full single chart name
                        //  If so, add it
                        //  Otherwise, append a sep character so that similar paths are distinguished.
                        //  See FS#1060
                        if(!chart_full_path->IsSameAs(element_root))
                            element_root.Append(separator);	// Prevent comingling similar looking path names
                        if(chart_full_path->StartsWith(element_root))
                        {
                              bool b_add = true;
                              for(unsigned int k=0 ; k < pGroup->m_element_array.Item(j)->m_missing_name_array.GetCount(); k++)
                              {
                                    wxString missing_item = pGroup->m_element_array.Item(j)->m_missing_name_array.Item(k);
                                    if(chart_full_path->StartsWith(missing_item))
                                    {
                                          if(chart_full_path->IsSameAs( missing_item )) // missing item is full chart name
                                          {
                                                b_add = false;
                                                break;
                                          }
                                          else
                                          {
                                                if(wxDir::Exists(missing_item))     // missing item is a dir
                                                {
                                                      b_add = false;
                                                      break;
                                                }
                                          }
                                    }
                              }

                              if(b_add)
                                    pcte->AddIntToGroupArray( igroup + 1 ); 
                        }
                  }
            }
      }

}
