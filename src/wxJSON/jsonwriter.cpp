/////////////////////////////////////////////////////////////////////////////
// Name:        jsonwriter.cpp
// Purpose:     the wxJSONWriter class: a JSON text generator
// Author:      Luciano Cattani
// Created:     2007/10/12
// RCS-ID:      $Id: jsonwriter.cpp,v 1.6 2008/03/03 19:05:47 luccat Exp $
// Copyright:   (c) 2007 Luciano Cattani
// Licence:     wxWidgets licence
/////////////////////////////////////////////////////////////////////////////

//#ifdef __GNUG__
//    #pragma implementation "jsonwriter.cpp"
//#endif

// make wxLogTrace a noop, it's really slow
// must be defined before including debug.h
#define wxDEBUG_LEVEL 0

#include <wx/jsonwriter.h>

#include <wx/sstream.h>
#include <wx/mstream.h>
#include <wx/debug.h>
#include <wx/log.h>

#if wxDEBUG_LEVEL > 0
static const wxChar* writerTraceMask = _T("traceWriter");
#endif

/*! \class wxJSONWriter
 \brief The JSON document writer

 This class is a JSON document writer and it is used to write a
 wxJSONValue object to an output stream or to a string object.
 The ctor accepts some parameters which can be used to
 change the style of the output.
 The default output is in human-readable format that uses a three-space
 indentation for object / array sub-items and separates every
 value with a linefeed character.


 \par Examples

 Using the default writer constructor

 \code
   // construct the JSON value object and add values to it
   wxJSONValue root;
   root["key1"] = "some value";
   ...

   // construct the string that will contain the JSON text
   wxString     str;

   // construct a JSON writer: use the default writer's settings
   wxJSONWriter writer;

   // call the writer's Write() memberfunction
   writer.Write( root, str );
 \endcode


 To write a JSON value object using a four-spaces indentation and forcing all
 comment strings to apear before the value they refer to, use the following code:
 \code
  wxJSONWriter writer( wxJSONWRITER_STYLED |   // want a styled output
                wxJSONWRITER_WRITE_COMMENTS |  // want comments in the document
                wxJSONWRITER_COMMENTS_BEFORE,  // force comments before value
                0,                             // initial indentation
                4);                            // indentation step
  writer.Write( value, document );
 \endcode

 The following code construct a JSON writer that produces the most compact
 text output but it is hard to read by humans:

 \code
  wxJSONWriter writer( wxJSONWRITER_NONE );
  writer.Write( value, document );
 \endcode


 \par The two types of output objects

 You can write JSON text to two different kind of objects:

 \li a string object (\b wxString)
 \li a stream object (\b wxOutputStream)

 When writing to a string object, the output is platform- and mode-dependent.
 In ANSI builds, the JSON text output in the string object will
 contain one-byte characters: the actual characters represented is
 locale dependent.
 In Unicode builds, the JSON text output in the string contains
 wide characters which encoding format is platform dependent: UCS-2 in
 Windows, UCS-4 in GNU/Linux.
 Starting from wxWidgets version 2.9 the internal encoding for Unicode
 builds in linux/unix systems is UTF-8.

 When writing to a stream object, the JSON text output is always
 encoded in UTF-8 in both ANSI and Unicode builds.
 In ANSI builds the user may want to suppress UTF-8 encoding so
 that the JSON text can be stored in ANSI format.
 Note that this is not valid JSON text unless all characters written
 to the JSON text document are in the US-ASCII character ser (0x00..0x7F).
 To know more read \ref wxjson_tutorial_unicode_ansi

 \par Efficiency

 In versions up to 1.0 the JSON writer wrote every character to the
 output object (the string or the stream).
 This is very inefficient becuase the writer converted each char to
 UTF-8 when writing to streams but we have to note that only string values
 have to be actually converted.
 Special JSON characters, numbers and literals do not need the conversion
 because they lay in the US-ASCII plane (0x00-0x7F)
 and no conversion is needed as the UTF-8 encoding is the same as US-ASCII.

 For more info about the unicode topic see \ref wxjson_tutorial_unicode.

 \par The problem of writing doubles

 You can customize the ouput of doubles by specifing  the format string
 that has to be used by the JSON writer class. To know more about this issue
 read \ref wxjson_tutorial_write_doubles
*/

//! Ctor.
/*!
 Construct the JSON writer object with the specified parameters.
 Note that if \c styled is FALSE the indentation is totally suppressed
 and the values of the other two parameters are simply ignored.

 \param indent the initial indentation in number of spaces. Default is ZERO.
    If you specify the wxJSONWRITER_TAB_INDENT flag for the \e style,
    this value referes to the number of TABs in the initial indentation

 \param step the indentation increment for new objects/arrays in number of spaces
     (default is 3).
    This value is ignored if you specify the wxJSONWRITER_TAB_INDENT flag for
    the \e style: the indentation increment is only one TAB character.

 \param style this is a combination of the following constants OR'ed togheter:
    \li wxJSONWRITER_NONE: no indentation is performed and no LF character is
        written between values.
        This style produces strict JSON text but it is hard to read by humans
    \li wxJSONWRITER_STYLED: output is human-readable: values are separated by
        LF characters and sub-items are indented.
        This style produces strict JSON text that is easy to read by humans.
    \li wxJSONWRITER_WRITE_COMMENTS: this flag force the writer to write C/C++
        comment strings, if any. The comments will be written in their original position.
        C/C++ comments may not be recognized by other JSON implementations because
        they are not strict JSON text.
    \li wxJSONWRITER_COMMENTS_BEFORE: this flag force the writer to write C/C++ comments
        always before the value they refer to.
        In order for this style to take effect, you also have to specify the
        wxJSONWRITER_WRITE_COMMENTS flag.
    \li wxJSONWRITER_COMMENTS_AFTER: this flag force the writer to write C/C++ comments
        always after the value they refer to.
        In order for this style to take effect, you also have to specify the
        wxJSONWRITER_WRITE_COMMENTS flag.
    \li wxJSONWRITER_SPLIT_STRINGS: this flag cause the writer to split strings
        in more than one line if they are too long.
    \li wxJSONWRITER_NO_LINEFEEDS: this flag cause the JSON writer to not add
        newlines between values. It is ignored if wxJSONWRITER_STYLED is not set.
        This style produces strict JSON text.
    \li wxJSONWRITER_ESCAPE_SOLIDUS: the solidus character (/) should only be
        escaped if the JSON text is meant for embedding in HTML.
        Unlike in older 0.x versions, it is disabled by default and this flag cause
        the solidus char to be escaped.
        This style produces strict JSON text.
    \li wxJSONWRITER_MULTILINE_STRING:this is a multiline-string mode where newlines
        and tabs are not escaped. This is not strict JSON, but it helps immensely when
        manually editing json files with multiline strings
    \li wxJSONWRITER_RECOGNIZE_UNSIGNED: this flag cause the JSON writer to prepend
        a plus sign (+) to unsigned integer values. This is used by the wxJSON reader to
        force the integer to be stored in an \b unsigned \b int. Note that this
        feature may be incompatible with other JSON implementations.
    \li wxJSONWRITER_TAB_INDENT: this flag cause the indentation of sub-objects / arrays
        to be done using a TAB character instead of SPACES.
        In order for this style to take effect, you also have to specify the
        wxJSONWRITER_STYLED flag.
        This style produces strict JSON text.
    \li wxJSONWRITER_NO_INDENTATION: this flag cause the JSON writer to not add
        indentation. It is ignored if wxJSONWRITER_STYLED is not set.
        This style produces strict JSON text.
    \li wxJSONWRITER_NOUTF8_STREAM: suppress UTF-8 conversion when writing string
        values to the stream thus producing ANSI text output; only meaningfull in
        ANSI builds, this flag is simply ignored in Unicode builds.
    \li wxJSONWRITER_MEMORYBUFF: 


 Note that for the style wxJSONWRITER_NONE the JSON text output is a bit
 different from that of old 0.x versions although it is syntactically equal.
 If you rely on the old JSON output formatting read the following page
 \ref wxjson_tutorial_style_none.
 To know more about the writer's styles see \ref wxjson_tutorial_style
*/
wxJSONWriter::wxJSONWriter( int style, int indent, int step )
{
    m_indent = indent;
    m_step   = step;
    m_style  = style;
    m_noUtf8 = false;
    if ( m_style == wxJSONWRITER_NONE )  {
        m_indent = 0;
        m_step   = 0;
    }
    // set the default format string for doubles as
    // 10 significant digits and suppress trailing ZEROes
    SetDoubleFmtString( "%.10g") ;

#if !defined( wxJSON_USE_UNICODE )
    // in ANSI builds we can suppress UTF-8 conversion for both the writer and the reader
    if ( m_style == wxJSONWRITER_NOUTF8_STREAM )    {
        m_noUtf8 = true;
    }
#endif
}

//! Dtor - does nothing
wxJSONWriter::~wxJSONWriter()
{
}

//! Write the JSONvalue object to a JSON text.
/*!
 The two overloaded versions of this function let the user choose
 the output object which can be:

 \li a string object (\b wxString)
 \li a stream object ( \b wxOutputStream)

 The two types of output object are very different because the
 text outputted is encoded in different formats depending on the
 build mode.
 When writing to a string object, the JSON text output is encoded
 differently depending on the build mode and the platform.
 Writing to a stream always produce UTF-8 encoded text.
 To know more about this topic read \ref wxjson_tutorial_unicode.

 Also note that the Write() function does not return a status code.
 If you are writing to a string, you do not have to warry about this
 issue: no errors can occur when writing to strings.
 On the other hand, wehn writing to a stream there could be errors
 in the write operation.
 If an error occurs, the \c Write(9 function immediatly returns
 without trying further output operations.
 You have to check the status of the stream by calling the stream's
 memberfunctions. Example:

 \code
   // construct the JSON value object and add values to it
   wxJSONValue root;
   root["key1"] = "some value";

   // write to a stream
   wxMemoryOutputStream mem;
   wxJSONWriter writer;
   writer.Write( root, mem );
   wxStreamError err = mem.GetLastError();
   if ( err != wxSTREAM_NO_ERROR )  {
     MessageBox( _T("ERROR: cannot write the JSON text output"));
   }
\endcode
*/
void
wxJSONWriter::Write( const wxJSONValue& value, wxString& str )
{
#if !defined( wxJSON_USE_UNICODE )
    // in ANSI builds output to a string never use UTF-8 conversion
    bool noUtf8_bak = m_noUtf8;        // save the current setting
    m_noUtf8 = true;
#endif

    wxMemoryOutputStream os;
    Write( value, os );

    // get the address of the buffer
    wxFileOffset len = os.GetLength();
    wxStreamBuffer* osBuff = os.GetOutputStreamBuffer();
    void* buffStart = osBuff->GetBufferStart();

    if ( m_noUtf8 )    {
        str = wxString::From8BitData( (const char*) buffStart, len );
    }
    else    {
        str = wxString::FromUTF8( (const char*) buffStart, len );
    }
#if !defined( wxJSON_USE_UNICODE )
    m_noUtf8 = noUtf8_bak;        // restore the old setting
#endif
}

//! \overload Write( const wxJSONValue&, wxString& )
void
wxJSONWriter::Write( const wxJSONValue& value, wxOutputStream& os )
{
    m_level = 0;
    DoWrite( os, value, 0, false );
}

//! Set the format string for double values.
/*!
 This function sets the format string used for printing double values.
 Double values are outputted to JSON text using the \b snprintf function
 with a default format string of:
 \code
     %.10g
 \endcode
 which prints doubles with a precision of 10 decimal digits and suppressing
 trailing ZEROes.

 Note that the parameter is a pointer to \b char and not to \b wxChar. This
 is because the JSON writer always procudes UTF-8 encoded text and decimal
 digits in UTF-8 are made of only one UTF-8 code-unit (1 byte).
*/
void
wxJSONWriter::SetDoubleFmtString( const char* fmt )
{
    m_fmt = (char*) fmt;
}



//! Perform the real write operation.
/*!
 This is a recursive function that gets the type of the \c value object and
 calls several protected functions depending on the type:

 \li \c WriteNullvalue for type NULL
 \li \c WriteStringValue() for STRING and CSTRING types
 \li \c WriteIntValue for INT types
 \li \c WriteUIntValue for UINT types
 \li \c WriteBoolValue for BOOL types
 \li \c WriteDoubleValue for DOUBLE types
 \li \c WriteMemoryBuff for MEMORYBUFF types

 If the value is an array or key/value map (types ARRAY and OBJECT), the function
 iterates through all JSON value object in the array/map and calls itself for every
 item in the container.
*/
int
wxJSONWriter::DoWrite( wxOutputStream& os, const wxJSONValue& value, const wxString* key, bool comma )
{
    // note that this function is recursive

    // some variables that cannot be allocated in the switch statement
    const wxJSONInternalMap* map = 0;
    int size;
    m_colNo = 1; m_lineNo = 1;
    // determine the comment position; it is one of:
    //
    //  wxJSONVALUE_COMMENT_BEFORE
    //  wxJSONVALUE_COMMENT_AFTER
    //  wxJSONVALUE_COMMENT_INLINE
    //
    // or -1 if comments have not to be written
    int commentPos = -1;
    if ( value.GetCommentCount() > 0 && (m_style & wxJSONWRITER_WRITE_COMMENTS))  {
        commentPos = value.GetCommentPos();
        if ( ( m_style & wxJSONWRITER_COMMENTS_BEFORE) != 0 ) {
            commentPos = wxJSONVALUE_COMMENT_BEFORE;
        }
        else if ( (m_style & wxJSONWRITER_COMMENTS_AFTER) != 0 ) {
            commentPos = wxJSONVALUE_COMMENT_AFTER;
        }
    }

    int lastChar = 0;  // check if WriteComment() writes the last LF char

    // first write the comment if it is BEFORE
    if ( commentPos == wxJSONVALUE_COMMENT_BEFORE )   {
        lastChar = WriteComment( os, value, true );
        if ( lastChar < 0 )   {
            return lastChar;
        }
        else if ( lastChar != '\n' )  {
            WriteSeparator( os );
        }
    }

    lastChar = WriteIndent( os );
    if ( lastChar < 0 )   {
        return lastChar;
    }

    // now write the key if it is not NULL
    if ( key )   {
        lastChar = WriteKey( os, *key );
    }
    if ( lastChar < 0 )   {
        return lastChar;
    }

    // now write the value
    wxJSONInternalMap::const_iterator it;    // declare the map object
    long int count = 0;

    wxJSONType t = value.GetType();
    switch ( t )  {
    case wxJSONTYPE_INVALID :
        WriteInvalid( os );
        wxFAIL_MSG( _T("wxJSONWriter::WriteEmpty() cannot be called (not a valid JSON text"));
        break;

    case wxJSONTYPE_INT :
    case wxJSONTYPE_SHORT :
    case wxJSONTYPE_LONG :
    case wxJSONTYPE_INT64 :
        lastChar = WriteIntValue( os, value );
        break;

    case wxJSONTYPE_UINT :
    case wxJSONTYPE_USHORT :
    case wxJSONTYPE_ULONG :
    case wxJSONTYPE_UINT64 :
        lastChar = WriteUIntValue( os, value );
        break;

    case wxJSONTYPE_NULL :
        lastChar = WriteNullValue( os );
        break;
    case wxJSONTYPE_BOOL :
        lastChar = WriteBoolValue( os, value );
        break;

    case wxJSONTYPE_DOUBLE :
        lastChar = WriteDoubleValue( os, value );
        break;

    case wxJSONTYPE_STRING :
    case wxJSONTYPE_CSTRING :
        lastChar = WriteStringValue( os, value.AsString());
        break;

    case wxJSONTYPE_MEMORYBUFF :
        lastChar = WriteMemoryBuff( os, value.AsMemoryBuff());
        break;

    case wxJSONTYPE_ARRAY :
        ++m_level;
        os.PutC( '[' );
        // the inline comment for objects and arrays are printed in the open char
        if ( commentPos == wxJSONVALUE_COMMENT_INLINE )   {
            commentPos = -1;  // we have already written the comment
            lastChar = WriteComment( os, value, false );
            if ( lastChar < 0 )   {
                return lastChar;
            }
            if ( lastChar != '\n' )   {
                lastChar = WriteSeparator( os );
            }
        }
        else   {    // comment is not to be printed inline, so write a LF
            lastChar = WriteSeparator( os );
            if ( lastChar < 0 )   {
                return lastChar;
            }
        }

        // now iterate through all sub-items and call DoWrite() recursively
        size = value.Size();
        for ( int i = 0; i < size; i++ )  {
            bool comma = false;
            if ( i < size - 1 )  {
                comma = true;
            }
            wxJSONValue v = value.ItemAt( i );
            lastChar = DoWrite( os, v, 0, comma );
            if ( lastChar < 0 )  {
                return lastChar;
            }

        }
        --m_level;
        lastChar = WriteIndent( os );
        if ( lastChar < 0 )   {
            return lastChar;
        }
        os.PutC( ']' );
        break;

    case wxJSONTYPE_OBJECT :
        ++m_level;

        os.PutC( '{' );
        // the inline comment for objects and arrays are printed in the open char
        if ( commentPos == wxJSONVALUE_COMMENT_INLINE )   {
            commentPos = -1;  // we have already written the comment
            lastChar = WriteComment( os, value, false );
            if ( lastChar < 0 )   {
                return lastChar;
            }
            if ( lastChar != '\n' )   {
                WriteSeparator( os );
            }
        }
        else   {
            lastChar = WriteSeparator( os );
        }

        map = value.AsMap();
        size = value.Size();
        count = 0;
        for ( it = map->begin(); it != map->end(); ++it )  {
            // get the key and the value
            wxString key = it->first;
            const wxJSONValue& v = it->second;
            bool comma = false;
            if ( count < size - 1 )  {
                comma = true;
            }
            lastChar = DoWrite( os, v, &key, comma );
            if ( lastChar < 0 )  {
                return lastChar;
            }
            count++;
        }
        --m_level;
        lastChar = WriteIndent( os );
        if ( lastChar < 0 )   {
            return lastChar;
        }
        os.PutC( '}' );
        break;

    default :
        // a not yet defined wxJSONType: we FAIL
        wxFAIL_MSG( _T("wxJSONWriter::DoWrite() undefined wxJSONType type"));
        break;
    }

    // writes the comma character before the inline comment
    if ( comma )  {
        os.PutC( ',' );
    }

    if ( commentPos == wxJSONVALUE_COMMENT_INLINE )   {
        lastChar = WriteComment( os, value, false );
        if ( lastChar < 0 )   {
            return lastChar;
        }
    }
    else if ( commentPos == wxJSONVALUE_COMMENT_AFTER )   {
        WriteSeparator( os );
        lastChar = WriteComment( os, value, true );
        if ( lastChar < 0 )   {
            return lastChar;
        }
    }
    if ( lastChar != '\n' )  {
        lastChar = WriteSeparator( os );
    }
    return lastChar;
}


//! Write the comment strings, if any.
int
wxJSONWriter::WriteComment( wxOutputStream& os, const wxJSONValue& value, bool indent )
{
    // the function returns the last character written which should be
    // a LF char or -1 in case of errors
    // if nothing is written, returns ZERO
    int lastChar = 0;

    // only write comments if the style include the WRITE_COMMENTS flag
    if ( (m_style & wxJSONWRITER_WRITE_COMMENTS ) == 0 ) {
        return lastChar;
    }

    const wxArrayString cmt = value.GetCommentArray();
    int cmtSize = cmt.GetCount();
    for ( int i = 0; i < cmtSize; i++ )    {
        if ( indent )    {
            WriteIndent( os );
        }
        else    {
            os.PutC( '\t' );
        }
        WriteString( os, cmt[i]);
        lastChar = cmt[i].Last();
        if ( lastChar != '\n' )   {
            os.PutC( '\n' );
            lastChar = '\n';
        }
    }
    return lastChar;
}

//! Writes the indentation to the JSON text.
/*!
 The two functions write the indentation as \e spaces in the JSON output
 text. When called with a int parameter, the function
 writes the specified number of spaces.
 If no parameter is given, the function computes the number of spaces
 using the following formula:
 If the wxJSONWRITER_TAB_INDENT flag is used in the writer's cnstructor,
 the function calls WriteTabIndent().

 The function also checks that wxJSONWRITER_STYLED is set and the
 wxJSONWRITER_NO_INDENTATION is not set.
*/
int
wxJSONWriter::WriteIndent( wxOutputStream& os )
{
    int lastChar = WriteIndent( os, m_level );
    return lastChar;
}

//! Write the specified number of indentation (spaces or tabs)
/*!
 The function is called by WriteIndent() and other writer's functions.
 It writes the indentation as specified in the \c num parameter which is
 the actual \b level of annidation.
 The function checks if wxJSONWRITER_STYLED is set: if not, no indentation
 is performed.
 Also, the function checks if wxJSONWRITER_TAB_INDENT is set: if it is,
 indentation is done by writing \b num TAB characters otherwise,
 it is performed by writing a number of spaces computed as:
 \code
  numSpaces = m_indent + ( m_step * num )
 \endcode

*/
int
wxJSONWriter::WriteIndent( wxOutputStream& os, int num )
{
    int lastChar = 0;
    if ( !(m_style & wxJSONWRITER_STYLED) || (m_style & wxJSONWRITER_NO_INDENTATION))  {
        return lastChar;
    }

    int numChars = m_indent + ( m_step * num );
    char c = ' ';
    if ( m_style & wxJSONWRITER_TAB_INDENT ) {
        c = '\t';
        numChars = num;
    }

    for ( int i = 0; i < numChars; i++ )  {
        os.PutC( c );
        if ( os.GetLastError() != wxSTREAM_NO_ERROR )    {
            return -1;
        }

    }
    return c;
}


//! Write the provided string to the output object.
/*!
 The function writes the string \c str to the output object that
 was specified in the wxJSONWriter::Write() function.
 The function may split strings in two or more lines if the
 string contains LF characters if the \c m_style data member contains
 the wxJSONWRITER_SPLIT_STRING flag.

 The function does not actually write the string: for every character
 in the provided string the function calls WriteChar() which does
 the actual character output.

 The function returns ZERO on success or -1 in case of errors.
*/
int
wxJSONWriter::WriteStringValue( wxOutputStream& os, const wxString& str )
{
    // JSON values of type STRING are written by converting the whole string
    // to UTF-8 and then copying the UTF-8 buffer to the 'os' stream
    // one byte at a time and processing them
    os.PutC( '\"' );        // open quotes

    // the buffer that has to be written is either UTF-8 or ANSI c_str() depending
    // on the 'm_noUtf8' flag
    char* writeBuff = 0;
    wxCharBuffer utf8CB = str.ToUTF8();        // the UTF-8 buffer
#if !defined( wxJSON_USE_UNICODE )
    wxCharBuffer ansiCB( str.c_str());        // the ANSI buffer
    if ( m_noUtf8 )    {
        writeBuff = ansiCB.data();
    }
    else    {
        writeBuff = utf8CB.data();
    }
#else
        writeBuff = utf8CB.data();
#endif

    // NOTE: in ANSI builds UTF-8 conversion may fail (see samples/test5.cpp,
    // test 7.3) although I do not know why
    if ( writeBuff == 0 )    {
        const char* err = "<wxJSONWriter::WriteStringValue(): error converting the string to a UTF8 buffer>";
        os.Write( err, strlen( err ));
        return 0;
    }
    size_t len = strlen( writeBuff );
    int lastChar = 0;

    // store the column at which the string starts
    // splitting strings only happen if the string starts within
    // column wxJSONWRITER_LAST_COL (default 50)
    // see 'include/wx/json_defs.h' for the defines
    int tempCol = m_colNo;

    // now write the UTF8 buffer processing the bytes
    size_t i;
    for ( i = 0; i < len; i++ ) {
        bool shouldEscape = false;
        unsigned char ch = *writeBuff;
        ++writeBuff;        // point to the next byte

        // the escaped character
        char escCh = 0;

        // for every character we have to check if it is a character that
        // needs to be escaped: note that characters that should be escaped
        // may be not if some writer's flags are specified
        switch ( ch )  {
        case '\"' :     // quotes
            shouldEscape = true;
            escCh = '\"';
            break;
        case '\\' :     // reverse solidus
            shouldEscape = true;
            escCh = '\\';
            break;
        case '/'  :     // solidus
            shouldEscape = true;
            escCh = '/';
            break;
        case '\b' :     // backspace
            shouldEscape = true;
            escCh = 'b';
            break;
        case '\f' :     // formfeed
            shouldEscape = true;
            escCh = 'f';
            break;
        case '\n' :     // newline
            shouldEscape = true;
            escCh = 'n';
            break;
        case '\r' :     // carriage-return
            shouldEscape = true;
            escCh = 'r';
            break;
        case '\t' :      // horizontal tab
            shouldEscape = true;
            escCh = 't';
            break;
        default :
            shouldEscape = false;
            break;
        }        // end switch


        // if the character is a control character that is not identified by a
        // lowercase letter, we should escape it
        if ( !shouldEscape && ch < 32 )  {
            char b[8];
            snprintf( b, 8, "\\u%04X", (int) ch );
            os.Write( b, 6 );
            if ( os.GetLastError() != wxSTREAM_NO_ERROR )    {
                return -1;
            }
        }

        // the char is not a control character
        else {
            // some characters that should be escaped are not escaped
            // if the writer was constructed with some flags
            if ( shouldEscape && !( m_style & wxJSONWRITER_ESCAPE_SOLIDUS) )  {
                if ( ch == '/' )  {
                    shouldEscape = false;
                }
            }
            if ( shouldEscape && (m_style & wxJSONWRITER_MULTILINE_STRING))  {
                if ( ch == '\n' || ch == '\t' )  {
                    shouldEscape = false;
                }
            }


            // now write the character prepended by ESC if it should be escaped
            if ( shouldEscape )  {
                os.PutC( '\\' );
                os.PutC( escCh );
                if ( os.GetLastError() != wxSTREAM_NO_ERROR )    {
                    return -1;
                }
            }
            else {
                //  a normal char or a UTF-8 units: write the character
                os.PutC( ch );
                if ( os.GetLastError() != wxSTREAM_NO_ERROR )    {
                    return -1;
                }
            }
        }

        // check if SPLIT_STRING flag is set and if the string has to
        // be splitted
        if ( (m_style & wxJSONWRITER_STYLED) && (m_style & wxJSONWRITER_SPLIT_STRING))   {
            // split the string if the character written is LF
            if ( ch == '\n' ) {
                // close quotes and CR
                os.Write( "\"\n", 2 );
                lastChar = WriteIndent( os, m_level + 2 );     // write indentation
                os.PutC( '\"' );               // reopen quotes
                if ( lastChar < 0 )  {
                    return lastChar;
                }
            }
            // split the string only if there is at least wxJSONWRITER_MIN_LENGTH
            // character to write and the character written is a punctuation or space
            // BUG: the following does not work because the columns are not counted
            else if ( (m_colNo >= wxJSONWRITER_SPLIT_COL)
                     && (tempCol <= wxJSONWRITER_LAST_COL )) {
                if ( IsSpace( ch ) || IsPunctuation( ch ))  {
                    if ( len - i > wxJSONWRITER_MIN_LENGTH )  {
                        // close quotes and CR
                        os.Write( "\"\n", 2 );
                        lastChar = WriteIndent( os, m_level + 2 );     // write indentation
                        os.PutC( '\"' );           // reopen quotes
                        if ( lastChar < 0 )  {
                            return lastChar;
                        }
                    }
                }
            }
        }
    }            // end for
    os.PutC( '\"' );    // close quotes
    return 0;
}



//! Write a generic string
/*!
 The function writes the wxString object \c str to the output object.
 The string is written as is; you cannot use it to write JSON strings
 to the output text.
 The function converts the string \c str to UTF-8 and writes the buffer..
*/
int
wxJSONWriter::WriteString( wxOutputStream& os, const wxString& str )
{
    wxLogTrace( writerTraceMask, _T("(%s) string to write=%s"),
                  __PRETTY_FUNCTION__, str.c_str() );
    int lastChar = 0;
    char* writeBuff = 0;

    // the buffer that has to be written is either UTF-8 or ANSI c_str() depending
    // on the 'm_noUtf8' flag
    wxCharBuffer utf8CB = str.ToUTF8();        // the UTF-8 buffer
#if !defined( wxJSON_USE_UNICODE )
    wxCharBuffer ansiCB( str.c_str());        // the ANSI buffer

    if ( m_noUtf8 )    {
        writeBuff = ansiCB.data();
    }
    else    {
        writeBuff = utf8CB.data();
    }
#else
    writeBuff = utf8CB.data();
#endif

    // NOTE: in ANSI builds UTF-8 conversion may fail (see samples/test5.cpp,
    // test 7.3) although I do not know why
    if ( writeBuff == 0 )    {
        const char* err = "<wxJSONWriter::WriteComment(): error converting the string to UTF-8>";
        os.Write( err, strlen( err ));
        return 0;
    }
    size_t len = strlen( writeBuff );

    os.Write( writeBuff, len );
    if ( os.GetLastError() != wxSTREAM_NO_ERROR )    {
        return -1;
    }

    wxLogTrace( writerTraceMask, _T("(%s) result=%d"),
                  __PRETTY_FUNCTION__, lastChar );
    return lastChar;
}

//! Write the NULL JSON value to the output stream.
/*!
 The function writes the \b null literal string to the output stream.
*/
int
wxJSONWriter::WriteNullValue( wxOutputStream& os )
{
    os.Write( "null", 4 );
    if ( os.GetLastError() != wxSTREAM_NO_ERROR )    {
        return -1;
    }
    return 0;
}


//! Writes a value of type INT.
/*!
 This function is called for every value objects of INT type.
 This function uses the \n snprintf function to get the US-ASCII
 representation of the integer and simply copy it to the output stream.
 Returns -1 on stream errors or ZERO if no errors.
*/
int
wxJSONWriter::WriteIntValue( wxOutputStream& os, const wxJSONValue& value )
{
    int r = 0;
    char buffer[32];        // need to store 64-bits integers (max 20 digits)
    size_t len;

    wxJSONRefData* data = value.GetRefData();
    wxASSERT( data );

#if defined( wxJSON_64BIT_INT )
    #if wxCHECK_VERSION(2, 9, 0 ) || !defined( wxJSON_USE_UNICODE )
        // this is fine for wxW 2.9 and for wxW 2.8 ANSI
        snprintf( buffer, 32, "%" wxLongLongFmtSpec "d",
        data->m_value.m_valInt64 );
    #else
        // this is for wxW 2.8 Unicode: in order to use the cross-platform
        // format specifier, we use the wxString's sprintf() function and then
        // convert to UTF-8 before writing to the stream
        wxString s;
        s.Printf( _T("%") wxLongLongFmtSpec _T("d"),
                                                data->m_value.m_valInt64 );
        wxCharBuffer cb = s.ToUTF8();
        const char* cbData = cb.data();
        len = strlen( cbData );
        wxASSERT( len < 32 );
        memcpy( buffer, cbData, len );
        buffer[len] = 0;
    #endif
#else
    snprintf( buffer, 32, "%ld", data->m_value.m_valLong );
#endif
    len = strlen( buffer );
    os.Write( buffer, len );
    if ( os.GetLastError() != wxSTREAM_NO_ERROR )    {
        r = -1;
    }
    return r;
}

//! Writes a value of type UNSIGNED INT.
/*!
 This function is called for every value objects of UINT type.
 This function uses the \n snprintf function to get the US-ASCII
 representation of the integer and simply copy it to the output stream.
 The function prepends a \b plus \b sign if the \c wxJSONWRITER_RECOGNIZE_UNSIGNED
 flag is set in the \c m_flags data member.
 Returns -1 on stream errors or ZERO if no errors.
*/
int
wxJSONWriter::WriteUIntValue( wxOutputStream& os, const wxJSONValue& value )
{
    int r = 0; size_t len;

    // prepend a plus sign if the style specifies that unsigned integers
    // have to be recognized by the JSON reader
    if ( m_style & wxJSONWRITER_RECOGNIZE_UNSIGNED )  {
        os.PutC( '+' );
    }

    char buffer[32];        // need to store 64-bits integers (max 20 digits)
    wxJSONRefData* data = value.GetRefData();
    wxASSERT( data );

#if defined( wxJSON_64BIT_INT )
    #if wxCHECK_VERSION(2, 9, 0 ) || !defined( wxJSON_USE_UNICODE )
        // this is fine for wxW 2.9 and for wxW 2.8 ANSI
        snprintf( buffer, 32, "%" wxLongLongFmtSpec "u",
        data->m_value.m_valUInt64 );
    #else
        // this is for wxW 2.8 Unicode: in order to use the cross-platform
        // format specifier, we use the wxString's sprintf() function and then
        // convert to UTF-8 before writing to the stream
        wxString s;
        s.Printf( _T("%") wxLongLongFmtSpec _T("u"),
                                                data->m_value.m_valInt64 );
        wxCharBuffer cb = s.ToUTF8();
        const char* cbData = cb.data();
        len = strlen( cbData );
        wxASSERT( len < 32 );
        memcpy( buffer, cbData, len );
        buffer[len] = 0;
    #endif
#else
    snprintf( buffer, 32, "%lu", data->m_value.m_valULong );
#endif
    len = strlen( buffer );
    os.Write( buffer, len );
    if ( os.GetLastError() != wxSTREAM_NO_ERROR )    {
        r = -1;
    }
    return r;
}

//! Writes a value of type DOUBLE.
/*!
 This function is called for every value objects of DOUBLE type.
 This function uses the \n snprintf function to get the US-ASCII
 representation of the integer and simply copy it to the output stream.
 Returns -1 on stream errors or ZERO if no errors.

 Note that writing a double to a decimal ASCII representation could
 lay to unexpected results depending on the format string used in the
 conversion.
 See SetDoubleFmtString for details.
*/
int
wxJSONWriter::WriteDoubleValue( wxOutputStream& os, const wxJSONValue& value )
{
    int r = 0;

    char buffer[32];
    wxJSONRefData* data = value.GetRefData();
    wxASSERT( data );
    snprintf( buffer, 32, m_fmt, data->m_value.m_valDouble );
    size_t len = strlen( buffer );
    os.Write( buffer, len );
    if ( os.GetLastError() != wxSTREAM_NO_ERROR )    {
        r = -1;
    }
    return r;
}

//! Writes a value of type BOOL.
/*!
 This function is called for every value objects of BOOL type.
 This function prints the literals \b true or \b false depending on the
 value in \c value.
 Returns -1 on stream errors or ZERO if no errors.
*/
int
wxJSONWriter::WriteBoolValue( wxOutputStream& os, const wxJSONValue& value )
{
    int r = 0;
    const char* f = "false"; const char* t = "true";
    wxJSONRefData* data = value.GetRefData();
    wxASSERT( data );

    const char* c = f;    // defaults to FALSE

    if ( data->m_value.m_valBool )    {
        c = t;
    }

    size_t len = strlen( c );
    os.Write( c, len );
    if ( os.GetLastError() != wxSTREAM_NO_ERROR )    {
        r = -1;
    }
    return r;
}




//! Write the key of a key/value element to the output stream.
int
wxJSONWriter::WriteKey( wxOutputStream& os, const wxString& key )
{
    wxLogTrace( writerTraceMask, _T("(%s) key write=%s"),
                  __PRETTY_FUNCTION__, key.c_str() );

    int lastChar = WriteStringValue( os, key );
    os.Write( " : ", 3 );
    return lastChar;
}

//! Write the invalid JSON value to the output stream.
/*!
 An invalid wxJSONValue is a value that was not initialized and it is
 an error. You should never write invalid values to JSON text because
 the output is not valid JSON text.
 Note that the NULL value is a legal JSON text and it is written:
 \code
  null
 \endcode

 This function writes a non-JSON text to the output stream:
 \code
  <invalid JSON value>
 \endcode
 In debug mode, the function always fails with an wxFAIL_MSG failure.
*/
int
wxJSONWriter::WriteInvalid( wxOutputStream& os )
{
    wxFAIL_MSG( _T("wxJSONWriter::WriteInvalid() cannot be called (not a valid JSON text"));
    int lastChar = 0;
    os.Write( "<invalid JSON value>", 9 );
    return lastChar;
}

//! Write a JSON value of type \e memory \e buffer
/*!
 The type wxJSONTYPE_MEMORYBUFF is a \b wxJSON extension that is not correctly read by
 other JSON implementations.
 By default, the function writes such a type as an array of INTs as follows:
 \code
   [ 0,32,45,255,6,...]
 \endcode
 If the writer object was constructed using the \c wxJSONWRITER_MEMORYBUFF flag, then
 the output is much more compact and recognized by the \b wxJSON reader as a memory buffer
 type:
 \code
   '00203FFF06..'
 \endcode

*/
int
wxJSONWriter::WriteMemoryBuff( wxOutputStream& os, const wxMemoryBuffer& buff )
{
#define MAX_BYTES_PER_ROW	20
    char str[16];

    // if STYLED and SPLIT_STRING flags are set, the function writes 20 bytes on every row
    // the following is the counter of bytes written.
    // the string is splitted only for the special meory buffer type, not for array of INTs
    int bytesWritten = 0;
    bool splitString = false;
    if ( (m_style & wxJSONWRITER_STYLED) && 
               (m_style & wxJSONWRITER_SPLIT_STRING))   {
        splitString = true;
    }

    size_t buffLen = buff.GetDataLen();
    unsigned char* ptr = (unsigned char*) buff.GetData();
    wxASSERT( ptr );
    char openChar = '\'';
    char closeChar = '\'';
    bool asArray = false;

    if ( (m_style & wxJSONWRITER_MEMORYBUFF ) == 0 )  {
        // if the special flag is not specified, write as an array of INTs
        openChar = '[';
        closeChar = ']';
        asArray = true;
    }
    // write the open character
    os.PutC( openChar );

    for ( size_t i = 0; i < buffLen; i++ )  {
        unsigned char c = *ptr;
        ++ptr;

        if ( asArray )  {
            snprintf( str, 14, "%d", c );
            size_t len = strlen( str );
            wxASSERT( len <= 3 );
            wxASSERT( len >= 1 );
            str[len] = ',';
            // do not write the comma char for the last element
            if ( i < buffLen - 1 )    {
                ++len;
            }
            os.Write( str, len );
            if ( os.GetLastError() != wxSTREAM_NO_ERROR )    {
                return -1;
            }
        }
        else {
            // now convert the byte in two hex digits
            char c1 = c / 16;
            char c2 = c % 16;
            c1 += '0';
            c2 += '0';
            if ( c1 > '9' )  {
                c1 += 7;
            }
            if ( c2 > '9' )  {
                c2 += 7;
            }
            os.PutC( c1 );
            os.PutC( c2 );
            if ( os.GetLastError() != wxSTREAM_NO_ERROR )    {
                return -1;
            }
            if ( splitString )  {
                ++bytesWritten;
            }

            if (( bytesWritten >= MAX_BYTES_PER_ROW ) && ((buffLen - i ) >= 5 ))   {
                // split the string if we wrote 20 bytes, but only is we have to
                // write at least 5 bytes
                os.Write( "\'\n", 2 );
                int lastChar = WriteIndent( os, m_level + 2 );     // write indentation
                os.PutC( '\'' );               // reopen quotes
                if ( lastChar < 0 )  {
                    return lastChar;
                }
                bytesWritten = 0;
            }
        }
    }

    // write the close character
    os.PutC( closeChar );
    return closeChar;
}


//! Writes the separator between values
/*!
 The function is called when a value has been written to the JSON
 text output and it writes the separator character: LF.
 The LF char is actually written only if the wxJSONWRITER_STYLED flag
 is specified and wxJSONWRITER_NO_LINEFEEDS is not set.

 Returns the last character written which is LF itself or -1 in case
 of errors. Note that LF is returned even if the character is not
 actually written.
*/
int
wxJSONWriter::WriteSeparator( wxOutputStream& os )
{
    int lastChar = '\n';
    if ( (m_style & wxJSONWRITER_STYLED) && !(m_style & wxJSONWRITER_NO_LINEFEEDS ))  {
        os.PutC( '\n' );
    }
    return lastChar;
}

//! Returns TRUE if the character is a space character.
bool
wxJSONWriter::IsSpace( wxChar ch )
{
    bool r = false;
    switch ( ch ) {
        case ' ' :
        case '\t' :
        case '\r' :
        case '\f' :
        case '\n' :
            r = true;
            break;
        default :
            break;
    }
    return r;
}

//! Returns TRUE if the character if a puctuation character
bool
wxJSONWriter::IsPunctuation( wxChar ch )
{
    bool r = false;
    switch ( ch ) {
        case '.' :
        case ',' :
        case ';' :
        case ':' :
        case '!' :
        case '?' :
            r = true;
            break;
        default :
            break;
    }
    return r;
}


/*
{
}
*/


