/******************************************************************************
 * $Id: nmea.h,v 1.32 2010/06/04 22:36:27 bdbcat Exp $
 *
 * Project:  OpenCP
 * Purpose:  NMEA Data Object
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
 *
 * Revision 1.25  2010/01/25 01:31:53  badfeed
 * $Log: nmea.h,v $
 * Revision 1.32  2010/06/04 22:36:27  bdbcat
 * 604
 *
 * Revision 1.31  2010/05/31 00:41:28  bdbcat
 * 530
 *
 * Revision 1.30  2010/05/25 00:11:05  bdbcat
 * libgps
 *
 * Revision 1.29  2010/04/27 01:44:56  bdbcat
 * Build 426
 *
 * Revision 1.28  2010/03/29 02:59:02  bdbcat
 * 2.1.0 Beta Initial
 *
 * Revision 1.27  2010/02/03 02:55:14  badfeed
 * Re-enable GSocket rename workaround for gtk, take 2.
 *
 * Revision 1.26  2010/01/25 20:27:15  badfeed
 * Undo re-enabled GSocket rename workaround for gtk, until a better approach is found
 *
 * Re-enable GSocket rename workaround for gtk
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

#ifdef __WXGTK__
// newer versions of glib define its own GSocket but we unfortunately use this
// name in our own (semi-)public header and so can't change it -- rename glib
// one instead
#include <gtk/gtk.h>
#define GSocket GlibGSocket
#endif

#include "wx/socket.h"
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

enum
{
    EVT_NMEA_DIRECT,
    EVT_NMEA_PARSE_RX
};

//          Fwd Declarations
class     CSyncSerialComm;
class     MyFrame;
class     OCP_NMEA_Thread;
class     OCP_GARMIN_Thread;
class     ComPortManager;



// Class declarations

//    A generic Position Data structure
typedef struct {
      double kLat;
      double kLon;
      double kCog;
      double kSog;
      double kVar;            // Variation, typically from RMC message
      time_t FixTime;
      int    nSats;
} GenericPosDat;


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

private:
      void OnSocketEvent(wxSocketEvent& event);
      void OnTimerLIBGPS(wxTimerEvent& event);
      void OnTimerNMEA(wxTimerEvent& event);

      wxIPV4address     m_addr;
      struct gps_data_t *m_gps_data;

      wxSocketClient    *m_sock;
      bool              m_busy;
      wxTimer           TimerLIBGPS;
      wxTimer           TimerNMEA;
      wxFrame           *m_parent_frame;
      int               m_handler_id;

      wxString          m_data_source_string;
      wxEvtHandler      *m_pParentEventHandler;
      wxMutex           *m_pShareMutex;
      wxThread          *m_pSecondary_Thread;
      wxMutex           *m_pPortMutex;

      bool              m_bGarmin_host;

      bool              m_brequest_thread_pause;

      bool              m_bsec_thread_active;
      int               m_gpsd_major;
      int               m_gpsd_minor;
      bool              m_bgot_version;

      //    libgps dynamic load parameters
      void              *m_lib_handle;

      struct gps_data_t *(*m_fn_gps_open)(char *, char *);
      int                (*m_fn_gps_close)(struct gps_data_t *);
      int                (*m_fn_gps_poll)(struct gps_data_t *);
      bool               (*m_fn_gps_waiting)(struct gps_data_t *);
      void               (*m_fn_gps_set_raw_hook)(struct gps_data_t *, void (*)(struct gps_data_t *, char *, size_t));
      int                (*m_fn_gps_stream)(struct gps_data_t *, unsigned int, void *);
      bool               m_bgps_present;


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


};



//-------------------------------------------------------------------------------------------------------------
//
//    Garmin Input Thread
//
//    This thread manages reading the positioning data stream from the declared Garmin USB device
//
//-------------------------------------------------------------------------------------------------------------

#ifdef __WXMSW__
#include <windows.h>
#endif


//      Local Endian conversions
void le_write16(void *addr, const unsigned value);
void le_write32(void *addr, const unsigned value);
signed int le_read16(const void *addr);
signed int le_read32(const void *addr);



//              Some Garmin Data Structures and Constants
#define GARMIN_USB_API_VERSION 1
#define GARMIN_USB_MAX_BUFFER_SIZE 4096
#define GARMIN_USB_INTERRUPT_DATA_SIZE 64

#define IOCTL_GARMIN_USB_API_VERSION CTL_CODE \
        (FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GARMIN_USB_INTERRUPT_IN CTL_CODE \
        (FILE_DEVICE_UNKNOWN, 0x850, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GARMIN_USB_BULK_OUT_PACKET_SIZE CTL_CODE \
        (FILE_DEVICE_UNKNOWN, 0x851, METHOD_BUFFERED, FILE_ANY_ACCESS)


/*
 * New packet types in USB.
 */
#define GUSB_SESSION_START 5    /* We request units attention */
#define GUSB_SESSION_ACK   6    /* Unit responds that we have its attention */
#define GUSB_REQUEST_BULK  2    /* Unit requests we read from bulk pipe */

#define GUSB_RESPONSE_PVT  51   /* PVT Data Packet */
#define GUSB_RESPONSE_SDR  114  /* Satellite Data Record Packet */




typedef
union {
        struct {
        unsigned char type;
        unsigned char reserved1;
        unsigned char reserved2;
        unsigned char reserved3;
        unsigned char pkt_id[2];
        unsigned char reserved6;
        unsigned char reserved7;
        unsigned char datasz[4];
        unsigned char databuf[1]; /* actually a variable length array... */
        } gusb_pkt;
        unsigned char dbuf[1024];
} garmin_usb_packet;


typedef struct garmin_unit_info {
        unsigned long serial_number;
        unsigned long unit_id;
        unsigned long unit_version;
        char *os_identifier; /* In case the OS has another name for it. */
        char *product_identifier; /* From the hardware itself. */
} unit_info_type;



/*              Packet structure for Pkt_ID = 51 (PVT Data Record)   */
//#pragma pack(push)  /* push current alignment to stack */
//#pragma pack(1)     /* set alignment to 1 byte boundary */
#pragma pack(push,1) /* push current alignment to stack, set alignment to 1 byte boundary */

typedef struct {
        float   alt;
        float   epe;
        float   eph;
        float   epv;
        short   fix;
        double  tow;
        double  lat;
        double  lon;
        float   east;
        float   north;
        float   up;
        float   msl_hght;
        short   leap_scnds;
        long    wn_days;
} D800_Pvt_Data_Type;

#pragma pack(pop)   /* restore original alignment from stack */




/*              Packet structure for Pkt_ID = 114 (Satellite Data Record)   */
typedef    struct
           {
           unsigned char         svid;          //space vehicle identification (1-32 and 33-64 for WAAS)
           short                 snr;           //signal-to-noise ratio
           unsigned char         elev;          //satellite elevation in degrees
           short                 azmth;         //satellite azimuth in degrees
           unsigned char         status;        //status bit-field
           } cpo_sat_data;

/*
The status bit field represents a set of booleans described below:
       Bit      Meaning when bit is one (1)
        0       The unit has ephemeris data for the specified satellite.
        1       The unit has a differential correction for the specified satellite.
        2       The unit is using this satellite in the solution.
*/


enum {
        rs_fromintr,
        rs_frombulk
};



class OCP_GARMIN_Thread: public wxThread
{

public:

      OCP_GARMIN_Thread(NMEAHandler *Launcher, wxWindow *MessageTarget, wxMutex *pMutex, const wxString& PortName);
      ~OCP_GARMIN_Thread(void);
      void *Entry();

      void OnExit(void);

private:
#ifdef __WXMSW__
      HANDLE garmin_usb_start(HDEVINFO hdevinfo, SP_DEVICE_INTERFACE_DATA *infodata);
      bool gusb_syncup(void);

      int gusb_win_get(garmin_usb_packet *ibuf, size_t sz);
      int gusb_win_get_bulk(garmin_usb_packet *ibuf, size_t sz);
      int gusb_win_send(const garmin_usb_packet *opkt, size_t sz);

      int gusb_cmd_send(const garmin_usb_packet *opkt, size_t sz);
      int gusb_cmd_get(garmin_usb_packet *ibuf, size_t sz);
#endif

      void ThreadMsg(const wxString &msg);          // Send a wxLogMessage to main program event loop
      bool ResetGarminUSBDriver();

      wxEvtHandler            *m_pMainEventHandler;
      NMEAHandler             *m_launcher;
      wxMutex                 *m_pShareMutex;
      wxString                m_PortName;

#ifdef __WXMSW__
      HANDLE                  m_usb_handle;
#endif

      int                     m_max_tx_size;
      int                     m_receive_state;
      cpo_sat_data            m_sat_data[12];
      unit_info_type          grmin_unit_info[2];
      int                     m_nSats;
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




#endif

