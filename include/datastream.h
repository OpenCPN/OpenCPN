/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  NMEA Data Object
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
 *
 *
 *
 */


#ifndef __DATASTREAM_H__
#define __DATASTREAM_H__

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled header


#include <wx/datetime.h>


#ifdef __WXGTK__
// newer versions of glib define its own GSocket but we unfortunately use this
// name in our own (semi-)public header and so can't change it -- rename glib
// one instead
//#include <gtk/gtk.h>
#define GSocket GlibGSocket
#include "wx/socket.h"
#undef GSocket
#else
#include "wx/socket.h"
#endif

#ifndef __WXMSW__
#include <sys/socket.h>                 // needed for (some) Mac builds
#include <netinet/in.h>
#endif

#ifdef __WXMSW__
#include <windows.h>
#include <dbt.h>
#include <windows.h>
#include <winioctl.h>
#include <initguid.h>
#endif
#include <string>
#include "ConnectionParams.h"
#include "dsPortType.h"

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------
#ifndef PI
#define PI        3.1415926535897931160E0      /* pi */
#endif

#define TIMER_SOCKET   7006

//      Error codes, returned by GetLastError()
enum {
    DS_ERROR_PORTNOTFOUND
};


//      End-of-sentence types
enum {
    DS_EOS_CRLF,
    DS_EOS_LF,
    DS_EOS_CR
};

//      Serial port handshake type
enum {
    DS_HANDSHAKE_NONE,
    DS_HANDSHAKE_XON_XOFF
};

#define DS_SOCKET_ID             5001
#define DS_SERVERSOCKET_ID       5002
#define DS_ACTIVESERVERSOCKET_ID 5003

#define     MAX_RX_MESSSAGE_SIZE  4096
#define     RX_BUFFER_SIZE        4096


// Class declarations





// Class declarations
class OCP_DataStreamInput_Thread;
class DataStream;
class GarminProtocolHandler;

extern  const wxEventType wxEVT_OCPN_DATASTREAM;
extern  const wxEventType wxEVT_OCPN_THREADMSG;

//----------------------------------------------------------------------------
// DataStream
//
//      Physical port is specified by a string in the class ctor.
//      Examples strings:
//              Serial:/dev/ttyS0               (Standard serial port)
//              Serial:COM4
//              TCP:192.168.1.1:5200            (TCP source, address and port specified)
//              GPSD:192.168.2.3:5400           (GPSD Wire protocol, address and port specified)
//
//----------------------------------------------------------------------------

class DataStream: public wxEvtHandler
{
public:
    DataStream(wxEvtHandler *input_consumer,
               const wxString& Port,
               const wxString& BaudRate,
               dsPortType io_select,
               int priority = 0,
               bool bGarmin = false,
               int EOS_type = DS_EOS_CRLF,
               int handshake_type = DS_HANDSHAKE_NONE,
               void *user_data = NULL );

    ~DataStream();

    void Close(void);

    bool IsOk(){ return m_bok; }
    wxString GetPort(){ return m_portstring; }
    dsPortType GetIoSelect(){ return m_io_select; }
    int GetPriority(){ return m_priority; }
    void *GetUserData(){ return m_user_data; }
    
    bool SendSentence( const wxString &sentence );

    int GetLastError(){ return m_last_error; }

 //    Secondary thread life toggle
 //    Used to inform launching object (this) to determine if the thread can
 //    be safely called or polled, e.g. wxThread->Destroy();
    void SetSecThreadActive(void){m_bsec_thread_active = true;}
    void SetSecThreadInActive(void){m_bsec_thread_active = false;}
    bool IsSecThreadActive(){ return m_bsec_thread_active; }

    void SetChecksumCheck(bool check) { m_bchecksumCheck = check; }

    void SetInputFilter(wxArrayString filter) { m_input_filter = filter; }
    void SetInputFilterType(ListType filter_type) { m_input_filter_type = filter_type; }
    void SetOutputFilter(wxArrayString filter) { m_output_filter = filter; }
    void SetOutputFilterType(ListType filter_type) { m_output_filter_type = filter_type; }
    bool SentencePassesFilter(const wxString& sentence, FilterDirection direction);
    bool ChecksumOK(const std::string& sentence);
    bool GetGarminMode(){ return m_bGarmin_GRMN_mode; }


    wxString GetBaudRate(){ return m_BaudRate; }
    dsPortType GetPortType(){ return m_io_select; }
    wxArrayString GetInputSentenceList(){ return m_input_filter; }
    wxArrayString GetOutputSentenceList(){ return m_output_filter; }
    ListType GetInputSentenceListType(){ return m_input_filter_type; }
    ListType GetOutputSentenceListType(){ return m_output_filter_type; }
    bool GetChecksumCheck(){ return m_bchecksumCheck; }
    ConnectionType GetConnectionType(){ return m_connection_type; }

    int                 m_Thread_run_flag;
private:
    void Init(void);
    void Open(void);

    void OnSocketEvent(wxSocketEvent& event);
    void OnTimerSocket(wxTimerEvent& event);
    void OnSocketReadWatchdogTimer(wxTimerEvent& event);
    
    bool                m_bok;
    wxEvtHandler        *m_consumer;
    wxString            m_portstring;
    wxString            m_BaudRate;
    dsPortType          m_io_select;
    int                 m_priority;
    int                 m_handshake;
    void                *m_user_data;


    OCP_DataStreamInput_Thread *m_pSecondary_Thread;
    bool                m_bsec_thread_active;
    int                 m_last_error;

    wxIPV4address       m_addr;
    wxSocketBase        *m_sock;
    wxSocketBase        *m_tsock;
    bool                m_is_multicast;
    struct ip_mreq      m_mrq;      // mreq rather than mreqn for windows

    //  TCP Server support
    void OnServerSocketEvent(wxSocketEvent& event);             // The listener
    void OnActiveServerEvent(wxSocketEvent& event);             // The open connection
    // Setting output parameters
    bool SetOutputSocketOptions(wxSocketBase* tsock);

    wxSocketServer      *m_socket_server;                       //  The listening server
    wxSocketBase        *m_socket_server_active;                //  The active connection
    
    std::string         m_sock_buffer;
    wxString            m_net_addr;
    wxString            m_net_port;
    NetworkProtocol     m_net_protocol;
    ConnectionType      m_connection_type;

    bool                m_bchecksumCheck;
    wxArrayString       m_input_filter;
    ListType            m_input_filter_type;
    wxArrayString       m_output_filter;
    ListType            m_output_filter_type;

    bool                m_bGarmin_GRMN_mode;
    GarminProtocolHandler *m_GarminHandler;
    wxDateTime          m_connect_time;
    bool                m_brx_connect_event;
    wxTimer             m_socket_timer;
    int                 m_txenter;
    wxTimer             m_socketread_watchdog_timer;
    int                 m_dog_value;

DECLARE_EVENT_TABLE()
};


//extern const wxEventType EVT_THREADMSG;

//----------------------------------------------------------------------------
// Garmin Device Management
// Handle USB and Serial Port Garmin PVT protocol data interface.
//----------------------------------------------------------------------------


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

#ifdef __WXMSW__
// {2C9C45C2-8E7D-4C08-A12D-816BBAE722C0}
DEFINE_GUID(GARMIN_GUID, 0x2c9c45c2L, 0x8e7d, 0x4c08, 0xa1, 0x2d, 0x81, 0x6b, 0xba, 0xe7, 0x22, 0xc0);
#endif

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
        unsigned char databuf[5]; /* actually a variable length array... */
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
} D800_Pvt_Data_Type_Aligned;



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
 * The status bit field represents a set of booleans described below:
 *                     Bit      Meaning when bit is one (1)
 *                     0       The unit has ephemeris data for the specified satellite.
 *                     1       The unit has a differential correction for the specified satellite.
 *                     2       The unit is using this satellite in the solution.
 */


enum {
    rs_fromintr,
    rs_frombulk
};

#define TIMER_GARMIN1   7005

class GARMIN_Serial_Thread;
class GARMIN_USB_Thread;

class GarminProtocolHandler: public wxEvtHandler
{
public:
    GarminProtocolHandler(DataStream *parent, wxEvtHandler *MessageTarget,  bool bsel_usb);
    ~GarminProtocolHandler();

    void Close(void);


    void StopIOThread(bool b_pause);
    void RestartIOThread(void);

    void StopSerialThread(void);

    void OnTimerGarmin1(wxTimerEvent& event);

    bool FindGarminDeviceInterface();



    wxEvtHandler            *m_pMainEventHandler;
    DataStream              *m_pparent;

    int                     m_max_tx_size;
    int                     m_receive_state;
    cpo_sat_data            m_sat_data[12];
    unit_info_type          grmin_unit_info[2];
    int                     m_nSats;
    wxTimer                 TimerGarmin1;

    int                     m_Thread_run_flag;
    GARMIN_Serial_Thread    *m_garmin_serial_thread;
    GARMIN_USB_Thread       *m_garmin_usb_thread;
    bool                    m_bneed_int_reset;
    int                     m_ndelay;
    bool                    m_bOK;
    bool                    m_busb;
    wxString                m_port;

#ifdef __WXMSW__
    HANDLE garmin_usb_start();
    bool ResetGarminUSBDriver();
    static bool IsGarminPlugged();
    bool gusb_syncup(void);

    int gusb_win_get(garmin_usb_packet *ibuf, size_t sz);
    int gusb_win_get_bulk(garmin_usb_packet *ibuf, size_t sz);
    int gusb_win_send(const garmin_usb_packet *opkt, size_t sz);

    int gusb_cmd_send(const garmin_usb_packet *opkt, size_t sz);
    int gusb_cmd_get(garmin_usb_packet *ibuf, size_t sz);

    HANDLE                  m_usb_handle;

    WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam);
#endif

    DECLARE_EVENT_TABLE()
};


//-------------------------------------------------------------------------------------------------------------
//
//    Garmin Serial Port Worker Thread
//
//    This thread manages reading the positioning data stream from the declared Garmin GRMN Mode serial device
//
//-------------------------------------------------------------------------------------------------------------
class GARMIN_Serial_Thread: public wxThread
{

public:

    GARMIN_Serial_Thread(GarminProtocolHandler *parent,
                         DataStream *GParentStream,
                         wxEvtHandler *MessageTarget,
                         wxString port);
    ~GARMIN_Serial_Thread(void);
    void *Entry();
    void string(wxCharBuffer mb_str);


private:
    wxEvtHandler            *m_pMessageTarget;
    GarminProtocolHandler   *m_parent;
    DataStream              *m_parent_stream;


    wxString                m_port;
    bool                    m_bconnected;
    bool                    m_bdetected;

};



//-------------------------------------------------------------------------------------------------------------
//
//    Garmin USB Worker Thread
//
//    This thread manages reading the positioning data stream from the declared Garmin USB device
//
//-------------------------------------------------------------------------------------------------------------
class GARMIN_USB_Thread: public wxThread
{

public:

    GARMIN_USB_Thread(GarminProtocolHandler *parent,
                      DataStream *GParentStream,
                      wxEvtHandler *MessageTarget,
                      unsigned int device_handle,
                      size_t max_tx_size);
    ~GARMIN_USB_Thread(void);
    void *Entry();


private:
    DataStream *m_parent_stream;

    int gusb_win_get(garmin_usb_packet *ibuf, size_t sz);
    int gusb_win_get_bulk(garmin_usb_packet *ibuf, size_t sz);
    int gusb_cmd_get(garmin_usb_packet *ibuf, size_t sz);

    wxEvtHandler            *m_pMessageTarget;
    GarminProtocolHandler   *m_parent;


    int                     m_receive_state;
    cpo_sat_data            m_sat_data[12];
    unit_info_type          grmin_unit_info[2];
    int                     m_nSats;
    int                     m_max_tx_size;
#ifdef __WXMSW__
    HANDLE                  m_usb_handle;
#endif

};



#endif


