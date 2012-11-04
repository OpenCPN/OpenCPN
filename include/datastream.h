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
#ifdef __WXGTK__
#define GSocket GlibGSocket
//#include <gtk/gtk.h>
//#include <glib.h>
#include "wx/socket.h"
#undef GSocket
#else
#include "wx/socket.h"
#endif

#ifdef __POSIX__
#include <sys/termios.h>
#endif

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------

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

typedef enum ENUM_BUFFER_STATE
{
    RX_BUFFER_EMPTY,
    RX_BUFFER_FULL
}_ENUM_BUFFER_STATE;

enum
{
    EVT_NMEA_DIRECT,
    EVT_NMEA_PARSE_RX
};

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
} GenericPosDatEx;




// Class declarations
class OCP_DataStreamInput_Thread;
class DataStream;


//----------------------------------------------------------------------------
// OCPN_DataStreamEvent
//----------------------------------------------------------------------------

extern  const wxEventType wxEVT_OCPN_DATASTREAM;

class OCPN_DataStreamEvent: public wxEvent
{
public:
    OCPN_DataStreamEvent( wxEventType commandType = wxEVT_NULL, int id = 0 );

    OCPN_DataStreamEvent(const OCPN_DataStreamEvent & event)
            : wxEvent(event),
              m_NMEAstring(event.m_NMEAstring)
              { }

    ~OCPN_DataStreamEvent( );

    // accessors
    wxString GetNMEAString() { return m_NMEAstring; }
    wxString GetDataSource() { return m_datasource; }
    DataStream *GetDataStream() { return m_pDataStream; }
    int GetPrority() { return m_priority; }
    void SetNMEAString(wxString &string) { m_NMEAstring = string; }
    void SetDataSource(wxString &string) { m_datasource = string; }
    void SetPriority(int priority) { m_priority = priority; }
    void SetDataStream(DataStream *pds) { m_pDataStream = pds; }
    
    // required for sending with wxPostEvent()
    wxEvent *Clone() const;

private:
    wxString    m_datasource;
    wxString    m_NMEAstring;
    int         m_priority;
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
               int EOS_type = DS_EOS_CRLF,
               int handshake_type = DS_HANDSHAKE_NONE,
               void *user_data = NULL );

    ~DataStream();

    void Close(void);

    bool IsOk(){ return m_bok; }
    wxString GetPort(){ return m_portstring; }
    dsPortType GetIoSelect(){ return m_io_select; }
    int GetPrority(){ return m_priority; }
    void *GetUserData(){ return m_user_data; }

    bool SendSentence( const wxString &sentence );

    int GetLastError(){ return m_last_error; }

 //    Secondary thread life toggle
 //    Used to inform launching object (this) to determine if the thread can
 //    be safely called or polled, e.g. wxThread->Destroy();
    void SetSecThreadActive(void){m_bsec_thread_active = true;}
    void SetSecThreadInActive(void){m_bsec_thread_active = false;}

    void SetChecksumCheck(bool check) { m_bchecksumCheck = check; }

    void SetInputFilter(wxArrayString filter) { m_input_filter = filter; }
    void SetInputFilterType(ListType filter_type) { m_input_filter_type = filter_type; }
    void SetOutputFilter(wxArrayString filter) { m_output_filter = filter; }
    void SetOutputFilterType(ListType filter_type) { m_output_filter_type = filter_type; }
    bool SentencePassesFilter(const wxString& sentence, FilterDirection direction);
    bool ChecksumOK(const wxString& sentence);

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

    wxMutex                 *m_pShareMutex;
    wxMutex                 *m_pPortMutex;

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

#endif


