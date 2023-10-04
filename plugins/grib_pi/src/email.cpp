/////////////////////////////////////////////////////////////////////////////
// Name:        email.h
// Purpose:     wxEmail: portable email client class
// Author:      Julian Smart
// Modified by:
// Created:     2001-08-21
// RCS-ID:      $Id: email.cpp 35650 2005-09-23 12:56:45Z MR $
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/string.h"
#include "email.h"

#ifdef __WXMSW__
#include "smapi.h"
#endif

#ifdef __UNIX__
#include "wx/filefn.h"
#include "wx/timer.h"
#include "wx/wfstream.h"
#include "stdlib.h"
#include "unistd.h"
#endif

// Send a message.
// The 'from' field is not needed by MAPI, and on Unix, is needed only by
// certain MTA ( for example ssmtp) if not enter by user , will be supplied by
// the system

#ifdef __WXMSW__
bool wxEmail::Send(wxMailMessage& message, int sendMethod,
                   const wxString& profileName, const wxString& sendMail2,
                   const wxString& sendMail1, const wxString& sendMail0) {
  wxString mailURL = _T("mailto:");
  mailURL +=
      message.m_to[0] + _T("?&subject=") + message.m_subject + _T("&body=");
  wxString msgBody = message.m_body;
  msgBody.Replace(" ", "%20");
  msgBody.Replace("\n", "%0A");
  mailURL += msgBody;

  wxLaunchDefaultBrowser(mailURL);
  return true;
#if 0
   // wxASSERT (message.m_to.GetCount() > 0) ;
    wxASSERT (!message.m_to.IsEmpty()) ;
    wxString profile(profileName);
    if (profile.IsEmpty())
        profile = wxGetUserName();

    wxMapiSession session;

    if (!session.MapiInstalled())
        return FALSE;
    if (!session.Logon(profile))
        return FALSE;

    return session.Send(message);
#endif
}
#elif defined(__UNIX__)
bool wxEmail::Send(wxMailMessage& message, int sendMethod,
                   const wxString& profileName, const wxString& sendMail2,
                   const wxString& sendMail1, const wxString& sendMail0) {
  wxASSERT_MSG(!message.m_to.IsEmpty(), _T("no recipients to send mail to"));

  wxString from = message.m_from;
  if (from.empty()) {
    from = wxGetEmailAddress();
  }

  wxString msg, sendmail;

  if (sendMethod == 0) {  // with xdg-email via local mail system (MUA)
#ifdef __WXMAC__
    wxString addr;
    for (size_t rcpt = 0; rcpt < message.m_to.GetCount(); rcpt++) {
      if (rcpt > 0) addr << ",";
      addr << message.m_to[rcpt];
    }
    wxString msg = wxString::Format(
        "sh -c \"open 'mailto:%s?subject=%s&body=%s'\"", addr.c_str(),
        message.m_subject.c_str(), message.m_body.c_str());
    long ret = wxExecute(msg.c_str());
    return ret != 0;  // 0 means the execution failed
#else
    if (wxFileExists(sendMail0))
      sendmail << sendMail0;
    else if (wxFileExists(sendMail1))
      sendmail << sendMail1;
    else {
      wxLogMessage(
          _T("MAIL Error: xdg-email is not installed on this computer!"));
      return false;
    }

    msg << "sh -c \" " << sendmail
        << " --utf8  --subject '" << message.m_subject << "' "
        <<"--body '"  << message.m_body << "'";
    for (size_t rcpt = 0; rcpt < message.m_to.GetCount(); rcpt++)
      msg << " '" << message.m_to[rcpt] << "'";  // add recipients
    msg << "\"";

    wxSystem(msg.c_str());
    return true;

  } else {  // directly with sendmail
    msg << wxT("To: ");
    for (size_t rcpt = 0; rcpt < message.m_to.GetCount(); rcpt++) {
      if (rcpt) msg << wxT(", ");
      msg << message.m_to[rcpt];
    }
    msg << wxT("\nFrom: ") << from << wxT("\nSubject: ") << message.m_subject;
    msg << wxT("\n\n") << message.m_body;

    wxString filename;
    filename.Printf(wxT("/tmp/msg-%ld-%ld-%ld.txt"), (long)getpid(),
                    wxGetLocalTime(), (long)rand());

    wxFileOutputStream stream(filename);
    if (stream.Ok())
      stream.Write(msg.ToUTF8(), msg.Length());
    else
      return FALSE;

    // TODO search for a suitable sendmail if sendMail is empty
    sendmail << sendMail2;

    wxString cmd;
    cmd << sendmail << wxT(" < ") << filename;

    // TODO: check return code
    wxSystem(cmd.c_str());

    wxRemoveFile(filename);

    return TRUE;
#endif
  }
  return FALSE;
}
#else
wxLogMessage(_T("Send eMail not yet implemented for this platform"));
return false;
//#error Send not yet implemented for this platform.
#endif
