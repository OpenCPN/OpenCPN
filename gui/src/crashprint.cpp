//////////////////////////////////////////////////////////////////////////////
// File:        crashprint.cpp
// Purpose:     wxCrashPrint
// Maintainer:  Wyo
// Created:     2004-09-28
// Copyright:   (c) 2004 wxCode
// Licence:     wxWindows
//////////////////////////////////////////////////////////////////////////////

/**
 * \file
 *
 * Implement crashprint.h  --  dump debug info on crash.
 */

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#if defined(__linux__)
#include <execinfo.h>  // Needed for backtrace
#include <cxxabi.h>    // Needed for __cxa_demangle
#include <unistd.h>
#endif

#include <wx/string.h>

#include "crashprint.h"  // crash print support

wxCrashPrint::wxCrashPrint(int flags, const wxString &fname) {
  m_flags = flags;
  m_fname = fname;
};

void wxCrashPrint::Report() {
#if defined(__linux__)
  wxString appname = wxTheApp->GetAppName();

  // get the backtrace with symbols
  int btCount;
  btCount = backtrace(m_btBuffer, maxBtCount);
  if (btCount < 0) {
    wxPrintf("\n%s: Backtrace could not be created\n", appname.c_str());
  }
  m_btStrings = backtrace_symbols(m_btBuffer, btCount);
  if (!m_btStrings) {
    wxPrintf("\n%s: Backtrace could not get symbols\n", appname.c_str());
  }

  // print backtrace announcement
  wxPrintf("\n*** %s (%s) crashed ***, see backtrace!\n", appname.c_str(),
           wxVERSION_STRING);

  // format backtrace lines
  int status;
  wxString cur, addr, func, addrs;
  wxArrayString lines;
  size_t pos1, pos2;
  if (m_btStrings)
    for (int i = 0; i < btCount; ++i) {
      cur = wxString::FromAscii(m_btStrings[i]);
      pos1 = cur.rfind('[');
      pos2 = cur.rfind(']');
      if ((pos1 != wxString::npos) && (pos2 != wxString::npos)) {
        addr = cur.substr(pos1 + 1, pos2 - pos1 - 1);
        addrs.Append(addr + " ");
      }
      pos1 = cur.rfind("_Z");
      pos2 = cur.rfind('+');
      if (pos2 == wxString::npos) pos2 = cur.rfind(')');
      if (pos1 != wxString::npos) {
        func = cur.substr(pos1, pos2 - pos1);
        func = wxString::FromAscii(
            abi::__cxa_demangle(func.mb_str(), 0, 0, &status));
      } else {
        pos1 = cur.rfind('(');
        if (pos1 != wxString::npos) {
          func = cur.substr(pos1 + 1, pos2 - pos1 - 1);
        } else {
          pos2 = cur.rfind('[');
          func = cur.substr(0, pos2 - 1);
        }
      }
      lines.Add(addr + " in " + func);
      if (func == "main") break;
    }

  // determine line from address
  wxString cmd = wxString::Format("addr2line -e /proc/%d/exe -s ", getpid());
  wxArrayString fnames;
  if (wxExecute(cmd + addrs, fnames) != -1) {
    for (size_t i = 0; i < fnames.GetCount(); ++i) {
      wxPrintf("%s at %s\n", lines[i].c_str(), fnames[i].c_str());
    }
  } else {
    for (size_t i = 0; i < lines.GetCount(); ++i) {
      wxPrintf("%s\n", lines[i].c_str());
    }
  }
#endif
}
