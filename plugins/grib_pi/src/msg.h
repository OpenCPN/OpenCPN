/////////////////////////////////////////////////////////////////////////////
// Name:        msg.h
// Purpose:     wxMailMessage
// Author:      Julian Smart
// Modified by:
// Created:     2001-08-21
// RCS-ID:      $Id: msg.h 28475 2004-07-25 15:44:47Z VZ $
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////
/**
 * \file
 * Email Message Encapsulation.
 *
 * Provides a cross-platform email message class for GRIB data requests:
 *
 * Features:
 * - Multiple recipient handling (To/CC/BCC)
 * - File attachment support
 * - UTF-8 encoding
 * - Email header management
 * - Address validation
 * - MIME type handling
 *
 * The message class provides a structured way to compose and format email
 * requests for weather data from GRIB servers and services that support
 * email-based data delivery.
 */
#ifndef _WX_MSG_H_
#define _WX_MSG_H_

#define WXDLLIMPEXP_NETUTILS
#define WXDLLIMPEXP_DATA_NETUTILS(type) type

/*
 * wxMailMessage
 * Encapsulates an email message
 */

class WXDLLIMPEXP_NETUTILS wxMailMessage {
public:
  // A common usage
  wxMailMessage(const wxString& subject, const wxString& to,
                const wxString& body, const wxString& from = wxEmptyString,
                const wxString& attachment = wxEmptyString,
                const wxString& attachmentTitle = wxEmptyString) {
    m_to.Add(to);
    m_subject = subject;
    m_body = body;
    m_from = from;
    if (!attachment.IsEmpty()) {
      m_attachments.Add(attachment);
      m_attachmentTitles.Add(attachmentTitle);
    }
  }

  wxMailMessage() {};

  //// Accessors

  void AddTo(const wxString& to) { m_to.Add(to); }
  void AddCc(const wxString& cc) { m_cc.Add(cc); }
  void AddBcc(const wxString& bcc) { m_bcc.Add(bcc); }
  void AddAttachment(const wxString& attach,
                     const wxString& title = wxEmptyString) {
    m_attachments.Add(attach);
    m_attachmentTitles.Add(title);
  }

  void SetSubject(const wxString& subject) { m_subject = subject; }
  void SetBody(const wxString& body) { m_body = body; }
  void SetFrom(const wxString& from) { m_from = from; }

public:
  wxArrayString m_to;           // The To: Recipients
  wxString m_from;              // The From: email address (optional)
  wxArrayString m_cc;           // The CC: Recipients
  wxArrayString m_bcc;          // The BCC Recipients
  wxString m_subject;           // The Subject of the message
  wxString m_body;              // The Body of the message
  wxArrayString m_attachments;  // Files to attach to the email
  wxArrayString
      m_attachmentTitles;  // Titles to use for the email file attachments
};

#endif  // _WX_MSG_H_
