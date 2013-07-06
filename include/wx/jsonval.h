/////////////////////////////////////////////////////////////////////////////
// Name:        jsonval.h
// Purpose:     the wxJSONValue class: it holds a JSON value
// Author:      Luciano Cattani
// Created:     2007/09/15
// RCS-ID:      $Id: jsonval.h,v 1.4 2008/01/10 21:27:15 luccat Exp $
// Copyright:   (c) 2007 Luciano Cattani
// Licence:     wxWidgets licence
/////////////////////////////////////////////////////////////////////////////

#if !defined( _WX_JSONVAL_H )
#define _WX_JSONVAL_H

//#ifdef __GNUG__
//    #pragma interface "jsonval.h"
//#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include <wx/object.h>
    #include <wx/hashmap.h>
    #include <wx/dynarray.h>
    #include <wx/arrstr.h>
#endif


#include "json_defs.h"

// forward declarations
class WXDLLIMPEXP_JSON wxJSONReader;
class WXDLLIMPEXP_JSON wxJSONRefData;

#if defined( wxJSON_USE_STL )
    // if compiling on MinGW we use the STL-style declaration of wxWidget's
    // container classes
    class WXDLLIMPEXP_JSON wxJSONValue;
    WX_DECLARE_OBJARRAY( wxJSONValue, wxJSONInternalArray );
    WX_DECLARE_STRING_HASH_MAP( wxJSONValue, wxJSONInternalMap );
#else
    class WXDLLIMPEXP_JSON wxJSONInternalMap;
    class WXDLLIMPEXP_JSON wxJSONInternalArray;
#endif


//! The type of the value held by the wxJSONRefData class
enum wxJSONType {
    wxJSONTYPE_INVALID = 0,  /*!< the object is not uninitialized        */
    wxJSONTYPE_NULL,       /*!< the object contains a NULL value         */
    wxJSONTYPE_INT,        /*!< the object contains an integer           */
    wxJSONTYPE_UINT,       /*!< the object contains an unsigned integer  */
    wxJSONTYPE_DOUBLE,     /*!< the object contains a double             */
    wxJSONTYPE_STRING,     /*!< the object contains a wxString object    */
    wxJSONTYPE_CSTRING,    /*!< the object contains a static C-string    */
    wxJSONTYPE_BOOL,       /*!< the object contains a boolean            */
    wxJSONTYPE_ARRAY,      /*!< the object contains an array of values   */
    wxJSONTYPE_OBJECT,     /*!< the object contains a map of keys/values */
    wxJSONTYPE_LONG,       /*!< the object contains a 32-bit integer     */
    wxJSONTYPE_INT64,      /*!< the object contains a 64-bit integer     */
    wxJSONTYPE_ULONG,      /*!< the object contains an unsigned 32-bit integer  */
    wxJSONTYPE_UINT64,     /*!< the object contains an unsigned 64-bit integer  */
    wxJSONTYPE_SHORT,      /*!< the object contains a 16-bit integer            */
    wxJSONTYPE_USHORT,     /*!< the object contains a 16-bit unsigned integer   */
    wxJSONTYPE_MEMORYBUFF  /*!< the object contains a binary memory buffer   */
};

// the comment position: every value only has one comment position
// althrough comments may be splitted into several lines
enum {
  wxJSONVALUE_COMMENT_DEFAULT = 0,
  wxJSONVALUE_COMMENT_BEFORE,
  wxJSONVALUE_COMMENT_AFTER,
  wxJSONVALUE_COMMENT_INLINE,
};

/***********************************************************************

            class wxJSONValue

***********************************************************************/


// class WXDLLIMPEXP_JSON wxJSONValue : public wxObject
class WXDLLIMPEXP_JSON wxJSONValue
{
    friend class wxJSONReader;

public:

    // ctors and dtor
    wxJSONValue();
    wxJSONValue( wxJSONType type );
    wxJSONValue( int i );
    wxJSONValue( unsigned int i );
    wxJSONValue( short i );
    wxJSONValue( unsigned short i );
    wxJSONValue( long int i );
    wxJSONValue( unsigned long int i );
#if defined( wxJSON_64BIT_INT)
    wxJSONValue( wxInt64 i );
    wxJSONValue( wxUint64 ui );
#endif
    wxJSONValue( bool b  );
    wxJSONValue( double d );
    wxJSONValue( const wxChar* str );     // assume static ASCIIZ strings
    wxJSONValue( const wxString& str );
    wxJSONValue( const wxMemoryBuffer& buff );
    wxJSONValue( const void* buff, size_t len );
    wxJSONValue( const wxJSONValue& other );
    virtual ~wxJSONValue();

    // functions for retrieving the value type
    wxJSONType  GetType() const;
    bool IsValid() const;
    bool IsNull() const;
    bool IsInt() const;
    bool IsUInt() const;
    bool IsShort() const;
    bool IsUShort() const;
    bool IsLong() const;
    bool IsULong() const;
#if defined( wxJSON_64BIT_INT)
    bool IsInt32() const;
    bool IsInt64() const;
    bool IsUInt32() const;
    bool IsUInt64() const;
#endif
    bool IsBool() const;
    bool IsDouble() const;
    bool IsString() const;
    bool IsCString() const;
    bool IsArray() const;
    bool IsObject() const;
    bool IsMemoryBuff() const;

    // function for retireving the value as ...
    int            AsInt() const;
    unsigned int   AsUInt() const;
    short          AsShort() const;
    unsigned short AsUShort() const;
    long int       AsLong() const;
    unsigned long  AsULong() const;
    bool        AsInt( int& i ) const;
    bool        AsUInt( unsigned int& ui ) const;
    bool        AsShort( short int& s ) const;
    bool        AsUShort( unsigned short& us ) const;
    bool        AsLong( long int& l ) const;
    bool        AsULong( unsigned long& ul ) const;
#if defined( wxJSON_64BIT_INT)
    wxInt32        AsInt32() const;
    wxUint32       AsUInt32() const;
    wxInt64        AsInt64() const;
    wxUint64       AsUInt64() const;
    bool        AsInt32( wxInt32& i32 ) const;
    bool        AsUInt32( wxUint32& ui32 ) const;
    bool        AsInt64( wxInt64& i64 ) const;
    bool        AsUInt64( wxUint64& ui64 ) const;
#endif
    bool           AsBool() const;
    double         AsDouble() const;
    wxString       AsString() const;
    const wxChar*  AsCString() const;
    bool        AsBool( bool& b ) const;
    bool        AsDouble( double& d ) const;
    bool        AsString( wxString& str ) const;
    bool        AsCString( wxChar* ch ) const;
    wxMemoryBuffer AsMemoryBuff() const;
    bool        AsMemoryBuff( wxMemoryBuffer& buff ) const;

    const wxJSONInternalMap*   AsMap() const;
    const wxJSONInternalArray* AsArray() const;

    // get members names, size and other info
    bool      HasMember( unsigned index ) const;
    bool      HasMember( const wxString& key ) const;
    int       Size() const;
    wxArrayString  GetMemberNames() const;

    // appending items, resizing and deleting items
    wxJSONValue& Append( const wxJSONValue& value );
    wxJSONValue& Append( bool b );
    wxJSONValue& Append( int i );
    wxJSONValue& Append( unsigned int ui );
    wxJSONValue& Append( short int i );
    wxJSONValue& Append( unsigned short int ui );
    wxJSONValue& Append( long int l );
    wxJSONValue& Append( unsigned long int ul );
#if defined( wxJSON_64BIT_INT )
    wxJSONValue& Append( wxInt64 i );
    wxJSONValue& Append( wxUint64 ui );
#endif
    wxJSONValue& Append( double d );
    wxJSONValue& Append( const wxChar* str );
    wxJSONValue& Append( const wxString& str );
    wxJSONValue& Append( const wxMemoryBuffer& buff );
    wxJSONValue& Append( const void* buff, size_t len );
    bool         Remove( int index );
    bool         Remove( const wxString& key );
    void         Clear();
    bool         Cat( const wxChar* str );
    bool         Cat( const wxString& str );
    bool         Cat( const wxMemoryBuffer& buff );

    // retrieve an item
    wxJSONValue& Item( unsigned index );
    wxJSONValue& Item( const wxString& key );
    wxJSONValue  ItemAt( unsigned index ) const;
    wxJSONValue  ItemAt( const wxString& key ) const;

    wxJSONValue& operator [] ( unsigned index );
    wxJSONValue& operator [] ( const wxString& key );

    wxJSONValue& operator = ( int i );
    wxJSONValue& operator = ( unsigned int ui );
    wxJSONValue& operator = ( short int i );
    wxJSONValue& operator = ( unsigned short int ui );
    wxJSONValue& operator = ( long int l );
    wxJSONValue& operator = ( unsigned long int ul );
#if defined( wxJSON_64BIT_INT )
    wxJSONValue& operator = ( wxInt64 i );
    wxJSONValue& operator = ( wxUint64 ui );
#endif
    wxJSONValue& operator = ( bool b );
    wxJSONValue& operator = ( double d );
    wxJSONValue& operator = ( const wxChar* str );
    wxJSONValue& operator = ( const wxString& str );
    wxJSONValue& operator = ( const wxMemoryBuffer& buff );
    // wxJSONValue& operator = ( const void* buff, size_t len ); cannot be declared
    wxJSONValue& operator = ( const wxJSONValue& value );

    // get the value or a default value
    wxJSONValue  Get( const wxString& key, const wxJSONValue& defaultValue ) const;

    // comparison function
    bool         IsSameAs( const wxJSONValue& other ) const;

    // comment-related functions
    int      AddComment( const wxString& str, int position = wxJSONVALUE_COMMENT_DEFAULT );
    int      AddComment( const wxArrayString& comments, int position = wxJSONVALUE_COMMENT_DEFAULT );
    wxString GetComment( int idx = -1 ) const;
    int      GetCommentPos() const;
    int      GetCommentCount() const;
    void     ClearComments();
    const wxArrayString& GetCommentArray() const;

    // debugging functions
    wxString         GetInfo() const;
    wxString         Dump( bool deep = false, int mode = 0 ) const;

    //misc functions
    wxJSONRefData*   GetRefData() const;
    wxJSONRefData*   SetType( wxJSONType type );
    int              GetLineNo() const;
    void             SetLineNo( int num );

    // public static functions: mainly used for debugging
    static  wxString TypeToString( wxJSONType type );
    static  wxString MemoryBuffToString( const wxMemoryBuffer& buff, size_t len = -1 );
    static  wxString MemoryBuffToString( const void* buff, size_t len, size_t actualLen = -1 );
    static  int      CompareMemoryBuff( const wxMemoryBuffer& buff1, const wxMemoryBuffer& buff2 );
    static  int      CompareMemoryBuff( const wxMemoryBuffer& buff1, const void* buff2 );
    static wxMemoryBuffer ArrayToMemoryBuff( const wxJSONValue& value );

protected:
    wxJSONValue*  Find( unsigned index ) const;
    wxJSONValue*  Find( const wxString& key ) const;
    void          DeepCopy( const wxJSONValue& other );

    wxJSONRefData*  Init( wxJSONType type );
    wxJSONRefData*  COW();

    // overidden from wxObject
    virtual wxJSONRefData*  CloneRefData(const wxJSONRefData *data) const;
    virtual wxJSONRefData*  CreateRefData() const;

    void            SetRefData(wxJSONRefData* data);
    void            Ref(const wxJSONValue& clone);
    void            UnRef();
    void            UnShare();
    void            AllocExclusive();

    //! the referenced data
    wxJSONRefData*  m_refData;


    // used for debugging purposes: only in debug builds.
#if defined( WXJSON_USE_VALUE_COUNTER )
    int         m_progr;
    static int  sm_progr;
#endif
};


#if !defined( wxJSON_USE_STL )
    // if using wxWidget's implementation of container classes we declare
    // the OBJARRAY are HASH_MAP _after_ the wxJSONValue is fully known
    WX_DECLARE_OBJARRAY( wxJSONValue, wxJSONInternalArray );
    WX_DECLARE_STRING_HASH_MAP( wxJSONValue, wxJSONInternalMap );
#endif


/***********************************************************************

            class wxJSONRefData

***********************************************************************/




//! The actual value held by the wxJSONValue class (internal use)
/*!
 Note that this structure is a \b union as in versions prior to 0.4.x
 The union just stores primitive types and not complex types which are
 stored in separate data members of the wxJSONRefData structure.

 This organization give us more flexibility when retrieving compatible
 types such as ints unsigned ints, long and so on.
 To know more about the internal structure of the wxJSONValue class
 see \ref pg_json_internals.
*/
union wxJSONValueHolder  {
    int             m_valInt;
    unsigned int    m_valUInt;
    short int       m_valShort;
    unsigned short  m_valUShort;
    long int        m_valLong;
    unsigned long   m_valULong;
    double          m_valDouble;
    const wxChar*   m_valCString;
    bool            m_valBool;
#if defined( wxJSON_64BIT_INT )
    wxInt64         m_valInt64;
    wxUint64        m_valUInt64;
#endif
    };

//
// access to the (unsigned) integer value is done through
// the VAL_INT macro which expands to the 'long' integer
// data member of the 'long long' integer if 64-bits integer
// support is enabled
#if defined( wxJSON_64BIT_INT )
 #define VAL_INT  m_valInt64
 #define VAL_UINT m_valUInt64
#else
 #define VAL_INT  m_valLong
 #define VAL_UINT m_valULong
#endif



// class WXDLLIMPEXP_JSON wxJSONRefData : public wxObjectRefData
class WXDLLIMPEXP_JSON wxJSONRefData
{
    // friend class wxJSONReader;
    friend class wxJSONValue;
    friend class wxJSONWriter;

public:

    wxJSONRefData();
    virtual ~wxJSONRefData();

    int GetRefCount() const;

    // there is no need to define copy ctor

    //! the references count
    int               m_refCount;

    //! The actual type of the value held by this object.
    wxJSONType        m_type;

    //! The JSON value held by this object.
    /*!
    This data member contains the JSON data types defined by the
    JSON syntax with the exception of the complex objects.
    This data member is an union of the primitive types
    so that it is simplier to cast them in other compatible types.
    */
    wxJSONValueHolder m_value;

    //! The JSON string value.
    wxString            m_valString;

    //! The JSON array value.
    wxJSONInternalArray m_valArray;

    //! The JSON object value.
    wxJSONInternalMap   m_valMap;

    //! The position of the comment line(s), if any.
    /*!
    The data member contains one of the following constants:
    \li \c wxJSONVALUE_COMMENT_BEFORE
    \li \c wxJSONVALUE_COMMENT_AFTER
    \li \c wxJSONVALUE_COMMENT_INLINE
    */
    int               m_commentPos;

    //! The array of comment lines; may be empty.
    wxArrayString     m_comments;

    //! The line number when this value was read
    /*!
    This data member is used by the wxJSONReader class and it is
    used to store the line number of the JSON text document where
    the value appeared. This value is compared to the line number
    of a comment line in order to obtain the value which a
    comment refersto.
    */
    int               m_lineNo;

    //! The pointer to the memory buffer object
    /*!
     Note that despite using reference counting, the \b wxMemoryBuffer is not a 
     \e copy-on-write structure so the wxJSON library uses some tricks in order to
     avoid the side effects of copying / assigning wxMemoryBuffer objects
    */
    wxMemoryBuffer* m_memBuff;

    // used for debugging purposes: only in debug builds.
#if defined( WXJSON_USE_VALUE_COUNTER )
    int         m_progr;
    static int  sm_progr;
#endif
};



#endif            // not defined _WX_JSONVAL_H


