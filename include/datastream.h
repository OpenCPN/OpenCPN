/******************************************************************************
 *
 * Project:  OpenCP
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

#ifdef __POSIX__
#include <sys/termios.h>
#endif

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

#ifdef __WXMSW__
#include <windows.h>
#include <dbt.h>
#include <windows.h>
#include <winioctl.h>
#include <initguid.h>
#endif

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------
#ifndef PI
#define PI        3.1415926535897931160E0      /* pi */
#endif

//      Port I/O type
typedef enum {
    DS_TYPE_INPUT,
    DS_TYPE_OUTPUT,
    DS_TYPE_INPUT_OUTPUT
}dsPortType;


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

//Type definitions for connection parameters
typedef enum
{
    Serial = 0,
    Network = 1
} ConnectionType;

typedef enum
{
    TCP = 0,
    UDP = 1,
    GPSD = 2
} NetworkProtocol;

typedef enum
{
    WHITELIST = 0,
    BLACKLIST = 1
} ListType;

typedef enum
{
    FILTER_INPUT = 0,
    FILTER_OUTPUT = 1
} FilterDirection;

typedef enum
{
    PROTO_NMEA0183 = 0,
    PROTO_SEATALK = 1,
    PROTO_NMEA2000 = 2
} DataProtocol;

#define DS_SOCKET_ID             5001

#define     MAX_RX_MESSSAGE_SIZE  4096
#define     RX_BUFFER_SIZE        4096


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
    double kVMG;
} GenericPosDatEx;




// Class declarations
class OCP_DataStreamInput_Thread;
class DataStream;
class GarminProtocolHandler;

//----------------------------------------------------------------------------
// OCPN_DataStreamEvent
//----------------------------------------------------------------------------

extern  const wxEventType wxEVT_OCPN_DATASTREAM;

class OCPN_DataStreamEvent: public wxEvent
{
public:
    OCPN_DataStreamEvent( wxEventType commandType = wxEVT_NULL, int id = 0 );
    ~OCPN_DataStreamEvent( );

    // accessors
    std::string GetNMEAString() { return m_NMEAstring; }
    DataStream *GetDataStream() { return m_pDataStream; }
    void SetNMEAString(std::string string) { m_NMEAstring = string; }
    void SetDataStream(DataStream *pds) { m_pDataStream = pds; }
    
    // required for sending with wxPostEvent()
    wxEvent *Clone() const;

private:
    std::string m_NMEAstring;
    DataStream  *m_pDataStream;

            //            DECLARE_DYNAMIC_CLASS(OCPN_DataStreamEvent)
};





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
    bool ChecksumOK(const wxString& sentence);
    bool GetGarminUploadMode(){ return m_bGarmin_GRM_upload; }
    bool GetGarminMode(){ return m_bGarmin_GRMN_mode; }
    void SetGarminUploadMode(bool b){ m_bGarmin_GRM_upload = b; }
   
    
    wxString GetBaudRate(){ return m_BaudRate; }
    dsPortType GetPortType(){ return m_io_select; }
    wxArrayString GetInputSentenceList(){ return m_input_filter; }
    wxArrayString GetOutputSentenceList(){ return m_output_filter; }
    ListType GetInputSentenceListType(){ return m_input_filter_type; }
    ListType GetOutputSentenceListType(){ return m_output_filter_type; }
    bool GetChecksumCheck(){ return m_bchecksumCheck; }
    
    

    int                 m_Thread_run_flag;
private:
    void Init(void);
    void Open(void);

    void OnSocketEvent(wxSocketEvent& event);
    void OnTimerNMEA(wxTimerEvent& event);


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
    wxString            m_sock_buffer;
    wxString            m_net_addr;
    wxString            m_net_port;
    NetworkProtocol     m_net_protocol;
    ConnectionType      m_connection_type;

    bool                m_bchecksumCheck;
    wxArrayString       m_input_filter;
    ListType            m_input_filter_type;
    wxArrayString       m_output_filter;
    ListType            m_output_filter_type;
    
    bool                m_bGarmin_GRM_upload;
    bool                m_bGarmin_GRMN_mode;
    GarminProtocolHandler *m_GarminHandler;

DECLARE_EVENT_TABLE()
};



//-------------------------------------------------------------------------------------------------------------
//
//    DataStream Input Thread
//
//    This thread manages reading the NMEA data stream from the declared source
//
//-------------------------------------------------------------------------------------------------------------

#ifdef __WXMSW__
#include <windows.h>
#include <winioctl.h>
#include <initguid.h>
#include "setupapi.h"                   // presently stored in opencpn/src
#endif

//    Constants
typedef enum DS_ENUM_BUFFER_STATE
{
      DS_RX_BUFFER_EMPTY,
      DS_RX_BUFFER_FULL
}_DS_ENUM_BUFFER_STATE;

//#define MAX_RX_MESSSAGE_SIZE  4096
#define DS_RX_BUFFER_SIZE 4096



//          Inter-thread communication event declaration

extern const wxEventType EVT_THREADMSG;


class OCP_DataStreamInput_Thread: public wxThread
{
public:

    OCP_DataStreamInput_Thread(DataStream *Launcher,
                                  wxEvtHandler *MessageTarget,
                                  const wxString& PortName,
                                  const wxString& strBaudRate );

    ~OCP_DataStreamInput_Thread(void);
    void *Entry();
    int SendMsg(const wxString& msg);

    void OnExit(void);

private:
    void ThreadMessage(const wxString &msg);
    void Parse_And_Send_Posn(wxString &str_temp_buf);
    int OpenComPortPhysical(wxString &com_name, int baud_rate);
    int CloseComPortPhysical(int fd);
    int WriteComPortPhysical(int port_descriptor, const wxString& string);
    int WriteComPortPhysical(int port_descriptor, unsigned char *msg, int count);
    int ReadComPortPhysical(int port_descriptor, int count, unsigned char *p);
    bool CheckComPortPhysical(int port_descriptor);

    wxEvtHandler            *m_pMessageTarget;
    DataStream              *m_launcher;
    wxString                m_PortName;

    char                    *put_ptr;
    char                    *tak_ptr;

    char                    *rx_buffer;
    char                    *temp_buf;

    unsigned long           error;

    int                     m_gps_fd;
    int                     m_baud;
    int                     m_n_timeout;

#ifdef __WXMSW__
    HANDLE                  m_hSerialComm;
#endif

};

//----------------------------------------------------------------------------------------------------------
//    Connection parameters class
//----------------------------------------------------------------------------------------------------------
class ConnectionParams
{
public:
    ConnectionParams();
    ConnectionParams(wxString &configStr);

    ConnectionType  Type;
    NetworkProtocol NetProtocol;
    wxString        NetworkAddress;
    int             NetworkPort;

    DataProtocol    Protocol;
    wxString        Port;
    int             Baudrate;
    bool            ChecksumCheck;
    bool            Garmin;
    bool            GarminUpload;
    bool            FurunoGP3X;
    bool            Output;
    ListType        InputSentenceListType;
    wxArrayString   InputSentenceList;
    ListType        OutputSentenceListType;
    wxArrayString   OutputSentenceList;
    int             Priority;

    wxString        Serialize();
    void            Deserialize(wxString &configStr);

    wxString GetSourceTypeStr();
    wxString GetAddressStr();
    wxString GetParametersStr();
    wxString GetOutputValueStr();
    wxString GetFiltersStr();
    wxString GetDSPort();

    bool            Valid;
private:
    wxString FilterTypeToStr(ListType type);
};

WX_DEFINE_ARRAY(ConnectionParams *, wxArrayOfConnPrm);

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
    bool IsGarminPlugged();
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


