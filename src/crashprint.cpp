//////////////////////////////////////////////////////////////////////////////
// File:        crashprint.cpp
// Purpose:     wxCrashPrint
// Maintainer:  Wyo
// Created:     2004-09-28
// RCS-ID:      $Id: crashprint.cpp,v 1.11 2005-04-14 19:41:33 wyo Exp $
// Copyright:   (c) 2004 wxCode
// Licence:     wxWindows
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
// information
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// headers
//----------------------------------------------------------------------------

// For compilers that support precompilation, includes <wx/wx.h>.
#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all 'standard' wxWidgets headers)
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

//! standard header
#if defined(__linux__)
#include <execinfo.h>    // Needed for backtrace
#include <cxxabi.h>      // Needed for __cxa_demangle
#include <unistd.h>
#endif

// wxWidgets headers
#include <wx/string.h>   // strings support

// crashprint headers
#include "crashprint.h"   // crash print support


//----------------------------------------------------------------------------
// resources
//----------------------------------------------------------------------------


//============================================================================
// declarations
//============================================================================


//============================================================================
// implementation
//============================================================================

//----------------------------------------------------------------------------
// wxCrashPrint
//----------------------------------------------------------------------------

wxCrashPrint::wxCrashPrint (int flags, const wxString &fname) {

    m_flags = flags;
    m_fname = fname;

};

//----------------------------------------------------------------------------
// settings functions

//----------------------------------------------------------------------------
// general functions

void wxCrashPrint::Report () {
    wxString appname = wxTheApp->GetAppName();

    // get the backtrace with symbols
    int btCount;
    btCount = backtrace (m_btBuffer, maxBtCount);
    if (btCount < 0) {
        wxPrintf (_T("\n%s: Backtrace could not be created\n"), appname.c_str());
    }
    m_btStrings = backtrace_symbols (m_btBuffer, btCount);
    if (!m_btStrings) {
        wxPrintf (_T("\n%s: Backtrace could not get symbols\n"), appname.c_str());
    }

    // print backtrace announcement
    wxPrintf (_T("\n*** %s (%s) crashed ***, see backtrace!\n"), appname.c_str(), wxVERSION_STRING);

    // format backtrace lines
    int status;
    wxString cur, addr, func, addrs;
    wxArrayString lines;
    size_t pos1, pos2;
    for (int i = 0; i < btCount; ++i) {
        cur = wxString::FromAscii  (m_btStrings[i]);
        pos1 = cur.rfind ('[');
        pos2 = cur.rfind (']');
        if ((pos1 != wxString::npos) && (pos2 != wxString::npos)) {
            addr = cur.substr (pos1 + 1, pos2 - pos1 - 1);
            addrs.Append (addr + _T(" "));
        }
        pos1 = cur.rfind (_T("_Z"));
        pos2 = cur.rfind ('+');
        if (pos2 == wxString::npos) pos2 = cur.rfind (')');
        if (pos1 != wxString::npos) {
            func = cur.substr (pos1, pos2 - pos1);
            func = wxString::FromAscii (abi::__cxa_demangle (func.mb_str(), 0, 0, &status));
        }else{
            pos1 = cur.rfind ('(');
            if (pos1 != wxString::npos) {
                func = cur.substr (pos1 + 1, pos2 - pos1 - 1);
            }else{
                pos2 = cur.rfind ('[');
                func = cur.substr (0, pos2 - 1);
            }
        }
        lines.Add (addr + _T(" in ") + func);
        if (func == _T("main")) break;
    }

    // determine line from address
    wxString cmd = wxString::Format (_T("addr2line -e /proc/%d/exe -s "), getpid());
    wxArrayString fnames;
    if (wxExecute (cmd + addrs, fnames) != -1) {
        for (size_t i = 0; i < fnames.GetCount(); ++i) {
            wxPrintf (_T("%s at %s\n"), lines[i].c_str(), fnames[i].c_str());
        }
    }else{
        for (size_t i = 0; i < lines.GetCount(); ++i) {
            wxPrintf (_T("%s\n"), lines[i].c_str());
        }
    }

}
