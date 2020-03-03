// -*- C++ -*- 
/* 
   wxServDisc.h: wxServDisc API definition

   This file is part of wxServDisc, a crossplatform wxWidgets
   Zeroconf service discovery module.
 
   Copyright (C) 2008 Christian Beier <dontmind@freeshell.org>
 
   wxServDisc is free software; you can redistribute it and/or modify 
   it under the terms of the GNU General Public License as published by 
   the Free Software Foundation; either version 2 of the License, or 
   (at your option) any later version. 
 
   wxServDisc is distributed in the hope that it will be useful, 
   but WITHOUT ANY WARRANTY; without even the implied warranty of 
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
   GNU General Public License for more details. 
 
   You should have received a copy of the GNU General Public License 
   along with this program; if not, write to the Free Software 
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. 
*/

#ifndef WXSERVDISC_H
#define WXSERVDISC_H

#include <wx/event.h>
#include <wx/string.h>
#include <wx/hashmap.h>
#include <wx/stopwatch.h>
#include <vector>

#include "1035.h"
#include "mdnsd.h"

// all the nice socket includes in one place here
#ifdef _WIN32
// https://stackoverflow.com/questions/5004858/stdmin-gives-error
#define NOMINMAX
// mingw/ visual studio socket includes
#include <winsock2.h>
#include <ws2tcpip.h>
#define SHUT_RDWR SD_BOTH
#else // proper UNIX
typedef int SOCKET;       // under windows, SOCKET is unsigned
#define INVALID_SOCKET -1 // so there also is no -1 return value
#define closesocket(s) close(s) // under windows, it's called closesocket
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif




// make available custom notify event if getResults() would yield sth new
#if wxVERSION_NUMBER < 2900
DECLARE_EVENT_TYPE(wxServDiscNOTIFY, -1);
#else
wxDECLARE_EVENT(wxServDiscNOTIFY, wxCommandEvent);
#endif




// resource name with ip addr and port number
struct wxSDEntry
{
  wxString name;
  wxString ip;
  int port;
  long time;
  wxSDEntry() { port=0; time=0; }
};



// our main class
class wxServDisc: public wxObject, public wxThreadHelper
{
public:
  // type can be one of QTYPE_A, QTYPE_NS, QTYPE_CNAME, QTYPE_PTR or QTYPE_SRV 
  wxServDisc(void* parent, const wxString& what, int type);
  ~wxServDisc();
 
  /// Returns true if service discovery successfully started. If not, getErr() may contain a hint.
  bool isOK() const { return err.length() == 0; };
 
  // yeah well...
  std::vector<wxSDEntry> getResults() const;
  size_t getResultCount() const;

  // get query name
  const wxString& getQuery() const { const wxString& ref = query; return ref; };
  // get error string
  const wxString& getErr() const { const wxString& ref = err; return ref; };


private:
  SOCKET mSock;
  wxString err;
  void *parent;
  wxString query;
  int querytype; 
WX_DECLARE_STRING_HASH_MAP(wxSDEntry, wxSDMap);
  wxSDMap results;
  wxStopWatch mWallClock;
  
  // this runs as a separate thread
  virtual wxThread::ExitCode Entry();

  // create a multicast 224.0.0.251:5353 socket, windows or unix style
  SOCKET msock(); 
  // send/receive message m
  bool sendm(struct message *m, SOCKET s, unsigned long int ip, unsigned short int port);
  int  recvm(struct message *m, SOCKET s, unsigned long int *ip, unsigned short int *port);
  // callback for the mdns resolver
  static int ans(mdnsda a, void *caller);

  void post_notify();
};



#endif // WXSERVDISC_H












