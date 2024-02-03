/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:
 * Author:   David Register, Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2023 by David Register, Alec Leamas                     *
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
 **************************************************************************/

#ifndef _COMMDRIVERN2KNET_H
#define _COMMDRIVERN2KNET_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled header

#include "comm_drv_n2k.h"
#include "conn_params.h"

#include <wx/datetime.h>

#ifdef __WXGTK__
// newer versions of glib define its own GSocket but we unfortunately use this
// name in our own (semi-)public header and so can't change it -- rename glib
// one instead
//#include <gtk/gtk.h>
#define GSocket GlibGSocket
#include <wx/socket.h>
#undef GSocket
#else
#include <wx/socket.h>
#endif

#ifndef __WXMSW__
#include <sys/socket.h>  // needed for (some) Mac builds
#include <netinet/in.h>
#endif

#if !defined(__WXMSW__) && !defined(__WXMAC__)
#include <linux/can.h>
#include <linux/can/raw.h>
#endif

#if defined (__WXMAC__)
#include can.h
#endif

#define RX_BUFFER_SIZE_NET 4096

#define ESCAPE 0x10
#define STARTOFTEXT 0x02
#define ENDOFTEXT 0x03

#define MsgTypeN2kData 0x93
#define MsgTypeN2kRequest 0x94

//typedef struct can_frame CanFrame;

typedef enum
{
  N2KFormat_Undefined = 0,
  N2KFormat_YD_RAW,
  N2KFormat_Actisense_RAW_ASCII,
  N2KFormat_Actisense_N2K_ASCII,
  N2KFormat_Actisense_N2K,
  N2KFormat_Actisense_RAW,
  N2KFormat_Actisense_NGT
} N2K_Format;

class CommDriverN2KNetEvent;  // Internal
class MrqContainer;
class FastMessageMap;

/// CAN v2.0 29 bit header as used by NMEA 2000
class CanHeader {
public:
  CanHeader();
    /** Construct a CanHeader by parsing a frame */
  CanHeader(can_frame frame);

  /** Return true if header reflects a multipart fast message. */
  bool IsFastMessage() const;

  unsigned char priority;
  unsigned char source;
  unsigned char destination;
  int pgn;
};

class circular_buffer {
public:
  circular_buffer(size_t size);
  void reset();
  size_t capacity() const;
  size_t size() const;
  bool empty() const;
  bool full() const;
  void put(unsigned char item);
  unsigned char get();

private:
  std::mutex mutex_;
  std::unique_ptr<unsigned char[]> buf_;
  size_t head_ = 0;
  size_t tail_ = 0;
  const size_t max_size_;
  bool full_ = 0;
};

class CommDriverN2KNet : public CommDriverN2K, public wxEvtHandler {
public:
  CommDriverN2KNet();
  CommDriverN2KNet(const ConnectionParams* params, DriverListener& listener);

  virtual ~CommDriverN2KNet();

  /** Register driver and possibly do other post-ctor steps. */
  void Activate() override;
  void SetListener(DriverListener& l) override{};

  void Open();
  void Close();
  ConnectionParams GetParams() const { return m_params; }

  bool SetOutputSocketOptions(wxSocketBase* tsock);
  bool SendSentenceNetwork(const wxString& payload);
  void OnServerSocketEvent(wxSocketEvent& event);  // The listener
  void OnTimerSocket(wxTimerEvent& event);
  void OnSocketEvent(wxSocketEvent& event);
  void OpenNetworkGPSD();
  void OpenNetworkTCP(unsigned int addr);
  void OpenNetworkUDP(unsigned int addr);
  void OnSocketReadWatchdogTimer(wxTimerEvent& event);

  bool SendMessage(std::shared_ptr<const NavMsg> msg,
                   std::shared_ptr<const NavAddr> addr) override;
  wxSocketBase* GetSock() const { return m_sock; }

private:
  ConnectionParams m_params;
  DriverListener& m_listener;

  void handle_N2K_MSG(CommDriverN2KNetEvent& event);
  wxString GetNetPort() const { return m_net_port; }
  wxIPV4address GetAddr() const { return m_addr; }
  wxTimer* GetSocketThreadWatchdogTimer() {
    return &m_socketread_watchdog_timer;
  }
  wxTimer* GetSocketTimer() { return &m_socket_timer; }
  void SetSock(wxSocketBase* sock) { m_sock = sock; }
  void SetTSock(wxSocketBase* sock) { m_tsock = sock; }
  wxSocketBase* GetTSock() const { return m_tsock; }
  void SetSockServer(wxSocketServer* sock) { m_socket_server = sock; }
  wxSocketServer* GetSockServer() const { return m_socket_server; }
  void SetMulticast(bool multicast) { m_is_multicast = multicast; }
  bool GetMulticast() const { return m_is_multicast; }

  NetworkProtocol GetProtocol() { return m_net_protocol; }
  void SetBrxConnectEvent(bool event) { m_brx_connect_event = event; }
  bool GetBrxConnectEvent() { return m_brx_connect_event; }

  void SetConnectTime(wxDateTime time) { m_connect_time = time; }
  wxDateTime GetConnectTime() { return m_connect_time; }

  dsPortType GetPortType() const { return m_io_select; }
  wxString GetPort() const { return m_portstring; }

  std::vector<unsigned char> PushFastMsgFragment(const CanHeader& header,
                                                 int position);
  std::vector<unsigned char> PushCompleteMsg(const CanHeader header,
                                             int position,
                                             const can_frame frame);

  void HandleCanFrameInput(can_frame frame);

  ConnectionType GetConnectionType() const { return m_connection_type; }

  bool ChecksumOK(const std::string& sentence);
  void SetOk(bool ok) { m_bok = ok; };

  N2K_Format DetectFormat(std::vector<unsigned char> packet);
  bool ProcessActisense_ASCII_RAW(std::vector<unsigned char> packet);
  bool ProcessActisense_ASCII_N2K(std::vector<unsigned char> packet);
  bool ProcessActisense_N2K(std::vector<unsigned char> packet);
  bool ProcessActisense_RAW(std::vector<unsigned char> packet);
  bool ProcessActisense_NGT(std::vector<unsigned char> packet);

  wxString m_net_port;
  NetworkProtocol m_net_protocol;
  wxIPV4address m_addr;
  wxSocketBase* m_sock;
  wxSocketBase* m_tsock;
  wxSocketServer* m_socket_server;
  bool m_is_multicast;
  MrqContainer  *m_mrq_container;

  int m_txenter;
  int m_dog_value;
  std::string m_sock_buffer;
  wxString m_portstring;
  dsPortType m_io_select;
  wxDateTime m_connect_time;
  bool m_brx_connect_event;
  bool m_bchecksumCheck;
  ConnectionType m_connection_type;

  wxTimer m_socket_timer;
  wxTimer m_socketread_watchdog_timer;

  bool m_bok;
  int m_ib;
  bool m_bInMsg, m_bGotESC, m_bGotSOT;

  circular_buffer *m_circle;
  unsigned char *rx_buffer;
  std::string m_sentence;

  FastMessageMap *fast_messages;
  N2K_Format m_n2k_format;

  DECLARE_EVENT_TABLE()
};

#endif  // guard
