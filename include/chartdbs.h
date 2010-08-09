/******************************************************************************
* $Id: chartdbs.h,v 1.11 2010/05/27 19:00:26 bdbcat Exp $
*
* Project:  ChartManager
* Purpose:  Basic Chart Info Storage
* Author:   David S Register, Mark A Sikes
*
***************************************************************************
*   Copyright (C) 2010 by David S. Register *
*   $EMAIL$      *
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
* $Log: chartdbs.h,v $
* Revision 1.11  2010/05/27 19:00:26  bdbcat
* 527a
*
* Revision 1.10  2010/05/19 01:05:06  bdbcat
* Build 518
*
* Revision 1.9  2010/05/02 03:04:27  bdbcat
* Build 501
*
* Revision 1.8  2010/04/27 01:44:36  bdbcat
* Build 426
*
* Revision 1.7  2010/03/29 03:31:16  bdbcat
* 2.1.0 Beta Initial
*
* Revision 1.6  2010/03/29 03:18:13  bdbcat
* 2.1.0 Beta Initial
*
* Revision 1.4  2010/02/09 01:58:44  badfeed
* fix a few case-sensitivity problems, particularly with cm93
*
* Revision 1.3  2010/01/02 02:15:35  bdbcat
* Correct to disallow multiple same chart additions
*
* Revision 1.2  2009/12/17 02:45:00  bdbcat
* Beta 1216
*
* Revision 1.1  2009/12/10 21:21:20  bdbcat
* Beta 1210
*
*/

#ifndef __CHARTDBS_H__
#define __CHARTDBS_H__

#include "wx/dynarray.h"
#include "wx/file.h"
#include "wx/stream.h"
#include "wx/wfstream.h"
#include "wx/tokenzr.h"
#include "wx/dir.h"
#include "wx/filename.h"

#include "chartbase.h"

class wxProgressDialog;
///////////////////////////////////////////////////////////////////////

static const int DB_VERSION_PREVIOUS = 15;
static const int DB_VERSION_CURRENT = 16;

class ChartDatabase;

struct ChartTableEntry_onDisk_16
{
      int         EntryOffset;
      int         ChartType;
      float       LatMax;
      float       LatMin;
      float       LonMax;
      float       LonMin;

      int         Scale;
      int         edition_date;
      int         file_date;

      int         nPlyEntries;
      int         nAuxPlyEntries;

      float       skew;
      int         ProjectionType;
      bool        bValid;
};


struct ChartTableEntry_onDisk_15
{
    int         EntryOffset;
    int         ChartType;
    float       LatMax;
    float       LatMin;
    float       LonMax;
    float       LonMin;

    int         Scale;
    time_t      edition_date;
    time_t      file_date;

    int         nPlyEntries;
    int         nAuxPlyEntries;

    bool        bValid;
};

struct ChartTableEntry_onDisk_14
{
    int         EntryOffset;
    int         ChartType;
    char        ChartID[16];
    float       LatMax;
    float       LatMin;
    float       LonMax;
    float       LonMin;
    char        *pFullPath;
    int         Scale;
    time_t      edition_date;
    float       *pPlyTable;
    int         nPlyEntries;
    int         nAuxPlyEntries;
    float       **pAuxPlyTable;
    int         *pAuxCntTable;
    bool        bValid;
};

struct ChartTableHeader
{
    ChartTableHeader() {}
    ChartTableHeader(int dirEntries, int tableEntries) :
                nTableEntries(tableEntries), nDirEntries(dirEntries) {}

    void Read(wxInputStream &is);
    void Write(wxOutputStream &os);
    bool CheckValid();
    int GetDirEntries() const { return nDirEntries; }
    int GetTableEntries() const { return nTableEntries; }
    char *GetDBVersionString(){ return dbVersion; }

private:
    // NOTE: on-disk structure - cannot add, remove, or reorder!
    char dbVersion[4];
    int nTableEntries;
    int nDirEntries;
};

struct ChartTableEntry
{
    ChartTableEntry() { Clear(); }
    ChartTableEntry(ChartBase &theChart);
    ~ChartTableEntry();

    bool IsEqualToOrEarlierThan(const ChartTableEntry &cte) const;
    bool Read(const ChartDatabase *pDb, wxInputStream &is);
    bool Write(const ChartDatabase *pDb, wxOutputStream &os);
    void Clear();
    void Disable();
    void SetValid(bool valid) { bValid = valid; }
    time_t GetFileTime() const { return file_date; }
    int GetnAuxPlyEntries() const { return nAuxPlyEntries; }
    int GetnPlyEntries() const { return nPlyEntries; }
    float *GetpPlyTable() const { return pPlyTable; }
    float *GetpAuxPlyTableEntry(int index) const { return pAuxPlyTable[index];}
    int GetAuxCntTableEntry(int index) const { return pAuxCntTable[index];}

    char *GetpFullPath() const { return pFullPath; }
    float GetLonMax() const { return LonMax; }
    float GetLonMin() const { return LonMin; }
    float GetLatMax() const { return LatMax; }
    float GetLatMin() const { return LatMin; }
    int GetScale() const { return Scale; }
    int GetChartType() const;
    int GetChartProjectionType() const { return ProjectionType; }
    float GetChartSkew() const { return Skew; }

    bool GetbValid(){ return bValid;}
    void SetEntryOffset(int n) { EntryOffset = n;}

  private:
    int         EntryOffset;
    int         ChartType;
    float       LatMax;
    float       LatMin;
    float       LonMax;
    float       LonMin;
    char        *pFullPath;
    int         Scale;
    time_t      edition_date;
    time_t      file_date;
    float       *pPlyTable;
    int         nPlyEntries;
    int         nAuxPlyEntries;
    float       **pAuxPlyTable;
    int         *pAuxCntTable;
    float       Skew;
    int         ProjectionType;
    bool        bValid;
};

///////////////////////////////////////////////////////////////////////
// Chart Database
///////////////////////////////////////////////////////////////////////

WX_DECLARE_OBJARRAY(ChartTableEntry, ChartTable);

class ChartDatabase
{
public:
    ChartDatabase();
    virtual ~ChartDatabase(){};

    bool Create(ArrayOfCDI& dir_array, wxProgressDialog *pprog);
    bool Update(ArrayOfCDI& dir_array, bool bForce, wxProgressDialog *pprog);

    bool Read(const wxString &filePath);
    bool Write(const wxString &filePath);

    int GetChartTableEntries() const { return chartTable.size(); }
    const ChartTableEntry &GetChartTableEntry(int index) const;

    bool IsValid() const { return bValid; }
    int DisableChart(wxString& PathToDisable);
    bool GetCentroidOfLargestScaleChart(double *clat, double *clon, ChartFamilyEnum family);
    int GetDBChartType(int dbIndex);
    float GetDBChartSkew(int dbIndex);
    int GetDBChartProj(int dbIndex);
    int GetDBChartScale(int dbIndex);

    bool GetDBBoundingBox(int dbindex, wxBoundingBox *box);
    int  GetnAuxPlyEntries(int dbIndex);
    int  GetDBPlyPoint(int dbIndex, int plyindex, float *lat, float *lon);
    int  GetDBAuxPlyPoint(int dbIndex, int plyindex, int iAuxPly, float *lat, float *lon);
    int  GetVersion(){ return m_dbversion; }
    wxString GetFullChartInfo(ChartBase *pc, int dbIndex, int max_width);
    int FinddbIndex(wxString PathToFind);

protected:
    virtual ChartBase *GetChart(const wxChar *theFilePath) const;
    int AddChartDirectory(const wxString &theDir, bool bshow_prog);
    void SetValid(bool valid) { bValid = valid; }
    ChartTableEntry *CreateChartTableEntry(const wxString &filePath);

private:
    bool IsChartDirUsed(const wxString &theDir);
    int SearchDirAndAddCharts(wxString& dir, const wxString& filespec, wxProgressDialog *pprog);
    int TraverseDirAndAddCharts(ChartDirInfo& dir_info, wxProgressDialog *pprog, wxString& dir_magic, bool bForce);
    bool DetectDirChange(wxString dir_path, wxString magic, wxString &new_magic, wxProgressDialog *pprog);
    wxString Check_CM93_Structure(wxString dir_name);

    bool          bValid;
    wxArrayString chartDirs;
    int           m_dbversion;
    ChartTable    chartTable;

    ChartTableEntry m_ChartTableEntryDummy;           // used for return value if database is not valid
};

#endif


#ifndef __CHARTDBS_H__
#define __CHARTDBS_H__

#include "wx/dynarray.h"
#include "wx/file.h"
#include "wx/stream.h"
#include "wx/wfstream.h"
#include "wx/tokenzr.h"
#include "wx/dir.h"
#include "wx/filename.h"

#include "chartbase.h"

class wxProgressDialog;
///////////////////////////////////////////////////////////////////////

static const int DB_VERSION_PREVIOUS = 14;
static const int DB_VERSION_CURRENT = 15;

class ChartDatabase;

struct ChartTableEntry_onDisk_15
{
    int         EntryOffset;
    int         ChartType;
    float       LatMax;
    float       LatMin;
    float       LonMax;
    float       LonMin;
//    char        *pFullPath;
    int         Scale;
    time_t      edition_date;
    time_t      file_date;
//    float       *pPlyTable;
    int         nPlyEntries;
    int         nAuxPlyEntries;
//    float       **pAuxPlyTable;
//    int         *pAuxCntTable;
    bool        bValid;
};

struct ChartTableEntry_onDisk_14
{
    int         EntryOffset;
    int         ChartType;
    char        ChartID[16];
    float       LatMax;
    float       LatMin;
    float       LonMax;
    float       LonMin;
    char        *pFullPath;
    int         Scale;
    time_t      edition_date;
    float       *pPlyTable;
    int         nPlyEntries;
    int         nAuxPlyEntries;
    float       **pAuxPlyTable;
    int         *pAuxCntTable;
    bool        bValid;
};

struct ChartTableHeader
{
    ChartTableHeader() {}
    ChartTableHeader(int dirEntries, int tableEntries) :
                nTableEntries(tableEntries), nDirEntries(dirEntries) {}

    void Read(wxInputStream &is);
    void Write(wxOutputStream &os);
    bool CheckValid();
    int GetDirEntries() const { return nDirEntries; }
    int GetTableEntries() const { return nTableEntries; }
    char *GetDBVersionString(){ return dbVersion; }

private:
    // NOTE: on-disk structure - cannot add, remove, or reorder!
    char dbVersion[4];
    int nTableEntries;
    int nDirEntries;
};

class ChartTableEntry
{
    ChartTableEntry() { Clear(); }
    ChartTableEntry(ChartBase &theChart);
    ~ChartTableEntry();

    bool IsEqualToOrEarlierThan(const ChartTableEntry &cte) const;
    bool Read(const ChartDatabase *pDb, wxInputStream &is);
    bool Write(const ChartDatabase *pDb, wxOutputStream &os);
    void Clear();
    void Disable();
    void SetValid(bool valid) { bValid = valid; }
    time_t GetFileTime() const { return file_date; }
    int GetnAuxPlyEntries() const { return nAuxPlyEntries; }
    int GetnPlyEntries() const { return nPlyEntries; }
    float *GetpPlyTable() const { return pPlyTable; }
    float *GetpAuxPlyTableEntry(int index) const { return pAuxPlyTable[index];}
    int GetAuxCntTableEntry(int index) const { return pAuxCntTable[index];}

    char *GetpFullPath() const { return pFullPath; }
    float GetLonMax() const { return LonMax; }
    float GetLonMin() const { return LonMin; }
    float GetLatMax() const { return LatMax; }
    float GetLatMin() const { return LatMin; }
    int GetScale() const { return Scale; }
    int GetChartType() const;
    bool GetbValid() const { return bValid;}
    void SetEntryOffset(int n) { EntryOffset = n;}

  private:
    int         EntryOffset;
    int         ChartType;
    float       LatMax;
    float       LatMin;
    float       LonMax;
    float       LonMin;
    char        *pFullPath;
    int         Scale;
    time_t      edition_date;
    time_t      file_date;
    float       *pPlyTable;
    int         nPlyEntries;
    int         nAuxPlyEntries;
    float       **pAuxPlyTable;
    int         *pAuxCntTable;
    bool        bValid;
};

///////////////////////////////////////////////////////////////////////
// Chart Database
///////////////////////////////////////////////////////////////////////

WX_DECLARE_OBJARRAY(ChartTableEntry, ChartTable);

class ChartDatabase
{
public:
    bool Create(ArrayOfCDI& dir_array, wxProgressDialog *pprog);
    bool Update(ArrayOfCDI& dir_array, bool bForce, wxProgressDialog *pprog);

    bool Read(const wxString &filePath);
    bool Write(const wxString &filePath);

    int GetChartTableEntries() const { return chartTable.size(); }
    const ChartTableEntry &GetChartTableEntry(int index) const { return chartTable[index]; }

    bool IsValid() const { return bValid; }
    int DisableChart(wxString& PathToDisable);
    bool GetCentroidOfLargestScaleChart(double *clat, double *clon, ChartFamilyEnum family);
    int GetDBChartType(int dbIndex);
    bool GetDBBoundingBox(int dbindex, wxBoundingBox *box);
    int  GetnAuxPlyEntries(int dbIndex);
    int  GetDBPlyPoint(int dbIndex, int plyindex, float *lat, float *lon);
    int  GetDBAuxPlyPoint(int dbIndex, int plyindex, int iAuxPly, float *lat, float *lon);
    int  GetVersion(){ return m_dbversion; }
protected:
    virtual ChartBase *GetChart(const wxChar *theFilePath) const;
    int AddChartDirectory(const wxString &theDir, bool bshow_prog);
    void SetValid(bool valid) { bValid = valid; }
    ChartTableEntry *CreateChartTableEntry(const wxString &filePath);

private:
    bool IsChartDirUsed(const wxString &theDir);
    int SearchDirAndAddCharts(wxString& dir, const wxString& filespec, wxProgressDialog *pprog);
    int TraverseDirAndAddCharts(ChartDirInfo& dir_info, wxProgressDialog *pprog, wxString& dir_magic, bool bForce);
    bool DetectDirChange(wxString dir_path, wxString magic, wxString &new_magic, wxProgressDialog *pprog);
    wxString Check_CM93_Structure(wxString dir_name);

    bool          bValid;
    wxArrayString chartDirs;
    int           m_dbversion;
    ChartTable    chartTable;
};

#endif

