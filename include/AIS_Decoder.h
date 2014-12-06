/***************************************************************************
 *
 * Project:  OpenCPN
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
 ***************************************************************************
 */

#ifndef __AIS_DECODER_H__
#define __AIS_DECODER_H__

#include "ais.h"
#include <map>

#define TRACKTYPE_DEFAULT       0
#define TRACKTYPE_ALWAYS        1
#define TRACKTYPE_NEVER         2

class MMSIProperties
{
public:
    MMSIProperties(){};
    MMSIProperties( int mmsi ){ Init(); MMSI = mmsi; }
    MMSIProperties( wxString &spec );
    
    ~MMSIProperties();
    
    wxString Serialize();
    
    void Init( void );
    int         MMSI;
    int         TrackType;
    bool        m_bignore;
    bool        m_bMOB;
    bool        m_bVDM;
    bool        m_bPersistentTrack;
};

WX_DECLARE_OBJARRAY(MMSIProperties *,      ArrayOfMMSIProperties);

class AIS_Decoder : public wxEvtHandler
{

public:
    AIS_Decoder(wxFrame *parent);

    ~AIS_Decoder(void);

    void OnEvtAIS(OCPN_DataStreamEvent& event);
    AIS_Error Decode(const wxString& str);
    AIS_Target_Hash *GetTargetList(void) {return AISTargetList;}
    AIS_Target_Hash *GetAreaNoticeSourcesList(void) {return AIS_AreaNotice_Sources;}
    AIS_Target_Data *Get_Target_Data_From_MMSI(int mmsi);
    int GetNumTargets(void){ return m_n_targets;}
    bool IsAISSuppressed(void){ return m_bSuppressed; }
    bool IsAISAlertGeneral(void) { return m_bGeneralAlert; }
    AIS_Error DecodeSingleVDO( const wxString& str, GenericPosDatEx *pos, wxString *acc );
    void DeletePersistentTrack( Track *track );
    std::map<int, Track*> m_persistent_tracks;
    
private:
    void OnActivate(wxActivateEvent& event);
    void OnTimerAIS(wxTimerEvent& event);
    void OnTimerAISAudio(wxTimerEvent& event);
    void OnTimerDSC( wxTimerEvent& event );
    
    bool NMEACheckSumOK(const wxString& str);
    bool Parse_VDXBitstring(AIS_Bitstring *bstr, AIS_Target_Data *ptd);
    void UpdateAllCPA(void);
    void UpdateOneCPA(AIS_Target_Data *ptarget);
    void UpdateAllAlarms(void);
    void UpdateAllTracks(void);
    void UpdateOneTrack(AIS_Target_Data *ptarget);
    void BuildERIShipTypeHash(void);
    AIS_Target_Data *ProcessDSx( const wxString& str, bool b_take_dsc = false );
    
    AIS_Target_Hash *AISTargetList;
    AIS_Target_Hash *AIS_AreaNotice_Sources;
    AIS_Target_Name_Hash *AISTargetNames;

    bool              m_busy;
    wxTimer           TimerAIS;
    wxFrame           *m_parent_frame;

    int               nsentences;
    int               isentence;
    wxString          sentence_accumulator;
    bool              m_OK;

    AIS_Target_Data   *m_pLatestTargetData;

    bool             m_bAIS_Audio_Alert_On;
    wxTimer          m_AIS_Audio_Alert_Timer;
    OCPN_Sound       m_AIS_Sound;
    int              m_n_targets;
    bool             m_bSuppressed;
    bool             m_bGeneralAlert;
    AIS_Target_Data  *m_ptentative_dsctarget;
    wxTimer          m_dsc_timer;
    wxString         m_dsc_last_string;

DECLARE_EVENT_TABLE()
};

#endif
