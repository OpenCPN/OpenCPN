/******************************************************************************
*
* Project:  ChartManager
* Purpose:  Basic Chart Info Storage
* Author:   David Register, Mark A Sikes
*
***************************************************************************
*   Copyright (C) 2010 by David S. Register  *
*   bdbcat@yahoo.com	      *
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
*/

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/arrimpl.cpp>
#include <wx/encconv.h>
#include <wx/regex.h>
#include <wx/generic/progdlgg.h>

#include "chartdbs.h"
#include "chartbase.h"
#include "pluginmanager.h"

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

          return false;       // no match....

/*
          // Try previous version....
          sprintf(vb, "V%03d", DB_VERSION_PREVIOUS);
          if (strncmp(vb, dbVersion, sizeof(dbVersion)))
                return false;
          else
          {
                wxLogMessage(_T("   Scheduling db upgrade to current db version on Toolbox->Charts page visit..."));
                return true;
          }
*/

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
    Scale = theChart.GetNativeScale();

    Skew = theChart.GetChartSkew();
    ProjectionType = theChart.GetChartProjectionType();

    wxDateTime ed = theChart.GetEditionDate();
    if(theChart.GetEditionDate().IsValid())
          edition_date = theChart.GetEditionDate().GetTicks();

    wxFileName fn(theChart.GetFullPath());
    if(fn.GetModificationTime().IsValid())
          file_date = fn.GetModificationTime().GetTicks();

    Extent ext;
    theChart.GetChartExtent(&ext);
    LatMax = ext.NLAT;
    LatMin = ext.SLAT;
    LonMin = ext.WLON;
    LonMax = ext.ELON;

    // Fill in the PLY information

    // If  COVR table has only one entry, us it for the primary Ply Table
    if (theChart.GetCOVREntries() == 1) {
          nPlyEntries = theChart.GetCOVRTablePoints(0);
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
                float *pf_entry = (float *)malloc(theChart.GetCOVRTablePoints(j) * 2 * sizeof(float));
                memcpy(pf_entry, theChart.GetCOVRTableHead(j), theChart.GetCOVRTablePoints(j) * 2 * sizeof(float));
                pft0[j] = pf_entry;
                pip[j] = theChart.GetCOVRTablePoints(j);
          }

          pAuxPlyTable = pfp;
          pAuxCntTable = pip;

    }
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
}

///////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////

bool ChartTableEntry::IsEqualToOrEarlierThan(const ChartTableEntry &cte) const {
    wxDateTime mine(edition_date);
    wxDateTime theirs(cte.edition_date);
    return (mine.IsEarlierThan(theirs) | mine.IsEqualTo(theirs));
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




bool ChartTableEntry::Read(const ChartDatabase *pDb, wxInputStream &is)
{
    char path[4096], *cp;

    Clear();

    //      Allow reading of current db format, and maybe others
    ChartDatabase *pD = (ChartDatabase *)pDb;
    int db_version = pD->GetVersion();

    if(db_version == 16)
    {
      // Read the path first
          for (cp = path; (*cp = (char)is.GetC()) != 0; cp++);
     // TODO: optimize prepended dir
          pFullPath = (char *)malloc(cp - path + 1);
          strncpy(pFullPath, path, cp - path + 1);
          wxLogVerbose(_T("  Chart %s"), pFullPath);

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
    // TODO: unoptimize prepended dir
    os.Write(pFullPath, strlen(pFullPath) + 1);

    //      Write the current version type only
    //      Create an on_disk table entry
    ChartTableEntry_onDisk_16 cte;

      //    Transcribe the elements....
    cte.EntryOffset = EntryOffset;
    cte.ChartType = ChartType;
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

    os.Write(&cte, sizeof(ChartTableEntry_onDisk_16));
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

    return true;
}

///////////////////////////////////////////////////////////////////////

void ChartTableEntry::Clear()
{
    memset(this, 0, sizeof(ChartTableEntry));
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
      if(index < GetChartTableEntries())
            return chartTable[index];
      else
            return m_ChartTableEntryDummy;
}


bool ChartDatabase::Read(const wxString &filePath)
{
    ChartTableEntry entry;
    int entries;

    bValid = false;

    wxFileName file(filePath);
    if (!file.FileExists()) return false;

    m_DBFileName = filePath;

    wxFileInputStream ifs(filePath);
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
    chartTable.Alloc(entries);
    while (entries-- && entry.Read(this, ifs))
        chartTable.Add(entry);

    entry.Clear();
    bValid = true;
    return true;

read_error:
    bValid = false;
    return false;
}

///////////////////////////////////////////////////////////////////////

bool ChartDatabase::Write(const wxString &filePath)
{
    wxFileName file(filePath);
    wxFileName dir(file.GetPath(wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME, wxPATH_NATIVE));

    if (!dir.DirExists() && !dir.Mkdir()) return false;

    wxFileOutputStream ofs(filePath);
    if(!ofs.Ok()) return false;

    ChartTableHeader cth(m_chartDirs.GetCount(), chartTable.GetCount());
    cth.Write(ofs);

    for (int iDir = 0; iDir < cth.GetDirEntries(); iDir++) {
        wxString &dir = m_chartDirs[iDir];
        int dirlen = dir.length();
        char s[200];
        strncpy(s, dir.mb_str(wxConvUTF8), 199);
        dirlen = strlen(s);
        ofs.Write(&dirlen, sizeof(int));
//        ofs.Write(dir.fn_str(), dirlen);
        ofs.Write(s, dirlen);
    }

    for (UINT32 iTable = 0; iTable < chartTable.size(); iTable++)
        chartTable[iTable].Write(this, ofs);

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
            if(longline.Len() > target_width)
            {
                  line += SplitPath(wxString(cte.GetpFullPath(), wxConvUTF8), _T("/,\\"), target_width, 15, &ncr);
//                  line += _T("\r\n");
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
                  if(longline.Len() > target_width)
                  {
                        line += SplitPath(pc->GetName(), _T(" "), target_width, 12, &ncr);
//                        line += _T("\r\n");
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

            line.Printf(_(" Scale:  1:%d"), cte.GetScale() );
            line += _T("\n");
            max_width = wxMax(max_width, line.Len());
            r += line;
            lc++;

            line.Empty();
            if(pc)
            {
                  line = _(" ID:  ");
                  line += pc->GetID();
            }
            line += _T("\n");
            max_width = wxMax(max_width, line.Len());
            r += line;
            lc++;

            line.Empty();
            if(pc)
            {
                  line = _(" Depth Units:  ");
                  line += pc->GetDepthUnits();
            }
            line += _T("\n");
            max_width = wxMax(max_width, line.Len());
            r += line;
            lc++;

            line.Empty();
            if(pc)
            {
                  line = _(" Soundings:  ");
                  line += pc->GetSoundingsDatum();
            }
            line += _T("\n");
            max_width = wxMax(max_width, line.Len());
            r += line;
            lc++;

            line.Empty();
            if(pc)
            {
                  line = _(" Datum:  ");
                  line += pc->GetDatumString();
            }
            line += _T("\n");
            max_width = wxMax(max_width, line.Len());
            r += line;
            lc++;

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
            }
            line += _T("\n");
            max_width = wxMax(max_width, line.Len());
            r += line;
            lc++;

            line.Empty();
            if(pc)
            {
                  line = _(" Updated:  ");
                  wxDateTime ed = pc->GetEditionDate();
                  line += ed.FormatISODate();
            }
            line += _T("\n");
            max_width = wxMax(max_width, line.Len());
            r += line;
            lc++;

            line.Empty();
            if(pc && pc->GetExtraInfo().Len())
            {
                  line += pc->GetExtraInfo();
                  line += _("\n");
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
      chartTable.Clear();
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
      for(unsigned int i=0 ; i<chartTable.GetCount() ; i++)
            chartTable[i].SetValid(false);

      m_chartDirs.Clear();

      if(bForce)
            chartTable.Clear();

      bool lbForce = bForce;

      //    Do a dB Version upgrade if the current one is obsolete
      if(s_dbVersion != DB_VERSION_CURRENT)
      {

            chartTable.Clear();
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


      for(unsigned int i=0 ; i<chartTable.GetCount() ; i++)
      {
            if(!chartTable[i].GetbValid())
            {
                  chartTable.RemoveAt(i);
                  i--;                 // entry is gone, recheck this index for next entry
            }
      }

      //    And once more, setting the Entry index field
      for(unsigned int i=0 ; i<chartTable.GetCount() ; i++)
            chartTable[i].SetEntryOffset( i );


      bValid = true;
      return true;
}

//-------------------------------------------------------------------
//    Find Chart dbIndex
//-------------------------------------------------------------------

int ChartDatabase::FinddbIndex(wxString PathToFind)
{
      //    Find the chart
      for(unsigned int i=0 ; i<chartTable.GetCount() ; i++)
      {
            if(PathToFind.IsSameAs(wxString(chartTable[i].GetpFullPath(), wxConvUTF8)))
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
      for(unsigned int i=0 ; i<chartTable.GetCount() ; i++)
      {
            if(PathToDisable.IsSameAs(wxString(chartTable[i].GetpFullPath(), wxConvUTF8)))
            {
                  ChartTableEntry *pentry = &chartTable[i];
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
      wxString cm93_cell_name = Check_CM93_Structure(dir_path);
      if(cm93_cell_name.Len())
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

            int nEntries = chartTable.GetCount();

            for(int ic=0 ; ic<nEntries ; ic++)
            {

                  wxFileName fn(wxString(chartTable[ic].GetpFullPath(), wxConvUTF8));
                  wxString t = fn.GetPath();


                  while(fn.GetDirCount() >= dir_path_count)
                  {
                        t = fn.GetPath();
                        if(fn.GetPath() == dir_path)
                        {
                              chartTable[ic].SetValid(true);
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

bool ChartDatabase::DetectDirChange(wxString dir_path, wxString magic, wxString &new_magic, wxProgressDialog *pprog)
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

      //    Arbitrarily, we decide if the dir has more than a specified number of files
      //    then don't scan it.  Takes too long....

      if(n_files > 10000)
      {
            new_magic = _T("");
            return true;
      }

      //Traverse the list of files, getting their interesting stuff to add to accumulator
      for(int ifile=0 ; ifile < n_files ; ifile++)
      {
            if(pprog && ((((ifile * 100) / n_files) % 20) == 0))
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
    for (UINT32 i = 0; i < chartTable.GetCount(); i++) {
        wxString chartPath(chartTable[i].GetpFullPath(), wxConvUTF8);
        if (chartPath.Matches(dir))
            return true;
    }
    return false;
}


//-----------------------------------------------------------------------------
// Validate a given directory as a cm93 root database
// If it appears to be a cm93 database, then return the name of an existing cell file
// If not cm93, return empty string
//-----------------------------------------------------------------------------
wxString ChartDatabase::Check_CM93_Structure(wxString dir_name)
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

                              wxString dir_name_plus = dir_luk;                 // be very specific about the dir_name, to shorten GatAllFiles()
//                              gaf_flags =  wxDIR_FILES;                 // dont recurse

                              wxDir dir_get(dir_name_plus);
                              wxString one_file;
                              dir_get.GetFirst(&one_file);
                              filespec = one_file;                       // so adjust the filespec
                        }

                  }
            }
      }

      return filespec;
}




// ----------------------------------------------------------------------------
// Populate Chart Table by directory search for specified file type
// If bupdate flag is true, search the Chart Table for matching chart.
//  if target chart is already in table, mark it valid and skip chart processing
// ----------------------------------------------------------------------------

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

      wxString cm93_cell_name = Check_CM93_Structure(dir_name);
      if(cm93_cell_name.Len())
      {
            if (filespec != _T("00300000.A"))
                  return false;
            else
                  filespec = cm93_cell_name;
      }


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

      int nFile = FileList.GetCount();

      if(!nFile)
            return false;


      int nDirEntry = 0;

      //    Check to see if there are any charts in the DB which refer to this directory
      //    If none at all, there is no need to scan the DB for fullpath match of each potential addition
      //    and bthis_dir_in_dB is false.
      bool bthis_dir_in_dB = IsChartDirUsed(dir_name);

      int isearch = 0;              // create a smarter search index
                                          // indexing the DB starting from the last found item

      if(pprog)
            pprog->SetTitle(_("OpenCPN Chart Add...."));

      for(int ifile=0 ; ifile < nFile ; ifile++)
      {
            wxFileName file(FileList.Item(ifile));
            wxString full_name = file.GetFullPath();
            wxString file_name = file.GetFullName();

            //    Validate the file name again, considering MSW's semi-random treatment of case....
            // TODO...something fishy here - may need to normalize saved name?
            if(!file_name.Matches(lowerFileSpec) && !file_name.Matches(filespec))
                continue;

            if(pprog)
                  pprog->Update(wxMin((ifile * 100) /nFile, 100), full_name);


            //    As a speed optimization, use strcmp instead of wxString::IsSameAs()
 //           char path_test_str[512];
 //           strncpy(path_test_str, (const char *)full_name.fn_str(), 511);


            ChartTableEntry *pnewChart = NULL;
            bool bAddFinal = true;

            pnewChart = CreateChartTableEntry(full_name, chart_desc);
            if(!pnewChart)
            {
                  bAddFinal = false;
                  wxString msg = _T("   CreateChartTableEntry() failed for file: ");
                  msg.Append(full_name);
                  wxLogMessage(msg);
            }
            else         // traverse the existing database looking for duplicates, and choosing the right one
            {
                  int nEntry = chartTable.GetCount();
                  for(int i=0 ; i<nEntry ; i++)
                  {
                        wxString table_file_name(chartTable[isearch].GetpFullPath(), wxConvUTF8);

                        //    If the chart file paths are exactly the same, select the newer one
//                        if(bthis_dir_in_dB && !strcmp(path_test_str, chartTable[isearch].GetpFullPath()))
                        if(bthis_dir_in_dB && full_name.IsSameAs(table_file_name))
                        {
                              //    Check the file modification time
                              time_t t_oldFile = chartTable[isearch].GetFileTime();
                              time_t t_newFile = file.GetModificationTime().GetTicks();

                              if(t_oldFile <= t_newFile)
                              {
                                    bAddFinal = false;
                                    chartTable[isearch].SetValid(true);
                              }
                              else
                              {
                                    bAddFinal = true;
                                    chartTable[isearch].SetValid(false);
                                    wxString msg = _T("   Replacing older chart file of same path: ");
                                    msg.Append(full_name);
                                    wxLogMessage(msg);
                              }

                              break;
                        }


                  //  Look at the chart file name for a further check for duplicates
                  //  This catches the case in which the "same" chart is in different locations,
                  //  and one may be newer than the other.
                        wxFileName table_file(table_file_name);

                        if( table_file.GetFullName() == file_name )
                        {
                              if(pnewChart->IsEqualToOrEarlierThan(chartTable[isearch]))
                              {
                                    //    Make sure the compare file actually exists
                                    if(table_file.IsFileReadable())
                                    {
                                          chartTable[isearch].SetValid(true);
                                          bAddFinal = false;
                                          wxString msg = _T("   Retaining newer chart file of same name: ");
                                          msg.Append(full_name);
                                          wxLogMessage(msg);

                                    }
                              }
                              else
                              {
                                    chartTable[isearch].SetValid(false);
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
                  chartTable.Add(pnewChart);
                  nDirEntry++;
            }
            else
            {
                  wxString msg = _T("   Not adding chart file: ");
                  msg.Append(full_name);
                  wxLogMessage(msg);
            }
      }

      return nDirEntry;
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

      int nEntry = chartTable.GetCount();

      for(int i=0 ; i<nEntry ; i++)
      {
            if(GetChartFamily(chartTable[i].GetChartType()) == family)
            {
                  if(chartTable[i].GetScale() > cur_max_scale)
                  {
                        cur_max_scale = chartTable[i].GetScale();
                        cur_max_i = i;
                  }
            }
      }

      if(cur_max_i == -1)
            return false;                             // nothing found
      else
      {
            *clat = (chartTable[cur_max_i].GetLatMax() + chartTable[cur_max_i].GetLatMin()) / 2.;
            *clon = (chartTable[cur_max_i].GetLonMin() + chartTable[cur_max_i].GetLonMax()) /2.;
      }
      return true;
}

//-------------------------------------------------------------------
//    Get DBChart Projection
//-------------------------------------------------------------------
int ChartDatabase::GetDBChartProj(int dbIndex)
{
      if((bValid) && (dbIndex >= 0) && (dbIndex < (int)chartTable.size()))
            return chartTable[dbIndex].GetChartProjectionType();
      else
            return PROJECTION_UNKNOWN;
}

//-------------------------------------------------------------------
//    Get DBChart Family
//-------------------------------------------------------------------
int ChartDatabase::GetDBChartFamily(int dbIndex)
{
      if((bValid) && (dbIndex >= 0) && (dbIndex < (int)chartTable.size()))
            return chartTable[dbIndex].GetChartFamily();
      else
            return CHART_FAMILY_UNKNOWN;
}

//-------------------------------------------------------------------
//    Get DBChart FullFileName
//-------------------------------------------------------------------
wxString ChartDatabase::GetDBChartFileName(int dbIndex)
{
      if((bValid) && (dbIndex >= 0) && (dbIndex < (int)chartTable.size()))
      {
            return wxString(chartTable[dbIndex].GetpFullPath(), wxConvUTF8);
      }
      else
            return _T("");
}


//-------------------------------------------------------------------
//    Get DBChart Type
//-------------------------------------------------------------------
int ChartDatabase::GetDBChartType(int dbIndex)
{
      if((bValid) && (dbIndex >= 0) && (dbIndex < (int)chartTable.size()))
            return chartTable[dbIndex].GetChartType();
      else
            return 0;
}

//-------------------------------------------------------------------
//    Get DBChart Skew
//-------------------------------------------------------------------
float ChartDatabase::GetDBChartSkew(int dbIndex)
{
      if((bValid) && (dbIndex >= 0) && (dbIndex < (int)chartTable.size()))
            return chartTable[dbIndex].GetChartSkew();
      else
            return 0.;
}

//-------------------------------------------------------------------
//    Get DBChart Scale
//-------------------------------------------------------------------
int ChartDatabase::GetDBChartScale(int dbIndex)
{
      if((bValid) && (dbIndex >= 0) && (dbIndex < (int)chartTable.size()))
            return chartTable[dbIndex].GetScale();
      else
            return 1;
}

//-------------------------------------------------------------------
//    Get Lat/Lon Bounding Box from db
//-------------------------------------------------------------------
bool ChartDatabase::GetDBBoundingBox(int dbIndex, wxBoundingBox *box)
{
      if((bValid) && (dbIndex >= 0) && (dbIndex < (int)chartTable.size()))
      {
            const ChartTableEntry &entry = GetChartTableEntry(dbIndex);
            box->SetMax(entry.GetLonMax(), entry.GetLatMax());
            box->SetMin(entry.GetLonMin(), entry.GetLatMin());
      }

      return true;
}

//-------------------------------------------------------------------
//    Get PlyPoint from Database
//-------------------------------------------------------------------
int ChartDatabase::GetDBPlyPoint(int dbIndex, int plyindex, float *lat, float *lon)
{
      if((bValid) && (dbIndex >= 0) && (dbIndex < (int)chartTable.size()))
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
      if((bValid) && (dbIndex >= 0) && (dbIndex < (int)chartTable.size()))
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
      if((bValid) && (dbIndex >= 0) && (dbIndex < (int)chartTable.size()))
      {
            const ChartTableEntry &entry = GetChartTableEntry(dbIndex);
            return entry.GetnAuxPlyEntries();
      }
      else
            return 0;
}

