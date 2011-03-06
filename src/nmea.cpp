/******************************************************************************
 * $Id: nmea.cpp,v 1.63 2010/06/21 01:57:16 bdbcat Exp $
 *
 * Project:  OpenCPN
 * Purpose:  NMEA Data Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   $EMAIL$   *
 *
 *   Garmin JEEPS Code Copyright (C) 2006 Robert Lipe                      *
 *   GPSBabel and JEEPS code are released under GPL V2                     *
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

 ***************************************************************************
 *  Parts of this file were adapted from source code found in              *
 *  John F. Waers (jfwaers@csn.net) public domain program MacGPS45         *
 ***************************************************************************
 *
 */
#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include "wx/tokenzr.h"
#include <wx/datetime.h>

#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "dychart.h"

#include "nmea.h"
#include "georef.h"
#include "garmin/jeeps/garmin_wrapper.h"

#ifdef __WXOSX__
#include "macsercomm.h"
#endif

#ifdef BUILD_WITH_LIBGPS
      #ifdef __WXOSX__ // begin rdm
            #define policy_t gps_policy_t
            #include <gps.h>
            #undef policy_t
      #else
            #include <gps.h>
      #endif // end rdm

      #include <dlfcn.h>
#endif


#define SERIAL_OVERLAPPED

CPL_CVSID("$Id: nmea.cpp,v 1.63 2010/06/21 01:57:16 bdbcat Exp $");

extern int              g_nNMEADebug;
extern ComPortManager   *g_pCommMan;
extern bool             g_bGPSAISMux;
extern int              g_total_NMEAerror_messages;

int                      s_dns_test_flag;

//    A static structure storing generic position data
//    Used to communicate from NMEA threads to main application thread
static      GenericPosDat     ThreadPositionData;

extern bool             g_bDebugGPSD;
extern MyFrame          *gFrame;

//------------------------------------------------------------------------------
//    NMEA Event Implementation
//------------------------------------------------------------------------------

const wxEventType wxEVT_OCPN_NMEA = wxNewEventType();

OCPN_NMEAEvent::OCPN_NMEAEvent( wxEventType commandType, int id )
      :wxEvent(id, commandType)
{
}




OCPN_NMEAEvent::~OCPN_NMEAEvent( )
{
}

wxEvent* OCPN_NMEAEvent::Clone() const
{
      OCPN_NMEAEvent *newevent=new OCPN_NMEAEvent(*this);
      newevent->m_NMEAstring=this->m_NMEAstring.c_str();  // this enforces a deep copy of the string data
      return newevent;
}

#ifdef __WXMSW__

class       GARMIN_IO_Thread;

//--------------------------------------------------------
//              Some Garmin Data Structures and Constants
//--------------------------------------------------------
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





//----------------------------------------------------------------------------
// Garmin Device Monitor Window Definition
//----------------------------------------------------------------------------

class DeviceMonitorWindow: public wxWindow
{
      public:
            DeviceMonitorWindow(NMEAHandler *parent, wxWindow *MessageTarget, wxMutex *pMutex);
            ~DeviceMonitorWindow();

            void OnClose(wxCloseEvent& event);

            WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam);

            void StopIOThread(bool b_pause);
            void RestartIOThread(void);

            void OnTimerGarmin1(wxTimerEvent& event);
            void OnEvtGarmin(GarminEvent &event);

            bool FindGarminDeviceInterface();
            bool IsGarminPlugged();
            HANDLE garmin_usb_start();
            bool gusb_syncup(void);

            int gusb_win_get(garmin_usb_packet *ibuf, size_t sz);
            int gusb_win_get_bulk(garmin_usb_packet *ibuf, size_t sz);
            int gusb_win_send(const garmin_usb_packet *opkt, size_t sz);

            int gusb_cmd_send(const garmin_usb_packet *opkt, size_t sz);
            int gusb_cmd_get(garmin_usb_packet *ibuf, size_t sz);

            bool ResetGarminUSBDriver();

            wxEvtHandler            *m_pMainEventHandler;
            NMEAHandler             *m_pparent;

            wxMutex                 *m_pShareMutex;
            HANDLE                  m_usb_handle;
            int                     m_max_tx_size;
            int                     m_receive_state;
            cpo_sat_data            m_sat_data[12];
            unit_info_type          grmin_unit_info[2];
            int                     m_nSats;
            wxTimer                 TimerGarmin1;

            int                     m_Thread_run_flag;
            GARMIN_IO_Thread        *m_io_thread;
		bool                    m_bneed_int_reset;
            int                     m_ndelay;

            DECLARE_EVENT_TABLE()
};
#endif  //__WXMSW__


//------------------------------------------------------------------------------
//    NMEA Event Handler Implementation
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(NMEAHandler, wxEvtHandler)

            EVT_SOCKET(SOCKET_ID, NMEAHandler::OnSocketEvent)
            EVT_TIMER(TIMER_LIBGPS1, NMEAHandler::OnTimerLIBGPS)
            EVT_TIMER(TIMER_NMEA1, NMEAHandler::OnTimerNMEA)

  END_EVENT_TABLE()

// constructor
NMEAHandler::NMEAHandler(int handler_id, wxFrame *frame, const wxString& NMEADataSource, const wxString& strBaudRate, wxMutex *pMutex, bool bGarmin)
{
      m_handler_id = handler_id;

      m_parent_frame = frame;
      m_pParentEventHandler = m_parent_frame->GetEventHandler();

      m_pShareMutex = pMutex;

      m_pdevmon = NULL;
      m_pSecondary_Thread = NULL;
      m_sock = NULL;
      SetSecThreadInActive();

      TimerLIBGPS.SetOwner(this, TIMER_LIBGPS1);
      TimerLIBGPS.Stop();

      TimerNMEA.SetOwner(this, TIMER_NMEA1);
      TimerNMEA.Stop();

      m_data_source_string = NMEADataSource;

      m_bGarmin_host = bGarmin;
      m_pPortMutex = new wxMutex;
      m_brequest_thread_pause = false;

      m_gpsd_major = 0;
      m_gpsd_minor = 0;
      m_bgot_version = false;

      m_gps_data = NULL;
      m_bgps_present = false;

      ThreadPositionData.FixTime = 0;
      ThreadPositionData.kLat = 0.;
      ThreadPositionData.kLon = 0.;
      ThreadPositionData.kCog = 0.;
      ThreadPositionData.kSog = 0.;
      ThreadPositionData.nSats = 0;

//      Create and manage NMEA data stream source

//    Decide upon NMEA source
      wxString msg(_T("NMEA Data Source is...."));
      msg.Append(m_data_source_string);
      wxLogMessage(msg);

//    NMEA Data Source is shared with AIS port, AIS does the muxing
//    and we don't have anything else to do......
      g_bGPSAISMux = false;
      if(m_data_source_string.Contains(_T("AIS")))
            g_bGPSAISMux = true;

//    NMEA Data Source is specified serial port
      else if(m_data_source_string.Contains(_T("Serial")))
      {
          wxString comx;
          comx =  m_data_source_string.AfterFirst(':');      // strip "Serial:"

#ifdef __WXMSW__
          wxString scomx = comx;
          scomx.Prepend(_T("\\\\.\\"));                  // Required for access to Serial Ports greater than COM9

//  As a quick check, verify that the specified port is available
            HANDLE m_hSerialComm = CreateFile(scomx.fn_str(),       // Port Name
                                             GENERIC_READ,
                                             0,
                                             NULL,
                                             OPEN_EXISTING,
                                             0,
                                             NULL);

            if(m_hSerialComm == INVALID_HANDLE_VALUE)
            {
                  wxString msg(comx);
                  msg.Prepend(_("  Could not open serial port '"));
                  msg.Append(_("'\nSuggestion: Try closing other applications."));
                  wxMessageDialog md(m_parent_frame, msg, _("OpenCPN Message"), wxICON_ERROR );
                  md.ShowModal();

                  return;
            }

            else
                  CloseHandle(m_hSerialComm);

//    Kick off the NMEA RX thread
            m_pSecondary_Thread = new OCP_NMEA_Thread(this, frame, pMutex, m_pPortMutex, comx, g_pCommMan, strBaudRate);
            m_Thread_run_flag = 1;
            m_pSecondary_Thread->Run();

#endif

#ifdef __POSIX__
//    Kick off the NMEA RX thread
            m_pSecondary_Thread = new OCP_NMEA_Thread(this, frame, pMutex, m_pPortMutex, comx, g_pCommMan, strBaudRate, true);
            m_Thread_run_flag = 1;
            m_pSecondary_Thread->Run();
#endif

      }

#ifdef __WXMSW__
      else if(m_data_source_string.Contains(_T("GARMIN")))
      {
            m_pdevmon = new DeviceMonitorWindow(this, frame, pMutex);
      }
#endif

#ifdef BUILD_WITH_LIBGPS
        else if(m_data_source_string.Contains(_T("LIBGPS")))
        {
            wxString NMEA_data_ip;
            NMEA_data_ip = m_data_source_string.Mid(7);         // extract the IP

#define DYNAMIC_LOAD_LIBGPS 1
#ifdef DYNAMIC_LOAD_LIBGPS

            m_lib_handle = dlopen("libgps.so.19", RTLD_LAZY);

            if(!m_lib_handle)
            {
                  wxLogMessage(_("Failed to load libgps.so.19"));
                  wxLogMessage(_("Attempting to load libgps.so"));
                  m_lib_handle = dlopen("libgps.so", RTLD_LAZY);
            }


            if(!m_lib_handle)
            {
                  wxLogMessage(_("Failed to load libgps.so"));
                  wxString msg(_("Unable to load libgps.so"));
                  wxMessageDialog md(m_parent_frame, msg, _("OpenCPN Message"), wxICON_ERROR );
                  md.ShowModal();
                  return;
            }

            else
            {
                  void *p = dlsym(m_lib_handle, "gps_open");
                  m_fn_gps_open =                  (gps_data_t *(*)(char *, char *))p;

                  p = dlsym(m_lib_handle, "gps_close");
                  m_fn_gps_close =                 (int(*)(struct gps_data_t *))p;

                  p = dlsym(m_lib_handle, "gps_poll");
                  m_fn_gps_poll =                (int(*)(struct gps_data_t *))p;

                  p = dlsym(m_lib_handle, "gps_waiting");
                  m_fn_gps_waiting =              (bool(*)(struct gps_data_t *))p;

                  p = dlsym(m_lib_handle, "gps_set_raw_hook");
                  m_fn_gps_set_raw_hook =          (void (*)(struct gps_data_t *, void (*)(struct gps_data_t *, char *, size_t)))p;

                  p = dlsym(m_lib_handle, "gps_stream");
                  m_fn_gps_stream =                (int  (*)(struct gps_data_t *, unsigned int, void *))p;

                  if(!m_fn_gps_open          ||
                      !m_fn_gps_close        ||
                      !m_fn_gps_poll         ||
                      !m_fn_gps_waiting      ||
                      !m_fn_gps_set_raw_hook ||
                      !m_fn_gps_stream       )
                  {
                        wxString msg(NMEA_data_ip);
                        msg.Prepend(_("Unable to initialize libgps.\n\ngpsd host is: "));
                        wxMessageDialog md(m_parent_frame, msg, _("OpenCPN Message"), wxICON_ERROR );
                        md.ShowModal();
                        return;
                  }


            }
#else

            //set up function pointers to required libgps functions

            m_fn_gps_open =                (gps_data_t *(*)(char *, char *))gps_open;
            m_fn_gps_close =               (int(*)(struct gps_data_t *))gps_close;
            m_fn_gps_poll =                (int(*)(struct gps_data_t *))gps_poll;
            m_fn_gps_waiting =             (bool(*)(struct gps_data_t *))gps_waiting;
            m_fn_gps_set_raw_hook =        (void (*)(struct gps_data_t *, void (*)(struct gps_data_t *, char *, size_t)))gps_set_raw_hook;
            m_fn_gps_stream =              (int  (*)(struct gps_data_t *, unsigned int, void *))gps_stream;


#endif

            //    Try to open the library
            m_gps_data = m_fn_gps_open(NMEA_data_ip.char_str(), (char *)DEFAULT_GPSD_PORT);

            if(!m_gps_data)
            {
                wxString msg(NMEA_data_ip);
                msg.Prepend(_("Call to gps_open failed.\nIs gpsd running?\n\ngpsd host is: "));
                wxMessageDialog md(m_parent_frame, msg, _("OpenCPN Message"), wxICON_ERROR );
                md.ShowModal();
                return;
            }

            int n_check_version = 5;            // check up to five times
            bool b_version_match = false;
            bool b_use_lib = false;
            bool b_version_set = false;
            struct version_t check_version;
            check_version.proto_major =0; check_version.proto_minor = 0;

            while(n_check_version)
            {
            //    Check library version
                  if(m_fn_gps_waiting(m_gps_data))
                        m_fn_gps_poll(m_gps_data);

                  if(m_gps_data->set & VERSION_SET)
                  {
                        b_version_set = true;
                        check_version = m_gps_data->version;

                        if((check_version.proto_major >= 3) && (check_version.proto_minor >= 0))
                        {
                              b_version_match = true;
                              b_use_lib = true;
                              break;
                        }
                        else
                              break;
                  }

                  n_check_version--;
            }

            if(!b_version_set)
            {
                  wxString msg(_("Possible libgps API version mismatch.\nlibgps did not reasonably respond to version request\n\n\
                              Would you like to use this version of libgps anyway?"));
                  wxMessageDialog md(m_parent_frame, msg, _("OpenCPN Message"), wxICON_EXCLAMATION | wxYES_NO | wxYES_DEFAULT );

                  if(wxID_YES == md.ShowModal())
                        b_use_lib = true;
            }

            if( b_version_set && !b_version_match )
            {
                  wxString msg;
                  msg.Printf(_("libgps API version mismatch.\nOpenCPN found version %d.%d, but requires at least version 3.0\n\n\
Would you like to use this version of libgps anyway?"),
                             check_version.proto_major, check_version.proto_minor);
                  wxMessageDialog md(m_parent_frame, msg, _("OpenCPN Message"), wxICON_EXCLAMATION | wxYES_NO | wxYES_DEFAULT );

                  if(wxID_YES == md.ShowModal())
                        b_use_lib = true;
            }

            wxString msg;
            msg.Printf(_T("Found libgps version %d.%d"), check_version.proto_major, check_version.proto_minor);
            wxLogMessage(msg);

            if(b_use_lib)
            {
                  m_fn_gps_stream(m_gps_data, WATCH_ENABLE, NULL);
                  TimerLIBGPS.Start(TIMER_LIBGPS_MSEC,wxTIMER_CONTINUOUS);
                  m_bgps_present = true;              // assume this for now....
            }
            else
            {
                  m_fn_gps_close(m_gps_data);
                  m_gps_data = NULL;

                  return;
            }

        }
#endif


#ifndef OCPN_DISABLE_SOCKETS
//      NMEA Data Source is private TCP/IP Server
        else if(m_data_source_string.Contains(_T("GPSD")))
        {
            wxString NMEA_data_ip;
            NMEA_data_ip = m_data_source_string.Mid(5);         // extract the IP

        // Create the socket
            m_sock = new wxSocketClient();

        // Setup the event handler and subscribe to most events
            m_sock->SetEventHandler(*this, SOCKET_ID);

            m_sock->SetNotify(wxSOCKET_CONNECTION_FLAG |
                    wxSOCKET_INPUT_FLAG |
                    wxSOCKET_LOST_FLAG);
            m_sock->Notify(TRUE);
            m_sock->SetTimeout(1);              // Short timeout

            m_busy = FALSE;


//    Build the target address

//    n.b. Win98
//    wxIPV4address::Hostname() uses sockets function gethostbyname() for address resolution
//    Implications...Either name target must exist in c:\windows\hosts, or
//                            a DNS server must be active on the network.
//    If neither true, then wxIPV4address::Hostname() will block (forever?)....
//
//    Workaround....
//    Use a thread to try the name lookup, in case it hangs

            DNSTestThread *ptest_thread = NULL;
            ptest_thread = new DNSTestThread(NMEA_data_ip);

            ptest_thread->Run();                      // Run the thread from ::Entry()


//    Sleep and loop for N seconds
#define SLEEP_TEST_SEC  2

            for(int is=0 ; is<SLEEP_TEST_SEC * 10 ; is++)
            {
                wxMilliSleep(100);
                if(s_dns_test_flag)
                    break;
            }

            if(!s_dns_test_flag)
            {

                wxString msg(NMEA_data_ip);
                msg.Prepend(_T("Could not resolve TCP/IP host '"));
                msg.Append(_T("'\n Suggestion: Try 'xxx.xxx.xxx.xxx' notation"));
                wxMessageDialog md(m_parent_frame, msg, _T("OpenCPN Message"), wxICON_ERROR );
                md.ShowModal();

                m_sock->Notify(FALSE);
                m_sock->Destroy();
                m_sock = NULL;

                return;
            }

            //      Resolved the name, somehow, so Connect() the socket
            m_addr.Hostname(NMEA_data_ip);
            m_addr.Service(GPSD_PORT_NUMBER);
            m_sock->Connect(m_addr, FALSE);       // Non-blocking connect

            TimerNMEA.Start(TIMER_NMEA_MSEC,wxTIMER_CONTINUOUS);
    }
#endif


}

NMEAHandler::~NMEAHandler()
{
}

void NMEAHandler::Close()
{
//    Kill off the libgpsd Socket if alive
#ifdef BUILD_WITH_LIBGPS
      if(m_gps_data)
      {
         m_fn_gps_close(m_gps_data);
         m_gps_data = NULL;
         TimerLIBGPS.Stop();
      }
#endif

//    Kill off the NMEA Socket if alive
      if(m_sock)
      {
            m_sock->Notify(FALSE);
            m_sock->Destroy();
            TimerNMEA.Stop();
      }

//    Kill off the Secondary RX Thread if alive
      if(m_pSecondary_Thread)
      {
          if(m_bsec_thread_active)              // Try to be sure thread object is still alive
          {
                wxLogMessage(_T("Stopping Secondary Thread"));

                m_Thread_run_flag = 0;
                int tsec = 5;
                while((m_Thread_run_flag >= 0) && (tsec--))
                {
                    wxSleep(1);
                }

                wxString msg;
                if(m_Thread_run_flag < 0)
                      msg.Printf(_T("Stopped in %d sec."), 5 - tsec);
                else
                     msg.Printf(_T("Not Stopped after 5 sec."));
                wxLogMessage(msg);

          }

          m_pSecondary_Thread = NULL;
          m_bsec_thread_active = false;
      }

#ifdef __WXMSW__
      if(m_pdevmon)
      {
            m_pdevmon->Close();
            m_pdevmon->Destroy();
            m_pdevmon = NULL;
      }

#endif

      delete m_pPortMutex;
      m_pPortMutex = NULL;

}


void NMEAHandler::GetSource(wxString& source)
{
      source = m_data_source_string;
}


void NMEAHandler::Pause(void)
{
      TimerNMEA.Stop();

      if(m_sock)
            m_sock->Notify(FALSE);
}

void NMEAHandler::UnPause(void)
{
    TimerNMEA.Start(TIMER_NMEA_MSEC,wxTIMER_CONTINUOUS);

      if(m_sock)
            m_sock->Notify(TRUE);
}


int ic;
void NMEAHandler::OnTimerLIBGPS(wxTimerEvent& event)
{
#ifdef BUILD_WITH_LIBGPS
      TimerLIBGPS.Stop();

      if(g_bDebugGPSD) printf("%d\n", ic++);

      while(m_fn_gps_waiting(m_gps_data))
      {
            m_gps_data->set = 0;

            m_fn_gps_poll(m_gps_data);
            if(g_bDebugGPSD) printf("  Poll Set: %0X\n", m_gps_data->set);

            if (!(m_gps_data->set & PACKET_SET))
            {
                  if(g_bDebugGPSD)  printf("Probably lost GPSD\n");
                  m_bgps_present = false;

                  break;                  // this is what happens when gpsd is killed or dies
            }


            if (m_gps_data->set & (DEVICE_SET))
            {
                  if (m_gps_data->dev.activated < 1.0)
                  m_bgps_present = false;
                  else
                        m_bgps_present = true;
            }

            if (m_gps_data->set & DEVICELIST_SET)
            {
                  if (m_gps_data->devices.ndevices == 1)
                  {
                        if (m_gps_data->devices.list[0].activated < 1.0)
                              m_bgps_present = false;
                        else
                              m_bgps_present = true;
                  }
            }


            if(!m_bgps_present)
            {
                  if(g_bDebugGPSD)printf("  no gps device\n");
            }
            else
            {
                  if(g_bDebugGPSD)printf("  GPS!\n");
            }


            if(m_gps_data->set & TIME_SET)
                  ThreadPositionData.FixTime = (time_t)m_gps_data->fix.time;

            if(m_gps_data->set & LATLON_SET)
            {
                  if(g_bDebugGPSD) printf("  LATLON  %g  %g \n", m_gps_data->fix.latitude, m_gps_data->fix.longitude );
                  ThreadPositionData.kLat = m_gps_data->fix.latitude;
                  ThreadPositionData.kLon = m_gps_data->fix.longitude;
            }

            if(m_gps_data->set & TRACK_SET)
            {
                  if(g_bDebugGPSD) printf("  TRACK_SET\n");
                  ThreadPositionData.kCog = 0.;
                  if(!wxIsNaN(m_gps_data->fix.track))
                        ThreadPositionData.kCog = m_gps_data->fix.track;
            }

            if(m_gps_data->set & SPEED_SET)
            {
                  if(g_bDebugGPSD) printf("  SPEED_SET\n");
                  ThreadPositionData.kSog = 0.;
                  if(!wxIsNaN(m_gps_data->fix.speed))
                        ThreadPositionData.kSog = m_gps_data->fix.speed * 3600. / 1852.;      // convert from m/s to knots
            }

            if(m_gps_data->set & SATELLITE_SET)
            {
                  if(g_bDebugGPSD) printf("  SATELLITE_SET  %d\n", m_gps_data->satellites_visible);
                  ThreadPositionData.nSats = m_gps_data->satellites_visible;
            }

            if(m_gps_data->set & ERROR_SET)
            {
                  if(g_bDebugGPSD)
                  {
                        char error[sizeof(m_gps_data->error) + 1];
                        strncpy(error, m_gps_data->error, sizeof(m_gps_data->error));
                        printf("  ERROR_SET  %s\n", error);
                  }

            }

            if(m_bgps_present)  // GPS must be online
            {
                  if(g_bDebugGPSD) printf("  STATUS_SET: %d status %d\n", (m_gps_data->set & STATUS_SET) != 0, m_gps_data->status);
                  if((m_gps_data->set & STATUS_SET) && (m_gps_data->status > 0)) // and producing a fix
                  {
                        wxCommandEvent event( EVT_NMEA,  m_handler_id );
                        event.SetEventObject( (wxObject *)this );
                        event.SetExtraLong(EVT_NMEA_DIRECT);
                        event.SetClientData(&ThreadPositionData);
                        m_pParentEventHandler->AddPendingEvent(event);
                        if(g_bDebugGPSD)  printf(" Sending Event\n");
                   }
            }
      }

      TimerLIBGPS.Start(TIMER_LIBGPS_MSEC,wxTIMER_CONTINUOUS);
#endif
}


void NMEAHandler::OnSocketEvent(wxSocketEvent& event)
{

#define RD_BUF_SIZE    200

    unsigned char *bp;
    unsigned char buf[RD_BUF_SIZE + 1];
    int char_count;
    wxString token;
    double dglat, dglon, dgcog, dgsog;
    double dtime;
    wxDateTime fix_time;
    wxString str_buf;

  switch(event.GetSocketEvent())
  {
      case wxSOCKET_INPUT :                     // from gpsd Daemon
            m_sock->SetFlags(wxSOCKET_WAITALL | wxSOCKET_BLOCK);      // was (wxSOCKET_NOWAIT);
                                                                      // We use wxSOCKET_BLOCK to avoid Yield() reentrancy problems
                                                                      // if a long ProgressDialog is active, as in S57 SENC creation.


            //    Disable input event notifications to preclude re-entrancy on non-blocking socket
 //           m_sock->SetNotify(wxSOCKET_LOST_FLAG);

//          Read the reply, one character at a time, looking for 0x0a (lf)
//          If the reply contains no lf, break on the buffer full

            bp = buf;
            char_count = 0;

            while (char_count < RD_BUF_SIZE)
            {
                m_sock->Read(bp, 1);

                if(m_sock->Error())
                      break;                    // non-specific error, maybe timeout...

                if(*bp == 0x0a)                 // end of sentence
                      break;

                bp++;
                char_count++;
            }

            *bp = 0;                        // end string

//          Validate the string

            str_buf = (wxString((const char *)buf, wxConvUTF8));

            if(str_buf.StartsWith(_T("GPSD,O")))           // valid position data?
            {
                if(str_buf.GetChar(7) != '?')              // GPSd connected and we get real data
                {
                    wxStringTokenizer tkz(str_buf, _T(" "));
                    token = tkz.GetNextToken();

                    token = tkz.GetNextToken();
                    if(token.ToDouble(&dtime))
                         ThreadPositionData.FixTime = (time_t)floor(dtime);

                    token = tkz.GetNextToken();         // skip to lat

                    token = tkz.GetNextToken();
                    if(token.ToDouble(&dglat))
                          ThreadPositionData.kLat = dglat;

                    token = tkz.GetNextToken();
                    if(token.ToDouble(&dglon))
                          ThreadPositionData.kLon = dglon;

                    token = tkz.GetNextToken();         // skip to tmg
                    token = tkz.GetNextToken();
                    token = tkz.GetNextToken();

                    token = tkz.GetNextToken();
                    if(token.ToDouble(&dgcog))
                          ThreadPositionData.kCog = dgcog;

                    token = tkz.GetNextToken();
                    if(token.ToDouble(&dgsog))
                    {
                          ThreadPositionData.kSog = dgsog;                                // default is knots

                          if(m_bgot_version)
                          {
                                if((m_gpsd_major >= 2) && (m_gpsd_minor > 23))            // Speed data will be in meters/sec.
                                      ThreadPositionData.kSog = dgsog * 3600. / 1852.;      // convert from m/s to knots
                          }
                    }


//    Signal the main program

                    wxCommandEvent event( EVT_NMEA,  m_handler_id );
                    event.SetEventObject( (wxObject *)this );
                    event.SetExtraLong(EVT_NMEA_DIRECT);
                    event.SetClientData(&ThreadPositionData);
                    m_pParentEventHandler->AddPendingEvent(event);
                }
            }

            else if(str_buf.StartsWith(_T("GPSD,L")))           // version report?
            {
                  //    Need to see if there are 3 or 4 fields
                 wxStringTokenizer tkz(str_buf, _T(" "));

                 wxString token1 = tkz.GetNextToken();
                 wxString token2 = tkz.GetNextToken();
                 wxString token3 = tkz.GetNextToken();
                 wxString token4 = tkz.GetNextToken();

                 double gpsd_ver;
                 wxString ver_token;
                 if(token4.Len() == 0)                      // 3 fields
                       ver_token = token2;
                 else
                       ver_token = token3;

                 if(ver_token.ToDouble(&gpsd_ver))
                 {
                      m_gpsd_major = (int)floor(gpsd_ver);
                      m_gpsd_minor = (int)floor(((gpsd_ver - m_gpsd_major) * 100) + 0.5);       // substitue for rint
                 }

                 m_bgot_version = true;
            }


                     // Enable input events again.
 //           m_sock->SetNotify(wxSOCKET_LOST_FLAG | wxSOCKET_INPUT_FLAG);

            break;


    case wxSOCKET_LOST:
    {
//          wxSocketError e = m_sock->LastError();          // this produces wxSOCKET_WOULDBLOCK    The socket is non-blocking and the operation would block.
                                                            // which seems to be extraneous
          break;
    }

    case wxSOCKET_CONNECTION :
    {
          //      Sign up for watcher mode
//          char c[10];
//          strcpy(c, "w+\n");
//          m_sock->Write(c, strlen(c));

          break;
    }

    default                  :
          break;
  }
}

extern double gCog, gLat, gLon;

void NMEAHandler::OnTimerNMEA(wxTimerEvent& event)
{
      TimerNMEA.Stop();

      if(!m_bgot_version)                             // Get the gpsd version number
      {
            if(m_sock)
            {
                  if(m_sock->IsConnected())
                  {
                        unsigned char c = 'l';
                        m_sock->Write(&c, 1);
                  }
            }
      }


      if(m_sock)
      {
        if(m_sock->IsConnected())
        {
            unsigned char c = 'O';                    // request data
            m_sock->Write(&c, 1);
        }
        else                                    // try to connect
        {
            m_sock->SetNotify(wxSOCKET_CONNECTION_FLAG |
                          wxSOCKET_INPUT_FLAG |
                          wxSOCKET_LOST_FLAG);

            m_sock->Connect(m_addr, FALSE);       // Non-blocking connect
        }
      }

      //--------------Simulator
#if(0)
      {
            if(m_pShareMutex)
                  wxMutexLocker stateLocker(*m_pShareMutex) ;
            float kSog = 8.5;
            float kCog = 306.;//gCog;// 28.0;                // gCog to simulate, see hotkey arrows

            //    Kludge the startup case
            if(ThreadPositionData.kLat < 1.0)
                  ThreadPositionData.kLat = gLat; //START_LAT;
            if(fabs(ThreadPositionData.kLon) < 1.0)
                  ThreadPositionData.kLon = gLon;//START_LON;

            double pred_lat;
            double pred_lon;

            double dist = kSog / 3600.;
            ll_gc_ll(ThreadPositionData.kLat, ThreadPositionData.kLon, kCog, dist, &pred_lat, &pred_lon);


            ThreadPositionData.kCog = kCog;
            ThreadPositionData.kSog = kSog;
            ThreadPositionData.kLat = pred_lat;
            ThreadPositionData.kLon = pred_lon;
            ThreadPositionData.FixTime = 0;

    //    Signal the main program thread

            wxCommandEvent event( EVT_NMEA,  m_handler_id );
            event.SetEventObject( (wxObject *)this );
            event.SetExtraLong(EVT_NMEA_DIRECT);
            event.SetClientData(&ThreadPositionData);
            m_pParentEventHandler->AddPendingEvent(event);



            // or parsable NMEA string
            wxString buf = _T("$IIGLL,3932.39,N,00320.12,E,,A\r\n");
            buf = _T("$IIHDM,122,M\r\n");
            buf = _T("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47\r\n");
            buf = _T("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,\r\n");
            buf = _T("$GPGLL, 3730.554, N, 122.700, W\r\n");
            buf = _T("$APHDG,323.5,,,,*5F\r\n");
            buf = _T("$APHDM,323.5,M*34\r\n");
            buf = _T("$GPRMC,043047.000,A,4715.3472,N,12217.9889,W,0.00,,180510,,,A*69\r\n");

            buf = _T("$GPGGA,153006.0,4936.950097,N,12400.449524,W,0,05,1.0,0.0,M,,,,*1E\r\n");
            buf = _T("$GPGGA,153856.000,5006.0230,N,01425.8313,E,1,05,1.5,252.6,M,44.3,M,,0000*58\r\n");
            OCPN_NMEAEvent Nevent(wxEVT_OCPN_NMEA, 0);
            Nevent.SetNMEAString(buf);
///            m_pParentEventHandler->AddPendingEvent(Nevent);

      }
#endif



      TimerNMEA.Start(TIMER_NMEA_MSEC,wxTIMER_CONTINUOUS);
}


bool NMEAHandler::SendRouteToGPS(Route *pr, wxString &com_name, bool bsend_waypoints, wxGauge *pProgress)
{
      bool ret_bool = false;

#ifdef __WXMSW__
      if(com_name.Upper().Matches(_T("*GARMIN*")))                // Garmin USB Mode
      {
            if(m_pdevmon)
                  m_pdevmon->StopIOThread(true);

            int v_init = Garmin_GPS_Init(NULL, wxString(_T("usb:")));

            if(v_init < 0)
            {
                  wxString msg(_T("   Garmin USB GPS could not be initialized"));
                  wxLogMessage(msg);
                  msg.Printf(_T("   Error Code is %d"), v_init);
                  wxLogMessage(msg);
                  msg = _T("   LastGarminError is: ");
                  msg += GetLastGarminError();
                  wxLogMessage(msg);

                  ret_bool = false;
            }
            else
		{
                  wxLogMessage(_T("Garmin USB Initialized"));

                  wxString msg = _T("USB Unit identifies as: ");
                  wxString GPS_Unit = Garmin_GPS_GetSaveString();
                  msg += GPS_Unit;
                  wxLogMessage(msg);

                  wxLogMessage(_T("Sending Routes..."));
                  int ret1 = Garmin_GPS_SendRoute(NULL, wxString(_T("usb:")), pr, pProgress);

                  if(ret1 != 1)
                  {
                        wxLogMessage(_T("   Error Sending Routes"));
						wxString msg;
                        msg = _T("   LastGarminError is: ");
                        msg += GetLastGarminError();
						wxLogMessage(msg);

                        ret_bool = false;
                  }
				  else
					  ret_bool = true;
		}

            if(m_pdevmon)
                  m_pdevmon->RestartIOThread();

		return ret_bool;
      }
#endif


      if(m_bGarmin_host)
      {
            int ret_val;

            //    Request that the thread should pause
            m_brequest_thread_pause = true;

            ::wxSleep(1);
            bool b_gotPort = false;
            //    Poll, waiting for the Mutex.  Abort after 5 seconds
            long time = ::wxGetLocalTime();
            while(::wxGetLocalTime() < time + 5)
            {
                  if(wxMUTEX_BUSY != m_pPortMutex->TryLock())
                  {
                        b_gotPort = true;
                        break;
                  }
            }

            if(!b_gotPort)
            {
                  wxString msg(_T("Error Sending Route...could not lock Mutex on port: "));
                  msg +=com_name;
                  wxLogMessage(msg);

                  return false;
            }


            if ( pProgress )
            {
                  pProgress->SetValue ( 20 );
                  pProgress->Refresh();
                  pProgress->Update();
            }

            //    Initialize the Garmin receiver, build required Jeeps internal data structures
            int v_init = Garmin_GPS_Init(g_pCommMan, com_name);
            if(v_init < 0)
            {
                  wxString msg(_T("Garmin GPS could not be initialized on port: "));
                  msg +=com_name;
                  wxString err;
                  err.Printf(_T("  Error Code is %d"), v_init);
                  msg += err;

                  msg += _T("\n  LastGarminError is: ");
                  msg += GetLastGarminError();

                  wxLogMessage(msg);

                  ret_bool = false;
                  goto ret_point;
            }
            else
            {
                  wxString msg(_T("Sent Route to Garmin GPS on port: "));
                  msg +=com_name;
                  msg += _T("\n  Unit identifies as: ");
                  wxString GPS_Unit = Garmin_GPS_GetSaveString();
                  msg += GPS_Unit;

                  wxLogMessage(msg);
            }

            if ( pProgress )
            {
                  pProgress->SetValue ( 40 );
                  pProgress->Refresh();
                  pProgress->Update();
            }

            ret_val = Garmin_GPS_SendRoute(g_pCommMan, com_name, pr, pProgress);
            if(ret_val != 1)
            {
                  wxString msg(_T("Error Sending Route to Garmin GPS on port: "));
                  msg +=com_name;
                  wxString err;
                  err.Printf(_T("  Error Code is %d"), ret_val);

                  msg += _T("\n  LastGarminError is: ");
                  msg += GetLastGarminError();

                  msg += err;
                  wxLogMessage(msg);

                  ret_bool = false;
                  goto ret_point;
            }
            else
                  ret_bool = true;

ret_point:
            //    Release the Mutex
            m_brequest_thread_pause = false;
            m_pPortMutex->Unlock();

            if ( pProgress )
            {
                  pProgress->SetValue ( 100 );
                  pProgress->Refresh();
                  pProgress->Update();
            }

            wxMilliSleep ( 500 );

            return ret_bool;

      }
      else                          // Standard NMEA mode
      {


            SENTENCE    snt;
            NMEA0183    oNMEA0183;
            oNMEA0183.TalkerID = _T ( "EC" );

            int nProg = pr->pRoutePointList->GetCount() + 1;
            if ( pProgress )
                 pProgress->SetRange ( 100 );

            int port_fd = g_pCommMan->OpenComPort ( com_name, 4800 );

            //    Send out the waypoints, in order
            if ( bsend_waypoints )
            {
                  wxRoutePointListNode *node = pr->pRoutePointList->GetFirst();

                  int ip = 1;
                  while ( node )
                  {
                        RoutePoint *prp = node->GetData();

                        if ( prp->m_lat < 0. )
                              oNMEA0183.Wpl.Position.Latitude.Set ( -prp->m_lat, _T ( "S" ) );
                        else
                              oNMEA0183.Wpl.Position.Latitude.Set ( prp->m_lat, _T ( "N" ) );

                        if ( prp->m_lon < 0. )
                              oNMEA0183.Wpl.Position.Longitude.Set ( -prp->m_lon, _T ( "W" ) );
                        else
                        oNMEA0183.Wpl.Position.Longitude.Set ( prp->m_lon, _T ( "E" ) );


                        oNMEA0183.Wpl.To = prp->m_MarkName.Truncate ( 6 );

                        oNMEA0183.Wpl.Write ( snt );

                        g_pCommMan->WriteComPort ( com_name, snt.Sentence );

                        if ( pProgress )
                        {
                              pProgress->SetValue ( ( ip * 100 ) / nProg );
                              pProgress->Refresh();
                              pProgress->Update();
                        }

                        wxMilliSleep ( 1000 );

                        node = node->GetNext();

                        ip++;
                  }
            }

            //    Create the NMEA Rte sentence

            oNMEA0183.Rte.Empty();
            oNMEA0183.Rte.TypeOfRoute = CompleteRoute;

            if ( pr->m_RouteNameString.IsEmpty() )
                  oNMEA0183.Rte.RouteName = _T ( "1" );
            else
                  oNMEA0183.Rte.RouteName = pr->m_RouteNameString;

            oNMEA0183.Rte.total_number_of_messages     = 1;
            oNMEA0183.Rte.message_number               = 1;

            //    add the waypoints
            wxRoutePointListNode *node = pr->pRoutePointList->GetFirst();
            while ( node )
            {
                  RoutePoint *prp = node->GetData();
                  oNMEA0183.Rte.AddWaypoint ( prp->m_MarkName.Truncate ( 6 ) );
                  node = node->GetNext();
            }


            oNMEA0183.Rte.Write ( snt );

      //      printf("%s", snt.Sentence.mb_str());
            g_pCommMan->WriteComPort ( com_name, snt.Sentence );


            if ( pProgress )
            {
                  pProgress->SetValue ( 100 );
                  pProgress->Refresh();
                  pProgress->Update();
            }

            wxMilliSleep ( 500 );

            g_pCommMan->CloseComPort ( port_fd );

            return true;
      }
}


bool NMEAHandler::SendWaypointToGPS(RoutePoint *prp, wxString &com_name,  wxGauge *pProgress)
{
      bool ret_bool = false;
#ifdef __WXMSW__
      if(com_name.Upper().Matches(_T("*GARMIN*")))                // Garmin USB Mode
      {
            if(m_pdevmon)
                  m_pdevmon->StopIOThread(true);

            int v_init = Garmin_GPS_Init(NULL, wxString(_T("usb:")));

            if(v_init < 0)
            {
                  wxString msg(_T("   Garmin USB GPS could not be initialized"));
                  wxLogMessage(msg);
                  msg.Printf(_T("   Error Code is %d"), v_init);
                  wxLogMessage(msg);
                  msg = _T("   LastGarminError is: ");
                  msg += GetLastGarminError();
                  wxLogMessage(msg);

                  ret_bool = false;
            }
            else
            {
                  wxLogMessage(_T("Garmin USB Initialized"));

                  wxString msg = _T("USB Unit identifies as: ");
                  wxString GPS_Unit = Garmin_GPS_GetSaveString();
                  msg += GPS_Unit;
                  wxLogMessage(msg);

                  wxLogMessage(_T("Sending Waypoint..."));

                  //    Create a RoutePointList with one item
                  RoutePointList rplist;
                  rplist.Append(prp);

                  int ret1 = Garmin_GPS_SendWaypoints(NULL, wxString(_T("usb:")), &rplist);

                  if(ret1 != 1)
                  {
                        wxLogMessage(_T("   Error Sending Waypoint to Garmin USB"));
                        wxString msg;
                        msg = _T("   LastGarminError is: ");
                        msg += GetLastGarminError();
                        wxLogMessage(msg);

                        ret_bool = false;
                  }
  				  else
					  ret_bool = true;

            }

            if(m_pdevmon)
                  m_pdevmon->RestartIOThread();

            return ret_bool;
      }
#endif

      //    Are we using Garmin Host mode for uploads?
      if(m_bGarmin_host)
      {
            RoutePointList rplist;
            int ret_val;

            //    Request that the thread should pause
            m_brequest_thread_pause = true;

            bool b_gotPort = false;
            //    Poll, waiting for the Mutex.  Abort after 5 seconds
            long time = ::wxGetLocalTime();
            while(::wxGetLocalTime() < time + 5)
            {
                  if(wxMUTEX_BUSY != m_pPortMutex->TryLock())
                  {
                        b_gotPort = true;
                        break;
                  }
            }

            if(!b_gotPort)
            {
                  wxString msg(_T("Error Sending waypoint(s)...could not lock Mutex on port: "));
                  msg +=com_name;
                  wxLogMessage(msg);

                  return false;
            }


            //    Initialize the Garmin receiver, build required Jeeps internal data structures
            int v_init = Garmin_GPS_Init(g_pCommMan, com_name);
            if(v_init < 0)
            {
                  wxString msg(_T("Garmin GPS could not be initialized on port: "));
                  msg +=com_name;
                  wxString err;
                  err.Printf(_T("  Error Code is %d"), v_init);
                  msg += err;

                  msg += _T("\n  LastGarminError is: ");
                  msg += GetLastGarminError();

                   wxLogMessage(msg);

                  ret_bool = false;
                  goto ret_point;
            }
            else
            {
                  wxString msg(_T("Sent waypoint(s) to Garmin GPS on port: "));
                  msg +=com_name;
                  msg += _T("\n  Unit identifies as: ");
                  wxString GPS_Unit = Garmin_GPS_GetSaveString();
                  msg += GPS_Unit;
                  wxLogMessage(msg);
            }

            //    Create a RoutePointList with one item
            rplist.Append(prp);

            ret_val = Garmin_GPS_SendWaypoints(g_pCommMan, com_name, &rplist);
            if(ret_val != 1)
            {
                  wxString msg(_T("Error Sending Waypoint(s) to Garmin GPS on port: "));
                  msg +=com_name;
                  wxString err;
                  err.Printf(_T("  Error Code is %d"), ret_val);
                  msg += err;

                  msg += _T("\n  LastGarminError is: ");
                  msg += GetLastGarminError();

                  wxLogMessage(msg);

                  ret_bool = false;
                  goto ret_point;
            }
            else
                  ret_bool = true;

ret_point:
            //    Release the Mutex
            m_brequest_thread_pause = false;
            m_pPortMutex->Unlock();

            return ret_bool;

      }
      else                          // Standard NMEA mode
      {
            SENTENCE    snt;
            NMEA0183    oNMEA0183;
            oNMEA0183.TalkerID = _T ( "EC" );

            if ( pProgress )
                  pProgress->SetRange ( 100 );

            int port_fd = g_pCommMan->OpenComPort ( com_name, 4800 );

            if ( prp->m_lat < 0. )
                  oNMEA0183.Wpl.Position.Latitude.Set ( -prp->m_lat, _T ( "S" ) );
            else
                  oNMEA0183.Wpl.Position.Latitude.Set ( prp->m_lat, _T ( "N" ) );

            if ( prp->m_lon < 0. )
                  oNMEA0183.Wpl.Position.Longitude.Set ( -prp->m_lon, _T ( "W" ) );
            else
                  oNMEA0183.Wpl.Position.Longitude.Set ( prp->m_lon, _T ( "E" ) );


            oNMEA0183.Wpl.To = prp->m_MarkName.Truncate ( 6 );

            oNMEA0183.Wpl.Write ( snt );

            g_pCommMan->WriteComPort ( com_name, snt.Sentence );

            if ( pProgress )
            {
                  pProgress->SetValue ( 100 );
                  pProgress->Refresh();
                  pProgress->Update();
            }

            wxMilliSleep ( 500 );

            g_pCommMan->CloseComPort ( port_fd );

            return true;
      }

}

//-------------------------------------------------------------------------------------------------------------
//
//    A simple thread to test host name resolution without blocking the main thread
//    Dontcha' love socket's name resolution logic??
//
//-------------------------------------------------------------------------------------------------------------
DNSTestThread::DNSTestThread(wxString &name_or_ip)
{
      m_pip = new wxString(name_or_ip);

      Create();
}

DNSTestThread::~DNSTestThread()
{
      delete m_pip;
}


void *DNSTestThread::Entry()
{
      s_dns_test_flag = 0;

      wxIPV4address     addr;
      addr.Hostname(*m_pip);                          // this may block forever if DNS is not active

      s_dns_test_flag = 1;                            // came back OK
      return NULL;
}


//-------------------------------------------------------------------------------------------------------------
//
//    NMEA Serial Input Thread
//
//    This thread manages reading the NMEA data stream from the declared NMEA serial port
//
//-------------------------------------------------------------------------------------------------------------

//          Inter-thread communication event implementation
DEFINE_EVENT_TYPE(EVT_NMEA)
DEFINE_EVENT_TYPE(EVT_THREADMSG)



//-------------------------------------------------------------------------------------------------------------
//    OCP_NMEA_Thread Static data store
//-------------------------------------------------------------------------------------------------------------

extern char                         rx_share_buffer[];
extern unsigned int                 rx_share_buffer_length;
extern ENUM_BUFFER_STATE            rx_share_buffer_state;


//-------------------------------------------------------------------------------------------------------------
//    OCP_NMEA_Thread Implementation
//-------------------------------------------------------------------------------------------------------------

//    ctor

OCP_NMEA_Thread::OCP_NMEA_Thread(NMEAHandler *Launcher, wxWindow *MessageTarget, wxMutex *pMutex, wxMutex *pPortMutex,
                                 const wxString& PortName, ComPortManager *pComMan, const wxString& strBaudRate, bool bGarmin)
{
      m_launcher = Launcher;                        // This thread's immediate "parent"

      m_pMainEventHandler = MessageTarget->GetEventHandler();

      m_PortName = PortName;


      m_pPortMutex = pPortMutex;

      rx_buffer = new char[RX_BUFFER_SIZE + 1];
      temp_buf = new char[RX_BUFFER_SIZE + 1];

      put_ptr = rx_buffer;                            // local circular queue
      tak_ptr = rx_buffer;

      m_pShareMutex = pMutex;                         // IPC buffer
      rx_share_buffer_state = RX_BUFFER_EMPTY;

      m_pCommMan = pComMan;                           // store a local copy of the ComPortManager pointer

      m_baud = 4800;
      if(strBaudRate == _T("9600"))
            m_baud = 9600;
      else if(strBaudRate == _T("38400"))
            m_baud = 38400;


      Create();

}

OCP_NMEA_Thread::~OCP_NMEA_Thread(void)
{
      delete[] rx_buffer;
      delete[] temp_buf;
}

void OCP_NMEA_Thread::OnExit(void)
{
//  Mark the global status as dead, gone
//    m_launcher->m_pNMEA_Thread = NULL;
}

//      Sadly, the thread itself must implement the underlying OS serial port
//      in a very machine specific way....

#ifdef __POSIX__
//    Entry Point
void *OCP_NMEA_Thread::Entry()
{
      m_launcher->SetSecThreadActive();               // I am alive

      bool not_done = true;
      bool nl_found;
      wxString msg;


      //    Request the com port from the comm manager
      if ((m_gps_fd = m_pCommMan->OpenComPort(m_PortName, m_baud)) < 0)
      {
            wxString msg(_T("NMEA input device open failed: "));
            msg.Append(m_PortName);
            ThreadMessage(msg);
            goto thread_exit;
      }

      if(wxMUTEX_NO_ERROR != m_pPortMutex->Lock())              // I have the ball
      {
            wxString msg(_T("NMEA input device failed to lock Mutex on port : "));
            msg.Append(m_PortName);
            ThreadMessage(msg);
            goto thread_exit;
      }




//    The main loop

    while((not_done) && (m_launcher->m_Thread_run_flag > 0))
    {
        if(TestDestroy())
            not_done = false;                               // smooth exit

        if( m_launcher->IsPauseRequested())                 // external pause requested?
        {
              m_pCommMan->CloseComPort(m_gps_fd);
              m_pPortMutex->Unlock();                       // release the port

              wxThread::Sleep(2000);                        // stall for a bit

              //  Now try to regain the Mutex
              while(wxMUTEX_BUSY == m_pPortMutex->TryLock()){};

              //  Re-initialize the port
              if ((m_gps_fd = m_pCommMan->OpenComPort(m_PortName, m_baud)) < 0)
              {
                    wxString msg(_T("NMEA input device open failed after requested Pause on port: "));
                    msg.Append(m_PortName);
                    ThreadMessage(msg);
                    goto thread_exit;
              }

        }

      //    Blocking, timeout protected read of one character at a time
      //    Timeout value is set by c_cc[VTIME]
      //    Storing incoming characters in circular buffer
      //    And watching for new line character
      //     On new line character, send notification to parent

            char next_byte = 0;
            ssize_t newdata;
            newdata = read(m_gps_fd, &next_byte, 1);            // read of one char
                                                                  // return (-1) if no data available, timeout

      #ifdef __WXOSX__
            if (newdata < 0 )
                  wxThread::Sleep(100) ;
      #endif


      // Fulup patch for handling hot-plug or wakeup events
      // from serial port drivers
            {
                  static int maxErrorLoop;

                  if (newdata > 0)
                  {
            // we have data, so clear error
                        maxErrorLoop =0;
                  }
                  else
                  {
            // no need to retry every 1ms when on error
                        sleep (1);

            // if we have more no character for 5 second then try to reopen the port
                        if (maxErrorLoop++ > 5)
                        {

            // do not retry for the next 5s
                              maxErrorLoop = 0;

            //  Turn off Open/Close logging
                              bool blog = m_pCommMan->GetLogFlag();
                              m_pCommMan->SetLogFlag(false);

            // free old unplug current port
                              m_pCommMan->CloseComPort(m_gps_fd);

            //    Request the com port from the comm manager
                              if ((m_gps_fd = m_pCommMan->OpenComPort(m_PortName, m_baud)) < 0)  {
                                    wxString msg(_T("NMEA input device open failed (will retry): "));
                                    msg.Append(m_PortName);
                                    ThreadMessage(msg);
                              } else {
                                    wxString msg(_T("NMEA input device open on hotplug OK: "));
                              }

            //      Reset the log flag
                              m_pCommMan->SetLogFlag(blog);
                        }
                  }
            } // end Fulup hack


            //  And process any character

            if(newdata > 0)
            {
      //              printf("%c", next_byte);

                  nl_found = false;

                  *put_ptr++ = next_byte;
                  if((put_ptr - rx_buffer) > RX_BUFFER_SIZE)
                  put_ptr = rx_buffer;

                  if(0x0a == next_byte)
                  nl_found = true;


      //    Found a NL char, thus end of message?
                  if(nl_found)
                  {
                  char *tptr;
                  char *ptmpbuf;


      //    Copy the message into a temporary _buffer

                  tptr = tak_ptr;
                  ptmpbuf = temp_buf;

                  while((*tptr != 0x0a) && (tptr != put_ptr))
                  {
                        *ptmpbuf++ = *tptr++;

                        if((tptr - rx_buffer) > RX_BUFFER_SIZE)
                        tptr = rx_buffer;

                        wxASSERT_MSG((ptmpbuf - temp_buf) < RX_BUFFER_SIZE, (const wxChar *)"temp_buf overrun1");

                  }

                  if((*tptr == 0x0a) && (tptr != put_ptr))    // well formed sentence
                  {
                        *ptmpbuf++ = *tptr++;
                        if((tptr - rx_buffer) > RX_BUFFER_SIZE)
                        tptr = rx_buffer;

                        wxASSERT_MSG((ptmpbuf - temp_buf) < RX_BUFFER_SIZE, (const wxChar *)"temp_buf overrun2");

                        *ptmpbuf = 0;

                        tak_ptr = tptr;

      //    Message is ready to parse and send out
                        wxString str_temp_buf(temp_buf, wxConvUTF8);
                        Parse_And_Send_Posn(str_temp_buf);
                  }

                  }                   //if nl
            }                       // if newdata > 0
      //              ThreadMessage(_T("Timeout 1"));

    }                          // the big while...


//          Close the port cleanly
    m_pCommMan->CloseComPort(m_gps_fd);



thread_exit:
      m_launcher->SetSecThreadInActive();             // I am dead
      m_launcher->m_Thread_run_flag = -1;
      return 0;

}


#endif          //__POSIX__


#ifdef __WXMSW__

//    Entry Point
void *OCP_NMEA_Thread::Entry()
{
      wxString msg;

      m_launcher->SetSecThreadActive();       // I am alive

      OVERLAPPED osReader = {0};

      bool not_done;
      BOOL fWaitingOnRead = FALSE;
      HANDLE hSerialComm = (HANDLE)(-1);

           //    Request the com port from the comm manager
      if ((m_gps_fd = m_pCommMan->OpenComPort(m_PortName, m_baud)) < 0)
      {
            wxString msg(_T("NMEA input device open failed: "));
            msg.Append(m_PortName);
            wxString msg_error;
            msg_error.Printf(_T("...GetLastError():  %d"), GetLastError());
            msg.Append(msg_error);

            ThreadMessage(msg);
            goto thread_exit;
      }

      hSerialComm = (HANDLE)m_gps_fd;


#ifdef SERIAL_OVERLAPPED
//    Set up read event specification

      if(!SetCommMask((HANDLE)m_gps_fd, EV_RXCHAR)) // Setting Event Type
      {
            wxString msg(_T("NMEA input device (overlapped) SetCommMask failed: "));
            msg.Append(m_PortName);
            wxString msg_error;
            msg_error.Printf(_T("...GetLastError():  %d"), GetLastError());
            msg.Append(msg_error);

            ThreadMessage(msg);
            goto thread_exit;
      }

// Create the overlapped event. Must be closed before exiting
// to avoid a handle leak.
      osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

      if (osReader.hEvent == NULL)
      {
            wxString msg(_T("NMEA input device (overlapped) CreateEvent failed: "));
            msg.Append(m_PortName);
            wxString msg_error;
            msg_error.Printf(_T("...GetLastError():  %d"), GetLastError());
            msg.Append(msg_error);

            ThreadMessage(msg);
            goto thread_exit;
      }

      if(wxMUTEX_NO_ERROR != m_pPortMutex->Lock())              // I have the ball
      {
            wxString msg(_T("NMEA input device failed to lock Mutex on port : "));
            msg.Append(m_PortName);
            ThreadMessage(msg);
            goto thread_exit;
      }

      not_done = true;
      bool nl_found;

#define READ_BUF_SIZE 20
      char szBuf[READ_BUF_SIZE];

      DWORD dwRead;


//    The main loop

      while((not_done) && (m_launcher->m_Thread_run_flag > 0))
      {
            if(TestDestroy())
            {
                  not_done = false;                               // smooth exit
                  goto thread_exit;                               // smooth exit
            }

            //    Was port closed due to error condition?
            while(!m_gps_fd)
            {
                  if((TestDestroy()) || (m_launcher->m_Thread_run_flag == 0))
                        goto thread_exit;                               // smooth exit

                  if ((m_gps_fd = m_pCommMan->OpenComPort(m_PortName, m_baud)) < 0)
                        m_gps_fd = NULL;
                  wxThread::Sleep(2000);                        // stall for a bit

            }

            if( m_launcher->IsPauseRequested())                 // external pause requested?
            {
                  m_pCommMan->CloseComPort(m_gps_fd);
                  m_pPortMutex->Unlock();                       // release the port

                  wxThread::Sleep(2000);                        // stall for a bit

              //  Now try to regain the Mutex
                  while(wxMUTEX_BUSY == m_pPortMutex->TryLock()){};

              //  Re-initialize the port
                  if ((m_gps_fd = m_pCommMan->OpenComPort(m_PortName, m_baud)) < 0)
                  {
                        wxString msg(_T("NMEA input device open failed after requested Pause on port: "));
                        msg.Append(m_PortName);
                        ThreadMessage(msg);
                        goto thread_exit;
                  }

            }


            if (!fWaitingOnRead)
            {
   // Issue read operation.
                if (!ReadFile(hSerialComm, szBuf, READ_BUF_SIZE, &dwRead, &osReader))
                {
                    if (GetLastError() != ERROR_IO_PENDING)     // read not delayed?
                    {
//                          m_pCommMan->CloseComPort(m_gps_fd);
//                          m_gps_fd = NULL;
                          dwRead = 0;
                          nl_found = false;
                          fWaitingOnRead = FALSE;
                    }
                    else
                        fWaitingOnRead = TRUE;
                }
                else
                {      // read completed immediately
                    goto HandleASuccessfulRead;
                }
            }


            // Read command has been issued, and did not return immediately

#define READ_TIMEOUT      200      // milliseconds

            DWORD dwRes;

            if (fWaitingOnRead)
            {
                  //    Loop forever, checking for thread exit request
                  while(fWaitingOnRead)
                  {
                        if((TestDestroy()) || (m_launcher->m_Thread_run_flag == 0))
                            goto fail_point;                               // smooth exit


                        dwRes = WaitForSingleObject(osReader.hEvent, READ_TIMEOUT);
                        switch(dwRes)
                        {
                              case WAIT_OBJECT_0:
                                    if (!GetOverlappedResult(hSerialComm, &osReader, &dwRead, FALSE))
                                    {
//                                          m_pCommMan->CloseComPort(m_gps_fd);
//                                          m_gps_fd = NULL;
                                          dwRead = 0;
                                          nl_found = false;
                                          fWaitingOnRead = FALSE;
                                    }
                                  else
             // Read completed successfully.
                                    goto HandleASuccessfulRead;

                                  break;

                              case WAIT_TIMEOUT:
                                    if((g_total_NMEAerror_messages < g_nNMEADebug) && (g_nNMEADebug > 1000))
                                    {
                                          g_total_NMEAerror_messages++;
                                          wxString msg;
                                          msg.Printf(_T("NMEA timeout"));
                                          ThreadMessage(msg);
                                    }
                                    break;

                              default:
                                  break;
                        }     // switch
                  }           // while
            }                 // if




HandleASuccessfulRead:

            if(dwRead > 0)
            {
                  if((g_total_NMEAerror_messages < g_nNMEADebug) && (g_nNMEADebug > 1000))
                  {
                        g_total_NMEAerror_messages++;
                        wxString msg;
                        msg.Printf(_T("NMEA activity...%d bytes"), dwRead);
                        ThreadMessage(msg);
                  }

                  int nchar = dwRead;
                  char *pb = szBuf;

                  while(nchar)
                  {
                        if(0x0a == *pb)
                              nl_found = true;

                        *put_ptr++ = *pb++;
                        if((put_ptr - rx_buffer) > RX_BUFFER_SIZE)
                              put_ptr = rx_buffer;

                        nchar--;
                  }
                  if((g_total_NMEAerror_messages < g_nNMEADebug) && (g_nNMEADebug > 1000))
                  {
                        g_total_NMEAerror_messages++;
                        wxString msg1 = _T("Buffer is: ");
                        int nc = dwRead;
                        char *pb = szBuf;
                        while(nc)
                        {
                              msg1.Append(*pb++);
                              nc--;
                        }
                        ThreadMessage(msg1);
                  }
            }


//    Found a NL char, thus end of message?
            if(nl_found)
            {
                  char *tptr;
                  char *ptmpbuf;

                  bool partial = false;
                  while (!partial)
                  {

            //    Copy the message into a temp buffer

                        tptr = tak_ptr;
                        ptmpbuf = temp_buf;

                        while((*tptr != 0x0a) && (tptr != put_ptr))
                        {
                              *ptmpbuf++ = *tptr++;

                              if((tptr - rx_buffer) > RX_BUFFER_SIZE)
                                    tptr = rx_buffer;
                              wxASSERT_MSG((ptmpbuf - temp_buf) < RX_BUFFER_SIZE, _T("temp_buf overrun"));
                        }

                        if((*tptr == 0x0a) && (tptr != put_ptr))    // well formed sentence
                        {
                              *ptmpbuf++ = *tptr++;
                              if((tptr - rx_buffer) > RX_BUFFER_SIZE)
                                    tptr = rx_buffer;
                              wxASSERT_MSG((ptmpbuf - temp_buf) < RX_BUFFER_SIZE, _T("temp_buf overrun"));

                              *ptmpbuf = 0;

                              tak_ptr = tptr;

      // parse and send the message
                              wxString str_temp_buf(temp_buf, wxConvUTF8);
                              Parse_And_Send_Posn(str_temp_buf);
                        }
                        else
                        {
                              partial = true;
                        }
                }                 // while !partial

#ifdef __WXMSW__
//                if(!_CrtCheckMemory())
//                      _CrtDbgBreak( );
#endif

            }           // nl found

       fWaitingOnRead = FALSE;

      }           // the big while...



fail_point:
thread_exit:

//          Close the port cleanly
      m_pCommMan->CloseComPort(m_gps_fd);

      if (osReader.hEvent)
            CloseHandle(osReader.hEvent);

      m_launcher->SetSecThreadInActive();             // I am dead
      m_launcher->m_Thread_run_flag = -1;

      return 0;

#else                   // non-overlapped
//    Set up read event specification

      if(!SetCommMask((HANDLE)m_gps_fd, EV_RXCHAR)) // Setting Event Type
            goto thread_exit;

      not_done = true;
      bool nl_found;

#define READ_BUF_SIZE 20
      char szBuf[READ_BUF_SIZE];

      DWORD dwRead;
      DWORD dwOneRead;
      DWORD dwCommEvent;
      char  chRead;
      int ic;

//    The main loop

      while((not_done) && (m_launcher->m_Thread_run_flag > 0))
      {
            if(TestDestroy())
                  not_done = false;                               // smooth exit


            dwRead = 0;
            ic = 0;

            if (WaitCommEvent((HANDLE)m_gps_fd, &dwCommEvent, NULL))
            {
                do {
                      if (ReadFile((HANDLE)m_gps_fd, &chRead, 1, &dwOneRead, NULL))
                      {
                            szBuf[ic] = chRead;
                            dwRead++;
                            if(ic++ > READ_BUF_SIZE - 1)
                                  goto HandleASuccessfulRead;
                      }
                      else
                      {            // An error occurred in the ReadFile call.
                            goto fail_point;                               // smooth exit

                      }
                } while (dwOneRead);
            }
            else
                  goto fail_point;                               // smooth exit



HandleASuccessfulRead:

            if(dwRead > 0)
            {
                  if((g_total_NMEAerror_messages < g_nNMEADebug) && (g_nNMEADebug > 1000))
                  {
                        g_total_NMEAerror_messages++;
                        wxString msg;
                        msg.Printf(_T("NMEA activity...%d bytes"), dwRead);
                        ThreadMessage(msg);
                  }

                  int nchar = dwRead;
                  char *pb = szBuf;

                  while(nchar)
                  {
                        if(0x0a == *pb)
                              nl_found = true;

                        *put_ptr++ = *pb++;
                        if((put_ptr - rx_buffer) > RX_BUFFER_SIZE)
                              put_ptr = rx_buffer;

                        nchar--;
                  }
                  if((g_total_NMEAerror_messages < g_nNMEADebug) && (g_nNMEADebug > 1000))
                  {
                        g_total_NMEAerror_messages++;
                        wxString msg1 = _T("Buffer is: ");
                        int nc = dwRead;
                        char *pb = szBuf;
                        while(nc)
                        {
                              msg1.Append(*pb++);
                              nc--;
                        }
                        ThreadMessage(msg1);
                  }
            }

//    Found a NL char, thus end of message?
            if(nl_found)
            {
                  char *tptr;
                  char *ptmpbuf;

                  bool partial = false;
                  while (!partial)
                  {

            //    Copy the message into a temp buffer

                        tptr = tak_ptr;
                        ptmpbuf = temp_buf;

                        while((*tptr != 0x0a) && (tptr != put_ptr))
                        {
                              *ptmpbuf++ = *tptr++;

                              if((tptr - rx_buffer) > RX_BUFFER_SIZE)
                                    tptr = rx_buffer;
                              wxASSERT_MSG((ptmpbuf - temp_buf) < RX_BUFFER_SIZE, "temp_buf overrun");
                        }

                        if((*tptr == 0x0a) && (tptr != put_ptr))    // well formed sentence
                        {
                              *ptmpbuf++ = *tptr++;
                              if((tptr - rx_buffer) > RX_BUFFER_SIZE)
                                    tptr = rx_buffer;
                              wxASSERT_MSG((ptmpbuf - temp_buf) < RX_BUFFER_SIZE, "temp_buf overrun");

                              *ptmpbuf = 0;

                              tak_ptr = tptr;

      // parse and send the message
                              if(g_bShowOutlines)
                              {
                                    wxString str_temp_buf(temp_buf, wxConvUTF8);
                                    Parse_And_Send_Posn(str_temp_buf);
                              }
                        }
                        else
                        {
                              partial = true;
                        }
                  }                 // while !partial


            }           // nl found
      }           // the big while...



fail_point:
thread_exit:

//          Close the port cleanly
      m_pCommMan->CloseComPort(m_gps_fd);

      m_launcher->SetSecThreadInActive();             // I am dead
      m_launcher->m_Thread_run_flag = -1;
      return 0;





#endif
}

#endif            // __WXMSW__


void OCP_NMEA_Thread::Parse_And_Send_Posn(wxString &str_temp_buf)
{
      if( g_nNMEADebug && (g_total_NMEAerror_messages < g_nNMEADebug) )
      {
            g_total_NMEAerror_messages++;
            wxString msg(_T("NMEA Sentence received..."));
            msg.Append(str_temp_buf);
            ThreadMessage(msg);
      }


      OCPN_NMEAEvent Nevent(wxEVT_OCPN_NMEA, 0);
      Nevent.SetNMEAString(str_temp_buf);
      m_pMainEventHandler->AddPendingEvent(Nevent);

      return;
}


void OCP_NMEA_Thread::ThreadMessage(const wxString &msg)
{

    //    Signal the main program thread
      wxCommandEvent event( EVT_THREADMSG,  GetId());
      event.SetEventObject( (wxObject *)this );
      event.SetString(msg);
      m_pMainEventHandler->AddPendingEvent(event);

}



//-------------------------------------------------------------------------------------------------------------
//
//    Garmin USB Interface
//
//-------------------------------------------------------------------------------------------------------------

#ifdef __WXMSW__
#include <windows.h>
#include <dbt.h>


// {2C9C45C2-8E7D-4C08-A12D-816BBAE722C0}
DEFINE_GUID(GARMIN_GUID, 0x2c9c45c2L, 0x8e7d, 0x4c08, 0xa1, 0x2d, 0x81, 0x6b, 0xba, 0xe7, 0x22, 0xc0);


void
le_write16(void *addr, const unsigned value)
{
      unsigned char *p = (unsigned char *)addr;
      p[0] = value;
      p[1] = value >> 8;

}

void
le_write32(void *addr, const unsigned value)
{
      unsigned char *p = (unsigned char *)addr;
      p[0] = value;
      p[1] = value >> 8;
      p[2] = value >> 16;
      p[3] = value >> 24;
}

signed int
le_read16(const void *addr)
{
      const unsigned char *p = (const unsigned char *)addr;
      return p[0] | (p[1] << 8);
}

signed int
le_read32(const void *addr)
{
      const unsigned char *p = (const unsigned char *)addr;
      return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}



//-------------------------------------------------------------------------------------------------------------
//
//    Garmin USB Worker Thread
//
//    This thread manages reading the positioning data stream from the declared Garmin USB device
//
//-------------------------------------------------------------------------------------------------------------
class GARMIN_IO_Thread: public wxThread
{

      public:

            GARMIN_IO_Thread(DeviceMonitorWindow *parent, wxEvtHandler *MessageTarget, wxMutex *pMutex, HANDLE device_handle, size_t max_tx_size);
            ~GARMIN_IO_Thread(void);
            void *Entry();


      private:
            int gusb_win_get(garmin_usb_packet *ibuf, size_t sz);
            int gusb_win_get_bulk(garmin_usb_packet *ibuf, size_t sz);
            int gusb_win_send(const garmin_usb_packet *opkt, size_t sz);

            int gusb_cmd_send(const garmin_usb_packet *opkt, size_t sz);
            int gusb_cmd_get(garmin_usb_packet *ibuf, size_t sz);

            wxEvtHandler            *m_pMainEventHandler;
            DeviceMonitorWindow     *m_parent;
            wxMutex                 *m_pShareMutex;

            HANDLE                  m_usb_handle;

            int                     m_receive_state;
            cpo_sat_data            m_sat_data[12];
            unit_info_type          grmin_unit_info[2];
            int                     m_nSats;
		int						m_max_tx_size;
};



//-------------------------------------------------------------------------------------------------------------
//    GARMIN_IO_Thread Implementation
//-------------------------------------------------------------------------------------------------------------

//    ctor

GARMIN_IO_Thread::GARMIN_IO_Thread(DeviceMonitorWindow *parent, wxEvtHandler *MessageTarget, wxMutex *pMutex, HANDLE device_handle, size_t max_tx_size)
{
      m_parent = parent;                        // This thread's immediate "parent"

      m_pMainEventHandler = MessageTarget;

      m_max_tx_size = max_tx_size;

      m_pShareMutex = pMutex;

      m_usb_handle = device_handle;

      Create();

}

GARMIN_IO_Thread::~GARMIN_IO_Thread()
{
}

void *GARMIN_IO_Thread::Entry()
{
      garmin_usb_packet iresp;
	  int n_short_read = 0;
	  wxMutexLocker *pStateLocker;

      m_receive_state = rs_fromintr;

      //    Here comes the big while loop
      while(m_parent->m_Thread_run_flag > 0)
      {
            if(TestDestroy())
                  goto thread_prexit;                               // smooth exit

      //    Get one  packet

            int nr = gusb_cmd_get(&iresp, sizeof(iresp));

            if(iresp.gusb_pkt.pkt_id[0] == GUSB_RESPONSE_SDR)     //Satellite Data Record
            {
                  unsigned char *t = (unsigned char *)&(iresp.gusb_pkt.databuf[0]);
                  for(int i=0 ; i < 12 ; i++)
                  {
                        m_sat_data[i].svid =  *t++;
                        m_sat_data[i].snr =   ((*t)<<8) + *(t+1); t += 2;
                        m_sat_data[i].elev =  *t++;
                        m_sat_data[i].azmth = ((*t)<<8) + *(t+1); t += 2;
                        m_sat_data[i].status = *t++;
                  }

                  m_nSats = 0;
                  for(int i=0 ; i < 12 ; i++)
                  {
                        if(m_sat_data[i].svid != 255)
                              m_nSats++;
                  }
            }

            if(iresp.gusb_pkt.pkt_id[0] == GUSB_RESPONSE_PVT)     //PVT Data Record
            {


                  D800_Pvt_Data_Type *ppvt = (D800_Pvt_Data_Type *)&(iresp.gusb_pkt.databuf[0]);

                  if(m_pShareMutex)
                        pStateLocker = new wxMutexLocker(*m_pShareMutex) ;

                  /*  Lat/Lon   */
                  ThreadPositionData.kLat = ppvt->lat * 180./PI;
                  ThreadPositionData.kLon = ppvt->lon * 180. / PI;

                  /* speed over ground */
                  ThreadPositionData.kSog = sqrt(ppvt->east*ppvt->east + ppvt->north*ppvt->north) * 3.6 / 1.852;

                  /* course over ground */
                  double course = atan2(ppvt->east, ppvt->north);
                  if (course < 0)
                        course += 2*PI;
                  ThreadPositionData.kCog = course * 180 / PI;

                  ThreadPositionData.FixTime = 0;
                  ThreadPositionData.nSats = m_nSats;

                  if(m_pShareMutex)
                        delete pStateLocker ;


                  if((ppvt->fix) >= 2 && (ppvt->fix <= 5))
                  {
    //    Signal the main program thread
                        wxCommandEvent event( EVT_NMEA,  GetId() );
                        event.SetEventObject( (wxObject *)this );
                        event.SetExtraLong(EVT_NMEA_DIRECT);
                        event.SetClientData(&ThreadPositionData);
                        if(m_pMainEventHandler)
                              m_pMainEventHandler->AddPendingEvent(event);
                  }
            }

      }

thread_prexit:
      m_parent->m_Thread_run_flag = -1;
      return 0;
}

int GARMIN_IO_Thread::gusb_cmd_send(const garmin_usb_packet *opkt, size_t sz)
{
      unsigned int rv;

      unsigned char *obuf = (unsigned char *) &opkt->dbuf[0];

      rv = gusb_win_send(opkt, sz);

      /*
      * Recursion, when used in a disciplined way, can be our friend.
      *
      * The Garmin protocol requires that packets that are exactly
      * a multiple of the max tx size be followed by a zero length
      * packet.  Do that here so we can see it in debugging traces.
      */

      if (sz && !(sz % m_max_tx_size)) {
            gusb_win_send(opkt, 0);
      }

      return (rv);
}

int GARMIN_IO_Thread::gusb_cmd_get(garmin_usb_packet *ibuf, size_t sz)
{
      int rv;
      unsigned char *buf = (unsigned char *) &ibuf->dbuf[0];
      int orig_receive_state;
top:
      orig_receive_state = m_receive_state;
      switch (m_receive_state) {
            case rs_fromintr:
                  rv = gusb_win_get(ibuf, sz);
                  break;
            case rs_frombulk:
                  rv = gusb_win_get_bulk(ibuf, sz);
                  break;
      }

      /* Adjust internal state and retry the read */
      if ((rv > 0) && (ibuf->gusb_pkt.pkt_id[0] == GUSB_REQUEST_BULK)) {
            m_receive_state = rs_frombulk;
            goto top;
      }
      /*
      * If we were reading from the bulk pipe and we just got
      * a zero request, adjust our internal state.
      * It's tempting to retry the read here to hide this "stray"
      * packet from our callers, but that only works when you know
      * there's another packet coming.   That works in every case
      * except the A000 discovery sequence.
      */
      if ((m_receive_state == rs_frombulk) && (rv <= 0)) {
            m_receive_state = rs_fromintr;
      }

      return rv;
}

int GARMIN_IO_Thread::gusb_win_get(garmin_usb_packet *ibuf, size_t sz)
{
      DWORD rxed = GARMIN_USB_INTERRUPT_DATA_SIZE;
      unsigned char *buf = (unsigned char *) &ibuf->dbuf[0];
      int tsz=0;

      while (sz)
      {
            /* The driver wrongly (IMO) rejects reads smaller than
            * GARMIN_USB_INTERRUPT_DATA_SIZE
            */
            if(!DeviceIoControl(m_usb_handle, IOCTL_GARMIN_USB_INTERRUPT_IN, NULL, 0,
                buf, GARMIN_USB_INTERRUPT_DATA_SIZE, &rxed, NULL))
            {
//                GPS_Serial_Error("Ioctl");
//                fatal("ioctl\n");
            }

            buf += rxed;
            sz  -= rxed;
            tsz += rxed;
            if (rxed < GARMIN_USB_INTERRUPT_DATA_SIZE)
                  break;
      }
      return tsz;
}

int GARMIN_IO_Thread::gusb_win_get_bulk(garmin_usb_packet *ibuf, size_t sz)
{
      int n;
      DWORD rsz;
      unsigned char *buf = (unsigned char *) &ibuf->dbuf[0];

      n = ReadFile(m_usb_handle, buf, sz, &rsz, NULL);

      return rsz;
}

int GARMIN_IO_Thread::gusb_win_send(const garmin_usb_packet *opkt, size_t sz)
{
      DWORD rsz;
      unsigned char *obuf = (unsigned char *) &opkt->dbuf[0];

      /* The spec warns us about making writes an exact multiple
      * of the packet size, but isn't clear whether we can issue
      * data in a single call to WriteFile if it spans buffers.
      */
      WriteFile(m_usb_handle, obuf, sz, &rsz, NULL);

//    if (rsz != sz)
//          fatal ("Error sending %d bytes.   Successfully sent %ld\n", sz, rsz);


      return rsz;
}





class GARMIN_Restart_Thread: public wxThread
{

      public:

            GARMIN_Restart_Thread(void);
            ~GARMIN_Restart_Thread(void);
            void *Entry();
};


GARMIN_Restart_Thread::GARMIN_Restart_Thread(void)
{
      Create();
}


GARMIN_Restart_Thread::~GARMIN_Restart_Thread(void)
{
}

void *GARMIN_Restart_Thread::Entry(void)
{
      HDEVINFO devs;
      SP_DEVINFO_DATA devInfo;
      SP_PROPCHANGE_PARAMS pchange;

      devs = SetupDiGetClassDevs( (GUID *) &GARMIN_GUID, NULL, NULL,
                                   DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
      if (devs == INVALID_HANDLE_VALUE)
            return false;

      devInfo.cbSize = sizeof(devInfo);
      SetupDiEnumDeviceInfo(devs,0,&devInfo);

      pchange.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
      pchange.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
      pchange.StateChange = DICS_PROPCHANGE;
      pchange.Scope = DICS_FLAG_CONFIGSPECIFIC;
      pchange.HwProfile = 0;

      if(!SetupDiSetClassInstallParams(devs,&devInfo,&pchange.ClassInstallHeader,sizeof(pchange)))
      {
//            wxLogMessage(_T("   GarminUSBDriver Reset1 failed..."));
            return false;
      }

      if(!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,devs,&devInfo))
      {
//            wxLogMessage(_T("   GarminUSBDriver Reset2 failed..."));
            return false;
      }

//      wxLogMessage(_T("GarminUSBDriver Reset succeeded."));

      return 0;

}








//----------------------------------------------------------------------------
// Garmin Event Definition
//----------------------------------------------------------------------------

const wxEventType wxEVT_OCPN_GARMIN = wxNewEventType();

class GarminEvent: public wxEvent
{
      public:
            GarminEvent( wxEventType commandType = wxEVT_NULL, int id = 0 );

            ~GarminEvent( );

    // required for sending with wxPostEvent()
            wxEvent *Clone() const;// { return new wxEVT_OCPN_GARMIN(*this); }

            unsigned char buffer[1024];
            size_t        buffer_size;
};

//------------------------------------------------------------------------------
//    Garmin Event Implementation
//------------------------------------------------------------------------------

GarminEvent::GarminEvent( wxEventType commandType, int id )
      :wxEvent(id, commandType)
{
}

GarminEvent::~GarminEvent( )
{
}

wxEvent* GarminEvent::Clone() const
{
      GarminEvent *newevent=new GarminEvent(*this);
      newevent->buffer_size = buffer_size;
      memcpy(&newevent->buffer, &buffer, wxMin(buffer_size, sizeof(buffer)));
      return newevent;
}


BOOL IsUserAdmin(VOID)
/*++
            Routine Description: This routine returns TRUE if the caller's
            process is a member of the Administrators local group. Caller is NOT
            expected to be impersonating anyone and is expected to be able to
            open its own process and process token.
            Arguments: None.
Return Value:
            TRUE - Caller has Administrators local group.
            FALSE - Caller does not have Administrators local group. --
*/
{
      BOOL b;
      SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
      PSID AdministratorsGroup;
      b = AllocateAndInitializeSid(
                                   &NtAuthority,
                                   2,
                                   SECURITY_BUILTIN_DOMAIN_RID,
                                   DOMAIN_ALIAS_RID_ADMINS,
                                   0, 0, 0, 0, 0, 0,
                                   &AdministratorsGroup);
      if(b)
      {
            if (!CheckTokenMembership( NULL, AdministratorsGroup, &b))
            {
                  b = FALSE;
            }
            FreeSid(AdministratorsGroup);
      }

      return(b);
}




//----------------------------------------------------------------------------
// Garmin Device Monitor Window Implementation
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(DeviceMonitorWindow, wxWindow)

            EVT_TIMER(TIMER_GARMIN1, DeviceMonitorWindow::OnTimerGarmin1)
            EVT_CLOSE(DeviceMonitorWindow::OnClose)


END_EVENT_TABLE()


DeviceMonitorWindow::DeviceMonitorWindow(NMEAHandler *parent, wxWindow *MessageTarget, wxMutex *pMutex):
           wxWindow(parent->GetParentFrame(), wxID_ANY,     wxPoint(40,40), wxSize(5,5), wxSIMPLE_BORDER)
{
      m_pparent = parent;
      Hide();

      m_pMainEventHandler = MessageTarget->GetEventHandler();

      m_io_thread = NULL;

      m_pShareMutex = pMutex;

      m_usb_handle = INVALID_HANDLE_VALUE;

      m_bneed_int_reset = true;
      m_receive_state = rs_fromintr;
      m_ndelay = 0;


//      Connect(wxEVT_OCPN_GARMIN, (wxObjectEventFunction)(wxEventFunction)&DeviceMonitorWindow::OnEvtGarmin);

      char  pvt_on[14] =
      {20, 0, 0, 0, 10, 0, 0, 0, 2, 0, 0, 0, 49, 0};

      char  pvt_off[14] =
      {20, 0, 0, 0, 10, 0, 0, 0, 2, 0, 0, 0, 50, 0};

      wxLogMessage(_T("Searching for Garmin DeviceInterface and Device..."));

      if(!FindGarminDeviceInterface())
      {
	  wxLogMessage(_T("   Find:Is the Garmin USB driver installed?"));
	}
	else
	{
	  if(!ResetGarminUSBDriver())
	  {
			  wxLogMessage(_T("   Reset:Is the Garmin USB Device plugged in?"));
     	  }
        else
        {
     //    Initialize the polling timer
            TimerGarmin1.SetOwner(this, TIMER_GARMIN1);
            TimerGarmin1.Start(100);
        }
      }
}



DeviceMonitorWindow::~DeviceMonitorWindow()
{
}

void DeviceMonitorWindow::OnClose(wxCloseEvent& event)
{
      StopIOThread(true);
}


void DeviceMonitorWindow::OnEvtGarmin(GarminEvent &event)
{
}

void DeviceMonitorWindow::StopIOThread(bool b_pause)
{
      wxLogMessage(_T("Stopping Garmin I/O thread"));

      if(b_pause)
            TimerGarmin1.Stop();

      if(m_io_thread)
      {
            m_Thread_run_flag = 0;

            int tsec = 5;
            while((m_Thread_run_flag >= 0) && (tsec--))
            {
                  wxSleep(1);
		}

		wxString msg;
            if(m_Thread_run_flag < 0)
			  msg.Printf(_T("Stopped in %d sec."), 5 - tsec);
		else
			  msg.Printf(_T("Not Stopped after 5 sec."));
		wxLogMessage(msg);
      }

      m_io_thread = NULL;

      if(m_usb_handle != INVALID_HANDLE_VALUE)
            CloseHandle(m_usb_handle);
      m_usb_handle = INVALID_HANDLE_VALUE;

      m_ndelay = 30;          // Fix delay for next restart

}

void DeviceMonitorWindow::RestartIOThread(void)
{
      wxLogMessage(_T("Restarting Garmin I/O thread"));
	  TimerGarmin1.Start(1000);
}




void DeviceMonitorWindow::OnTimerGarmin1(wxTimerEvent& event)
{
      char  pvt_on[14] =
      {20, 0, 0, 0, 10, 0, 0, 0, 2, 0, 0, 0, 49, 0};

      TimerGarmin1.Stop();

      //  Try to open the Garmin USB device
      if(INVALID_HANDLE_VALUE == m_usb_handle)
      {
            if(INVALID_HANDLE_VALUE != garmin_usb_start())
            {
                  {
                        {

                        //    Send out a request for Garmin PVT data
                              m_receive_state = rs_fromintr;
                              gusb_cmd_send((const garmin_usb_packet *) pvt_on, sizeof(pvt_on));

                        //    Start the pump
                              m_io_thread = new GARMIN_IO_Thread(this, m_pMainEventHandler, m_pShareMutex, m_usb_handle, m_max_tx_size);
                              m_Thread_run_flag = 1;
                              m_io_thread->Run();
                        }
                  }
            }
      }

      TimerGarmin1.Start(1000);
}


bool DeviceMonitorWindow::ResetGarminUSBDriver()
{
      OSVERSIONINFO version_info;
      version_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

      if(GetVersionEx(&version_info))
      {
            if(version_info.dwMajorVersion > 5)
            {
                  if(!IsUserAdmin())
                  {
                        wxLogMessage(_T("    GarminUSBDriver Reset skipped, requires elevated privileges on Vista and later...."));
                        return true;
                  }
            }
      }


      HDEVINFO devs;
      SP_DEVINFO_DATA devInfo;
      SP_PROPCHANGE_PARAMS pchange;

      devs = SetupDiGetClassDevs( (GUID *) &GARMIN_GUID, NULL, NULL,
                                   DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
      if (devs == INVALID_HANDLE_VALUE)
            return false;

      devInfo.cbSize = sizeof(devInfo);
      if(!SetupDiEnumDeviceInfo(devs,0,&devInfo))
      {
          wxLogMessage(_T("   GarminUSBDriver Reset0 failed..."));
		  return false;
	  }

      pchange.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
      pchange.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
      pchange.StateChange = DICS_PROPCHANGE;
      pchange.Scope = DICS_FLAG_CONFIGSPECIFIC;
      pchange.HwProfile = 0;

      if(!SetupDiSetClassInstallParams(devs,&devInfo,&pchange.ClassInstallHeader,sizeof(pchange)))
      {
            wxLogMessage(_T("   GarminUSBDriver Reset1 failed..."));
            return false;
      }

      if(!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,devs,&devInfo))
      {
            wxLogMessage(_T("   GarminUSBDriver Reset2 failed..."));
            return false;
      }

      wxLogMessage(_T("GarminUSBDriver Reset succeeded."));

      return true;

}



bool DeviceMonitorWindow::FindGarminDeviceInterface()
{      //    Search for a useable Garmin Device Interface Class

	HDEVINFO hdevinfo;
      SP_DEVINFO_DATA devInfo;

      hdevinfo = SetupDiGetClassDevs( (GUID *) &GARMIN_GUID, NULL, NULL,
                                          DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);

      if (hdevinfo != INVALID_HANDLE_VALUE)
	  {
	      devInfo.cbSize = sizeof(devInfo);
		  if(!SetupDiEnumDeviceInfo(hdevinfo,0,&devInfo))
		  {
			  return false;
		  }
	  }

      return true;
}


bool DeviceMonitorWindow::IsGarminPlugged()
{
      DWORD size = 0;

      HDEVINFO hdevinfo;
      SP_DEVICE_INTERFACE_DATA infodata;

      //    Search for the Garmin Device Interface Class
      hdevinfo = SetupDiGetClassDevs( (GUID *) &GARMIN_GUID, NULL, NULL,
                                       DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);

      if (hdevinfo == INVALID_HANDLE_VALUE)
            return INVALID_HANDLE_VALUE;

      infodata.cbSize = sizeof(infodata);

      bool bgarmin_unit_found = (SetupDiEnumDeviceInterfaces(hdevinfo,
                                 NULL,(GUID *) &GARMIN_GUID, 0, &infodata) != 0);

      if(!bgarmin_unit_found)
            return false;

      PSP_INTERFACE_DEVICE_DETAIL_DATA pdd = NULL;
      SP_DEVINFO_DATA devinfo;

      SetupDiGetDeviceInterfaceDetail(hdevinfo, &infodata,
                                            NULL, 0, &size, NULL);

      pdd = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(size);
      pdd->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

      devinfo.cbSize = sizeof(SP_DEVINFO_DATA);
      if (!SetupDiGetDeviceInterfaceDetail(hdevinfo, &infodata,
                 pdd, size, NULL, &devinfo))
      {
            free(pdd);
            return false;
      }

      free(pdd);

      return true;
}


HANDLE DeviceMonitorWindow::garmin_usb_start()
{
      DWORD size = 0;

      HDEVINFO hdevinfo;
      SP_DEVICE_INTERFACE_DATA infodata;

      //    Search for the Garmin Device Interface Class
      hdevinfo = SetupDiGetClassDevs( (GUID *) &GARMIN_GUID, NULL, NULL,
                                       DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);

      if (hdevinfo == INVALID_HANDLE_VALUE)
            return INVALID_HANDLE_VALUE;

      infodata.cbSize = sizeof(infodata);

      bool bgarmin_unit_found = (SetupDiEnumDeviceInterfaces(hdevinfo,
                                 NULL,(GUID *) &GARMIN_GUID, 0, &infodata) != 0);

      if(!bgarmin_unit_found)
            return INVALID_HANDLE_VALUE;

      wxLogMessage(_T("Garmin USB Device Found"));

      if((m_usb_handle == INVALID_HANDLE_VALUE) || (m_usb_handle == 0))
      {
            PSP_INTERFACE_DEVICE_DETAIL_DATA pdd = NULL;
            SP_DEVINFO_DATA devinfo;

            SetupDiGetDeviceInterfaceDetail(hdevinfo, &infodata,
                                            NULL, 0, &size, NULL);

            pdd = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(size);
            pdd->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

            devinfo.cbSize = sizeof(SP_DEVINFO_DATA);
            if (!SetupDiGetDeviceInterfaceDetail(hdevinfo, &infodata,
                 pdd, size, NULL, &devinfo))
            {
                  wxLogMessage(_T("   SetupDiGetDeviceInterfaceDetail failed for Garmin Device..."));
                  free(pdd);
                  return INVALID_HANDLE_VALUE;
            }

            /* Whew.  All that just to get something we can open... */
//            wxString msg;
//            msg.Printf(_T("Windows GUID for interface is %s"),pdd->DevicePath);
//            wxLogMessage(msg);

			if(m_bneed_int_reset)
			{
				ResetGarminUSBDriver();
				m_bneed_int_reset = false;
			}

            m_usb_handle = CreateFile(pdd->DevicePath, GENERIC_READ|GENERIC_WRITE,
                                      0, NULL, OPEN_EXISTING, 0, NULL );


            if (m_usb_handle == INVALID_HANDLE_VALUE)
            {
                  wxString msg;
                  msg.Printf(_T("   (usb) CreateFile on '%s' failed"), pdd->DevicePath);
                  wxLogMessage(msg);
            }

            DEV_BROADCAST_HANDLE filterData;
            filterData.dbch_size = sizeof(DEV_BROADCAST_HANDLE);
            filterData.dbch_devicetype = DBT_DEVTYP_HANDLE;
            filterData.dbch_reserved = 0;
            filterData.dbch_handle = m_usb_handle;     // file handle used in call to RegisterDeviceNotification
            filterData.dbch_hdevnotify = 0;            // returned from RegisterDeviceNotification

            HDEVNOTIFY m_hDevNotify = RegisterDeviceNotification( GetHWND(), &filterData, DEVICE_NOTIFY_WINDOW_HANDLE);


            free(pdd);
      }

	  m_max_tx_size = 0;

      if(!DeviceIoControl(m_usb_handle, IOCTL_GARMIN_USB_BULK_OUT_PACKET_SIZE,
          NULL, 0, &m_max_tx_size, GARMIN_USB_INTERRUPT_DATA_SIZE,
          &size, NULL))
      {
            wxLogMessage(_T("   Couldn't get Garmin USB packet size."));
            CloseHandle(m_usb_handle);
            m_usb_handle = INVALID_HANDLE_VALUE;
            return INVALID_HANDLE_VALUE;
      }

      if(!gusb_syncup())
      {
            CloseHandle(m_usb_handle);
            m_usb_handle = INVALID_HANDLE_VALUE;
      }

      return m_usb_handle;
}


bool DeviceMonitorWindow::gusb_syncup(void)
{
      static int unit_number;
      static const char  oinit[12] =
      {0, 0, 0, 0, GUSB_SESSION_START, 0, 0, 0, 0, 0, 0, 0};
      garmin_usb_packet iresp;
      int i;

      /*
      * This is our first communication with the unit.
      */


      m_receive_state = rs_fromintr;

      for(i = 0; i < 25; i++) {
            le_write16(&iresp.gusb_pkt.pkt_id[0], 0);
            le_write32(&iresp.gusb_pkt.datasz[0], 0);
            le_write32(&iresp.gusb_pkt.databuf[0], 0);

            if(gusb_cmd_send((const garmin_usb_packet *) oinit, sizeof(oinit)))
            {
                  gusb_cmd_get(&iresp, sizeof(iresp));

                  if ((le_read16(iresp.gusb_pkt.pkt_id) == GUSB_SESSION_ACK) &&
                       (le_read32(iresp.gusb_pkt.datasz) == 4))
                  {
      //                unsigned serial_number = le_read32(iresp.gusb_pkt.databuf);
      //                garmin_unit_info[unit_number].serial_number = serial_number;
      //                gusb_id_unit(&garmin_unit_info[unit_number]);

                        unit_number++;

                        wxLogMessage(_T("Successful Garmin USB syncup."));
                        return true;;
                  }
            }
      }
      wxLogMessage(_T("   Unable to establish Garmin USB syncup."));
      return false;
}

int DeviceMonitorWindow::gusb_cmd_send(const garmin_usb_packet *opkt, size_t sz)
{
      unsigned int rv;

      unsigned char *obuf = (unsigned char *) &opkt->dbuf[0];

      rv = gusb_win_send(opkt, sz);

      /*
      * Recursion, when used in a disciplined way, can be our friend.
      *
      * The Garmin protocol requires that packets that are exactly
      * a multiple of the max tx size be followed by a zero length
      * packet.  Do that here so we can see it in debugging traces.
      */

      if (sz && !(sz % m_max_tx_size)) {
    	wxLogMessage(_T("win_send_call1"));
            gusb_win_send(opkt, 0);
    	wxLogMessage(_T("win_send_ret1"));
      }

      return (rv);
}


int DeviceMonitorWindow::gusb_cmd_get(garmin_usb_packet *ibuf, size_t sz)
{
      int rv;
      unsigned char *buf = (unsigned char *) &ibuf->dbuf[0];
      int orig_receive_state;
top:
      orig_receive_state = m_receive_state;
      switch (m_receive_state) {
            case rs_fromintr:
                  rv = gusb_win_get(ibuf, sz);
                  break;
            case rs_frombulk:
                  rv = gusb_win_get_bulk(ibuf, sz);
                  break;

      }

      /* Adjust internal state and retry the read */
      if ((rv > 0) && (ibuf->gusb_pkt.pkt_id[0] == GUSB_REQUEST_BULK)) {
            m_receive_state = rs_frombulk;
            goto top;
      }
      /*
      * If we were reading from the bulk pipe and we just got
      * a zero request, adjust our internal state.
      * It's tempting to retry the read here to hide this "stray"
      * packet from our callers, but that only works when you know
      * there's another packet coming.   That works in every case
      * except the A000 discovery sequence.
      */
      if ((m_receive_state == rs_frombulk) && (rv <= 0)) {
            m_receive_state = rs_fromintr;
      }

      return rv;
}





int DeviceMonitorWindow::gusb_win_get(garmin_usb_packet *ibuf, size_t sz)
{
      DWORD rxed = GARMIN_USB_INTERRUPT_DATA_SIZE;
      unsigned char *buf = (unsigned char *) &ibuf->dbuf[0];
      int tsz=0;

      while (sz)
      {
            /* The driver wrongly (IMO) rejects reads smaller than
            * GARMIN_USB_INTERRUPT_DATA_SIZE
            */
            if(!DeviceIoControl(m_usb_handle, IOCTL_GARMIN_USB_INTERRUPT_IN, NULL, 0,
                buf, GARMIN_USB_INTERRUPT_DATA_SIZE, &rxed, NULL))
            {
//                GPS_Serial_Error("Ioctl");
//                fatal("ioctl\n");
            }

            buf += rxed;
            sz  -= rxed;
            tsz += rxed;
            if (rxed < GARMIN_USB_INTERRUPT_DATA_SIZE)
                  break;
      }
      return tsz;
}

int DeviceMonitorWindow::gusb_win_get_bulk(garmin_usb_packet *ibuf, size_t sz)
{
      int n;
      DWORD rsz;
      unsigned char *buf = (unsigned char *) &ibuf->dbuf[0];

      n = ReadFile(m_usb_handle, buf, sz, &rsz, NULL);

      return rsz;
}

int DeviceMonitorWindow::gusb_win_send(const garmin_usb_packet *opkt, size_t sz)
{
      DWORD rsz;
      unsigned char *obuf = (unsigned char *) &opkt->dbuf[0];

      /* The spec warns us about making writes an exact multiple
      * of the packet size, but isn't clear whether we can issue
      * data in a single call to WriteFile if it spans buffers.
      */
      WriteFile(m_usb_handle, obuf, sz, &rsz, NULL);

//    if (rsz != sz)
//          fatal ("Error sending %d bytes.   Successfully sent %ld\n", sz, rsz);


      return rsz;
}


WXLRESULT DeviceMonitorWindow::MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam)
{
    // did we process the message?
      bool processed = false;

    // the return value
      bool rc;
      PDEV_BROADCAST_HDR pDBHdr;
      PDEV_BROADCAST_HANDLE pDBHandle;

    // for most messages we should return 0 when we do process the message
      rc = 0;

      switch ( message )
      {
            case WM_DEVICECHANGE:
                  switch (wParam)
                  {
                        case DBT_DEVICEREMOVEPENDING:
				case DBT_DEVICEREMOVECOMPLETE:
							pDBHdr = (PDEV_BROADCAST_HDR) lParam;
                              switch (pDBHdr->dbch_devicetype)
                                    case DBT_DEVTYP_HANDLE:
                                    // A Device has been removed
                                    // Stop the IO thread and close open handle  to device

                                          pDBHandle = (PDEV_BROADCAST_HANDLE) pDBHdr;
										  HANDLE target_handle = pDBHandle->dbch_handle;

                                          wxLogMessage(_T("Garmin USB Device Removed"));
                                          StopIOThread(false);
                                          m_bneed_int_reset = true;
                                          processed = true;
                                          break;
                  }

                 break;

      }

      if ( !processed )
      {
            rc = (MSWDefWindowProc(message, wParam, lParam) != 0);
      }

      return rc;
}

#endif            // __WXMSW__



//-------------------------------------------------------------------------------------------------------------
//
//    Autopilot Class Implementation
//
//-------------------------------------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(AutoPilotWindow, wxWindow)
        EVT_CLOSE(AutoPilotWindow::OnCloseWindow)
END_EVENT_TABLE()

// Implement a constructor
AutoPilotWindow::AutoPilotWindow(wxFrame *frame, const wxString& AP_Port):
        wxWindow(frame, wxID_ANY,     wxPoint(20,20), wxSize(5,5), wxSIMPLE_BORDER)

{

      m_pdata_ap_port_string = new wxString(AP_Port);
      m_bOK = false;

//    Create and init the Serial Port for Autopilot control

      wxString msg(_T("NMEA AutoPilot Port is...."));
      msg.Append(*m_pdata_ap_port_string);
      wxLogMessage(msg);

      if((!m_pdata_ap_port_string->IsEmpty()) && (!m_pdata_ap_port_string->IsSameAs(_T("None"), false)))
      {
            m_port = m_pdata_ap_port_string->AfterFirst(':');    // Strip "Serial"

            m_bOK = OpenPort(m_port);
    }
    Hide();
}

AutoPilotWindow::~AutoPilotWindow()
{
    delete m_pdata_ap_port_string;
}

void AutoPilotWindow::GetAP_Port(wxString& source)
{
    source = *m_pdata_ap_port_string;
}

void AutoPilotWindow::OnCloseWindow(wxCloseEvent& event)
{
    if(m_bOK)
    {
          g_pCommMan->CloseComPort(m_ap_fd);
          m_bOK = false;
    }
}

bool AutoPilotWindow::OpenPort(wxString &port)
{
      wxString sport = port;

      if ((m_ap_fd = g_pCommMan->OpenComPort(sport, 4800)) < 0)
      {
            wxString msg(_T("Autopilot output device open failed: "));
            msg.Append(port);
            wxLogMessage(msg);
            return false;
      }

      return true;
}




int AutoPilotWindow::AutopilotOut(const wxString& Sentence)
{
      ssize_t status;
      status = g_pCommMan->WriteComPort(m_port, Sentence);
      return status;
}

//-------------------------------------------------------------------------------------------------------------
//
//    Communications Port Manager
//
//-------------------------------------------------------------------------------------------------------------
#include <wx/listimpl.cpp>
WX_DEFINE_LIST(ListOfOpenCommPorts);

ComPortManager:: ComPortManager()
{
      m_blog = true;
}

ComPortManager::~ComPortManager()
{
}

//    Common Methods

int ComPortManager::OpenComPort(wxString &com_name, int baud_rate)
{
      // Already open?
      int port_descriptor;
      OpenCommPortElement *pe = GetComPort(com_name);
      if(NULL == pe)
      {
            port_descriptor = OpenComPortPhysical(com_name, baud_rate);
            if( port_descriptor < 0)
                  return port_descriptor;                                // error

            OpenCommPortElement *pocpe = new OpenCommPortElement;
            pocpe->com_name = com_name;
            pocpe->port_descriptor = port_descriptor;
            pocpe->n_open = 1;
            pocpe->baud_rate = baud_rate;

            m_port_list.Append(pocpe);

            if(m_blog)
            {
                  wxString s;
                  s.Printf(_T("OpenPD: %d, new_count = %d for "), port_descriptor, pocpe->n_open );
                  s.Append(com_name);
                  wxLogMessage(s);
            }

      }
      else                    // port is already open, so increment its counter
      {
            pe->n_open++;
            port_descriptor = pe->port_descriptor;

            if(m_blog)
            {
                  wxString s;
                  s.Printf(_T("Re-OpenPD: %d, new_count = %d for "), port_descriptor, pe->n_open );
                  s.Append(com_name);
                  wxLogMessage(s);
            }
      }



      return port_descriptor;
}

int ComPortManager::CloseComPort(int fd)
{

      for ( ListOfOpenCommPorts::Node *node = m_port_list.GetFirst(); node; node = node->GetNext() )
      {
            OpenCommPortElement *current = node->GetData();

            if(current->port_descriptor == fd)
            {
                  current->n_open--;

                  if(m_blog)
                  {
                        wxString s;
                        s.Printf(_T("ClosePD: %d, count_after_close = %d for "), fd, current->n_open );
                        s.Append(current->com_name);
                        wxLogMessage(s);
                  }
                  if(0 == current->n_open)
                  {
                        CloseComPortPhysical(fd);
                        if(m_blog)
                              wxLogMessage(_T("  and so CloseComPortPhysical"));

                        m_port_list.DeleteObject(current);
                        delete current;
                        break;
                  }
            }
      }

      return 0;
}

//------------------------------------------------------------
//    GetComPort()
//    Return the descriptor for an already open com port.
//    return -1 if the port is not already open
//------------------------------------------------------------

OpenCommPortElement *ComPortManager::GetComPort(wxString &com_name)
{
      for ( ListOfOpenCommPorts::Node *node = m_port_list.GetFirst(); node; node = node->GetNext() )
      {
            OpenCommPortElement *current = node->GetData();

            if(current->com_name.IsSameAs(com_name))
                  return current;
      }

      return NULL;
}

int ComPortManager::WriteComPort(wxString& com_name, const wxString& string)
{
      int port_descriptor;
      int status;

      OpenCommPortElement *pe = GetComPort(com_name);

      if(NULL == pe)
            port_descriptor = OpenComPort(com_name, 4800);              // defaults to 4800
      else
            port_descriptor = pe->port_descriptor;

            status = WriteComPortPhysical(port_descriptor, string);

      return status;
}


int ComPortManager::WriteComPort(wxString& com_name, unsigned char *msg, int count)
{
      int port_descriptor;
      int status;

      OpenCommPortElement *pe = GetComPort(com_name);

      if(NULL == pe)
            port_descriptor = OpenComPort(com_name, 4800);              // defaults to 4800
      else
            port_descriptor = pe->port_descriptor;

      status = WriteComPortPhysical(port_descriptor, msg, count);

      return status;
}

int ComPortManager::ReadComPort(wxString& com_name, int count, unsigned char *p)
{
      int port_descriptor;

      OpenCommPortElement *pe = GetComPort(com_name);

      if(NULL == pe)
            return 0;
      else
            port_descriptor = pe->port_descriptor;

      return ReadComPortPhysical(port_descriptor, count, p);


}

bool ComPortManager::SerialCharsAvail(wxString& com_name)
{
      int port_descriptor;

      OpenCommPortElement *pe = GetComPort(com_name);

      if(NULL == pe)
            return false;
      else
            port_descriptor = pe->port_descriptor;

      return CheckComPortPhysical(port_descriptor);
}



#ifdef __POSIX__
typedef struct {
      int fd;         /* File descriptor */
      struct termios gps_ttysave;
} posix_serial_data;

int ComPortManager::OpenComPortPhysical(wxString &com_name, int baud_rate)
{

    // Declare the termios data structures
      termios ttyset_old;
      termios ttyset;

    // Open the serial port.
      int com_fd;
      if ((com_fd = open(com_name.mb_str(), O_RDWR|O_NONBLOCK|O_NOCTTY)) < 0)
//      if ((com_fd = open(com_name.mb_str(), O_RDWR|O_NOCTTY)) < 0)
            return com_fd;


      speed_t baud_parm;
      switch(baud_rate)
      {
            case 4800:
                  baud_parm = B4800;
                  break;
            case 9600:
                  baud_parm = B9600;
                  break;
            case 38400:
                  baud_parm = B38400;
                  break;
            default:
                  baud_parm = B4800;
                  break;
      }



     if (isatty(com_fd) != 0)
      {
            /* Save original terminal parameters */
            if (tcgetattr(com_fd,&ttyset_old) != 0)
                  return -128;

            memcpy(&ttyset, &ttyset_old, sizeof(termios));

      //  Build the new parms off the old

      //  Baud Rate
            cfsetispeed(&ttyset, baud_parm);
            cfsetospeed(&ttyset, baud_parm);

            tcsetattr(com_fd, TCSANOW, &ttyset);

      // Set blocking/timeout behaviour
            memset(ttyset.c_cc,0,sizeof(ttyset.c_cc));
            ttyset.c_cc[VTIME] = 5;                        // 0.5 sec timeout
            fcntl(com_fd, F_SETFL, fcntl(com_fd, F_GETFL) & !O_NONBLOCK);

      // No Flow Control

            ttyset.c_cflag &= ~(PARENB | PARODD | CRTSCTS);
            ttyset.c_cflag |= CREAD | CLOCAL;
            ttyset.c_iflag = ttyset.c_oflag = ttyset.c_lflag = (tcflag_t) 0;

            int stopbits = 1;
            char parity = 'N';
            ttyset.c_iflag &=~ (PARMRK | INPCK);
            ttyset.c_cflag &=~ (CSIZE | CSTOPB | PARENB | PARODD);
            ttyset.c_cflag |= (stopbits==2 ? CS7|CSTOPB : CS8);
            switch (parity)
            {
                  case 'E':
                        ttyset.c_iflag |= INPCK;
                        ttyset.c_cflag |= PARENB;
                        break;
                  case 'O':
                        ttyset.c_iflag |= INPCK;
                        ttyset.c_cflag |= PARENB | PARODD;
                        break;
            }
            ttyset.c_cflag &=~ CSIZE;
            ttyset.c_cflag |= (CSIZE & (stopbits==2 ? CS7 : CS8));
            if (tcsetattr(com_fd, TCSANOW, &ttyset) != 0)
                  return -129;

            tcflush(com_fd, TCIOFLUSH);
      }

      return com_fd;
}


int ComPortManager::CloseComPortPhysical(int fd)
{

      close(fd);

      return 0;
}


int ComPortManager::WriteComPortPhysical(int port_descriptor, const wxString& string)
{
      ssize_t status;
      status = write(port_descriptor, string.mb_str(), string.Len());

      return status;
}

int ComPortManager::WriteComPortPhysical(int port_descriptor, unsigned char *msg, int count)
{
      ssize_t status;
      status = write(port_descriptor, msg, count);

      return status;
}

int ComPortManager::ReadComPortPhysical(int port_descriptor, int count, unsigned char *p)
{
//    Blocking, timeout protected read of one character at a time
//    Timeout value is set by c_cc[VTIME]

      return read(port_descriptor, p, count);            // read of (count) characters
}


bool ComPortManager::CheckComPortPhysical(int port_descriptor)
{
      fd_set rec;
      struct timeval t;
//      posix_serial_data *psd = (posix_serial_data *)port_descriptor;
//      int fd = psd->fd;

      int fd = port_descriptor;
      FD_ZERO(&rec);
      FD_SET(fd,&rec);

      t.tv_sec  = 0;
      t.tv_usec = 1000;
      (void) select(fd+1,&rec,NULL,NULL,&t);
      if(FD_ISSET(fd,&rec))
            return true;

      return false;
}


#endif            // __POSIX__

#ifdef __WXMSW__
int ComPortManager::OpenComPortPhysical(wxString &com_name, int baud_rate)
{

//    Set up the serial port
      wxString xcom_name = com_name;
      xcom_name.Prepend(_T("\\\\.\\"));                  // Required for access to Serial Ports greater than COM9

#ifdef SERIAL_OVERLAPPED
      DWORD open_flags = FILE_FLAG_OVERLAPPED;
#else
      DWORD open_flags = 0;
#endif

      HANDLE hSerialComm = CreateFile(xcom_name.fn_str(),      // Port Name
                                 GENERIC_READ | GENERIC_WRITE,     // Desired Access
                                 0,                               // Shared Mode
                                 NULL,                            // Security
                                 OPEN_EXISTING,             // Creation Disposition
                                 open_flags,
                                 NULL);

      if(hSerialComm == INVALID_HANDLE_VALUE)
      {
            return (0 - abs((int)::GetLastError()));
      }

      if(!SetupComm(hSerialComm, 1024, 1024))
      {
            return (0 - abs((int)::GetLastError()));
      }


      DCB dcbConfig;

      if(GetCommState(hSerialComm, &dcbConfig))           // Configuring Serial Port Settings
      {
            dcbConfig.BaudRate = baud_rate;
            dcbConfig.ByteSize = 8;
            dcbConfig.Parity = NOPARITY;
            dcbConfig.StopBits = ONESTOPBIT;
            dcbConfig.fBinary = TRUE;
            dcbConfig.fParity = TRUE;
      }

      else
      {
            return (0 - abs((int)::GetLastError()));
      }

      if(!SetCommState(hSerialComm, &dcbConfig))
      {
            return (0 - abs((int)::GetLastError()));
      }

      COMMTIMEOUTS commTimeout;
      int TimeOutInSec = 2;
      commTimeout.ReadIntervalTimeout = 1000*TimeOutInSec;
      commTimeout.ReadTotalTimeoutConstant = 1000*TimeOutInSec;
      commTimeout.ReadTotalTimeoutMultiplier = 0;
      commTimeout.WriteTotalTimeoutConstant = 1000*TimeOutInSec;
      commTimeout.WriteTotalTimeoutMultiplier = 0;


      if(!SetCommTimeouts(hSerialComm, &commTimeout))
      {
            return (0 - abs((int)::GetLastError()));
      }


      return (int)hSerialComm;
}

int ComPortManager::CloseComPortPhysical(int fd)
{
      if((HANDLE)fd != INVALID_HANDLE_VALUE)
            CloseHandle((HANDLE)fd);
      return 0;
}

int ComPortManager::WriteComPortPhysical(int port_descriptor, const wxString& string)
{
      unsigned int dwSize = string.Len();
      char *pszBuf = (char *)malloc((dwSize + 1) * sizeof(char));
      strncpy(pszBuf, string.mb_str(), dwSize+1);

#ifdef SERIAL_OVERLAPPED

      OVERLAPPED osWrite = {0};
      DWORD dwWritten;
      int fRes;

   // Create this writes OVERLAPPED structure hEvent.
      osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
      if (osWrite.hEvent == NULL)
      // Error creating overlapped event handle.
            return 0;

   // Issue write.
      if (!WriteFile((HANDLE)port_descriptor, pszBuf, dwSize, &dwWritten, &osWrite))
      {
            if (GetLastError() != ERROR_IO_PENDING)
            {
         // WriteFile failed, but it isn't delayed. Report error and abort.
                  fRes = 0;
            }
            else
            {
         // Write is pending.
                  if (!GetOverlappedResult((HANDLE)port_descriptor, &osWrite, &dwWritten, TRUE))
                        fRes = 0;
                  else
            // Write operation completed successfully.
                        fRes = dwWritten;
            }
      }
      else
      // WriteFile completed immediately.
            fRes = dwWritten;

      CloseHandle(osWrite.hEvent);

      free (pszBuf);

#else
      DWORD dwWritten;
      int fRes;

        // Issue write.
      if (!WriteFile((HANDLE)port_descriptor, pszBuf, dwSize, &dwWritten, NULL))
            fRes = 0;         // WriteFile failed, . Report error and abort.
      else
            fRes = dwWritten;      // WriteFile completed immediately.

      free (pszBuf);

#endif

      return fRes;
}

int ComPortManager::WriteComPortPhysical(int port_descriptor, unsigned char *msg, int count)
{
      return 0;
}

int ComPortManager::ReadComPortPhysical(int port_descriptor, int count, unsigned char *p)
{
      return 0;
}


bool ComPortManager::CheckComPortPhysical(int port_descriptor)
{
      return false;
}




#endif            // __WXMSW__


