/******************************************************************************
 * $Id: ais.h,v 1.35 2010/06/16 03:50:51 bdbcat Exp $
 *
 * Project:  OpenCPN
 * Purpose:  AIS Decoder Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   $EMAIL$   *
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
 * 2010/02/01 17:30 pjotrc
 * - implement AIS AtoN
 *
 * $Log: ais.h,v $
 * Revision 1.35  2010/06/16 03:50:51  bdbcat
 * 615a
 *
 * Revision 1.34  2010/06/12 02:21:08  bdbcat
 * 611b
 *
 * Revision 1.33  2010/06/12 00:45:19  bdbcat
 * 611b
 *
 * Revision 1.32  2010/05/15 03:55:04  bdbcat
 * Build 514
 *
 * Revision 1.31  2010/04/27 01:44:36  bdbcat
 * Build 426
 *
 * Revision 1.30  2010/03/29 03:17:36  bdbcat
 * 2.1.0 Beta Initial
 *
 * Revision 1.29  2010/03/29 02:59:02  bdbcat
 * 2.1.0 Beta Initial
 *
 *
 */

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include <wx/datetime.h>
#include <wx/listctrl.h>

////////////TH//////////////////
#ifdef __WXGTK__
// newer versions of glib define its own GSocket but we unfortunately use this
// name in our own (semi-)public header and so can't change it -- rename glib
// one instead

#include <gtk/gtk.h>
#define GSocket GlibGSocket
#endif

#include "wx/socket.h"
///////////TH100126////////////////
#include "wx/sound.h"

#include "dychart.h"
#include "chart1.h"

#ifdef __POSIX__
#include <sys/termios.h>
#endif


//    Constants
#ifndef PI
#define PI        3.1415926535897931160E0      /* pi */
#endif



#define TIMER_AIS_MSEC      998
#define TIMER_AIS_AUDIO_MSEC 2000

#define ID_ACKNOWLEDGE        10001
#define ID_SILENCE            10002

typedef enum AIS_Error
{
    AIS_NoError = 0,
    AIS_Partial,
    AIS_NMEAVDM_TOO_LONG,
    AIS_NMEAVDM_CHECKSUM_BAD,
    AIS_NMEAVDM_BAD,
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
    UNDERWAY_SAILING

}_ais_nav_status;


//      Describe Transponder Class
typedef enum ais_transponder_class
{
      AIS_CLASS_A = 0,
      AIS_CLASS_B,
	AIS_ATON		// Aid to Navigation   pjotrc 2010/02/01

}_ais_transponder_class;

//    Describe AIS Alarm state
typedef enum ais_alarm_type
{
      AIS_NO_ALARM = 0,
      AIS_ALARM_SET,
      AIS_ALARM_ACKNOWLEDGED

}_ais_alarm_type;

//---------------------------------------------------------------------------------
//
//  AIS_Decoder Helpers
//
//---------------------------------------------------------------------------------

class AIS_Target_Data
{
public:

    AIS_Target_Data();
    wxString BuildQueryResult(void);
    char *Get_vessel_type_string(bool b_short = false);


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
    char                      CallSign[8];                // includes terminator
    char                      ShipName[21];
    unsigned char             ShipType;
    int                       IMO;

    int                       DimA;
    int                       DimB;
    int                       DimC;
    int                       DimD;

    int                       ETA_Mo;
    int                       ETA_Day;
    int                       ETA_Hr;
    int                       ETA_Min;

    double                    Draft;

    char                      Destination[21];

    time_t                    ReportTicks;
    int                       RecentPeriod;
    bool                      b_active;
    ais_alarm_type            n_alarm_state;
    bool                      b_suppress_audio;
    bool                      b_positionValid;
    bool                      b_nameValid;

    int                       m_utc_hour;
    int                       m_utc_min;
    int                       m_utc_sec;

    double                    Range_NM;
    double                    Brg;

    //      Per target collision parameters
    bool                      bCPA_Valid;
    double                    TCPA;                     // Minutes
    double                    CPA;                      // Nautical Miles


};



#define AIS_MAX_MESSAGE_LEN (10 * 82)           // AIS Spec allows up to 9 sentences per message, 82 bytes each
class AIS_Bitstring
{
public:

    AIS_Bitstring(const char *str);
    unsigned char to_6bit(const char c);
    int GetInt(int sp, int len);
    bool GetStr(int sp, int len, char *dest, int max_len);


private:

    unsigned char bitbytes[AIS_MAX_MESSAGE_LEN];
    int byte_length;
};




//      Implement the AISTargetList as a wxHashMap

WX_DECLARE_HASH_MAP( int, AIS_Target_Data*, wxIntegerHash, wxIntegerEqual, AIS_Target_Hash );



#define AIS_SOCKET_ID             7

enum
{
    EVT_AIS_DIRECT,
    EVT_AIS_PARSE_RX
};

//----------------------------------------------------------------------------
// AISEvent
//----------------------------------------------------------------------------

class OCPN_AISEvent: public wxEvent
{
      public:
            OCPN_AISEvent( wxEventType commandType = wxEVT_NULL, int id = 0 );

            OCPN_AISEvent(const OCPN_AISEvent & event)
            : wxEvent(event),
                m_NMEAstring(event.m_NMEAstring),
                m_extra(event.m_extra)
                { }

                ~OCPN_AISEvent( );

    // accessors
            wxString GetNMEAString() { return m_NMEAstring; }
            void SetNMEAString(wxString string) { m_NMEAstring = string; }

            void SetExtraLong(long n){ m_extra = n;}
            long GetExtraLong(){ return m_extra;}

    // required for sending with wxPostEvent()
            wxEvent *Clone() const;

      private:
            wxString    m_NMEAstring;
            long        m_extra;

};

//    DECLARE_EVENT_TYPE(wxEVT_OCPN_AIS, -1)
extern /*expdecl*/ const wxEventType wxEVT_OCPN_AIS;



//---------------------------------------------------------------------------------
//
//  AIS_Decoder Definition
//
//---------------------------------------------------------------------------------

class AIS_Decoder : public wxEvtHandler
{

public:
    AIS_Decoder(void);
    AIS_Decoder(int window_id, wxFrame *pParent, const wxString& AISDataSource,  wxMutex *pGPSMutex = 0);

    ~AIS_Decoder(void);


    void OnEvtAIS(OCPN_AISEvent& event);
    AIS_Error Decode(const wxString& str);
    void Pause(void);
    void UnPause(void);
    void GetSource(wxString& source);
    AIS_Target_Hash *GetTargetList(void) {return AISTargetList;}
    AIS_Target_Data *Get_Target_Data_From_MMSI(int mmsi);
    int GetNumTargets(void){ return m_n_targets;}
    bool IsAISSuppressed(void){ return m_bSuppressed; }
    bool IsAISAlertGeneral(void) { return m_bGeneralAlert; }


private:
    AIS_Error OpenDataSource(wxFrame *pParent, const wxString& AISDataSource);
    void OnActivate(wxActivateEvent& event);
    void OnSocketEvent(wxSocketEvent& event);
    void OnTimerAIS(wxTimerEvent& event);
    void OnTimerAISAudio(wxTimerEvent& event);

    bool NMEACheckSumOK(const wxString& str);
    bool Parse_VDMBitstring(AIS_Bitstring *bstr, AIS_Target_Data *ptd);
    void UpdateAllCPA(void);
    void UpdateOneCPA(AIS_Target_Data *ptarget);
    void UpdateAllAlarms(void);
    void Parse_And_Send_Posn(wxString &str_temp_buf);
    void ThreadMessage(const wxString &msg);


    AIS_Target_Hash *AISTargetList;


    wxIPV4address     addr;
    wxSocketClient    *m_sock;
    bool              m_busy;
    wxTimer           TimerAIS;
    wxFrame           *m_parent_frame;

    wxString          m_data_source_string;

    int               nsentences;
    int               isentence;
    wxString          sentence_accumulator;
    bool              m_OK;


    NMEA0183         m_NMEA0183;
    wxMutex          *m_pShareGPSMutex;
    wxEvtHandler     *m_pMainEventHandler;

    bool             m_bAIS_Audio_Alert_On;
    wxTimer          m_AIS_Audio_Alert_Timer;
    wxSound          m_AIS_Sound;
    int              m_n_targets;
    bool             m_bSuppressed;
    bool             m_bGeneralAlert;

DECLARE_EVENT_TABLE()


};

//-------------------------------------------------------------------------------------------------------------
//
//    AIS Input Thread
//
//    This thread manages reading the AIS data stream from the declared serial port
//
//-------------------------------------------------------------------------------------------------------------

#ifdef __WXMSW__
#include <windows.h>
#endif


class OCP_AIS_Thread: public wxThread
{

public:

      OCP_AIS_Thread(wxEvtHandler *pParent, const wxString& PortName);
      ~OCP_AIS_Thread(void);
      void *Entry();

      void OnExit(void);

private:
      bool HandleRead(char *buf, int character_count);

      wxEvtHandler            *m_pParentEventHandler;
      wxString                *m_pPortName;
      int                     TimeOutInSec;
      char                    *put_ptr;
      char                    *tak_ptr;

      char                    *rx_buffer;

      char                    *temp_buf;

      unsigned long           error;
      int                     nl_count;

#ifdef __POSIX__
      termios                 *pttyset;
      termios                 *pttyset_old;

      int                     m_ais_fd;
#endif

#ifdef __WXMSW__
      HANDLE                  m_hSerialComm;
#endif
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


            AISInfoWin        *m_pAlertTextCtl;
            int               m_target_mmsi;
            AIS_Decoder       *m_pdecoder;
            wxWindow          *m_pparent;
            wxFont            *m_pFont;
            wxString          m_alert_text;

};

//----------------------------------------------------------------------------------------------------------
//    AISTargetListDialog Specification
//----------------------------------------------------------------------------------------------------------
class AISTargetListDialog: public wxDialog
{
      DECLARE_CLASS( AISTargetListDialog )
      DECLARE_EVENT_TABLE()

      public:
            AISTargetListDialog( );
           ~AISTargetListDialog( );

            bool Create ( wxWindow *parent,
                          AIS_Decoder *pdecoder,
                          wxWindowID id = wxID_ANY,
                          const wxString& caption = _("AIS target list"),
                          const wxPoint& pos = wxDefaultPosition,
                          const wxSize& size = wxDefaultSize,
                          long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );

            void UpdateAISTargetList( );     // Rebuild AIS target list

      private:
            void CreateControls();
            void OnClose( wxCloseEvent& event );
            wxWindow          *m_pparent;
            wxListCtrl        *m_pListCtrlAISTargets;
            AIS_Decoder       *m_pdecoder;
            wxSize            m_size_min;

};




