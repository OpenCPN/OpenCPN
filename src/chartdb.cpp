/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Chart Database Object
 * Author:   David Register, Mark A Sikes
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/


// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
      #include "wx/wx.h"
#endif //precompiled headers

#include <wx/stopwatch.h>
#include <wx/regex.h>
#include "wx/tokenzr.h"
#include "wx/dir.h"

#include "chartdb.h"
#include "chartimg.h"
#include "chart1.h"
#include "thumbwin.h"

#ifdef ocpnUSE_GL
#include "glChartCanvas.h"
#endif

#include <stdio.h>
#include <math.h>

#include <wx/progdlg.h>

#include "chcanv.h"

#ifdef USE_S57
#include "s57chart.h"
#include "cm93.h"
#endif

extern ColorScheme GetColorScheme();

class s52plib;

extern ChartBase    *Current_Ch;
extern ThumbWin     *pthumbwin;
extern int          g_nCacheLimit;
extern int          g_memCacheLimit;
extern bool         g_bopengl;
extern ChartCanvas  *cc1;
extern int          g_GroupIndex;
extern s52plib      *ps52plib;
extern ChartDB      *ChartData;

bool G_FloatPtInPolygon(MyFlPoint *rgpts, int wnumpts, float x, float y) ;
bool GetMemoryStatus(int *mem_total, int *mem_used);


// ============================================================================
// ChartStack implementation
// ============================================================================

int ChartStack::GetCurrentEntrydbIndex(void)
{
      if(nEntry /*&& b_valid*/)
            return DBIndex[CurrentStackEntry];
      else
            return -1;
}

void ChartStack::SetCurrentEntryFromdbIndex(int current_db_index)
{
      for(int i=0 ; i < nEntry ; i++)
      {
            if(current_db_index == DBIndex[i])
                  CurrentStackEntry = i;
      }
}

int ChartStack::GetDBIndex(int stack_index)
{
      if((stack_index >= 0) && (stack_index < nEntry) && (stack_index < MAXSTACK))
            return DBIndex[stack_index];
      else
            return -1;
}

void ChartStack::SetDBIndex(int stack_index, int db_index)
{
      if((stack_index >= 0) && (stack_index < nEntry) && (stack_index < MAXSTACK))
            DBIndex[stack_index] = db_index;
}


bool ChartStack::DoesStackContaindbIndex(int db_index)
{
      for(int i=0 ; i < nEntry ; i++)
      {
            if(db_index == DBIndex[i])
                  return true;
      }

      return false;
}


void ChartStack::AddChart( int db_add )
{
    if( !ChartData ) return;
    
    if( !ChartData->IsValid() ) return;
    
    int db_index = db_add;
 
    int j = nEntry;
    
    if(db_index >= 0) {
         j++;
        nEntry = j;
        SetDBIndex(j-1, db_index);
    }
             //    Remove exact duplicates, i.e. charts that have exactly the same file name and 
            //     nearly the same mod time.
            //    These charts can be in the database due to having the exact same chart in different directories,
            //    as may be desired for some grouping schemes
            //    Note that if the target name is actually a directory, then windows fails to produce a valid
            //    file modification time.  Detect GetFileTime() == 0, and skip the test in this case     
            for(int id = 0 ; id < j-1 ; id++)
            {
                if(GetDBIndex(id) != -1)
                {
                    ChartTableEntry *pm = ChartData->GetpChartTableEntry(GetDBIndex(id));
                    
                    for(int jd = id+1; jd < j; jd++)
                    {
                        if(GetDBIndex(jd) != -1)
                        {
                            ChartTableEntry *pn = ChartData->GetpChartTableEntry(GetDBIndex(jd));
                            if( pm->GetFileTime() && pn->GetFileTime()) {
                                if( labs(pm->GetFileTime() - pn->GetFileTime()) < 60 ) {           // simple test
                                    if(pn->GetpFileName()->IsSameAs(*(pm->GetpFileName())))
                                        SetDBIndex(jd, -1);           // mark to remove
                                }
                            }
                        }
                    }
                }
            }
            
            int id = 0;
            while( (id < j) )
            {
                if(GetDBIndex(id) == -1)
                {
                    int jd = id+1;
                    while( jd < j )
                    {
                        int db_index = GetDBIndex(jd);
                        SetDBIndex(jd-1, db_index);
                        jd++;
                    }
                    
                    j--;
                    nEntry = j;
                    
                    id = 0;
                }
                else
                    id++;
            }
            
            
            
            
            
            
            //    Sort the stack on scale
            int swap = 1;
            int ti;
            while(swap == 1)
            {
                swap = 0;
                for(int i=0 ; i<j-1 ; i++)
                {
                    const ChartTableEntry &m = ChartData->GetChartTableEntry(GetDBIndex(i));
                    const ChartTableEntry &n = ChartData->GetChartTableEntry(GetDBIndex(i+1));
                    
                    
                    if(n.GetScale() < m.GetScale())
                    {
                        ti = GetDBIndex(i);
                        SetDBIndex(i, GetDBIndex(i+1));
                        SetDBIndex(i+1, ti);
                        swap = 1;
                    }
                }
            }
            
    
}


// ============================================================================
// ChartDB implementation
// ============================================================================

ChartDB::ChartDB()
{
      pChartCache = new wxArrayPtrVoid;

      SetValid(false);                           // until loaded or created
      UnLockCache();
      
      m_b_busy = false;
      m_ticks = 0;

      //    Report cache policy
      if(g_memCacheLimit)
      {
            wxString msg;
            msg.Printf(_T("ChartDB Cache policy:  Application target is %d MBytes"), g_memCacheLimit / 1024);
            wxLogMessage(msg);
      }
      else
      {
            wxString msg;
            msg.Printf(_T("ChartDB Cache policy:  Max open chart limit is %d."), g_nCacheLimit);
            wxLogMessage(msg);
      }

}


ChartDB::~ChartDB()
{
//    Empty the cache
      PurgeCache();

      delete pChartCache;
}

bool ChartDB::LoadBinary(const wxString & filename, ArrayOfCDI& dir_array_check)
{
      m_dir_array = dir_array_check;
      return ChartDatabase::Read(filename);

      // Check chartDirs against dir_array_check
}

void ChartDB::DeleteCacheEntry(CacheEntry *pce, bool bDelTexture, const wxString &msg)
{
     ChartBase *ch = (ChartBase *)pce->pChart;

     if (msg != wxEmptyString) {
         wxLogMessage(_T("%s%s"), msg.c_str(), ch->GetFullPath().c_str());
     }

     // If this chart should happen to be in the thumbnail window....
     if(pthumbwin)
     {
         if (pthumbwin->pThumbChart == ch)
            pthumbwin->pThumbChart = NULL;
     }

#ifdef ocpnUSE_GL
     // The glCanvas may be cacheing some information for this chart     
     if (g_bopengl && cc1)
         g_glTextureManager->PurgeChartTextures(ch, bDelTexture);
#endif

     pChartCache->Remove(pce);
     delete ch;
     delete pce;
}

void ChartDB::DeleteCacheEntry(int i, bool bDelTexture, const wxString &msg)
{
     CacheEntry *pce = (CacheEntry *)(pChartCache->Item(i));
     if (pce)
         DeleteCacheEntry(pce, bDelTexture, msg);
}

void ChartDB::PurgeCache()
{
//    Empty the cache
      wxLogMessage(_T("Chart cache purge"));

      if( wxMUTEX_NO_ERROR == m_cache_mutex.Lock() ){
        unsigned int nCache = pChartCache->GetCount();
        for(unsigned int i=0 ; i<nCache ; i++)
        {
               DeleteCacheEntry(0, true);
        }
        pChartCache->Clear();
        
        m_cache_mutex.Unlock();
      }
}


void ChartDB::PurgeCachePlugins()
{
    //    Empty the cache
    wxLogMessage(_T("Chart cache PlugIn purge"));
    
    if( wxMUTEX_NO_ERROR == m_cache_mutex.Lock() ){
        unsigned int nCache = pChartCache->GetCount();
        unsigned int i = 0;
        while(i < nCache){
            CacheEntry *pce = (CacheEntry *)(pChartCache->Item(i));
            ChartBase *Ch = (ChartBase *)pce->pChart;

            if(CHART_TYPE_PLUGIN == Ch->GetChartType()){
                DeleteCacheEntry(pce, true);
            
                nCache = pChartCache->GetCount();       // restart the while loop
                i = 0;
                
            }
            else
                i++;
        }
       
        m_cache_mutex.Unlock();
    }
}


void ChartDB::ClearCacheInUseFlags(void)
{
    if( wxMUTEX_NO_ERROR == m_cache_mutex.Lock() ){
        unsigned int nCache = pChartCache->GetCount();
        for(unsigned int i=0 ; i<nCache ; i++)
        {
            CacheEntry *pce = (CacheEntry *)(pChartCache->Item(i));
            pce->b_in_use = false;
        }
        m_cache_mutex.Unlock();
    }
}


//      Try to purge and delete charts from the cache until the application memory used is
//      until the application memory used is less than {factor * Limit}
//      Purge charts on LRU policy
void ChartDB::PurgeCacheUnusedCharts( double factor)
{
      //    Use memory limited cache policy, if defined....
      if(g_memCacheLimit)
      {
          if( wxMUTEX_NO_ERROR == m_cache_mutex.TryLock() ){
              
            //    Check memory status to see if above limit
                int mem_used;
                GetMemoryStatus(0, &mem_used);
                int mem_limit = g_memCacheLimit * factor;

                int nl = pChartCache->GetCount();       // max loop count, by definition
                    
                wxString msg(_T("Purging unused chart from cache: "));
                while( (mem_used > mem_limit) && (nl>0) )
                {
                    if( pChartCache->GetCount() < 2 ){
                        nl = 0;
                        break;
                    }
                    
                    CacheEntry *pce = FindOldestDeleteCandidate( false );
                    if(pce){
                        // don't purge background spooler
                        DeleteCacheEntry(pce, false /*true*/, msg);

                    }
                    else {
                        break;
                    }
                    
                    GetMemoryStatus(0, &mem_used);
                    
                    nl--;
                }
        }
        m_cache_mutex.Unlock();
      }
}




//-------------------------------------------------------------------------------------------------------
//      Create a Chart
//      This version creates a fully functional UI-capable chart.
//-------------------------------------------------------------------------------------------------------

ChartBase *ChartDB::GetChart(const wxChar *theFilePath, ChartClassDescriptor &chart_desc) const
{
      wxFileName fn(theFilePath);


      if(!fn.FileExists()) {
            //    Might be a directory
            if( !wxDir::Exists(theFilePath) ) {
                  wxLogMessage(wxT("   ...file does not exist: %s"), theFilePath);
                  return NULL;
            }
      }
      ChartBase *pch = NULL;

      wxString chartExt = fn.GetExt().Upper();

      if (chartExt == wxT("KAP")) {
            pch = new ChartKAP;
      }
      else if (chartExt == wxT("GEO")) {
            pch = new ChartGEO;
      }
#ifdef USE_S57
      else if (chartExt == wxT("000") || chartExt == wxT("S57")) {
            LoadS57();
            pch = new s57chart;
      }
#endif
      else if (chart_desc.m_descriptor_type == PLUGIN_DESCRIPTOR) {
            LoadS57();
            ChartPlugInWrapper *cpiw = new ChartPlugInWrapper(chart_desc.m_class_name);
            pch = (ChartBase *)cpiw;
      }

#ifdef USE_S57
      else
      {
            wxRegEx rxName(wxT("[0-9]+"));
            wxRegEx rxExt(wxT("[A-G]"));
            if (rxName.Matches(fn.GetName()) && rxExt.Matches(chartExt))
                  pch = new cm93compchart;
            else {
            //    Might be a directory
                  if( wxDir::Exists(theFilePath) )
                        pch = new cm93compchart;
            }
      }
#endif

      return pch;
}

//      Build a Chart Stack, and add the indicated chart to the stack, even if the chart does not
//      cover the lat/lon specification

int ChartDB::BuildChartStack(ChartStack * cstk, float lat, float lon, int db_add )
{
    BuildChartStack(cstk, lat, lon);
    
    if (db_add >= 0 )
        cstk->AddChart( db_add );
    
    return cstk->nEntry;
}


int ChartDB::BuildChartStack(ChartStack * cstk, float lat, float lon)
{
      int i=0;
      int j=0;

      if(!IsValid())
            return 0;                           // Database is not properly initialized

      if(!cstk)
            return 0;                           // Chartstack not ready yet

      int nEntry = GetChartTableEntries();

      for(int db_index=0 ; db_index<nEntry ; db_index++)
      {

            const ChartTableEntry &cte = GetChartTableEntry(db_index);
            
            //    Check to see if the candidate chart is in the currently active group
            bool b_group_add = false;
            if(g_GroupIndex > 0)
            {
                  const int ng = cte.GetGroupArray().GetCount();
                  for(int ig=0 ; ig < ng; ig++)
                  {
                        if(g_GroupIndex == cte.GetGroupArray().Item(ig))
                        {
                              b_group_add = true;
                              break;
                        }
                  }
            }
            else
                  b_group_add = true;

            bool b_pos_add = false;
            if(b_group_add)
            {
                  if(CheckPositionWithinChart(db_index, lat, lon)  &&  (j < MAXSTACK) )
                      b_pos_add = true;

                  //    Check the special case where chart spans the international dateline
                  else if( (cte.GetLonMax() > 180.) && (cte.GetLonMin() < 180.) )
                  {
                        if(CheckPositionWithinChart(db_index, lat, lon + 360.)  &&  (j < MAXSTACK) )
                            b_pos_add = true;
                  }
                  //    Western hemisphere, some type of charts
                  else if( (cte.GetLonMax() > 180.) && (cte.GetLonMin() > 180.) )       
                  {
                      if(CheckPositionWithinChart(db_index, lat, lon + 360.)  &&  (j < MAXSTACK) )
                          b_pos_add = true;
                  }
                  
                  
            }
            
            bool b_available = true;
            //  Verify PlugIn charts are actually available
            if(b_group_add && b_pos_add && (cte.GetChartType() == CHART_TYPE_PLUGIN)){
                
                ChartTableEntry *pcte = (ChartTableEntry*)&cte;
                if( !IsChartAvailable(db_index) ){
                    pcte->SetAvailable(false);
                    b_available = false;
                }
                else
                    pcte->SetAvailable(true);
            }
            
            if(b_group_add && b_pos_add && b_available){                // add it
                j++;
                cstk->nEntry = j;
                cstk->SetDBIndex(j-1, db_index);
            }
      }

      cstk->nEntry = j;


//    Remove exact duplicates, i.e. charts that have exactly the same file name and nearly the same mod time
//    These charts can be in the database due to having the exact same chart in different directories,
//    as may be desired for some grouping schemes
//    Note that if the target name is actually a directory, then windows fails to produce a valid
//    file modification time.  Detect GetFileTime() == 0, and skip the test in this case     
      for(int id = 0 ; id < j-1 ; id++)
      {
            if(cstk->GetDBIndex(id) != -1)
            {
                  const ChartTableEntry &ctem = GetChartTableEntry(cstk->GetDBIndex(id));
                  
                  for(int jd = id+1; jd < j; jd++)
                  {
                        if(cstk->GetDBIndex(jd) != -1)
                        {
                              const ChartTableEntry &cten = GetChartTableEntry(cstk->GetDBIndex(jd));
                              if( ctem.GetFileTime() && cten.GetFileTime()) {
                                    if( labs(ctem.GetFileTime() - cten.GetFileTime()) < 60 ) {           // simple test
                                        if(cten.GetpFileName()->IsSameAs(*(ctem.GetpFileName())))
                                           cstk->SetDBIndex(jd, -1);           // mark to remove
                                    }
                              }
                        }
                  }
            }
      }

      int id = 0;
      while( (id < j) )
      {
            if(cstk->GetDBIndex(id) == -1)
            {
                  int jd = id+1;
                  while( jd < j )
                  {
                        int db_index = cstk->GetDBIndex(jd);
                        cstk->SetDBIndex(jd-1, db_index);
                        jd++;
                  }

                  j--;
                  cstk->nEntry = j;

                  id = 0;
            }
            else
                  id++;
      }






//    Sort the stack on scale
      int swap = 1;
      int ti;
      while(swap == 1)
      {
            swap = 0;
            for(i=0 ; i<j-1 ; i++)
            {
                  const ChartTableEntry &m = GetChartTableEntry(cstk->GetDBIndex(i));
                  const ChartTableEntry &n = GetChartTableEntry(cstk->GetDBIndex(i+1));


                  if(n.GetScale() < m.GetScale())
                  {
                        ti = cstk->GetDBIndex(i);
                        cstk->SetDBIndex(i, cstk->GetDBIndex(i+1));
                        cstk->SetDBIndex(i+1, ti);
                        swap = 1;
                  }
            }
      }



      cstk->b_valid = true;

      return j;
}

bool ChartDB::IsChartInGroup(const int db_index, const int group)
{
      ChartTableEntry *pt = (ChartTableEntry *)&GetChartTableEntry(db_index);

      //    Check to see if the candidate chart is in the designated group
      bool b_in_group = false;
      if(group > 0)
      {
            for(unsigned int ig=0 ; ig < pt->GetGroupArray().GetCount(); ig++)
            {
                  if(group == pt->GetGroupArray().Item(ig))
                  {
                        b_in_group = true;
                        break;
                  }
            }
      }
      else
            b_in_group = true;

      return b_in_group;
}


//-------------------------------------------------------------------
//    Check to see it lat/lon is within a database chart at index
//-------------------------------------------------------------------
bool ChartDB::CheckPositionWithinChart(int index, float lat, float lon)
{
            const ChartTableEntry *pt = &GetChartTableEntry(index);

//    First check on rough Bounding box

            if((lat <= pt->GetLatMax()) &&
                (lat >= pt->GetLatMin()) &&
                (lon >= pt->GetLonMin()) &&
                (lon <= pt->GetLonMax()))
            {
//    Double check on Primary Ply points polygon

                  bool bInside = G_FloatPtInPolygon((MyFlPoint *)pt->GetpPlyTable(),
                              pt->GetnPlyEntries(),
                              lon, lat);

                  if(bInside )
                  {
                        if(pt->GetnAuxPlyEntries())
                        {
                              for(int k=0 ; k<pt->GetnAuxPlyEntries() ; k++)
                              {
                                    bool bAuxInside = G_FloatPtInPolygon((MyFlPoint *)pt->GetpAuxPlyTableEntry(k),
                                                                                    pt->GetAuxCntTableEntry(k),lon, lat);
                                    if(bAuxInside)
                                          return true;;
                              }

                        }
                        else
                              return true;
                  }
            }

            return false;
}





//-------------------------------------------------------------------
//    Compare Chart Stacks
//-------------------------------------------------------------------
bool ChartDB::EqualStacks(ChartStack *pa, ChartStack *pb)
{
      if((pa == 0) || (pb == 0))
            return false;
      if((!pa->b_valid) || (!pb->b_valid))
            return false;
      if(pa->nEntry != pb->nEntry)
            return false;

      for(int i=0 ; i<pa->nEntry ; i++)
      {
            if(pa->GetDBIndex(i) != pb->GetDBIndex(i))
                  return false;
      }

      return true;
}

//-------------------------------------------------------------------
//    Copy Chart Stacks
//-------------------------------------------------------------------
bool ChartDB::CopyStack(ChartStack *pa, ChartStack *pb)
{
      if((pa == 0) || (pb == 0))
            return false;
      pa->nEntry = pb->nEntry;

      for(int i=0 ; i<pa->nEntry ; i++)
            pa->SetDBIndex(i, pb->GetDBIndex(i));

      pa->CurrentStackEntry = pb->CurrentStackEntry;

      pa->b_valid = pb->b_valid;


      return true;
}


wxString ChartDB::GetFullPath(ChartStack *ps, int stackindex)
{
      int dbIndex = ps->GetDBIndex(stackindex);
      return wxString(GetChartTableEntry(dbIndex).GetpFullPath(),  wxConvUTF8);
}

//-------------------------------------------------------------------
//    Get PlyPoint from stack
//-------------------------------------------------------------------

int ChartDB::GetCSPlyPoint(ChartStack *ps, int stackindex, int plyindex, float *lat, float *lon)
{
      const ChartTableEntry &entry = GetChartTableEntry(ps->GetDBIndex(stackindex));
      if(entry.GetnPlyEntries())
      {
            float *fp = entry.GetpPlyTable();
            fp += plyindex*2;
            *lat = *fp;
            fp++;
            *lon = *fp;
      }

      return entry.GetnPlyEntries();
}




//-------------------------------------------------------------------
//    Get Chart Scale
//-------------------------------------------------------------------
int ChartDB::GetStackChartScale(ChartStack *ps, int stackindex, char *buf, int nbuf)
{
      const ChartTableEntry &entry = GetChartTableEntry(ps->GetDBIndex(stackindex));
      int sc = entry.GetScale();
      if(buf)
            sprintf(buf, "%d", sc);

      return sc;
}

//-------------------------------------------------------------------
//    Find ChartStack entry index corresponding to Full Path name, if present
//-------------------------------------------------------------------
int ChartDB::GetStackEntry(ChartStack *ps, wxString fp)
{
      for(int i=0 ; i<ps->nEntry ; i++)
      {
            const ChartTableEntry &entry = GetChartTableEntry(ps->GetDBIndex(i));
            if(fp.IsSameAs( wxString(entry.GetpFullPath(),  wxConvUTF8)) )
                  return i;
      }

      return -1;
}

//-------------------------------------------------------------------
//    Get CSChart Type
//-------------------------------------------------------------------
ChartTypeEnum ChartDB::GetCSChartType(ChartStack *ps, int stackindex)
{
      if((IsValid()) && (stackindex >= 0) && (stackindex < GetChartTableEntries()))
            return (ChartTypeEnum)GetChartTableEntry(ps->GetDBIndex(stackindex)).GetChartType();
      else
            return CHART_TYPE_UNKNOWN;
}


ChartFamilyEnum ChartDB::GetCSChartFamily(ChartStack *ps, int stackindex)
{
      if((IsValid()) && (stackindex < GetChartTableEntries()))
      {
            const ChartTableEntry &entry = GetChartTableEntry(ps->GetDBIndex(stackindex));

            ChartTypeEnum type = (ChartTypeEnum)entry.GetChartType();
            switch(type)
            {
                  case  CHART_TYPE_KAP:          return CHART_FAMILY_RASTER;
                  case  CHART_TYPE_GEO:          return CHART_FAMILY_RASTER;
                  case  CHART_TYPE_S57:          return CHART_FAMILY_VECTOR;
                  case  CHART_TYPE_CM93:         return CHART_FAMILY_VECTOR;
                  case  CHART_TYPE_CM93COMP:     return CHART_FAMILY_VECTOR;
                  case  CHART_TYPE_DUMMY:        return CHART_FAMILY_RASTER;
                  default:                       return CHART_FAMILY_UNKNOWN;
            }
      }
      else
            return CHART_FAMILY_UNKNOWN;
}


ArrayOfInts ChartDB::GetCSArray(ChartStack *ps)
{
      ArrayOfInts ret;

      if(ps)
      {
            for(int i=0 ; i<ps->nEntry ; i++)
            {
                  ret.Add(ps->GetDBIndex(i));
            }
      }

      return ret;
}


bool ChartDB::IsChartInCache(int dbindex)
{
      bool bInCache = false;

//    Search the cache
      if( wxMUTEX_NO_ERROR == m_cache_mutex.Lock() ){
          
        unsigned int nCache = pChartCache->GetCount();
        for(unsigned int i=0 ; i<nCache ; i++)
        {
                CacheEntry *pce = (CacheEntry *)(pChartCache->Item(i));
                if(pce->dbIndex == dbindex)
                {
                    if (pce->pChart != 0 && ((ChartBase *)pce->pChart)->IsReadyToRender())
                        bInCache = true;
                    break;
                }
        }
        m_cache_mutex.Unlock();
      }
      
      return bInCache;
}


bool ChartDB::IsChartInCache(wxString path)
{
    bool bInCache = false;
    if( wxMUTEX_NO_ERROR == m_cache_mutex.Lock() ){
        
        //    Search the cache
        unsigned int nCache = pChartCache->GetCount();
        for(unsigned int i=0 ; i<nCache ; i++)
        {
            CacheEntry *pce = (CacheEntry *)(pChartCache->Item(i));
            if(pce->FullPath == path)
            {
                if (pce->pChart != 0 && ((ChartBase *)pce->pChart)->IsReadyToRender())
                    bInCache = true;
                break;
            }
        }
        
        m_cache_mutex.Unlock();
    }
    return bInCache;
}

bool ChartDB::IsChartLocked( int index )
{
    
    if( wxMUTEX_NO_ERROR == m_cache_mutex.Lock() ){
        unsigned int nCache = pChartCache->GetCount();
        for(unsigned int i=0 ; i<nCache ; i++)
        {
            CacheEntry *pce = (CacheEntry *)(pChartCache->Item(i));
            if(pce->dbIndex == index)
            {
                bool ret = pce->n_lock > 0;
                m_cache_mutex.Unlock();
                return ret;
            }
        }
        m_cache_mutex.Unlock();
    }
    
    return false;
}
    
bool ChartDB::LockCacheChart( int index )
{
    //    Search the cache
    bool ret = false;
    if( wxMUTEX_NO_ERROR == m_cache_mutex.Lock() ){
        
        unsigned int nCache = pChartCache->GetCount();
        for(unsigned int i=0 ; i<nCache ; i++)
        {
            CacheEntry *pce = (CacheEntry *)(pChartCache->Item(i));
            if(pce->dbIndex == index )
            {
                pce->n_lock++;
                ret = true;
                break;
            }
        }
        m_cache_mutex.Unlock();
    }
    return ret;
}

void ChartDB::UnLockCacheChart( int index )
{
    //    Search the cache
    if( wxMUTEX_NO_ERROR == m_cache_mutex.Lock() ){
        unsigned int nCache = pChartCache->GetCount();
        for(unsigned int i=0 ; i<nCache ; i++)
        {
            CacheEntry *pce = (CacheEntry *)(pChartCache->Item(i));
            if(pce->dbIndex == index)
            {
                if(pce->n_lock > 0)
                    pce->n_lock--;
                break;
            }
        }
        m_cache_mutex.Unlock();
    }
}

void ChartDB::UnLockAllCacheCharts()
{
    //    Walk the cache
    if( wxMUTEX_NO_ERROR == m_cache_mutex.Lock() ){
        
        unsigned int nCache = pChartCache->GetCount();
        for(unsigned int i=0 ; i<nCache ; i++)
        {
            CacheEntry *pce = (CacheEntry *)(pChartCache->Item(i));
                if(pce->n_lock > 0)
                    pce->n_lock--;
        }
        m_cache_mutex.Unlock();
    }
}


//-------------------------------------------------------------------
//    Open Chart
//-------------------------------------------------------------------
ChartBase *ChartDB::OpenChartFromDB(int index, ChartInitFlag init_flag)
{
    return OpenChartUsingCache(index, init_flag);
}

ChartBase *ChartDB::OpenChartFromDB(wxString chart_path, ChartInitFlag init_flag)
{
    int dbii = FinddbIndex( chart_path );
    return OpenChartUsingCache(dbii, init_flag);
}


ChartBase *ChartDB::OpenChartFromStack(ChartStack *pStack, int StackEntry, ChartInitFlag init_flag)
{
    return OpenChartUsingCache(pStack->GetDBIndex(StackEntry), init_flag);
}

ChartBase *ChartDB::OpenChartFromDBAndLock( int index, ChartInitFlag init_flag, bool lock )
{
    wxCriticalSectionLocker locker(m_critSect);
    ChartBase *pret = OpenChartUsingCache(index, init_flag);
    if (lock && pret)
        LockCacheChart( index );
    return pret;
}

ChartBase *ChartDB::OpenChartFromDBAndLock(wxString chart_path, ChartInitFlag init_flag)
{
    int dbii = FinddbIndex( chart_path );
    return OpenChartFromDBAndLock(dbii, init_flag);
}

CacheEntry *ChartDB::FindOldestDeleteCandidate( bool blog)
{
    CacheEntry *pret = 0;
    
         
        unsigned int nCache = pChartCache->GetCount();
        if(nCache > 1)
        {
            if(blog)
                wxLogMessage(_T("Searching chart cache for oldest entry"));
            int LRUTime = m_ticks;
            int iOldest = 0;
            for(unsigned int i=0 ; i<nCache ; i++)
            {
                CacheEntry *pce = (CacheEntry *)(pChartCache->Item(i));
                if((ChartBase *)(pce->pChart) != Current_Ch)
                {
                    if(pce->RecentTime < LRUTime && !pce->n_lock)
                    {
                        LRUTime = pce->RecentTime;
                        iOldest = i;
                    }
                }
            }
            int dt = m_ticks - LRUTime;

            CacheEntry *pce = (CacheEntry *)(pChartCache->Item(iOldest));
            ChartBase *pDeleteCandidate =  (ChartBase *)(pce->pChart);
                
            if( (!pce->n_lock) && (Current_Ch != pDeleteCandidate) ){
                if(blog)
                    wxLogMessage(_T("Oldest unlocked cache index is %d, delta t is %d"), iOldest, dt);
                
                pret = pce;
            }
        }
        
    
    return pret;
}



ChartBase *ChartDB::OpenChartUsingCache(int dbindex, ChartInitFlag init_flag)
{
      if((dbindex < 0) || (dbindex > GetChartTableEntries()-1))
            return NULL;

//      printf("Opening chart %d   lock: %d\n", dbindex, m_b_locked);

      const ChartTableEntry &cte = GetChartTableEntry(dbindex);
      wxString ChartFullPath(cte.GetpFullPath(), wxConvUTF8 );
      ChartTypeEnum chart_type = (ChartTypeEnum)cte.GetChartType();
      ChartFamilyEnum chart_family = (ChartFamilyEnum)cte.GetChartFamily();
      
      ChartBase *Ch = NULL;
      CacheEntry *pce = NULL;
      int old_lock = 0;

      bool bInCache = false;
      m_ticks++;

//    Search the cache

      if( wxMUTEX_NO_ERROR == m_cache_mutex.Lock() ){
        unsigned int nCache = pChartCache->GetCount();
        for(unsigned int i=0 ; i<nCache ; i++)
        {
                pce = (CacheEntry *)(pChartCache->Item(i));
                if(pce->FullPath == ChartFullPath)
                {
                    Ch = (ChartBase *)pce->pChart;
                    bInCache = true;
                    break;
                }
        }
        m_cache_mutex.Unlock();
      }
      

      if(bInCache)
      {
          if(FULL_INIT == init_flag)                            // asking for full init?
          {
              if(Ch->IsReadyToRender())
              {
                    if(pce){
                        pce->RecentTime = m_ticks;           // chart is OK
                        pce->b_in_use = true;
                    }
                    return Ch;
              }
              else
              {
                    if(pthumbwin && pthumbwin->pThumbChart == Ch)
                       pthumbwin->pThumbChart = NULL;
                    delete Ch;                                  // chart is not useable
                    // but may be locked in quilt
                    if( wxMUTEX_NO_ERROR == m_cache_mutex.Lock() ){
                        old_lock = pce->n_lock;
                        pChartCache->Remove(pce);                   // so remove it
                        delete pce;
                        m_cache_mutex.Unlock();
                    }
                    // XXX and if there's a mutex error?
                        
                    bInCache = false;
              }
          }
          else                                                  // assume if in cache, the chart can do thumbnails
          {
               if(pce){
                   pce->RecentTime = m_ticks;
                   pce->b_in_use = true;
               }
               return Ch;
          }
      }

      if(!bInCache)                    // not in cache
      {
          m_b_busy = true;
          if( !m_b_locked && wxMUTEX_NO_ERROR == m_cache_mutex.Lock() ){
              
            
                //    Use memory limited cache policy, if defined....
                if(g_memCacheLimit)
                {

            //    Check memory status to see if enough room to open another chart
                    int mem_used;
                    GetMemoryStatus(0, &mem_used);
    //                  printf(" ChartdB Mem_total: %d  mem_used: %d  lock: %d\n", mem_total, mem_used, m_b_locked);
                    
                    if((mem_used > g_memCacheLimit * 8 / 10) && (pChartCache->GetCount() > 2)) {
                        wxString msg(_T("Removing oldest chart from cache: "));
                        while (1)
                        {
                          CacheEntry *pce = FindOldestDeleteCandidate(true);
                          if (pce == 0)
                              break;                      // no possible delete candidate
                          
                          // purge texture cache, really need memory here
                          DeleteCacheEntry(pce, true, msg);

                          GetMemoryStatus(0, &mem_used);
                          if((mem_used < g_memCacheLimit * 8 / 10) || (pChartCache->GetCount() <= 2)) 
                              break;
                                
                        }  // while
                    }
                }

                else        // Use n chart cache policy, if memory-limit  policy is not used
                {
    //      Limit cache to n charts, tossing out the oldest when space is needed
                    unsigned int nCache = pChartCache->GetCount();
                    if (nCache > (unsigned int)g_nCacheLimit && nCache > 2)
                    {
                        wxString msg(_T("Removing oldest chart from cache: "));
                        while (nCache > (unsigned int)g_nCacheLimit)
                        {
                            CacheEntry *pce = FindOldestDeleteCandidate( true );
                            if (pce == 0)
                                break;
                            
                            DeleteCacheEntry(pce, true, msg);
                            nCache--;
                        }
                    }
                    
                }
                
                m_cache_mutex.Unlock();
          }




//#endif      // ndef __WXMSW__

            if(chart_type == CHART_TYPE_KAP)
                  Ch = new ChartKAP();

            else if(chart_type == CHART_TYPE_GEO)
                  Ch = new ChartGEO();

#ifdef USE_S57
            else if(chart_type == CHART_TYPE_S57)
            {
                  LoadS57();
                  Ch = new s57chart();
                  s57chart *Chs57 = dynamic_cast<s57chart*>(Ch);

                  Chs57->SetNativeScale(cte.GetScale());

                  //    Explicitely set the chart extents from the database to
                  //    support the case wherein the SENC file has not yet been built
                  Extent ext;
                  ext.NLAT = cte.GetLatMax();
                  ext.SLAT = cte.GetLatMin();
                  ext.WLON = cte.GetLonMin();
                  ext.ELON = cte.GetLonMax();
                  Chs57->SetFullExtent(ext);
            }
#endif

#ifdef USE_S57
            else if(chart_type == CHART_TYPE_CM93)
            {
                  LoadS57();
                  Ch = new cm93chart();
                  cm93chart *Chcm93 = dynamic_cast<cm93chart*>(Ch);

                  Chcm93->SetNativeScale(cte.GetScale());

                  //    Explicitely set the chart extents from the database to
                  //    support the case wherein the SENC file has not yet been built
                  Extent ext;
                  ext.NLAT = cte.GetLatMax();
                  ext.SLAT = cte.GetLatMin();
                  ext.WLON = cte.GetLonMin();
                  ext.ELON = cte.GetLonMax();
                  Chcm93->SetFullExtent(ext);
            }

            else if(chart_type == CHART_TYPE_CM93COMP)
            {
                  LoadS57();
                  Ch = new cm93compchart();

                  cm93compchart *Chcm93 = dynamic_cast<cm93compchart*>(Ch);

                  Chcm93->SetNativeScale(cte.GetScale());

                  //    Explicitely set the chart extents from the database to
                  //    support the case wherein the SENC file has not yet been built
                  Extent ext;
                  ext.NLAT = cte.GetLatMax();
                  ext.SLAT = cte.GetLatMin();
                  ext.WLON = cte.GetLonMin();
                  ext.ELON = cte.GetLonMax();
                  Chcm93->SetFullExtent(ext);
            }



#endif

            else if(chart_type == CHART_TYPE_PLUGIN)
            {
                  wxFileName fn(ChartFullPath);
                  wxString ext = fn.GetExt();
                  ext.Prepend(_T("*."));
                  wxString ext_upper = ext.MakeUpper();
                  wxString ext_lower = ext.MakeLower();
                  wxString chart_class_name;

                  //    Search the array of chart class descriptors to find a match
                  //    bewteen the search mask and the the chart file extension

                  for(unsigned int i=0 ; i < m_ChartClassDescriptorArray.GetCount() ; i++)
                  {
                        if(m_ChartClassDescriptorArray.Item(i).m_descriptor_type == PLUGIN_DESCRIPTOR)
                        {
                              if(m_ChartClassDescriptorArray.Item(i).m_search_mask == ext_upper)
                              {
                                    chart_class_name = m_ChartClassDescriptorArray.Item(i).m_class_name;
                                    break;
                              }
                              if(m_ChartClassDescriptorArray.Item(i).m_search_mask == ext_lower)
                              {
                                    chart_class_name = m_ChartClassDescriptorArray.Item(i).m_class_name;
                                    break;
                              }
                        }
                  }

//                chart_class_name = cte.GetChartClassName();
                  if(chart_class_name.Len())
                  {
                        ChartPlugInWrapper *cpiw = new ChartPlugInWrapper(chart_class_name);
                        Ch = (ChartBase *)cpiw;
                        if(chart_family == CHART_FAMILY_VECTOR)
                            LoadS57();
                  }
            }


            else
                  Ch = NULL;


            if(Ch)
            {
                  InitReturn ir;
                  
#ifdef USE_S57
                  s52plib *plib = ps52plib;
#else
                  s52plib *plib = NULL;
#endif                  
                  

                  //    Vector charts need a PLIB for useful display....
                  if((chart_family != CHART_FAMILY_VECTOR) ||
                      ((chart_family == CHART_FAMILY_VECTOR) && plib) )
                  {
                        wxString msg(_T("Initializing Chart "));
                        msg.Append(ChartFullPath);
                        wxLogMessage(msg);

                        ir = Ch->Init(ChartFullPath, init_flag);    // using the passed flag
                        Ch->SetColorScheme(/*pParent->*/GetColorScheme());
                  }
                  else
                  {
                        wxString msg(_T("   No PLIB, Skipping vector chart "));
                        msg.Append(ChartFullPath);
                        wxLogMessage(msg);

                        ir = INIT_FAIL_REMOVE;

                  }

                  if(INIT_OK == ir)
                  {

//    always cache after a new chart has been created
//    or it may leak CacheEntry in createthumbnail
//                        if(FULL_INIT == init_flag)
                        {
                              pce = new CacheEntry;
                              pce->FullPath = ChartFullPath;
                              pce->pChart = Ch;
                              pce->dbIndex = dbindex;
//                              printf("    Adding chart %d\n", dbindex);
                              pce->RecentTime = m_ticks;
                              pce->n_lock = old_lock;

                              if( wxMUTEX_NO_ERROR == m_cache_mutex.Lock() ){
                                pChartCache->Add((void *)pce);
                                m_cache_mutex.Unlock();
                              }
                        }
                  }
                  else if(INIT_FAIL_REMOVE == ir)                 // some problem in chart Init()
                  {
                        delete Ch;
                        Ch = NULL;

//          Mark this chart in the database, so that it will not be seen during this run, but will stay in the database
                        DisableChart(ChartFullPath);
                  }
                  else if((INIT_FAIL_RETRY == ir) || (INIT_FAIL_NOERROR == ir))   // recoverable problem in chart Init()
                  {
                        delete Ch;
                        Ch = NULL;
                  }


                  if(INIT_OK != ir)
                  {
                        if(INIT_FAIL_NOERROR != ir)
                        {
                              wxString fp = ChartFullPath;
                              fp.Prepend(_T("   OpenChartFromStack...Error opening chart "));
                              wxString id;
                              id.Printf(_T("... return code %d"),  ir);
                              fp.Append(id);
                              wxLogMessage(fp);
                        }
                  }

            }


            m_b_busy = false;
            
            return Ch;
      }

      return NULL;
}

// 
bool ChartDB::DeleteCacheChart(ChartBase *pDeleteCandidate)
{
    bool retval = false;
    
    if( wxMUTEX_NO_ERROR == m_cache_mutex.Lock() ){
        
      if(Current_Ch != pDeleteCandidate)
      {

            // Find the chart in the cache
            CacheEntry *pce = NULL;
            for(unsigned int i=0 ; i< pChartCache->GetCount() ; i++)
            {
                  pce = (CacheEntry *)(pChartCache->Item(i));
                  if((ChartBase *)(pce->pChart) == pDeleteCandidate)
                  {
                        break;
                  }
            }

            if(pce)
            {
                  if(pce->n_lock > 0)
                    pce->n_lock--;

                  if( pce->n_lock == 0) {
                      DeleteCacheEntry( pce);
                      retval = true;
                  }
            }
      }
      
      m_cache_mutex.Unlock();
    }

      return retval;
}

/*
*/
void ChartDB::ApplyColorSchemeToCachedCharts(ColorScheme cs)
{
      ChartBase *Ch;
      CacheEntry *pce;
     //    Search the cache

      if( wxMUTEX_NO_ERROR == m_cache_mutex.Lock() ){
          
        unsigned int nCache = pChartCache->GetCount();
        for(unsigned int i=0 ; i<nCache ; i++)
        {
                pce = (CacheEntry *)(pChartCache->Item(i));
                Ch = (ChartBase *)pce->pChart;
                if(Ch)
                    Ch->SetColorScheme(cs, true);

        }
        
        m_cache_mutex.Unlock();
      }
}

//-------------------------------------------------------------------
//    Open a chart from the stack with conditions
//      a) Search Direction Start
//      b) Requested Chart Type
//-------------------------------------------------------------------

ChartBase *ChartDB::OpenStackChartConditional(ChartStack *ps, int index_start, bool bSearchDir, ChartTypeEnum New_Type, ChartFamilyEnum New_Family_Fallback)
{
      int index;

      int delta_index;
      ChartBase *ptc = NULL;

      if(bSearchDir == 1)
            delta_index = -1;

      else
            delta_index = 1;



      index = index_start;

      while((index >= 0) && (index < ps->nEntry))
      {
            ChartTypeEnum chart_type = (ChartTypeEnum)GetCSChartType(ps, index);
            if((chart_type == New_Type) || (New_Type == CHART_TYPE_DONTCARE))
            {
                  ptc = OpenChartFromStack(ps, index);
                  if (NULL != ptc)
                       break;
            }
                  index += delta_index;
      }

      //    Fallback, no useable chart of specified type found, so try for family match
      if(NULL == ptc)
      {
            index = index_start;

            while((index >= 0) && (index < ps->nEntry))
            {
                  ChartFamilyEnum chart_family = GetCSChartFamily(ps, index);
                  if(chart_family == New_Family_Fallback)
                  {
                        ptc = OpenChartFromStack(ps, index);

                        if (NULL != ptc)
                              break;

                  }
                  index += delta_index;
            }
      }

      return ptc;
}

wxXmlDocument ChartDB::GetXMLDescription(int dbIndex, bool b_getGeom)
{
      wxXmlDocument doc;
      if(!IsValid() || (dbIndex >= GetChartTableEntries()))
            return doc;

      bool b_remove = !IsChartInCache(dbIndex);

      wxXmlNode *pcell_node = NULL;
      wxXmlNode *node;
      wxXmlNode *tnode;

      //   Open the chart, without cacheing it
      ChartBase *pc = OpenChartFromDB(dbIndex, HEADER_ONLY);
      b_remove = !IsChartInCache(dbIndex);
      const ChartTableEntry &cte = GetChartTableEntry(dbIndex);


      if( CHART_FAMILY_RASTER ==  (ChartFamilyEnum)cte.GetChartFamily())
      {
            pcell_node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "chart" ) );

            wxString path = GetDBChartFileName(dbIndex);
            node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "path" ) );
            pcell_node->AddChild ( node );
            tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), path );
            node->AddChild ( tnode );

            wxFileName name(path);
            node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "name" ) );
            pcell_node->AddChild ( node );
            tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), name.GetName() );
            node->AddChild ( tnode );

            if(pc)
            {
                  node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "lname" ) );
                  pcell_node->AddChild ( node );
                  tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), pc->GetName() );
                  node->AddChild ( tnode );
            }

            wxString scale;
            scale.Printf(_T("%d"),cte.GetScale());
            node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "cscale" ) );
            pcell_node->AddChild ( node );
            tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), scale );
            node->AddChild ( tnode );

            wxDateTime file_date(cte.GetFileTime());
            file_date.MakeUTC();
            wxString sfile_date = file_date.FormatISODate();
            sfile_date += _T("T");
            sfile_date += file_date.FormatISOTime();
            sfile_date += _T("Z");
            node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "local_file_datetime_iso8601" ) );
            pcell_node->AddChild ( node );
            tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), sfile_date );
            node->AddChild ( tnode );


            if(pc)
            {
                  node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "source_edition" ) );
                  pcell_node->AddChild ( node );
                  tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), pc->GetSE() );
                  node->AddChild ( tnode );

                  wxDateTime sdt = pc->GetEditionDate();
                  wxString ssdt = _T("Unknown");
                  if(sdt.IsValid())
                        ssdt = sdt.Format(_T("%Y%m%d"));

                  node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "source_date" ) );
                  pcell_node->AddChild ( node );
                  tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), ssdt );
                  node->AddChild ( tnode );

            }

/*
            if (s == _T("number"))
///            if (s == _T("source_edition"))
            if (s == _T("raster_edition"))
            if (s == _T("ntm_edition"))
///            if (s == _T("source_date"))
            if (s == _T("ntm_date"))
            if (s == _T("source_edition_last_correction"))
            if (s == _T("raster_edition_last_correction"))
            if (s == _T("ntm_edition_last_correction"))
*/
      }

      else if( CHART_FAMILY_VECTOR ==  (ChartFamilyEnum)cte.GetChartFamily() )
      {
            pcell_node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "cell" ) );

            wxString path = GetDBChartFileName(dbIndex);
            node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "path" ) );
            pcell_node->AddChild ( node );
            tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), path );
            node->AddChild ( tnode );

            wxFileName name(path);
            node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "name" ) );
            pcell_node->AddChild ( node );
            tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), name.GetName() );
            node->AddChild ( tnode );

            wxString scale;
            scale.Printf(_T("%d"),cte.GetScale());
            node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "cscale" ) );
            pcell_node->AddChild ( node );
            tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), scale );
            node->AddChild ( tnode );

            wxDateTime file_date(cte.GetFileTime());
            file_date.MakeUTC();
            wxString sfile_date = file_date.FormatISODate();
            sfile_date += _T("T");
            sfile_date += file_date.FormatISOTime();
            sfile_date += _T("Z");
            node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "local_file_datetime_iso8601" ) );
            pcell_node->AddChild ( node );
            tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), sfile_date );
            node->AddChild ( tnode );


            if(pc)
            {
                  node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "edtn" ) );
                  pcell_node->AddChild ( node );
                  tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), pc->GetSE() );
                  node->AddChild ( tnode );
            }

#ifdef USE_S57
            
            s57chart *pcs57 = dynamic_cast<s57chart*>(pc);
            if(pcs57)
            {
                  node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "isdt" ) );
                  pcell_node->AddChild ( node );
                  tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), pcs57->GetISDT() );
                  node->AddChild ( tnode );

                  wxString LastUpdateDate;
                  int updn = pcs57->ValidateAndCountUpdates( path, _T(""), LastUpdateDate, false);

                  wxString supdn;
                  supdn.Printf(_T("%d"), updn);
                  node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "updn" ) );
                  pcell_node->AddChild ( node );
                  tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), supdn );
                  node->AddChild ( tnode );

                  node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "uadt" ) );
                  pcell_node->AddChild ( node );
                  tnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), LastUpdateDate );
                  node->AddChild ( tnode );

            }
#endif            
      }



      if(pcell_node && b_getGeom)
      {
            node = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "cov" ) );
            pcell_node->AddChild ( node );


            //    Primary table
            if(cte.GetnPlyEntries())
            {
                  wxXmlNode *panelnode = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "panel" ) );
                  node->AddChild ( panelnode );

                  wxString panel_no;
                  panel_no.Printf(_T("%d"), 0);
                  wxXmlNode *anode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), panel_no );
                  panelnode->AddChild ( anode );

                  float *pf = cte.GetpPlyTable();
                  for(int j=0 ; j < cte.GetnPlyEntries() ; j++)
                  {
                        wxXmlNode *vnode = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "vertex" ));
                        panelnode->AddChild ( vnode );

                        wxXmlNode *latnode = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "lat" ));
                        vnode->AddChild ( latnode );

                        float l = *pf++;
                        wxString sl;
                        sl.Printf(_T("%.5f"), l);
                        wxXmlNode *vtnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), sl);
                        latnode->AddChild ( vtnode );

                        wxXmlNode *lonnode = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "lon" ));
                        vnode->AddChild ( lonnode );

                        float ll = *pf++;
                        wxString sll;
                        sll.Printf(_T("%.5f"), ll);
                        wxXmlNode *vtlnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), sll);
                        lonnode->AddChild ( vtlnode );
                  }
            }


            for(int i=0 ; i < cte.GetnAuxPlyEntries() ; i++)
            {
                  wxXmlNode *panelnode = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "panel" ) );
                  node->AddChild ( panelnode );

                  wxString panel_no;
                  panel_no.Printf(_T("%d"), i+1);
                  wxXmlNode *anode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), panel_no );
                  panelnode->AddChild ( anode );

                  float *pf = cte.GetpAuxPlyTableEntry(i);
                  for(int j=0 ; j < cte.GetAuxCntTableEntry(i) ; j++)
                  {
                        wxXmlNode *vnode = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "vertex" ));
                        panelnode->AddChild ( vnode );

                        wxXmlNode *latnode = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "lat" ));
                        vnode->AddChild ( latnode );

                        float l = *pf++;
                        wxString sl;
                        sl.Printf(_T("%.5f"), l);
                        wxXmlNode *vtnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), sl);
                        latnode->AddChild ( vtnode );

                        wxXmlNode *lonnode = new wxXmlNode ( wxXML_ELEMENT_NODE, _T ( "lon" ));
                        vnode->AddChild ( lonnode );

                        float ll = *pf++;
                        wxString sll;
                        sll.Printf(_T("%.5f"), ll);
                        wxXmlNode *vtlnode = new wxXmlNode ( wxXML_TEXT_NODE, _T ( "" ), sll);
                        lonnode->AddChild ( vtlnode );
                  }
            }

      }

      doc.SetRoot(pcell_node);

      if(b_remove)
            DeleteCacheChart(pc);

      return doc;
}





//  Private version of PolyPt testing using floats instead of doubles

bool Intersect(MyFlPoint p1, MyFlPoint p2, MyFlPoint p3, MyFlPoint p4) ;
int CCW(MyFlPoint p0, MyFlPoint p1, MyFlPoint p2) ;

/*************************************************************************


 * FUNCTION:   G_FloatPtInPolygon
 *
 * PURPOSE
 * This routine determines if the point passed is in the polygon. It uses

 * the classical polygon hit-testing algorithm: a horizontal ray starting

 * at the point is extended infinitely rightwards and the number of
 * polygon edges that intersect the ray are counted. If the number is odd,
 * the point is inside the polygon.
 *
 * RETURN VALUE
 * (bool) TRUE if the point is inside the polygon, FALSE if not.
 *************************************************************************/


bool G_FloatPtInPolygon(MyFlPoint *rgpts, int wnumpts, float x, float y)

{

   MyFlPoint  *ppt, *ppt1 ;
   int   i ;
   MyFlPoint  pt1, pt2, pt0 ;
   int   wnumintsct = 0 ;

   pt0.x = x;
   pt0.y = y;

   pt1 = pt2 = pt0 ;
   pt2.x = 1.e6;

   // Now go through each of the lines in the polygon and see if it
   // intersects
   for (i = 0, ppt = rgpts ; i < wnumpts-1 ; i++, ppt++)
   {
      ppt1 = ppt;
        ppt1++;
        if (Intersect(pt0, pt2, *ppt, *(ppt1)))
         wnumintsct++ ;
   }

   // And the last line
   if (Intersect(pt0, pt2, *ppt, *rgpts))
      wnumintsct++ ;

//   return(wnumintsct&1);

   //       If result is false, check the degenerate case where test point lies on a polygon endpoint
   if(!(wnumintsct&1))
   {
         for (i = 0, ppt = rgpts ; i < wnumpts ; i++, ppt++)
         {
               if(((*ppt).x == x) && ((*ppt).y == y))
                     return true;
         }
   }
   else
       return true;

   return false;
}


/*************************************************************************


 * FUNCTION:   Intersect
 *
 * PURPOSE
 * Given two line segments, determine if they intersect.
 *
 * RETURN VALUE
 * TRUE if they intersect, FALSE if not.
 *************************************************************************/


inline bool Intersect(MyFlPoint p1, MyFlPoint p2, MyFlPoint p3, MyFlPoint p4) {
   return ((( CCW(p1, p2, p3) * CCW(p1, p2, p4)) <= 0)
        && (( CCW(p3, p4, p1) * CCW(p3, p4, p2)  <= 0) )) ;

}
/*************************************************************************


 * FUNCTION:   CCW (CounterClockWise)
 *
 * PURPOSE
 * Determines, given three points, if when travelling from the first to
 * the second to the third, we travel in a counterclockwise direction.
 *
 * RETURN VALUE
 * (int) 1 if the movement is in a counterclockwise direction, -1 if
 * not.
 *************************************************************************/


inline int CCW(MyFlPoint p0, MyFlPoint p1, MyFlPoint p2) {
   float dx1, dx2 ;
   float dy1, dy2 ;

   dx1 = p1.x - p0.x ; dx2 = p2.x - p0.x ;
   dy1 = p1.y - p0.y ; dy2 = p2.y - p0.y ;

   /* This is basically a slope comparison: we don't do divisions because

    * of divide by zero possibilities with pure horizontal and pure
    * vertical lines.
    */
   return ((dx1 * dy2 > dy1 * dx2) ? 1 : -1) ;

}

