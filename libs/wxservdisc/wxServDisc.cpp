/*
   wxServDisc.cpp: wxServDisc implementation

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


#include "wx/object.h"
#include "wx/thread.h"
#include "wx/intl.h"
#include "wx/log.h"

#include <fcntl.h>
#include <cerrno>
#include <csignal>



// only used by VC++
#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#endif

#include "wxServDisc.h"


// Compatability defines
#ifdef __APPLE__
#ifndef IPV6_ADD_MEMBERSHIP
#define IPV6_ADD_MEMBERSHIP IPV6_JOIN_GROUP
#define IPV6_DROP_MEMBERSHIP IPV6_LEAVE_GROUP
#endif
#endif


// define our new notify event!
#if wxVERSION_NUMBER < 2900
DEFINE_EVENT_TYPE(wxServDiscNOTIFY);
#else
wxDEFINE_EVENT(wxServDiscNOTIFY, wxCommandEvent);
#endif




/*
  private member functions

*/


wxThread::ExitCode wxServDisc::Entry()
{
  mdnsd d;
  struct message m;
  unsigned long int ip;
  unsigned short int port;
  struct timeval *tv;
  fd_set fds;
  bool exit = false;

  mWallClock.Start();

  d = mdnsd_new(1,1000);


  // register query(w,t) at mdnsd d, submit our address for callback ans()
  mdnsd_query(d, query.char_str(), querytype, ans, this);


#ifdef __WXGTK__
  // this signal is generated when we pop up a file dialog wwith wxGTK
  // we need to block it here cause it interrupts the select() call
  sigset_t            newsigs;
  sigset_t            oldsigs;
  sigemptyset(&newsigs);
  sigemptyset(&oldsigs);
  sigaddset(&newsigs, SIGRTMIN-1);
#endif


  while(!GetThread()->TestDestroy() && !exit)
    {
      //printf("TopLoop...\n");
      tv = mdnsd_sleep(d);

      long msecs = 100; //tv->tv_sec == 0 ? 100 : tv->tv_sec*1000; // so that the while loop beneath gets executed once
      //printf("wxServDisc %p: scanthread waiting for data, timeout %i seconds\n", this, (int)tv->tv_sec);


      // we split the one select() call into several ones every 100ms
      // to be able to catch TestDestroy()...
      int datatoread = 0;
      while(msecs > 0 && !GetThread()->TestDestroy() && !datatoread)
	{
          //printf("loop...\n");
	  // the select call leaves tv undefined, so re-set
	  tv->tv_sec = 0;
	  tv->tv_usec = 100000; // 100 ms

	  FD_ZERO(&fds);
	  FD_SET(mSock,&fds);


#ifdef __WXGTK__
	  sigprocmask(SIG_BLOCK, &newsigs, &oldsigs);
#endif
      //wxLogDebug(wxT("wxServDisc %p: select call, timeout %i seconds. %i usec."), this, (int)tv->tv_sec, (int)tv->tv_usec);
      //printf("wxServDisc %p: select call, timeout %i seconds. %i usec.\n", this, (int)tv->tv_sec, (int)tv->tv_usec);
	  datatoread = select(mSock+1,&fds,0,0,tv); // returns 0 if timeout expired

#ifdef __WXGTK__
	  sigprocmask(SIG_SETMASK, &oldsigs, NULL);
#endif

          //printf("out of select\n");
	  if(!datatoread){ // this is a timeout
            //printf("wxServDisc timeout %d\n", (int)msecs);
	    msecs-=100;
          }
	  if(datatoread == -1)
	    break;
	}

      //printf("wxServDisc %p: scanthread woke up, reason: incoming data(%i), timeout(%i), error(%i), deletion(%i)\n",
	//	 this, datatoread>0, msecs<=0, datatoread==-1, GetThread()->TestDestroy() );

      // receive
      if(FD_ISSET(mSock,&fds))
        {
	  while(recvm(&m, mSock, &ip, &port) > 0)
	    mdnsd_in(d, &m, ip, port);
        }

      // send
      while(mdnsd_out(d,&m,&ip,&port))
	if(!sendm(&m, mSock, ip, port))
	  {
	    exit = true;
	    break;
	  }
    }

  mdnsd_shutdown(d);
  mdnsd_free(d);


  if(mSock != INVALID_SOCKET)
    closesocket(mSock);


  //printf("wxServDisc %p: scanthread exiting\n", this);

  return NULL;
}




bool wxServDisc::sendm(struct message* m, SOCKET s, unsigned long int ip, unsigned short int port)
{
  struct sockaddr_in to;

  memset(&to, '\0', sizeof(to));

  to.sin_family = AF_INET;
  to.sin_port = port;
  to.sin_addr.s_addr = ip;

  if(sendto(s, (char*)message_packet(m), message_packet_len(m), 0,(struct sockaddr *)&to,sizeof(struct sockaddr_in)) != message_packet_len(m))
    {
      err.Printf(_("Can't write to socket: %s\n"),strerror(errno));
      return false;
    }

  return true;
}





int wxServDisc::recvm(struct message* m, SOCKET s, unsigned long int *ip, unsigned short int *port)
{
  struct sockaddr_in from;
  int bsize;
  static unsigned char buf[MAX_PACKET_LEN];
#ifdef __WIN32__
  int ssize  = sizeof(struct sockaddr_in);
#else
  socklen_t ssize  = sizeof(struct sockaddr_in);
#endif


  if((bsize = recvfrom(s, (char*)buf, MAX_PACKET_LEN, 0, (struct sockaddr*)&from, &ssize)) > 0)
    {
      memset(m, '\0', sizeof(struct message));
      message_parse(m,buf);
      *ip = (unsigned long int)from.sin_addr.s_addr;
      *port = from.sin_port;
      return bsize;
    }

#ifdef __WIN32__
  if(bsize < 0 && WSAGetLastError() != WSAEWOULDBLOCK)
#else
    if(bsize < 0 && errno != EAGAIN)
#endif
      {
	err.Printf(_("Can't read from socket %d: %s\n"),
		      errno,strerror(errno));
	return bsize;
      }

  return 0;
}





int wxServDisc::ans(mdnsda a, void *arg)
{
  wxServDisc *moi = (wxServDisc*)arg;

  wxString key;
  switch(a->type)
    {
    case QTYPE_PTR:
      // query result is key
      key = wxString((char*)a->rdname, wxConvUTF8);
      break;
    case QTYPE_A:
    case QTYPE_SRV:
      // query name is key
      key = wxString((char*)a->name, wxConvUTF8);
      break;
    default:
      break;
    }


  // insert answer data into result
  // depending on the query, not all fields have a meaning
  wxSDEntry result;

  result.name = wxString((char*)a->rdname, wxConvUTF8);

  result.time = moi->mWallClock.Time();

  struct in_addr ip;
  ip.s_addr =  ntohl(a->ip);
  result.ip = wxString(inet_ntoa(ip), wxConvUTF8);

  result.port = a->srv.port;


  if(a->ttl == 0)
    // entry was expired
    moi->results.erase(key);
  else
    // entry update
    moi->results[key] = result;

  moi->post_notify();


  wxLogDebug(wxT("wxServDisc %p: got answer:"), moi);
  wxLogDebug(wxT("wxServDisc %p:    key:  %s"), moi, key.c_str());
  wxLogDebug(wxT("wxServDisc %p:    ttl:  %i"), moi, (int)a->ttl);
  wxLogDebug(wxT("wxServDisc %p:    time: %lu"), moi, result.time);
  if(a->ttl != 0) {
    wxLogDebug(wxT("wxServDisc %p:    name: %s"), moi, moi->results[key].name.c_str());
    wxLogDebug(wxT("wxServDisc %p:    ip:   %s"), moi, moi->results[key].ip.c_str());
    wxLogDebug(wxT("wxServDisc %p:    port: %u"), moi, moi->results[key].port);
  }
  wxLogDebug(wxT("wxServDisc %p: answer end"),  moi);

  return 1;
}



// create a multicast 224.0.0.251:5353 socket,
// aproppriate for receiving and sending,
// windows or unix style
SOCKET wxServDisc::msock()
{
  SOCKET sock;

  int multicastTTL = 255; // multicast TTL, must be 255 for zeroconf!
  const char* mcAddrStr = "224.0.0.251";
  const char* mcPortStr = "5353";


  int flag = 1;
  int status;
  struct addrinfo hints;   // Hints for name lookup
  struct addrinfo* multicastAddr;  // Multicast address
  struct addrinfo* localAddr;      // Local address to bind to

  unsigned long block=1;


#ifdef __WIN32__
  /*
    Start up WinSock
   */
  WORD		wVersionRequested;
  WSADATA	wsaData;
  wVersionRequested = MAKEWORD(2, 2);
  if(WSAStartup(wVersionRequested, &wsaData) != 0)
    {
      WSACleanup();
      err.Printf(_("Failed to start WinSock!"));
      return INVALID_SOCKET;
    }
#endif


  /*
     Resolve the multicast group address
  */
  memset(&hints, 0, sizeof hints); // make sure the struct is empty
  hints.ai_family = PF_UNSPEC; // IPv4 or IPv6, we don't care
  hints.ai_flags  = AI_NUMERICHOST;
  if ((status = getaddrinfo(mcAddrStr, NULL, &hints, &multicastAddr)) != 0) {
    err.Printf(_("Could not get multicast address: %s\n"), gai_strerror(status));
    return INVALID_SOCKET;
  }

  wxLogDebug(wxT("wxServDisc %p: Using %s"), this, multicastAddr->ai_family == PF_INET6 ? wxT("IPv6") : wxT("IPv4"));


  /*
     Resolve a local address with the same family (IPv4 or IPv6) as our multicast group
  */
  memset(&hints, 0, sizeof hints); // make sure the struct is empty
  hints.ai_family   = multicastAddr->ai_family;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags    = AI_NUMERICHOST|AI_PASSIVE; // no name resolving, wildcard address
  if ((status = getaddrinfo(NULL, mcPortStr, &hints, &localAddr)) != 0 ) {
    err.Printf(_("Could not get local address: %s\n"), gai_strerror(status));
    freeaddrinfo(multicastAddr);
    return INVALID_SOCKET;
  }






  /*
    Create socket
  */
  if((sock = socket(localAddr->ai_family, localAddr->ai_socktype, 0)) == INVALID_SOCKET) {
    err.Printf(_("Could not create socket: %s\n"), strerror(errno));
    // not yet a complete cleanup!
    freeaddrinfo(localAddr);
    freeaddrinfo(multicastAddr);
    return INVALID_SOCKET;
  }



  /*
    set to reuse address (and maybe port - needed on OSX)
  */
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(flag));
#if defined (SO_REUSEPORT)
  setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (char*)&flag, sizeof(flag));
#endif



  /*
    Bind this socket to localAddr
   */
  if(bind(sock, localAddr->ai_addr, localAddr->ai_addrlen) != 0) {
    err.Printf(_("Could not bind socket: %s\n"), strerror(errno));
    goto CompleteCleanUp;
  }


  /*
    Set the multicast TTL.
  */
  if ( setsockopt(sock,
		  localAddr->ai_family == PF_INET6 ? IPPROTO_IPV6        : IPPROTO_IP,
		  localAddr->ai_family == PF_INET6 ? IPV6_MULTICAST_HOPS : IP_MULTICAST_TTL,
		  (char*) &multicastTTL, sizeof(multicastTTL)) != 0 ) {
    err.Printf(_("Could not set multicast TTL: %s\n"), strerror(errno));
    goto CompleteCleanUp;
   }



   /*
      Join the multicast group. We do this seperately depending on whether we
      are using IPv4 or IPv6.
   */
  if ( multicastAddr->ai_family  == PF_INET &&
       multicastAddr->ai_addrlen == sizeof(struct sockaddr_in) ) /* IPv4 */
    {
      struct ip_mreq multicastRequest;  // Multicast address join structure

      /* Specify the multicast group */
      memcpy(&multicastRequest.imr_multiaddr,
	     &((struct sockaddr_in*)multicastAddr->ai_addr)->sin_addr,
	     sizeof(multicastRequest.imr_multiaddr));

      /* Accept multicast from any interface */
      multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);

      /* Join the multicast address */
      if ( setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &multicastRequest, sizeof(multicastRequest)) != 0 )  {
	err.Printf(_("Could not join multicast group: %s\n"), strerror(errno));
	goto CompleteCleanUp;
      }

    }
  else if ( multicastAddr->ai_family  == PF_INET6 &&
	    multicastAddr->ai_addrlen == sizeof(struct sockaddr_in6) ) /* IPv6 */
    {
      struct ipv6_mreq multicastRequest;  /* Multicast address join structure */

      /* Specify the multicast group */
      memcpy(&multicastRequest.ipv6mr_multiaddr,
	     &((struct sockaddr_in6*)multicastAddr->ai_addr)->sin6_addr,
	     sizeof(multicastRequest.ipv6mr_multiaddr));

      /* Accept multicast from any interface */
      multicastRequest.ipv6mr_interface = 0;

      /* Join the multicast address */
      if ( setsockopt(sock, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (char*) &multicastRequest, sizeof(multicastRequest)) != 0 ) {
	err.Printf(_("Could not join multicast group: %s\n"), strerror(errno));
	goto CompleteCleanUp;
      }
    }
  else {
    err.Printf(_("Neither IPv4 or IPv6"));
    goto CompleteCleanUp;
  }




  /*
     Set to nonblock
  */
#ifdef _WIN32
  ioctlsocket(sock, FIONBIO, &block);
#else
  flag =  fcntl(sock, F_GETFL, 0);
  flag |= O_NONBLOCK;
  fcntl(sock, F_SETFL, flag);
#endif



  /*
    whooaa, that's it
  */
  freeaddrinfo(localAddr);
  freeaddrinfo(multicastAddr);

  return sock;



 CompleteCleanUp:

  closesocket(sock);
  freeaddrinfo(localAddr);
  freeaddrinfo(multicastAddr);
  return INVALID_SOCKET;

}





/*
  public member functions

*/


wxServDisc::wxServDisc(void* p, const wxString& what, int type)
{
  // save our caller
  parent = p;

  // save query
  query = what;
  querytype = type;

  wxLogDebug(wxT(""));
  wxLogDebug(wxT("wxServDisc %p: about to query '%s'"), this, query.c_str());

  if((mSock = msock()) == INVALID_SOCKET) {
    wxLogDebug(wxT("Ouch, error creating socket: ") + err);
    return;
  }

#if wxVERSION_NUMBER >= 2905 // 2.9.4 still has a bug here: http://trac.wxwidgets.org/ticket/14626
  if( CreateThread(wxTHREAD_DETACHED) != wxTHREAD_NO_ERROR )
#else
  if( Create() != wxTHREAD_NO_ERROR )
#endif
    err.Printf(_("Could not create scan thread!"));
  else
    if( GetThread()->Run() != wxTHREAD_NO_ERROR )
      err.Printf(_("Could not start scan thread!"));
}



wxServDisc::~wxServDisc()
{
  wxLogDebug(wxT("wxServDisc %p: before scanthread delete"), this);
  if(GetThread() && GetThread()->IsRunning()){
    GetThread()->Delete(); // blocks, this makes TestDestroy() return true and cleans up the thread
    wxMilliSleep(200);
  }

  wxLogDebug(wxT("wxServDisc %p: scanthread deleted, wxServDisc destroyed, query was '%s', lifetime was %ld"), this, query.c_str(), mWallClock.Time());
  wxLogDebug(wxT(""));
}





std::vector<wxSDEntry> wxServDisc::getResults() const
{
  std::vector<wxSDEntry> resvec;

  wxSDMap::const_iterator it;
  for(it = results.begin(); it != results.end(); it++)
    resvec.push_back(it->second);

  return resvec;
}



size_t wxServDisc::getResultCount() const
{
  return results.size();
}





void wxServDisc::post_notify()
{
  if(parent)
    {
      // new NOTIFY event, we got no window id
      wxCommandEvent event(wxServDiscNOTIFY, wxID_ANY);
      event.SetEventObject(this); // set sender

      // Send it
      wxQueueEvent((wxEvtHandler*)parent, event.Clone());
    }
}




