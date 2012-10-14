/******************************************************************************
 *
 * Project:  OpenCP
 * Purpose:  NMEA Data Object
 * Author:   David Register
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
 *
 *
 *
 *
 */


#ifndef __NMEA_H__
#define __NMEA_H__

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled header

//#include <gio/gio.h>

/////////////TH////////////////////////
#ifndef OCPN_NO_SOCKETS

#ifdef __WXGTK__
#ifdef ocpnHAS_GTK
// newer versions of glib define its own GSocket but we unfortunately use this
// name in our own (semi-)public header and so can't change it -- rename glib
// one instead
#include <gtk/gtk.h>
#define GSocket GlibGSocket
#endif
#endif

#include "wx/socket.h"
#endif
////////////////////TH100126/////////////////


#include <wx/datetime.h>


#include "nmea0183.h"
#include "navutil.h"          // for Routes and Waypoints

#ifdef __POSIX__
#include <sys/termios.h>
#endif

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------

#define TIMER_NMEA_MSEC      997
#define TIMER_LIBGPS_MSEC   997


#define GPSD_PORT_NUMBER      2947                  // Well Known Port number for GPSD

#define SOCKET_ID             5
#define TIMER_NMEA1           777
#define TIMER_LIBGPS1           999

#define TIMER_GARMIN1           950

enum
{
    EVT_NMEA_DIRECT,
    EVT_NMEA_PARSE_RX
};

//          Fwd Declarations
class     MyFrame;
class     OCP_NMEA_Thread;
class     OCP_GARMIN_Thread;
class     ComPortManager;
class     DeviceMonitorWindow;
class     GarminEvent;


// Class declarations


//    A generic Position Data structure
typedef struct {
      double kLat;
      double kLon;
      double kCog;
      double kSog;
      double kVar;            // Variation, typically from RMC message
      double kHdm;            // Magnetic heading
      double kHdt;            // true heading
      time_t FixTime;
      int    nSats;
      double kVMG;            // VMG
} GenericPosDatEx;

// The MY_FILETIME structure is a 64-bit value
//    representing the number of 100-nanosecond
//    intervals since January 1, 1601 (UTC).
// This is the format used in the NMEA server data packet
//    sigh....

typedef struct  {
  unsigned int low;
  unsigned int high;
} MyFileTime;



//----------------------------------------------------------------------------
// NMEAEvent
//----------------------------------------------------------------------------

class OCPN_NMEAEvent: public wxEvent
{
      public:
            OCPN_NMEAEvent( wxEventType commandType = wxEVT_NULL, int id = 0 );

            OCPN_NMEAEvent(const OCPN_NMEAEvent & event)
            : wxEvent(event),
              m_NMEAstring(event.m_NMEAstring)
              { }

             ~OCPN_NMEAEvent( );

    // accessors
            wxString GetNMEAString() { return m_NMEAstring; }
            void SetNMEAString(wxString &string) { m_NMEAstring = string; }


    // required for sending with wxPostEvent()
            wxEvent *Clone() const; // { return new OCPN_NMEAEvent(*this); }

      private:
            wxString    m_NMEAstring;

//            DECLARE_DYNAMIC_CLASS(OCPN_NMEAEvent)

};

    extern  const wxEventType wxEVT_OCPN_NMEA;




//----------------------------------------------------------------------------
// NMEAWindow
//----------------------------------------------------------------------------

class NMEAHandler: public wxEvtHandler
{
public:
      NMEAHandler(int window_id, wxFrame *frame, const wxString& NMEADataSource, const wxString& BaudRate, wxMutex *pMutex = 0, bool bGarmin = false);
      ~NMEAHandler();

      void Close(void);

      void GetSource(wxString& source);

      //    Stop/Start the NMEA Socket Client
      //    Used to prevent async interrupts at critical times
      void Pause(void);
      void UnPause(void);

          //    Secondary thread life toggle
          //    Used to inform launching object (this) to determine if the thread can
          //    be safely called or polled, e.g. wxThread->Destroy();
      void SetSecThreadActive(void){m_bsec_thread_active = true;}
      void SetSecThreadInActive(void){m_bsec_thread_active = false;}

      bool IsPauseRequested(void){ return m_brequest_thread_pause; }

      bool SendWaypointToGPS(RoutePoint *prp, wxString &com_name,  wxGauge *pProgress);
      bool SendRouteToGPS(Route *pr, wxString &com_name,  bool bsend_waypoints, wxGauge *pProgress);

      wxFrame *GetParentFrame(){ return m_parent_frame; }

      DeviceMonitorWindow     *m_pdevmon;
      int                     m_Thread_run_flag;

private:
      void OnSocketEvent(wxSocketEvent& event);
      void OnTimerLIBGPS(wxTimerEvent& event);
      void OnTimerNMEA(wxTimerEvent& event);

#ifndef OCPN_NO_SOCKETS
      wxIPV4address     m_addr;
      wxSocketClient    *m_sock;
#endif

      bool              m_busy;
      wxTimer           TimerNMEA;
      wxFrame           *m_parent_frame;
      int               m_handler_id;

      wxString          m_data_source_string;
      wxEvtHandler      *m_pParentEventHandler;
      wxMutex           *m_pShareMutex;
      wxThread          *m_pSecondary_Thread;
      wxMutex           *m_pPortMutex;
      wxThread          *m_gpsd_thread;

      bool              m_bGarmin_host;

      bool              m_brequest_thread_pause;

      bool              m_bsec_thread_active;
      int               m_gpsd_major;
      int               m_gpsd_minor;
      bool              m_bgot_version;

      //    libgps dynamic load parameters
      void              *m_lib_handle;



DECLARE_EVENT_TABLE()
};



//-------------------------------------------------------------------------------------------------------------
//
//    NMEA Input Thread
//
//    This thread manages reading the NMEA data stream from the declared NMEA or Garmin serial port
//
//-------------------------------------------------------------------------------------------------------------

#ifdef __WXMSW__
#include <windows.h>
#include <winioctl.h>
#include <initguid.h>
#include "setupapi.h"                   // presently stored in opencpn/src
#endif

//    Constants
typedef enum ENUM_BUFFER_STATE
{
      RX_BUFFER_EMPTY,
      RX_BUFFER_FULL
}_ENUM_BUFFER_STATE;

#define MAX_RX_MESSSAGE_SIZE  4096
#define     RX_BUFFER_SIZE                4096



//          Inter-thread communication event declaration

extern /*expdecl*/ const wxEventType EVT_NMEA;
extern /*expdecl*/ const wxEventType EVT_THREADMSG;


class OCP_NMEA_Thread: public wxThread
{

public:

      OCP_NMEA_Thread(NMEAHandler *Launcher, wxWindow *MessageTarget, wxMutex *pMutex, wxMutex *pPortMutex,
                      const wxString& PortName, ComPortManager *pComMan, const wxString& strBaudRate, bool bGarmin = false);
      ~OCP_NMEA_Thread(void);
      void *Entry();

      void OnExit(void);

private:
      void Parse_And_Send_Posn(wxString &str_temp_buf);
      void ThreadMessage(const wxString &msg);          // Send a wxLogMessage to main program event loop
      wxEvtHandler            *m_pMainEventHandler;
      NMEAHandler             *m_launcher;
      wxString                m_PortName;
      wxMutex                 *m_pShareMutex;
      wxMutex                 *m_pPortMutex;

      char                    *put_ptr;
      char                    *tak_ptr;

      char                    *rx_buffer;
      char                    *temp_buf;

      unsigned long           error;

      NMEA0183                m_NMEA0183;

      ComPortManager          *m_pCommMan;

      int                     m_gps_fd;
      int                     m_baud;
      int                     m_n_timeout;


};

//-------------------------------------------------------------------------------------------------------------
//
//    GPSD Library Input Thread
//
//-------------------------------------------------------------------------------------------------------------

class OCP_GPSD_Thread: public wxThread
{

      public:

            OCP_GPSD_Thread(NMEAHandler *Launcher, wxWindow *MessageTarget, wxString &ip_addr, int api);
            ~OCP_GPSD_Thread(void);
            void *Entry();

            void OnExit(void);

      private:
            bool OpenLibrary(void);
            void CloseLibrary(void);

            void ThreadMessage(const wxString &msg);          // Send a wxLogMessage to main program event loop
            wxEvtHandler            *m_pMainEventHandler;
            NMEAHandler             *m_launcher;
            bool                    m_bgps_present;
            int                     m_libgps_api;
            wxString                m_GPSD_data_ip;

            struct gps_data_t *m_pgps_data;

            unsigned int            m_PACKET_SET;
            unsigned int            m_TIME_SET;
            unsigned int            m_LATLON_SET;
            unsigned int            m_TRACK_SET;
            unsigned int            m_SPEED_SET;
            unsigned int            m_SATELLITE_SET;
            unsigned int            m_ERROR_SET;
            unsigned int            m_STATUS_SET;

            struct gps_fix_t  *m_pfix;
            int               *m_pstatus;
            int               *m_psats_viz;
};


//-------------------------------------------------------------------------------------------------------------
//
//    A simple thread to test host name resolution without blocking the main thread
//
//-------------------------------------------------------------------------------------------------------------
class DNSTestThread: public wxThread
{
public:

      DNSTestThread(wxString &name_or_ip);
      ~DNSTestThread();
      void *Entry();


private:
      wxString *m_pip;
};

//-------------------------------------------------------------------------------------------------------------
//
//    Autopilot Class Definition
//
//-------------------------------------------------------------------------------------------------------------

class AutoPilotWindow: public wxWindow
{
public:
    AutoPilotWindow(wxFrame *frame, const wxString& AP_Port);
    ~AutoPilotWindow();

    void OnCloseWindow(wxCloseEvent& event);
    void GetAP_Port(wxString& source);
    int AutopilotOut(const wxString& Sentence);
    bool IsOK(){ return m_bOK;}


private:
    bool            OpenPort(wxString &port);

    wxString        *m_pdata_ap_port_string;
    wxString        m_port;
    int             m_ap_fd;
    bool            m_bOK;

DECLARE_EVENT_TABLE()
};

//-------------------------------------------------------------------------------------------------------------
//
//    Communications Port Manager
//
//-------------------------------------------------------------------------------------------------------------

class OpenCommPortElement
{
public:
      wxString    com_name;
      int         port_descriptor;
      int         n_open;
      int         baud_rate;
};

//    Declare a list of open comm ports
WX_DECLARE_LIST(OpenCommPortElement, ListOfOpenCommPorts);


class ComPortManager:  public wxEvtHandler
{
public:
      ComPortManager();
      ~ComPortManager();

      int OpenComPort(wxString &com_name, int baud_rate);
      OpenCommPortElement *GetComPort(wxString &com_name);
      int CloseComPort(int fd);

      int WriteComPort(wxString& com_name, const wxString& string);
      int WriteComPort(wxString& com_name, unsigned char *msg, int count);
      int ReadComPort(wxString& com_name, int count, unsigned char *p);
      bool SerialCharsAvail(wxString& com_name);


      bool GetLogFlag(){ return m_blog; }
      void SetLogFlag(bool flag){ m_blog = flag; }

private:
      int OpenComPortPhysical(wxString &com_name, int baud_rate);
      int CloseComPortPhysical(int fd);
      int WriteComPortPhysical(int port_descriptor, const wxString& string);
      int WriteComPortPhysical(int port_descriptor, unsigned char *msg, int count);

      int ReadComPortPhysical(int port_descriptor, int count, unsigned char *p);
      bool CheckComPortPhysical(int port_descriptor);

      ListOfOpenCommPorts     m_port_list;

      bool        m_blog;

};


#ifdef __WXMSW__

#endif

#endif

