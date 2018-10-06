/////////////////////////////////////////////////////////////////////////////
// Name:        jsonreader.h
// Purpose:     the parser of JSON text
// Author:      Luciano Cattani
// Created:     2007/09/15
// RCS-ID:      $Id: jsonreader.h,v 1.3 2008/03/03 19:05:45 luccat Exp $
// Copyright:   (c) 2007 Luciano Cattani
// Licence:     wxWidgets licence
/////////////////////////////////////////////////////////////////////////////

#if !defined( _WX_JSONREADER_H )
#define _WX_JSONREADER_H

//#ifdef __GNUG__
//    #pragma interface "jsonreader.h"
//#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include <wx/stream.h>
    #include <wx/string.h>
    #include <wx/arrstr.h>
#endif


#include "json_defs.h"
#include "jsonval.h"

// The flags of the parser
enum {
    wxJSONREADER_STRICT          = 0,
    wxJSONREADER_ALLOW_COMMENTS  = 1,
    wxJSONREADER_STORE_COMMENTS  = 2,
    wxJSONREADER_CASE            = 4,
    wxJSONREADER_MISSING         = 8,
    wxJSONREADER_MULTISTRING     = 16,
    wxJSONREADER_COMMENTS_AFTER  = 32,
    wxJSONREADER_NOUTF8_STREAM   = 64,
    wxJSONREADER_MEMORYBUFF      = 128,

    wxJSONREADER_TOLERANT        = wxJSONREADER_ALLOW_COMMENTS | wxJSONREADER_CASE |
                                 wxJSONREADER_MISSING | wxJSONREADER_MULTISTRING,
    wxJSONREADER_COMMENTS_BEFORE = wxJSONREADER_ALLOW_COMMENTS | wxJSONREADER_STORE_COMMENTS
};


class WXDLLIMPEXP_JSON  wxJSONReader
{
public:
    wxJSONReader( int flags = wxJSONREADER_TOLERANT, int maxErrors = 30 );
    virtual ~wxJSONReader();

    int Parse( const wxString& doc, wxJSONValue* val );
    int Parse( wxInputStream& doc, wxJSONValue* val );

    int   GetDepth() const;
    int   GetErrorCount() const;
    int   GetWarningCount() const;
    const wxArrayString& GetErrors() const;
    const wxArrayString& GetWarnings() const;

    static int  UTF8NumBytes( char ch );

#if defined( wxJSON_64BIT_INT )
    static bool Strtoll( const wxString& str, wxInt64* i64 );
    static bool Strtoull( const wxString& str, wxUint64* ui64 );
    static bool DoStrto_ll( const wxString& str, wxUint64* ui64, wxChar* sign );
#endif

protected:

    int  DoRead( wxInputStream& doc, wxJSONValue& val );
    void AddError( const wxString& descr );
    void AddError( const wxString& fmt, const wxString& str );
    void AddError( const wxString& fmt, wxChar ch );
    void AddWarning( int type, const wxString& descr );
    int  GetStart( wxInputStream& is );
    int  ReadChar( wxInputStream& is );
    int  PeekChar( wxInputStream& is );
    void StoreValue( int ch, const wxString& key, wxJSONValue& value, wxJSONValue& parent );
    int  SkipWhiteSpace( wxInputStream& is );
    int  SkipComment( wxInputStream& is );
    void StoreComment( const wxJSONValue* parent );
    int  ReadString(  wxInputStream& is, wxJSONValue& val );
    int  ReadToken(  wxInputStream& is, int ch, wxString& s );
    int  ReadValue(  wxInputStream& is, int ch, wxJSONValue& val );
    int  ReadUES(  wxInputStream& is, char* uesBuffer );
    int  AppendUES( wxMemoryBuffer& utf8Buff, const char* uesBuffer );
    int  NumBytes( char ch );
    int  ConvertCharByChar( wxString& s, const wxMemoryBuffer& utf8Buffer );
    int  ReadMemoryBuff( wxInputStream& is, wxJSONValue& val );

    //! Flag that control the parser behaviour,
    int  m_flags;

    //! Maximum number of errors stored in the error's array
    int  m_maxErrors;

    //! The current line number (start at 1).
    int  m_lineNo;

    //! The current column number (start at 1).
    int  m_colNo;

    //! The current level of object/array annidation (start at ZERO).
    int  m_level;

    //! The depth level of the read JSON text
    int  m_depth;

    //! The pointer to the value object that is being read.
    wxJSONValue* m_current;

    //! The pointer to the value object that was last stored.
    wxJSONValue* m_lastStored;

    //! The pointer to the value object that will be read.
    wxJSONValue* m_next;

    //! The comment string read by SkipComment().
    wxString     m_comment;

    //! The starting line of the comment string.
    int          m_commentLine;

    //! The array of error messages.
    wxArrayString m_errors;

    //! The array of warning messages.
    wxArrayString m_warnings;

    //! The character read by the PeekChar() function (-1 none)
    int           m_peekChar;

    //! ANSI: do not convert UTF-8 strings
    bool        m_noUtf8;
};


#endif            // not defined _WX_JSONREADER_H


