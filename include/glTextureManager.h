/******************************************************************************
 *
 * Project:  OpenCPN
 * Authors:  David Register
 *           Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2016 by David S. Register                               *
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

#ifndef __GLTEXTUREMANAGER_H__
#define __GLTEXTUREMANAGER_H__

const wxEventType wxEVT_OCPN_COMPRESSIONTHREAD = wxNewEventType();

class JobTicket;

WX_DECLARE_LIST(JobTicket, JobList);

class OCPN_CompressionThreadEvent: public wxEvent
{
public:
    OCPN_CompressionThreadEvent( wxEventType commandType = wxEVT_NULL, int id = 0 );
    ~OCPN_CompressionThreadEvent( );
    
    // accessors
    void SetTicket( JobTicket *ticket ){m_ticket = ticket;}
    JobTicket *GetTicket(void){ return m_ticket; }
    
    // required for sending with wxPostEvent()
    wxEvent *Clone() const;
    
private:
    JobTicket  * m_ticket;
};

//      This is a hashmap with Chart full path as key, and glTexFactory as value
WX_DECLARE_STRING_HASH_MAP( glTexFactory*, ChartPathHashTexfactType );

//      glTextureManager Definition
class glTextureManager : public wxEvtHandler
{
public:
    glTextureManager();
    ~glTextureManager();

    void OnEvtThread( OCPN_CompressionThreadEvent & event );
    void OnTimer(wxTimerEvent &event);
    bool ScheduleJob( glTexFactory *client, const wxRect &rect, int level_min,
                      bool b_throttle_thread, bool b_nolimit, bool b_postZip, bool b_inplace);

    int GetRunningJobCount(){ return running_list.GetCount(); }
    int GetJobCount(){ return GetRunningJobCount() + todo_list.GetCount(); }
    bool AsJob( wxString const &chart_path ) const;
    void PurgeJobList( wxString chart_path = wxEmptyString );
    void ClearJobList();
    void ClearAllRasterTextures(void);
    bool PurgeChartTextures(ChartBase *pc, bool b_purge_factory = false);
    bool TextureCrunch(double factor);
    bool FactoryCrunch(double factor);

    //    This is a hash table
    //    key is Chart full path
    //    Value is glTexFactory*
    ChartPathHashTexfactType   m_chart_texfactory_hash;

private:    
    bool DoJob( JobTicket *pticket );
    bool DoThreadJob(JobTicket* pticket);
    bool StartTopJob();
    
    JobList             running_list;
    JobList             todo_list;
    int                 m_max_jobs;

    int		m_prevMemUsed;

    wxTimer     m_timer;
    size_t      m_ticks;
};

class glTextureDescriptor;
void GetFullMap( glTextureDescriptor *ptd,  const wxRect &rect, wxString chart_path, int level);
int TextureDim(int level);
int TextureTileSize(int level, bool compressed);
bool CompressUsingGPU(const unsigned char *data, int dim, int size,
                      unsigned char *tex_data, int level, bool inplace);

#endif
