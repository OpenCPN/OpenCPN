/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  AIS Decoder Object
 * Author:   David Register
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
 *
 */

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include <wx/datetime.h>
#include <wx/listctrl.h>
#include <wx/spinctrl.h>
#include <wx/aui/aui.h>
#include <wx/wxhtml.h>

#include <vector>

#include "wx/sound.h"

#include "dychart.h"
#include "chart1.h"

//    Constants
#ifndef PI
#define PI        3.1415926535897931160E0      /* pi */
#endif



#define TIMER_AIS_MSEC      998
#define TIMER_AIS_AUDIO_MSEC 2000

#define ID_ACKNOWLEDGE        10001
#define ID_SILENCE            10002
#define ID_AIS_TARGET_LIST    10003
#define ID_JUMPTO             10004

typedef enum AIS_Error
{
    AIS_NoError = 0,
    AIS_Partial,
    AIS_NMEAVDX_TOO_LONG,
    AIS_NMEAVDX_CHECKSUM_BAD,
    AIS_NMEAVDX_BAD,
    AIS_NO_SERIAL,
    AIS_NO_TCP
}_AIS_Error;


//      Describe NavStatus variable
typedef enum ais_nav_status
{
    UNDERWAY_USING_ENGINE = 0,
    AT_ANCHOR,
    NOT_UNDER_COMMAND,
    RESTRICTED_MANOEUVRABILITY,
    CONSTRAINED_BY_DRAFT,
    MOORED,
    AGROUND,
    FISHING,
    UNDERWAY_SAILING,
    HSC,
    WIG,
    RESERVED_11,
    RESERVED_12,
    RESERVED_13,
    RESERVED_14,
    UNDEFINED,
    ATON_VIRTUAL,
    ATON_VIRTUAL_ONPOSITION,
    ATON_VIRTUAL_OFFPOSITION,
    ATON_REAL,
    ATON_REAL_ONPOSITION,
    ATON_REAL_OFFPOSITION

}_ais_nav_status;


//      Describe Transponder Class
typedef enum ais_transponder_class
{
      AIS_CLASS_A = 0,
      AIS_CLASS_B,
      AIS_ATON,    // Aid to Navigation   pjotrc 2010/02/01
      AIS_BASE,     // Base station
      AIS_GPSG_BUDDY, // GpsGate Buddy object
      AIS_DSC,	// DSC target
      AIS_SART   // SART

}_ais_transponder_class;

//    Describe AIS Alarm state
typedef enum ais_alarm_type
{
      AIS_NO_ALARM = 0,
      AIS_ALARM_SET,
//      AIS_ALARM_ACKNOWLEDGED

}_ais_alarm_type;

class AISTargetTrackPoint
{
      public:
            double      m_lat;
            double      m_lon;
            time_t      m_time;
};


WX_DECLARE_LIST(AISTargetTrackPoint, AISTargetTrackList);



// IMO Circ. 289 Area Notices, based on libais
const size_t AIS8_001_22_NUM_NAMES=128;
const size_t AIS8_001_22_SUBAREA_SIZE=87;

extern const char *ais8_001_22_notice_names[AIS8_001_22_NUM_NAMES];

enum Ais8_001_22_AreaShapeEnum
{
    AIS8_001_22_SHAPE_ERROR = -1,
    AIS8_001_22_SHAPE_CIRCLE = 0, // OR Point
    AIS8_001_22_SHAPE_RECT = 1,
    AIS8_001_22_SHAPE_SECTOR = 2,
    AIS8_001_22_SHAPE_POLYLINE = 3,
    AIS8_001_22_SHAPE_POLYGON = 4,
    AIS8_001_22_SHAPE_TEXT = 5,
    AIS8_001_22_SHAPE_RESERVED_6 = 6,
    AIS8_001_22_SHAPE_RESERVED_7 = 7
};

struct Ais8_001_22_SubArea
{
    int shape;
    float longitude, latitude;
    int radius_m;
    int e_dim_m; // East dimension in meters
    int n_dim_m;
    int orient_deg; // Orientation in degrees from true north
    int left_bound_deg;
    int right_bound_deg;
    float angles[4];
    float dists_m[4];
    wxString text;
};

//WX_DECLARE_LIST(Ais8_001_22_SubArea, Ais8_001_22_SubAreaList);
typedef std::vector<Ais8_001_22_SubArea> Ais8_001_22_SubAreaList;

struct Ais8_001_22
{
    int link_id; // 10 bit id to match up text blocks
    int notice_type; // area_type / Notice Description
    int month; // These are in UTC
    int day;   // UTC!
    int hour;  // UTC!
    int minute;
    int duration_minutes; // Time from the start until the notice expires
    wxDateTime start_time;
    wxDateTime expiry_time;
    Ais8_001_22_SubAreaList sub_areas;
};


// key is link_id, which should be unique for a given mmsi
WX_DECLARE_HASH_MAP( int, Ais8_001_22, wxIntegerHash, wxIntegerEqual, AIS_Area_Notice_Hash );


//---------------------------------------------------------------------------------
//
//  AIS_Decoder Helpers
//
//---------------------------------------------------------------------------------

class AIS_Target_Data
{
public:

    AIS_Target_Data();
    ~AIS_Target_Data();

    wxString BuildQueryResult(void);
    wxString GetRolloverString(void);
    wxString Get_vessel_type_string(bool b_short = false);
    wxString Get_class_string(bool b_short = false);
	void Toggle_AIS_CPA(void); //TR 2012.06.28: Show AIS-CPA


    int                       MID;
    int                       MMSI;
    ais_transponder_class     Class;
    int                       NavStatus;
    int                       SyncState;
    int                       SlotTO;
    double                    SOG;
    double                    COG;
    double                    HDG;
    double                    Lon;
    double                    Lat;
    int                       ROTAIS;
    int                       ROTIND;
    char                      CallSign[8];                // includes terminator
    char                      ShipName[21];
    char                      ShipNameExtension[21];
    unsigned char             ShipType;
    int                       IMO;

    int                       DimA;
    int                       DimB;
    int                       DimC;
    int                       DimD;

    double                    Euro_Length;            // Extensions for European Inland AIS
    double                    Euro_Beam;
    double                    Euro_Draft;
    char                      Euro_VIN[8];
    int                       UN_shiptype;
    bool                      b_isEuroInland;
    bool                      b_blue_paddle;
    int                       blue_paddle;

    int                       ETA_Mo;
    int                       ETA_Day;
    int                       ETA_Hr;
    int                       ETA_Min;

    double                    Draft;

    char                      Destination[21];

    time_t                    PositionReportTicks;
    time_t                    StaticReportTicks;

    int                       RecentPeriod;
    bool                      b_active;
    bool                      b_lost;
    ais_alarm_type            n_alarm_state;
    bool                      b_suppress_audio;
    bool                      b_positionDoubtful;
    bool                      b_positionOnceValid;
    bool                      b_nameValid;
    bool                      b_OwnShip;

    int                       m_utc_hour;
    int                       m_utc_min;
    int                       m_utc_sec;
    wxDateTime                m_ack_time;
    bool                      b_in_ack_timeout;

    double                    Range_NM;
    double                    Brg;

    wxString                  MSG_14_text;

    //      Per target collision parameters
    bool                      bCPA_Valid;
    double                    TCPA;                     // Minutes
    double                    CPA;                      // Nautical Miles

    bool                      b_show_AIS_CPA;           //TR 2012.06.28: Show AIS-CPA

    AISTargetTrackList        *m_ptrack;

    AIS_Area_Notice_Hash     area_notices;
};

WX_DEFINE_SORTED_ARRAY(AIS_Target_Data *, ArrayOfAISTarget);
WX_DEFINE_SORTED_ARRAY_INT(int, ArrayOfMMSI);


#define AIS_MAX_MESSAGE_LEN (10 * 82)           // AIS Spec allows up to 9 sentences per message, 82 bytes each
class AIS_Bitstring
{
public:

    AIS_Bitstring(const char *str);
    unsigned char to_6bit(const char c);

    /// sp is starting bit, 1-based
    int GetInt(int sp, int len, bool signed_flag = false);
    int GetStr(int sp, int bit_len, char *dest, int max_len);
    int GetBitCount();


private:

    unsigned char bitbytes[AIS_MAX_MESSAGE_LEN];
    int byte_length;
};




//      Implement the AISTargetList as a wxHashMap

WX_DECLARE_HASH_MAP( int, AIS_Target_Data*, wxIntegerHash, wxIntegerEqual, AIS_Target_Hash );

//---------------------------------------------------------------------------------
//
//  AIS_Decoder Definition
//
//---------------------------------------------------------------------------------

class AIS_Decoder : public wxEvtHandler
{

public:
    AIS_Decoder(void);

    ~AIS_Decoder(void);

    void OnEvtAIS(OCPN_DataStreamEvent& event);
    AIS_Error Decode(const wxString& str);
    AIS_Target_Hash *GetTargetList(void) {return AISTargetList;}
    AIS_Target_Data *Get_Target_Data_From_MMSI(int mmsi);
    int GetNumTargets(void){ return m_n_targets;}
    bool IsAISSuppressed(void){ return m_bSuppressed; }
    bool IsAISAlertGeneral(void) { return m_bGeneralAlert; }

private:
    void OnActivate(wxActivateEvent& event);
    void OnTimerAIS(wxTimerEvent& event);
    void OnTimerAISAudio(wxTimerEvent& event);

    bool NMEACheckSumOK(const wxString& str);
    bool Parse_VDXBitstring(AIS_Bitstring *bstr, AIS_Target_Data *ptd);
    void UpdateAllCPA(void);
    void UpdateOneCPA(AIS_Target_Data *ptarget);
    void UpdateAllAlarms(void);
    void UpdateAllTracks(void);
    void UpdateOneTrack(AIS_Target_Data *ptarget);
    void BuildERIShipTypeHash(void);

    AIS_Target_Hash *AISTargetList;

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
    wxSound          m_AIS_Sound;
    int              m_n_targets;
    bool             m_bSuppressed;
    bool             m_bGeneralAlert;

DECLARE_EVENT_TABLE()
};

class AISInfoWin;
//----------------------------------------------------------------------------------------------------------
//    AISTargetAlertDialog Specification
//----------------------------------------------------------------------------------------------------------
class AISTargetAlertDialog: public wxDialog
{
      DECLARE_CLASS( AISTargetAlertDialog )
                  DECLARE_EVENT_TABLE()
      public:

           AISTargetAlertDialog( );

            ~AISTargetAlertDialog( );
            void Init();

            bool Create( int target_mmsi,
                         wxWindow *parent,
                         AIS_Decoder *pdecoder,
                         bool b_jumpto,
                         wxWindowID id = wxID_ANY,
                         const wxString& caption = _("AIS Alert"),
                         const wxPoint& pos = wxDefaultPosition,
                         const wxSize& size = wxDefaultSize,
                         long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

           void CreateControls();

           int Get_Dialog_MMSI(void){ return m_target_mmsi; }
           void UpdateText();

      private:
            bool GetAlertText(void);
            void OnClose(wxCloseEvent& event);
            void OnIdAckClick( wxCommandEvent& event );
            void OnMove( wxMoveEvent& event );
            void OnSize( wxSizeEvent& event );
            void OnIdSilenceClick( wxCommandEvent& event );
            void OnIdJumptoClick( wxCommandEvent& event );


            wxHtmlWindow      *m_pAlertTextCtl;
            int               m_target_mmsi;
            AIS_Decoder       *m_pdecoder;
            wxWindow          *m_pparent;
            wxFont            *m_pFont;
            wxString          m_alert_text;
            bool              m_bjumpto;

};

class OCPNListCtrl;
//----------------------------------------------------------------------------------------------------------
//    AISTargetListDialog Specification
//----------------------------------------------------------------------------------------------------------
class AISTargetListDialog: public wxPanel
{
      DECLARE_CLASS( AISTargetListDialog )

      public:
            AISTargetListDialog( wxWindow *parent, wxAuiManager *auimgr, AIS_Decoder *pdecoder );
           ~AISTargetListDialog( );

            void OnClose(wxCloseEvent &event);
            void Disconnect_decoder();

            void SetColorScheme( );
            void UpdateAISTargetList( );     // Rebuild AIS target list
            AIS_Target_Data   *GetpTarget(unsigned int list_item);

            OCPNListCtrl      *m_pListCtrlAISTargets;
            AIS_Decoder       *m_pdecoder;

            ArrayOfMMSI       *m_pMMSI_array;

      private:
            void OnPaneClose( wxAuiManagerEvent& event );
            void UpdateButtons();
            void OnTargetSelected( wxListEvent &event );
            void DoTargetQuery( int mmsi );
            void OnTargetDefaultAction( wxListEvent& event );
            void OnTargetQuery( wxCommandEvent& event );
            void OnTargetListColumnClicked( wxListEvent &event );
            void OnTargetScrollTo( wxCommandEvent& event );
            void OnLimitRange( wxCommandEvent& event );

            wxWindow          *m_pparent;
            wxAuiManager      *m_pAuiManager;
            wxButton          *m_pButtonInfo;
            wxButton          *m_pButtonJumpTo;
            wxStaticText      *m_pStaticTextRange;
            wxSpinCtrl        *m_pSpinCtrlRange;
            wxStaticText      *m_pStaticTextCount;
            wxTextCtrl        *m_pTextTargetCount;

            DECLARE_EVENT_TABLE()

};




