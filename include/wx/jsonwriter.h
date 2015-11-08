/////////////////////////////////////////////////////////////////////////////
// Name:        jsonwriter.h
// Purpose:     the generator of JSON text from a JSON value
// Author:      Luciano Cattani
// Created:     2007/09/15
// RCS-ID:      $Id: jsonwriter.h,v 1.4 2008/03/03 19:05:45 luccat Exp $
// Copyright:   (c) 2007 Luciano Cattani
// Licence:     wxWidgets licence
/////////////////////////////////////////////////////////////////////////////

#if !defined( _WX_JSONWRITER_H )
#define _WX_JSONWRITER_H

//#ifdef __GNUG__
//    #pragma interface "jsonwriter.h"
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
#endif

#include "json_defs.h"
#include "jsonval.h"

enum {
    wxJSONWRITER_NONE            = 0,
    wxJSONWRITER_STYLED          = 1,
    wxJSONWRITER_WRITE_COMMENTS  = 2,
    wxJSONWRITER_COMMENTS_BEFORE = 4,
    wxJSONWRITER_COMMENTS_AFTER  = 8,
    wxJSONWRITER_SPLIT_STRING    = 16,
    wxJSONWRITER_NO_LINEFEEDS    = 32,
    wxJSONWRITER_ESCAPE_SOLIDUS     = 64,
    wxJSONWRITER_MULTILINE_STRING   = 128,
    wxJSONWRITER_RECOGNIZE_UNSIGNED = 256,
    wxJSONWRITER_TAB_INDENT         = 512,
    wxJSONWRITER_NO_INDENTATION     = 1024,
    wxJSONWRITER_NOUTF8_STREAM      = 2048,
    wxJSONWRITER_MEMORYBUFF         = 4096
};

// class declaration

class WXDLLIMPEXP_JSON wxJSONWriter
{
public:
    wxJSONWriter( int style = wxJSONWRITER_STYLED, int indent = 0, int step = 3 );
    ~wxJSONWriter();

    void Write( const wxJSONValue& value, wxString& str );
    void Write( const wxJSONValue& value, wxOutputStream& os );
    void SetDoubleFmtString( const char* fmt );

protected:

    int  DoWrite( wxOutputStream& os, const wxJSONValue& value, const wxString* key, bool comma );
    int  WriteIndent( wxOutputStream& os );
    int  WriteIndent( wxOutputStream& os, int num );
    bool IsSpace( wxChar ch );
    bool IsPunctuation( wxChar ch );

    int  WriteString( wxOutputStream& os, const wxString& str );
    int  WriteStringValue( wxOutputStream& os, const wxString& str );
    int  WriteNullValue( wxOutputStream& os );
    int  WriteIntValue( wxOutputStream& os, const wxJSONValue& v );
    int  WriteUIntValue( wxOutputStream& os, const wxJSONValue& v );
    int  WriteBoolValue( wxOutputStream& os, const wxJSONValue& v );
    int  WriteDoubleValue( wxOutputStream& os, const wxJSONValue& v );
    int  WriteMemoryBuff( wxOutputStream& os, const wxMemoryBuffer& buff );

    int  WriteInvalid( wxOutputStream& os );
    int  WriteSeparator( wxOutputStream& os );

    int  WriteKey( wxOutputStream& os, const wxString& key );
    int  WriteComment( wxOutputStream& os, const wxJSONValue& value, bool indent );

    int  WriteError( const wxString& err );

private:
    //! The style flag is a combination of wxJSONWRITER_(something) constants.
    int   m_style;

    //! The initial indentation value, in number of spaces.
    int   m_indent;

    //! The indentation increment, in number of spaces.
    int   m_step;

    //! JSON value objects can be nested; this is the level of annidation (used internally).
    int   m_level;

    // The line number when printing JSON text output (not yet used)
    int   m_lineNo;

    // The column number when printing JSON text output
    int   m_colNo;

    // Flag used in ANSI mode that controls UTF-8 conversion
    bool  m_noUtf8;

    // The format string for printing doubles
    char* m_fmt;
};


#endif            // not defined _WX_JSONWRITER_H



