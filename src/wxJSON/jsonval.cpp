/////////////////////////////////////////////////////////////////////////////
// Name:        jsonval.cpp
// Purpose:     the wxJSON class that holds a JSON value
// Author:      Luciano Cattani
// Created:     2007/10/01
// RCS-ID:      $Id: jsonval.cpp,v 1.12 2008/03/06 10:25:18 luccat Exp $
// Copyright:   (c) 2007 Luciano Cattani
// Licence:     wxWidgets licence
/////////////////////////////////////////////////////////////////////////////

//#ifdef __GNUG__
//    #pragma implementation "jsonval.cpp"
//#endif

// make wxLogTrace a noop, it's really slow
// must be defined before including debug.h
#define wxDEBUG_LEVEL 0

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif


#include <wx/log.h>
#include <wx/arrimpl.cpp>

#include <wx/jsonval.h>


WX_DEFINE_OBJARRAY( wxJSONInternalArray );

#if wxCHECK_VERSION(3, 0, 0)
#define compatibleLongLongFmtSpec _T(wxLongLongFmtSpec)
#else
#define compatibleLongLongFmtSpec wxLongLongFmtSpec
#endif

#if wxDEBUG_LEVEL > 0
// the trace mask used in wxLogTrace() function
// static const wxChar* traceMask = _T("jsonval");
static const wxChar* traceMask = _T("jsonval");
static const wxChar* compareTraceMask = _T("sameas");
static const wxChar* cowTraceMask = _T("traceCOW" );
#endif


/*******************************************************************

            class wxJSONRefData

*******************************************************************/


/*! \class wxJSONRefData
 \brief The reference counted JSON value data (internal use).

 Starting from version 0.4, the JSON value class use the reference
 counting tecnique (also know as \e copy-on-write) described in the
 \b wxWidgets documentation in order to speed up processing.
 The class is used internally by the wxJSONValue class which does
 all processing.
 To know more about COW see \ref json_internals_cow
*/

#if defined( WXJSON_USE_VALUE_COUNTER )
    // The progressive counter (used for debugging only)
    int          wxJSONRefData::sm_progr = 1;
#endif

//! Constructor.
wxJSONRefData::wxJSONRefData()
{
    m_lineNo   = -1;
    m_refCount = 1;
    m_memBuff  = 0;

#if defined( WXJSON_USE_VALUE_COUNTER )
    m_progr = sm_progr;
    ++sm_progr;
    wxLogTrace( traceMask, _T("(%s) JSON refData ctor progr=%d"),
                __PRETTY_FUNCTION__, m_progr);
#endif
}

// Dtor
wxJSONRefData::~wxJSONRefData()
{
    if ( m_memBuff ) {
        delete m_memBuff;
    }
}

// Return the number of objects that reference this data.
int
wxJSONRefData::GetRefCount() const
{
    return m_refCount;
}


/*******************************************************************

            class wxJSONValue

*******************************************************************/


/*! \class wxJSONValue
 \brief The JSON value class implementation.

This class holds a JSON value which may be of variuos types (see the
wxJSONType constants for a description of the types).
To know more about the internal representation of JSON values see
\ref pg_json_internals.

Starting from version 0.5 the wxJSON library supports 64-bits integers on
platforms that have native support for very large integers.
Note that the integer type is still stored as a generic wxJSONTYPE_(U)INT
constant regardless the size of the value but the JSON value class defines
functions in order to let the user know if an integer value fits in 16, 32
or 64 bit integer.
To know more about 64-bits integer support see \ref json_internals_integer

Storing values in a JSON value object of this class is very simple.
The following is an example:
\code
    wxJSONValue v( _T( "A string"));  // store a string value in the object
    wxString s = v.AsString();        // get the string value

    v = 12;            // now 'v' contains an integer value
    int i = v.AsInt(); // get the integer
\endcode

 \par The C-string JSON value object

 The wxJSONValue(const wxChar*) ctor allows you to create a JSON value
 object that contains a string value which is stored as a
 \e pointer-to-static-string.
 Beware that this ctor DOES NOT copy the string: it only stores the
 pointer in a data member and the pointed-to buffer is not deleted
 by the dtor.
 If the string is not static you have to use the wxJSONValue(const wxString&)
 constructor.

 Also note that this does NOT mean that the value stored in this JSON
 object cannot change: you can assign whatever other value you want,
 an integer, a double or an array of values.
 What I intended is that the pointed-to string must exist for the lifetime
 of the wxJSONValue object.
 The following code is perfectly legal:
 \code
   wxJSONvalue aString( "this is a static string" );
   aString = 10;
 \endcode
 To know more about this topic see \ref json_internals_cstring.

 Starting from version 1.3 the class can hold binary memory buffers
 as an extension to the JSON syntax. Memory buffers are stored as
 \b wxMemoryBuffer objects which contain binary data. The class
 uses reference counting for the copy and assignment operation but
 it is not a \e copy-on-write structure.
 To know more about memory buffers read \ref wxjson_tutorial_memorybuff

 \sa the \ref wxjson_tutorial.
*/


#if defined( WXJSON_USE_VALUE_COUNTER )
    // The progressive counter (used for debugging only)
    int          wxJSONValue::sm_progr = 1;
#endif

//! Constructors.
/*!
 The overloaded constructors allow the user to construct a JSON value
 object that holds the specified value and type of value.
 The default ctor construct a valid JSON object that constains a \b null
 value.

 If you want to create an \b invalid JSON value object you have to use the
 \c wxJSONValue( wxJSONTYPE_INVALID ) ctor.
 Note that this object is not a valid JSON value - to know more about this
 topic see the SetType() function.

 To create an empty array or key/value map use the following:
 \code
   wxJSONvalue v1( wxJSONTYPE_ARRAY );
   wxJSONvalue v2( wxJSONTYPE_OBJECT );
 \endcode
*/
wxJSONValue::wxJSONValue()
{
    m_refData = 0;
    Init( wxJSONTYPE_NULL );
}

//! Initialize the JSON value class.
/*!
 The function is called by the ctors and allocates a new instance of
 the wxJSONRefData class and sets the type of the JSON value.
 Note that only the type is set, not the value.
 Also note that this function may be called from other memberfunctions
 if the \c m_refData data member is NULL.
*/
wxJSONRefData*
wxJSONValue::Init( wxJSONType type )
{
    wxJSONRefData* data = GetRefData();
    if ( data != 0 ) {
        UnRef();
    }

    // we allocate a new instance of the referenced data
    data = new wxJSONRefData();
    wxJSON_ASSERT( data );

    // in release builds we do not have ASSERT so we check 'data' before
    // using it
    if ( data )  {
        data->m_type = type;
        data->m_commentPos = wxJSONVALUE_COMMENT_BEFORE;
    }
    SetRefData( data );

#if defined( WXJSON_USE_VALUE_COUNTER )
    m_progr = sm_progr;
    ++sm_progr;
    wxLogTrace( cowTraceMask, _T("(%s) Init a new object progr=%d"),
             __PRETTY_FUNCTION__, m_progr );
#endif
    return data;
}


//! \overload wxJSONValue()
wxJSONValue::wxJSONValue( wxJSONType type )
{
    m_refData = 0;
    Init( type );
}

//! \overload wxJSONValue()
wxJSONValue::wxJSONValue( int i )
{
    m_refData = 0;
    wxJSONRefData* data = Init( wxJSONTYPE_INT );
    wxJSON_ASSERT( data );
    if ( data != 0 ) {
        // the 'VAL_INT' macro expands to 'm_valLong' or 'm_valInt64' depending
        // on 64-bits integer support being enabled on not
        data->m_value.VAL_INT = i;
    }
}


//! \overload wxJSONValue()
wxJSONValue::wxJSONValue( unsigned int ui )
{
    m_refData = 0;
    wxJSONRefData* data = Init( wxJSONTYPE_UINT );
    wxJSON_ASSERT( data );
    if ( data != 0 ) {
        // the 'VAL_UINT' macro expands to 'm_valULong' or 'm_valUInt64' depending
        // on 64-bits integer support being enabled on not
        data->m_value.VAL_UINT = ui;
    }
}

//! \overload wxJSONValue()
wxJSONValue::wxJSONValue( short int i )
{
    m_refData = 0;
    wxJSONRefData* data = Init( wxJSONTYPE_INT );
    wxJSON_ASSERT( data );
    if ( data != 0 ) {
        // the 'VAL_INT' macro expands to 'm_valLong' or 'm_valInt64' depending
        // on 64-bits integer support being enabled on not
        data->m_value.VAL_INT = i;
    }
}


//! \overload wxJSONValue()
wxJSONValue::wxJSONValue( unsigned short ui )
{
    m_refData = 0;
    wxJSONRefData* data = Init( wxJSONTYPE_UINT );
    wxJSON_ASSERT( data );
    if ( data != 0 ) {
        // the 'VAL_UINT' macro expands to 'm_valULong' or 'm_valUInt64' depending
        // on 64-bits integer support being enabled on not
    data->m_value.VAL_UINT = ui;
    }
}

//! \overload wxJSONValue()
wxJSONValue::wxJSONValue( bool b  )
{
    m_refData = 0;
    wxJSONRefData* data = Init( wxJSONTYPE_BOOL );
    wxJSON_ASSERT( data );
    if ( data != 0 ) {
        data->m_value.m_valBool = b;
    }
}

//! \overload wxJSONValue()
wxJSONValue::wxJSONValue( double d )
{
    m_refData = 0;
    wxJSONRefData* data = Init( wxJSONTYPE_DOUBLE );
    wxJSON_ASSERT( data );
    if ( data != 0 ) {
        data->m_value.m_valDouble = d;
    }
}

//! \overload wxJSONValue()
wxJSONValue::wxJSONValue( const wxChar* str )
{
    m_refData = 0;
    wxJSONRefData* data = Init( wxJSONTYPE_CSTRING );
    wxJSON_ASSERT( data );
    if ( data != 0 ) {
    #if !defined( WXJSON_USE_CSTRING )
        data->m_type = wxJSONTYPE_STRING;
        data->m_valString.assign( str );
    #else
        data->m_value.m_valCString = str;
    #endif
    }
}

//! \overload wxJSONValue()
wxJSONValue::wxJSONValue( const wxString& str )
{
    m_refData = 0;
    wxJSONRefData* data = Init( wxJSONTYPE_STRING );
    wxJSON_ASSERT( data );
    if ( data != 0 ) {
        data->m_valString.assign( str );
    }
}

//! \overload wxJSONValue()
wxJSONValue::wxJSONValue( long int l )
{
    m_refData = 0;
    wxJSONRefData* data = Init( wxJSONTYPE_INT );
    wxJSON_ASSERT( data );
    if ( data != 0 ) {
        data->m_value.VAL_INT = l;
    }
}

//! \overload wxJSONValue()
wxJSONValue::wxJSONValue( unsigned long int ul )
{
    m_refData = 0;
    wxJSONRefData* data = Init( wxJSONTYPE_UINT );
    wxJSON_ASSERT( data );
    if ( data != 0 ) {
        data->m_value.VAL_UINT = ul;
    }
}

//! Construct a JSON value object of type \e memory \e buffer
/*!
 Note that this ctor makes a deep copy of \c buff so changes made
 to the original buffer does not reflect to the buffer stored in this
 JSON value.
*/
wxJSONValue::wxJSONValue( const wxMemoryBuffer& buff )
{
    m_refData = 0;
    wxJSONRefData* data = Init( wxJSONTYPE_MEMORYBUFF );
    wxJSON_ASSERT( data );
    if ( data != 0 ) {
        data->m_memBuff = new wxMemoryBuffer();
        const void* ptr = buff.GetData();
        size_t buffLen  = buff.GetDataLen();
        if ( buffLen > 0 )  {
            data->m_memBuff->AppendData( ptr, buffLen );
        }
    }
}

//! Construct a JSON value object of type \e memory \e buffer
/*!
 Note that this ctor makes a deep copy of \c buff so changes made
 to the original buffer does not reflect to the buffer stored in this
 JSON value.
*/
wxJSONValue::wxJSONValue( const void* buff, size_t len )
{
    m_refData = 0;
    wxJSONRefData* data = Init( wxJSONTYPE_MEMORYBUFF );
    wxJSON_ASSERT( data );
    if ( data != 0 && len > 0 ) {
        data->m_memBuff = new wxMemoryBuffer();
        data->m_memBuff->AppendData( buff, len );
    }
}

//! Copy constructor
/*!
 The function copies the content of \c other in this
 object.
 Note that the JSON value object is not really copied;
 the function calls Ref() in order to increment
 the reference count of the \c wxJSONRefData structure.
*/
wxJSONValue::wxJSONValue( const wxJSONValue& other )
{
    m_refData = 0;
    Ref( other );

    // the progressive counter of the ctor is not copied from
    // the other wxJSONValue object: only data is shared, the
    // progressive counter is not shared because this object
    // is a copy of 'other' and it has its own progressive
#if defined( WXJSON_USE_VALUE_COUNTER )
    m_progr = sm_progr;
    ++sm_progr;
    wxLogTrace( cowTraceMask, _T("(%s) Copy ctor - progr=%d other progr=%d"),
              __PRETTY_FUNCTION__, m_progr, other.m_progr );
#endif
}


//! Dtor - calls UnRef().
wxJSONValue::~wxJSONValue()
{
    UnRef();
}


// functions for retreiving the value type: they are all 'const'


//! Return the type of the value stored in the object.
/*!
 This function is the only one that does not ASSERT that the
 \c m_refData data member is not NULL.
 In fact, if the JSON value object does not contain a pointer
 to a wxJSONRefData structure, the function returns the
 wxJSONTYPE_INVALID constant which represent an invalid JSON value object.
 Also note that the pointer to the referenced data structure
 should NEVER be NULL.

 \par Integer types

 Integers are stored internally in a \b signed/unsigned \b long \b int
 or, on platforms that support 64-bits integers, in a
 \b wx(U)Int64 data type.
 When constructed, it is assigned a generic integer type that only
 depends on the sign: wxJSON_(U)INT regardless the size of the
 stored value.

 This function can be used to know the actual size requirement
 of the stored value and how it can be retrieved. The value
 returned by this function is:

 - for signed integers:
   - \b wxJSONTYPE_SHORT if the value is between SHORT_MIN and SHORT_MAX
   - \b wxJSONTYPE_LONG if the value is between LONG_MIN and LONG_MAX
     and greater than SHORT_MAX and less than SHORT_MIN
   - \b wxJSONTYPE_INT64 if the value is greater than LONG_MAX and
     less than LONG_MIN

 - for unsigned integers:
   - \b wxJSONTYPE_USHORT if the value is between 0 and USHORT_MAX
   - \b wxJSONTYPE_ULONG if the value is between 0 and ULONG_MAX
     and greater than USHORT_MAX
   - \b wxJSONTYPE_UINT64 if the value is greater than ULONG_MAX

 Note that this function never returns the wxJSONTYPE_(U)INT constant
 because the \b int data type may have the same width as SHORT or LONG
 depending on the platform.
 This does not mean that you cannot use \b int as the return value: if
 you use \b wxWidgets to develop application in only one platform, you
 can use \b int because you know the size of the data type.
 Otherwise, if is preferable to always use \b long instead of \b int.

 Also note that the class defines the \c IsInt() memberfunction which
 works fine regardless the actual width of the \b int data type.
 This function returns TRUE if the stored value fits in a \b int data
 type whatever its size is on the current platform (16 or 32-bits).

 \sa SetType IsInt
*/
wxJSONType
wxJSONValue::GetType() const
{
    wxJSONRefData* data = GetRefData();
    wxJSONType type = wxJSONTYPE_INVALID;
    if ( data )  {
        type = data->m_type;

        // for integers and unsigned ints check the storage requirements
        // note that ints are stored as 'long' or as 'long long'
        switch ( type )  {
            case wxJSONTYPE_INT :
            // check if the integer fits in a SHORT INT
                if ( data->m_value.VAL_INT >= SHORT_MIN &&
                                data->m_value.VAL_INT <= SHORT_MAX ) {
                    type = wxJSONTYPE_SHORT;
                }
            // check if the value fits in LONG INT
                else if ( data->m_value.VAL_INT >= LONG_MIN
                                && data->m_value.VAL_INT <= LONG_MAX ) {
                    type = wxJSONTYPE_LONG;
                }
            else {
                type = wxJSONTYPE_INT64;
            }
            break;

            case wxJSONTYPE_UINT :
                if ( data->m_value.VAL_UINT <= USHORT_MAX ) {
                    type = wxJSONTYPE_USHORT;
                }
                else if ( data->m_value.VAL_UINT <= ULONG_MAX ) {
                    type = wxJSONTYPE_ULONG;
                }
                else  {
                    type = wxJSONTYPE_UINT64;
                }
                break;

            default :
                break;
        }
    }
    return type;
}


//! Return TRUE if the type of the value is wxJSONTYPE_NULL.
bool
wxJSONValue::IsNull() const
{
    wxJSONType type = GetType();
    bool r = false;
    if ( type == wxJSONTYPE_NULL )  {
        r = true;
    }
    return r;
}


//! Return TRUE if the value stored is valid
/*!
 The function returns TRUE if the wxJSONValue object was correctly
 initialized - that is it contains a valid value.
 A JSON object is valid if its type is not equal to wxJSONTYPE_INVALID.
 Please note that the default ctor of wxJSONValue constructs a \b valid
 JSON object of type \b null.
 To create an invalid object you have to use;
 \code
   wxJSONValue v( wxJSONTYPE_INVALID );
 \endcode
*/
bool
wxJSONValue::IsValid() const
{
    wxJSONType type = GetType();
    bool r = false;
    if ( type != wxJSONTYPE_INVALID )  {
        r = true;
    }
    return r;
}

//! Return TRUE if the type of the value stored is integer.
/*!
 This function returns TRUE if the stored value is of
 type signed integer and the numeric value fits in a
 \b int data type.
 In other words, the function returns TRUE if the \c wxJSONRefData::m_type
 data member is of type \c wxJSONTYPE_INT and:

 \code
   INT_MIN <= m_value <= INT_MAX
 \endcode

 Note that if you are developing cross-platform applications you should never
 use \b int as the integer data type but \b long for 32-bits integers and
 \b short for 16-bits integers.
 This is because the \b int data type may have different width on different
 platforms.
 Regardless the widht of the data type (16 or 32 bits), the function returns
 the correct result because it relies on the INT_MAX and INT_MIN macros.

 \sa \ref json_internals_integer
*/
bool
wxJSONValue::IsInt() const
{
    wxJSONType type = GetType();
    bool r = false;
    // if the type is SHORT the value fits into an INT, too
    if ( type == wxJSONTYPE_SHORT )  {
        r = true;
    }
    else if ( type == wxJSONTYPE_LONG )  {
        // in case of LONG, check if the bit width is the same
        if ( INT_MAX == LONG_MAX )  {
            r = true;
        }
    }
    return r;
}

//! Return TRUE if the type of the value stored is 16-bit integer.
/*!
 This function returns TRUE if the stored value is of
 type signed integer and the numeric value fits in a
 \b short \b int data type (16-bit integer).
 In other words, the function returns TRUE if the \c wxJSONRefData::m_type
 data member is of type \c wxJSONTYPE_INT and:

 \code
   SHORT_MIN <= m_value <= SHORT_MAX
 \endcode

 \sa \ref json_internals_integer
*/
bool
wxJSONValue::IsShort() const
{
    wxJSONType type = GetType();
    bool r = false;
    if ( type == wxJSONTYPE_SHORT )  {
        r = true;
    }
    return r;
}

//! Return TRUE if the type of the value stored is a unsigned int.
/*!
 This function returns TRUE if the stored value is of
 type unsigned integer and the numeric value fits int a
 \b int data type.
 In other words, the function returns TRUE if the \c wxJSONRefData::m_type
 data member is of type \c wxJSONTYPE_UINT and:

 \code
   0 <= m_value <= UINT_MAX
 \endcode

 Note that if you are developing cross-platform applications you should never
 use \b unsigned \b int as the integer data type but \b unsigned \b long for
 32-bits integers and \b unsigned \b short for 16-bits integers.
 This is because the \b unsigned \b int data type may have different width
 on different platforms.
 Regardless the widht of the data type (16 or 32 bits), the function returns
 the correct result because it relies on the UINT_MAX macro.


 \sa \ref json_internals_integer
*/
bool
wxJSONValue::IsUInt() const
{
    wxJSONType type = GetType();
    bool r = false;
    if ( type == wxJSONTYPE_USHORT )  {
        r = true;
    }
    else if ( type == wxJSONTYPE_ULONG )  {
        if ( INT_MAX == LONG_MAX )  {
            r = true;
        }
    }
    return r;
}

//! Return TRUE if the type of the value stored is a unsigned short.
/*!
 This function returns TRUE if the stored value is of
 type unsigned integer and the numeric value fits in a
 \b unsigned \b short \b int data type.
 In other words, the function returns TRUE if the \c wxJSONRefData::m_type
 data member is of type \c wxJSONTYPE_UINT and:

 \code
   0 <= m_value <= USHORT_MAX
 \endcode

 \sa \ref json_internals_integer
*/
bool
wxJSONValue::IsUShort() const
{
    wxJSONType type = GetType();
    bool r = false;
    if ( type == wxJSONTYPE_USHORT )  {
        r = true;
    }
    return r;
}


//! Return TRUE if the stored value is an integer which fits in a long int
/*!
 This function returns TRUE if the stored value is of
 type signed LONG integer and the numeric value fits int a
 \b long \b int data type.
 In other words, the function returns TRUE if the \c wxJSONRefData::m_type
 data member is of type \c wxJSONTYPE_INT and:

 \code
   LONG_MIN <= m_value <= LONG_MAX
 \endcode

 \sa \ref json_internals_integer
*/
bool
wxJSONValue::IsLong() const
{
    wxJSONType type = GetType();
    bool r = false;
    if ( type == wxJSONTYPE_LONG || type == wxJSONTYPE_SHORT )  {
        r = true;
    }
    return r;
}

//! Return TRUE if the stored value is an integer which fits in a unsigned long int
/*!
 This function returns TRUE if the stored value is of
 type unsigned LONG integer and the numeric value fits int a
 \b unsigned \b long \b int data type.
 In other words, the function returns TRUE if the \c wxJSONRefData::m_type
 data member is of type \c wxJSONTYPE_UINT and:

 \code
   0 <= m_value <= ULONG_MAX
 \endcode

 \sa \ref json_internals_integer
*/
bool
wxJSONValue::IsULong() const
{
    wxJSONType type = GetType();
    bool r = false;
    if ( type == wxJSONTYPE_ULONG || type == wxJSONTYPE_USHORT )  {
        r = true;
    }
    return r;
}



//! Return TRUE if the type of the value stored is a boolean.
bool
wxJSONValue::IsBool() const
{
    wxJSONType type = GetType();
    bool r = false;
    if ( type == wxJSONTYPE_BOOL )  {
        r = true;
    }
    return r;
}

//! Return TRUE if the type of the value stored is a double.
bool
wxJSONValue::IsDouble() const
{
    wxJSONType type = GetType();
    bool r = false;
    if ( type == wxJSONTYPE_DOUBLE )  {
        r = true;
    }
    return r;
}

//! Return TRUE if the type of the value stored is a wxString object.
bool
wxJSONValue::IsString() const
{
    wxJSONType type = GetType();
    bool r = false;
    if ( type == wxJSONTYPE_STRING )  {
        r = true;
    }
    return r;
}

//! Return TRUE if the type of the value stored is a pointer to a static C string.
/*!
 This function returns TRUE if, and only if the stored value is a
 pointer to a static C-string and the C-string storage is enabled in
 the wxJSON library.
 By default, C-string storage is not enabled in the library so this
 function always returns FALSE.
 To know more about C-strings read \ref json_internals_cstring
*/
bool
wxJSONValue::IsCString() const
{
    wxJSONType type = GetType();
    bool r = false;
    if ( type == wxJSONTYPE_CSTRING )  {
        r = true;
    }
    return r;
}

//! Return TRUE if the type of the value stored is an array type.
bool
wxJSONValue::IsArray() const
{
    wxJSONType type = GetType();
    bool r = false;
    if ( type == wxJSONTYPE_ARRAY )  {
        r = true;
    }
    return r;
}

//! Return TRUE if the type of this value is a key/value map.
bool
wxJSONValue::IsObject() const
{
    wxJSONType type = GetType();
    bool r = false;
    if ( type == wxJSONTYPE_OBJECT )  {
        r = true;
    }
    return r;
}

//! Return TRUE if the type of this value is a binary memory buffer.
bool
wxJSONValue::IsMemoryBuff() const
{
    wxJSONType type = GetType();
    bool r = false;
    if ( type == wxJSONTYPE_MEMORYBUFF )  {
        r = true;
    }
    return r;
}



// get the stored value; all these functions are 'const'

//! Return the stored value as an integer.
/*!
 The function returns the stored value as an integer.
 Note that the function does not check that the type of the
 value is actually an integer and it just returns the content
 of the wxJSONValueHolder union.
 However, in debug builds,  the function ASSERTs that the
 type of the stored value \c IsInt().

 \sa \ref json_internals_integer
 \sa \ref wxjson_tutorial_get
*/
int
wxJSONValue::AsInt() const
{
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );
    int i = (int) data->m_value.VAL_INT;

    wxJSON_ASSERT( IsInt());
    return i;
}

//! Return the stored value as a boolean.
/*!
 The function returns the stored value as a boolean.
 Note that the function does not check that the type of the
 value is actually a boolean and it just returns the content
 of the wxJSONValueHolder union.
 However, in debug builds,  the function ASSERTs that the
 type of the stored value is wxJSONTYPE_BOOL.

 \sa \ref wxjson_tutorial_get
*/
bool
wxJSONValue::AsBool() const
{
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );
    wxJSON_ASSERT( data->m_type == wxJSONTYPE_BOOL );
    return data->m_value.m_valBool;
}

//! Return the stored value as a double.
/*!
 The function returns the stored value as a double.
 Note that the function does not check that the type of the
 value is actually a double and it just returns the content
 of the wxJSONValueHolder union as if it was a double.
 However, in debug builds,  the function ASSERTs that the
 type of the stored value \c IsDouble().

 \sa \ref wxjson_tutorial_get
*/
double
wxJSONValue::AsDouble() const
{
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );
    double d = data->m_value.m_valDouble;
    wxJSON_ASSERT( IsDouble());
    return d;
}


//! Return the stored value as a wxWidget's string.
/*!
 The function returns a string representation of the value
 stored in the JSON object.
 All value types are converted to a string by this function
 and returned as a string:

 \li For integer the string is the string representation of
    the numerical value in decimal notation; the function uses the
    \b wxString::Printf() function for the conversion

 \li for doubles, the value is converted to a string using the
     \b wxString::Printf("%.10g") function; the format string specifies
     a precision of ten decimal digits and suppress trailing ZEROes

 \li for booleans the string returned is: \b true or \b false.

 \li if the value is a NULL value the \b null literal string is returned.

 \li if the value is of type wxJSONTYPE_INVALID, the literal string \b &lt;invalid&gt;
    is returned. Note that this is NOT a valid JSON text.
 
 \li if the value is of type wxJSONTYPE_MEMORYBUFF the string returned contains the
    hexadecimal digits of the first 5 bytes preceeded by the length of the buffer,
    enclosed in parenthesis

 If the value is an array or map, the returned string is the number of
 elements is the array/object enclosed in the JSON special characters that
 identifies the array/object. Example:

 \code
    [0]    // an empty array
    {12}   // an object of 12 elements
 \endcode

 \sa \ref wxjson_tutorial_get
*/
wxString
wxJSONValue::AsString() const
{
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );
    wxString s;
    int size = Size();
    switch ( data->m_type )  {
        case wxJSONTYPE_STRING :
            s.assign( data->m_valString);
            break;
        case wxJSONTYPE_CSTRING :
            s.assign( data->m_value.m_valCString);
            break;
        case wxJSONTYPE_INT :
            #if defined( wxJSON_64BIT_INT )
            s.Printf( _T("%") compatibleLongLongFmtSpec _T("i"),
                        data->m_value.m_valInt64 );
            #else
            s.Printf( _T("%ld"), data->m_value.m_valLong );
            #endif
            break;
        case wxJSONTYPE_UINT :
            #if defined( wxJSON_64BIT_INT )
            s.Printf( _T("%") compatibleLongLongFmtSpec _T("u"),
                        data->m_value.m_valUInt64 );
            #else
            s.Printf( _T("%lu"), data->m_value.m_valULong );
            #endif
            break;
        case wxJSONTYPE_DOUBLE :
            s.Printf( _T("%.10g"), data->m_value.m_valDouble );
            break;
        case wxJSONTYPE_BOOL :
            s.assign( ( data->m_value.m_valBool ?
                         _T("true") : _T("false") ));
            break;
        case wxJSONTYPE_NULL :
            s.assign( _T( "null"));
            break;
        case wxJSONTYPE_INVALID :
            s.assign( _T( "<invalid>"));
            break;
        case wxJSONTYPE_ARRAY :
            s.Printf( _T("[%d]"), size );
            break;
        case wxJSONTYPE_OBJECT :
            s.Printf( _T("{%d}"), size );
            break;
        case wxJSONTYPE_MEMORYBUFF :
            s = MemoryBuffToString( *(data->m_memBuff), 5 );
            break;
        default :
            s.assign( _T( "wxJSONValue::AsString(): Unknown JSON type \'"));
            s.append( TypeToString( data->m_type ));
            s.append( _T( "\'" ));
            wxFAIL_MSG( s );
            break;
    }
    return s;
}

//! Return the stored value as a pointer to a static C string.
/*!
 If the type of the value is stored as a C-string data type the
 function just returns that pointer.
 If the stored value is a wxString object, the function returns the
 pointer returned by the \b wxString::c_str() function.
 If the stored value is of all other JSON types, the functions returns a NULL pointer.

 Note that in versions prior to 0.5, the
 function returned a NULL pointer also if the value is a \c wxString object.

 \sa \ref json_internals_cstring
 \sa \ref wxjson_tutorial_get

*/
const wxChar*
wxJSONValue::AsCString() const
{
    const wxChar* s = 0;
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );
    switch ( data->m_type )  {
        case wxJSONTYPE_CSTRING :
            s = data->m_value.m_valCString;
            break;
        case wxJSONTYPE_STRING :
            s = data->m_valString.c_str();
            break;
        default :
            break;
    }
    return s;
}


//! Return the stored value as a unsigned int.
/*!
 The function returns the stored value as a unsigned integer.
 Note that the function does not check that the type of the
 value is actually a unsigned integer and it just returns the content
 of the wxJSONValueHolder union.
 However, in debug builds,  the function ASSERTs that the
 type of the stored value is wxJSONTYPE_UINT.

 \sa \ref json_internals_integer
 \sa \ref wxjson_tutorial_get
*/
unsigned int
wxJSONValue::AsUInt() const
{
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );
    unsigned int ui = (unsigned) data->m_value.VAL_UINT;

    wxJSON_ASSERT( IsUInt());
    return ui;
}


//! Returns the value as a long integer
/*!
 The function returns the stored value as a long integer.
 Note that the function does not check that the type of the
 value is actually a long integer and it just returns the content
 of the wxJSONValueHolder union.
 However, in debug builds,  the function ASSERTs that the
 type of the stored value \c IsLong().

 \sa \ref json_internals_integer
 \sa \ref wxjson_tutorial_get
*/
long int
wxJSONValue::AsLong() const
{
    long int l;
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );
    l = (long) data->m_value.VAL_INT;

    wxJSON_ASSERT( IsLong());
    return l;
}

//! Returns the value as a unsigned long integer
/*!
 The function returns the stored value as a unsigned long integer.
 Note that the function does not check that the type of the
 value is actually a unsigned long integer and it just returns the content
 of the wxJSONValueHolder union.
 However, in debug builds,  the function ASSERTs that the
 type of the stored value \c IsLong().

 \sa \ref json_internals_integer
 \sa \ref wxjson_tutorial_get
*/
unsigned long int
wxJSONValue::AsULong() const
{
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );
    unsigned long int ul = (unsigned long) data->m_value.VAL_UINT;

    wxJSON_ASSERT( IsULong());  // expands only in debug builds
    return ul;
}


//! Returns the value as a short integer
/*!
 The function returns the stored value as a short integer.
 Note that the function does not check that the type of the
 value is actually a short integer and it just returns the content
 of the wxJSONValueHolder union.
 However, in debug builds,  the function ASSERTs that the
 type of the stored value \c IsShort().

 \sa \ref json_internals_integer
 \sa \ref wxjson_tutorial_get
*/
short int
wxJSONValue::AsShort() const
{
    short int i;
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );
    i = (short) data->m_value.VAL_INT;

    wxJSON_ASSERT( IsShort());
    return i;
}

//! Returns the value as a unsigned short integer
/*!
 The function returns the stored value as a unsigned short integer.
 Note that the function does not check that the type of the
 value is actually a unsigned short and it just returns the content
 of the wxJSONValueHolder union.
 However, in debug builds,  the function ASSERTs that the
 type of the stored value \c IsUShort().

 \sa \ref json_internals_integer
 \sa \ref wxjson_tutorial_get
*/
unsigned short
wxJSONValue::AsUShort() const
{
    unsigned short ui;
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );
    ui = (unsigned short) data->m_value.VAL_UINT;

    wxJSON_ASSERT( IsUShort());
    return ui;
}



//! Stores the value of this object in the provided argument
/*!
 The functions of the form \c AsXxxxxx(T&) are the same as the \c AsXxxxxxx()
 but store the value in the provided argument and return TRUE if the value of
 this object is of the correct type.
 By using these functions you can get the value and test if the JSON value is
 of the expected type in only one step.
 For example:
 \code
   int i; wxJSONValue v(10);
   if ( !v.AsInt( i )) {
     cout << "Error: value is not of the expected type";
   }
 \endcode
 This is the same as:
 \code
   int i; wxJSONValue v(10);
   if ( v.IsInt() {
     i = v.AsInt();
   }
   else {
     cout << "Error: value is not of the expected type";
   }
 \endcode
 Thanks to \b catalin who has suggested this new feature.
*/
bool
wxJSONValue::AsInt( int& i ) const
{
    bool r = false;
    if ( IsInt() )    {
        i = AsInt();
        r = true;
    }
    return r;
}

bool
wxJSONValue::AsUInt( unsigned int& ui ) const
{
    bool r = false;
    if ( IsUInt() )    {
        ui = AsUInt();
        r = true;
    }
    return r;
}

bool
wxJSONValue::AsShort( short int& s ) const
{
    bool r = false;
    if ( IsShort() )    {
        s = AsShort();
        r = true;
    }
    return r;
}

bool
wxJSONValue::AsUShort( unsigned short& us ) const
{
    bool r = false;
    if ( IsUShort() )    {
        us = AsUShort();
        r = true;
    }
    return r;
}

bool
wxJSONValue::AsLong( long int& l ) const
{
    bool r = false;
    if ( IsLong() )    {
        l = AsLong();
        r = true;
    }
    return r;
}

bool
wxJSONValue::AsULong( unsigned long& ul ) const
{
    bool r = false;
    if ( IsULong() )    {
        ul = AsULong();
        r = true;
    }
    return r;
}


bool
wxJSONValue::AsBool( bool& b ) const
{
    bool r = false;
    if ( IsBool() )    {
        b = AsBool();
        r = true;
    }
    return r;
}

bool
wxJSONValue::AsDouble( double& d ) const
{
    bool r = false;
    if ( IsDouble() )    {
        d = AsDouble();
        r = true;
    }
    return r;
}

//! Return this string value in the provided argument
/*!
 This function is different from \c AsString because the latter always returns
 a string also when this object does not contain a string. In that case, a string
 representation of this value is returned.
 This function, instead, returns TRUE only if this object contains a string, that is
 only if \c IsString() returns TRUE.
 Also note that the string value is only stored in \c str if this object actually
 contains a \b string or \b c-string value.
 \c str will never contain a string representation of other types. 
*/
bool
wxJSONValue::AsString( wxString& str ) const
{
    bool r = IsString();
    if ( r )    {
        str = AsString();
    }
    return r;
}

bool
wxJSONValue::AsCString( wxChar* ch ) const
{
    bool r = IsCString();
    if ( r )    {
        ch = (wxChar*) AsCString();
    }
    return r;
}

//! Returns the value as a memory buffer
/*!
 The function returns the \e memory \e buffer object stored in
 this JSON object.
 Note that as of wxWidgets 2.8 and 2.9 the \b wxMemoryBuffer object uses
 reference counting when copying the actual buffer but the class itself
 is not a \e copy-on-write structure so changes made to one buffer affects
 all other copies made from it.
 This means that if you make a change to the returned copy of the memory
 buffer, the change affects also the memory buffer stored in this JSON value.

 If this JSON object does not contain a \e wxJSONTYPE_MEMORYBUFF type
 the function returns an empty memory buffer object.
 An empty memory buffer is also returned if this JSON
 type contains a valid, empty memory buffer.
 You have to use the IsMemoryBuff() function to known the type of the
 JSON value contained in this object, or the overloaded version of
 this function. 
*/
wxMemoryBuffer
wxJSONValue::AsMemoryBuff() const
{
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );
    wxMemoryBuffer buff;
    if ( data->m_memBuff ) {
        buff = *(data->m_memBuff);
    }

    wxJSON_ASSERT( IsMemoryBuff());
    return buff;
}


//! Returns the value as a memory buffer
/*!
 The function returns the \e memory \e buffer object stored in
 this JSON object.
 Note that as of wxWidgets 2.8 and 2.9 the \b wxMemoryBuffer object uses
 reference counting when copying the actual buffer but the class itself
 is not a \e copy-on-write structure so changes made to one buffer affects
 all other copies made from it.
 This means that if you make a change to the returned copy of the memory
 buffer, the change affects also the memory buffer stored in this JSON value.

 If this JSON object does not contain a \e wxJSONTYPE_MEMORYBUFF type
 the function returns an empty memory buffer object.
 An empty memory buffer is also returned if this JSON
 type contains a valid, empty memory buffer.
 You have to use the IsMemoryBuff() function to known the type of the
 JSON value contained in this object, or the overloaded version of
 this function. 
*/
bool
wxJSONValue::AsMemoryBuff( wxMemoryBuffer& buff ) const
{
    bool r = IsMemoryBuff();
    if ( r )    {
        buff = AsMemoryBuff();
    }
    return r;
}


// internal use

//! Return the stored value as a map object.
/*!
 This function is for testing and debugging purposes and you shold never use it.
 To retreive values from an array or map JSON object use the \c Item() or ItemAt()
 memberfunctions or the subscript operator.
 If the stored value is not a map type, returns a NULL pointer.
*/
const wxJSONInternalMap*
wxJSONValue::AsMap() const
{
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );

    const wxJSONInternalMap* v = 0;
    if ( data->m_type == wxJSONTYPE_OBJECT ) {
        v = &( data->m_valMap );
    }
    return v;
}

//! Return the stored value as an array object.
/*!
 This function is for testing and debugging purposes and you shold never use it.
 To retreive values from an array or map JSON object use the \c Item() or ItemAt()
 memberfunctions or the subscript operator.
 If the stored value is not an array type, returns a NULL pointer.
*/
const wxJSONInternalArray*
wxJSONValue::AsArray() const
{
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );

    const wxJSONInternalArray* v = 0;
    if ( data->m_type == wxJSONTYPE_ARRAY ) {
        v = &( data->m_valArray );
    }
    return v;
}

// retrieve the members and other info


//! Return TRUE if the object contains an element at the specified index.
/*!
 If the stoerd value is not an array or a map, the function returns FALSE.
*/
bool
wxJSONValue::HasMember( unsigned index ) const
{
    bool r = false;
    int size = Size();
    if ( index < (unsigned) size )  {
        r = true;
    }
    return r;
}

//! Return TRUE if the object contains an element at the specified key.
/*!
 If the stored value is not a key/map map, the function returns FALSE.
*/
bool
wxJSONValue::HasMember( const wxString& key ) const
{
    bool r = false;
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );

    if ( data && data->m_type == wxJSONTYPE_OBJECT )  {
        wxJSONInternalMap::iterator it = data->m_valMap.find( key );
        if ( it != data->m_valMap.end() )  {
            r = true;
        }
    }
    return r;
}

//! Return the size of the array or map stored in this value.
/*!
 Note that both the array and the key/value map may have a size of
 ZERO elements.
 If the stored value is not an array nor a key/value hashmap, the
 function returns -1.
*/
int
wxJSONValue::Size() const
{
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );

    int size = -1;
    if ( data->m_type == wxJSONTYPE_ARRAY )  {
        size = (int) data->m_valArray.GetCount();
    }
    if ( data->m_type == wxJSONTYPE_OBJECT )  {
        size = (int) data->m_valMap.size();
    }
    return size;
}

//! Return the array of keys of this JSON object.
/*!
 If the stored value is a key/value map, the function returns an
 array of strings containing the \e key of all elements.
 Note that the returned array may be empty if the map has ZERO
 elements.
 An empty string array is also returned if the stored value is
 not a key/value map.
 Also note that in debug builds, the function wxJSON_ASSERTs that the
 type of the stored object is wxJSONTYPE_OBJECT.
*/
wxArrayString
wxJSONValue::GetMemberNames() const
{
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );
    wxJSON_ASSERT( data->m_type == wxJSONTYPE_OBJECT );

    wxArrayString arr;
    if ( data->m_type == wxJSONTYPE_OBJECT )   {
        wxJSONInternalMap::iterator it;
        for ( it = data->m_valMap.begin(); it != data->m_valMap.end(); it++ )  {
            arr.Add( it->first );
        }
    }
    return arr;
}


// appending items, resizing and deleting items
// NOTE: these functions are not 'const' so we have to call
// the COW() function before accessing data

//! Append the specified value in the array.
/*!
 The function appends the value specified in the parameter to the array
 contained in this object.
 If this object does not contain an array type, the actual content is
 deleted, a new array type is created and the JSON value \c value is
 appended to the newly created array.
 Returns a reference to the appended object.
*/
wxJSONValue&
wxJSONValue::Append( const wxJSONValue& value )
{
    wxJSONRefData* data = COW();
    wxJSON_ASSERT( data );
    if ( data->m_type != wxJSONTYPE_ARRAY )  {
        // we have to change the type of the actual object to the array type
        SetType( wxJSONTYPE_ARRAY );
    }
    // we add the wxJSONValue object to the wxObjArray: note that the
    // array makes a copy of the JSON-value object by calling its
    // copy ctor thus using reference count
    data->m_valArray.Add( value );
    wxJSONValue& v = data->m_valArray.Last();
    return v;
}


//! \overload Append( const wxJSONValue& )
wxJSONValue&
wxJSONValue::Append( int i )
{
    wxJSONValue v( i );
    wxJSONValue& r = Append( v );
    return r;
}

//! \overload Append( const wxJSONValue& )
wxJSONValue&
wxJSONValue::Append( short int i )
{
    wxJSONValue v( i );
    wxJSONValue& r = Append( v );
    return r;
}

//! \overload Append( const wxJSONValue& )
wxJSONValue&
wxJSONValue::Append( long int l )
{
    wxJSONValue v( l );
    wxJSONValue& r = Append( v );
    return r;
}

//! \overload Append( const wxJSONValue& )
wxJSONValue&
wxJSONValue::Append( bool b )
{
    wxJSONValue v( b );
    wxJSONValue& r = Append( v );
    return r;
}

//! \overload Append( const wxJSONValue& )
wxJSONValue&
wxJSONValue::Append( unsigned int ui )
{
    wxJSONValue v( ui );
    wxJSONValue& r = Append( v );
    return r;
}

//! \overload Append( const wxJSONValue& )
wxJSONValue&
wxJSONValue::Append( unsigned short ui )
{
    wxJSONValue v( ui );
    wxJSONValue& r = Append( v );
    return r;
}

//! \overload Append( const wxJSONValue& )
wxJSONValue&
wxJSONValue::Append( unsigned long ul )
{
    wxJSONValue v( ul );
    wxJSONValue& r = Append( v );
    return r;
}

//! \overload Append( const wxJSONValue& )
wxJSONValue&
wxJSONValue::Append( double d )
{
    wxJSONValue v( d );
    wxJSONValue& r = Append( v );
    return r;
}

//! \overload Append( const wxJSONValue& )
wxJSONValue&
wxJSONValue::Append( const wxChar* str )
{
    wxJSONValue v( str );
    wxJSONValue& r = Append( v );
    return r;
}

//! \overload Append( const wxJSONValue& )
wxJSONValue&
wxJSONValue::Append( const wxString& str )
{
    wxJSONValue v( str );
    wxJSONValue& r = Append( v );
    return r;
}

//! \overload Append( const wxJSONValue& )
wxJSONValue&
wxJSONValue::Append( const wxMemoryBuffer& buff )
{
    wxJSONValue v( buff );
    wxJSONValue& r = Append( v );
    return r;
}

//! \overload Append( const wxJSONValue& )
wxJSONValue&
wxJSONValue::Append( const void* buff, size_t len )
{
    wxJSONValue v( buff, len );
    wxJSONValue& r = Append( v );
    return r;
}


//! Concatenate a string to this string object.
/*!
 The function concatenates \c str to the string contained
 in this object and returns TRUE if the operation is succefull.
 If the value stored in this value is not a string object
 the function does nothing and returns FALSE.
 Note that in order to be successfull, the value must contain
 a \b wxString object and not a pointer to C-string.
*/
bool
wxJSONValue::Cat( const wxString& str )
{
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );

    bool r = false;
    if ( data->m_type == wxJSONTYPE_STRING )  {
        wxJSONRefData* data = COW();
        wxJSON_ASSERT( data );
        data->m_valString.append( str );
    r = true;
    }
    return r;
}

//! Concatenate a memory buffer to this memory buffer object.
/*!
 The function concatenates \c buff to the \b wxMemoryBuffer object contained
 in this object and returns TRUE if the operation is succefull.
 If the value stored in this value is not a memory buffer object
 the function does nothing and returns FALSE.
*/
bool
wxJSONValue::Cat( const wxMemoryBuffer& buff )
{
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );

    bool r = false;
    if ( data->m_type == wxJSONTYPE_MEMORYBUFF )  {
        wxJSONRefData* data = COW();
        wxJSON_ASSERT( data );
        data->m_memBuff->AppendData( buff.GetData(), buff.GetDataLen());
        r = true;
    }
    return r;
}


//! \overload Cat( const wxString& )
bool
wxJSONValue::Cat( const wxChar* str )
{
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );

    bool r = false;
    if ( data->m_type == wxJSONTYPE_STRING )  {
        wxJSONRefData* data = COW();
        wxJSON_ASSERT( data );
        data->m_valString.append( str );
        r = true;
    }
    return r;
}


//! Remove the item at the specified index or key.
/*!
 The function removes the item at index \c index or at the specified
 key in the array or map.
 If this object does not contain an array (for a index parameter) or a map
 (for a key parameter), the function does nothing and returns FALSE.
 If the element does not exist, FALSE is returned.
*/
bool
wxJSONValue::Remove( int index )
{
    wxJSONRefData* data = COW();
    wxJSON_ASSERT( data );

    bool r = false;
    if ( data->m_type == wxJSONTYPE_ARRAY )  {
        data->m_valArray.RemoveAt( index );
        r = true;
    }
    return r;
}


//! \overload Remove( int )
bool
wxJSONValue::Remove( const wxString& key )
{
    wxJSONRefData* data = COW();
    wxJSON_ASSERT( data );

    bool r = false;
    if ( data->m_type == wxJSONTYPE_OBJECT )  {
        wxJSONInternalMap::size_type count = data->m_valMap.erase( key );
        if ( count > 0 )  {
            r = true;
        }
    }
    return r;
}


//! Clear the object value.
/*!
 This function causes the object to be empty.
 The function simply calls UnRef() making this object to become
 invalid and set its type to wxJSONTYPE_INVALID.
*/
void
wxJSONValue::Clear()
{
    UnRef();
    SetType( wxJSONTYPE_INVALID );
}

// retrieve an item

//! Return the item at the specified index.
/*!
 The function returns a reference to the object at the specified
 index.
 If the element does not exist, the array is enlarged to \c index + 1
 elements and a reference to the last element will be returned.
 New elements will contain NULL values.
 If this object does not contain an array, the old value is
 replaced by an array object which will be enlarged to the needed
 dimension.
*/
wxJSONValue&
wxJSONValue::Item( unsigned index )
{
    wxJSONRefData* data = COW();
    wxJSON_ASSERT( data );

    if ( data->m_type != wxJSONTYPE_ARRAY )  {
        data = SetType( wxJSONTYPE_ARRAY );
    }
    int size = Size();
    wxJSON_ASSERT( size >= 0 );
    // if the desired element does not yet exist, we create as many
    // elements as needed; the new values will be 'null' values
    if ( index >= (unsigned) size )  {
        wxJSONValue v( wxJSONTYPE_NULL);
        int missing = index - size + 1;
        data->m_valArray.Add( v, missing );
    }
    return data->m_valArray.Item( index );
}

//! Return the item at the specified key.
/*!
 The function returns a reference to the object in the map
 that has the specified key.
 If \c key does not exist, a new NULL value is created with
 the provided key and a reference to it is returned.
 If this object does not contain a map, the old value is
 replaced by a map object.
*/
wxJSONValue&
wxJSONValue::Item( const wxString& key )
{
    wxLogTrace( traceMask, _T("(%s) searched key=\'%s\'"), __PRETTY_FUNCTION__, key.c_str());
#if !wxCHECK_VERSION(2,9,0)
    wxLogTrace( traceMask, _T("(%s) actual object: %s"), __PRETTY_FUNCTION__, GetInfo().c_str());
#endif
    
    wxJSONRefData* data = COW();
    wxJSON_ASSERT( data );

    if ( data->m_type != wxJSONTYPE_OBJECT )  {
        // deletes the contained value;
        data = SetType( wxJSONTYPE_OBJECT );
        return data->m_valMap[key];
    }
    wxLogTrace( traceMask, _T("(%s) searching key \'%s' in the actual object"),
                 __PRETTY_FUNCTION__, key.c_str() );
    return data->m_valMap[key];
}


//! Return the item at the specified index.
/*!
 The function returns a copy of the object at the specified
 index.
 If the element does not exist, the function returns an \b invalid value.
*/
wxJSONValue
wxJSONValue::ItemAt( unsigned index ) const
{
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );

    wxJSONValue v( wxJSONTYPE_INVALID );
    if ( data->m_type == wxJSONTYPE_ARRAY )  {
        int size = Size();
        wxJSON_ASSERT( size >= 0 );
        if ( index < (unsigned) size )  {
            v = data->m_valArray.Item( index );
        }
    }
    return v;
}

//! Return the item at the specified key.
/*!
 The function returns a copy of the object in the map
 that has the specified key.
 If \c key does not exist, an \b invalid value is returned.
*/
wxJSONValue
wxJSONValue::ItemAt( const wxString& key ) const
{
    wxLogTrace( traceMask, _T("(%s) searched key=\'%s\'"), __PRETTY_FUNCTION__, key.c_str());
#ifndef __WXOSX__
    wxLogTrace( traceMask, _T("(%s) actual object: %s"), __PRETTY_FUNCTION__, GetInfo().c_str());
#endif
    
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );

    wxJSONValue v( wxJSONTYPE_INVALID );
    if ( data->m_type == wxJSONTYPE_OBJECT )  {
        wxJSONInternalMap::const_iterator it = data->m_valMap.find( key );
        if ( it != data->m_valMap.end() )  {
            v = it->second;
        }
    }
    return v;
}


//! Return the item at the specified index.
/*!
 The function returns a reference to the object at the specified
 index.
 If the element does not exist, the array is enlarged to \c index + 1
 elements and a reference to the last element will be returned.
 New elements will contain NULL values.
 If this object does not contain an array, the old value is
 replaced by an array object.
*/
wxJSONValue&
wxJSONValue::operator [] ( unsigned index )
{
    wxJSONValue& v = Item( index );
    return v;
}

//! Return the item at the specified key.
/*!
 The function returns a reference to the object in the map
 that has the specified key.
 If \c key does not exist, a new NULL value is created with
 the provided key and a reference to it is returned.
 If this object does not contain a map, the old value is
 replaced by a map object.
*/
wxJSONValue&
wxJSONValue::operator [] ( const wxString& key )
{
    wxJSONValue& v = Item( key );
    return v;
}

//
// assignment operators
// note that reference counting is only used if the original
// value is a wxJSONValue object
// in all other cases, the operator= function deletes the old
// content and assigns the new one


//! Assign the specified value to this object replacing the old value.
/*!
 The assignment operator assigns to this object the value specified in the
 right operand of the assignment operator.
 Note that the old value is deleted but not the other data members
 in the wxJSONRefData structure.
 This is particularly usefull for the parser class which stores
 comment lines in a temporary wxJSONvalue object that is of type
 wxJSONTYPE_INVALID.
 As comment lines may apear before the value they refer to, comments
 are stored in a value that is not yet being read.
 when the value is read, it is assigned to the temporary JSON value
 object without deleting the comment lines.
*/
wxJSONValue&
wxJSONValue::operator = ( int i )
{
    wxJSONRefData* data = SetType( wxJSONTYPE_INT );
    data->m_value.VAL_INT = i;
    return *this;
}


//! \overload operator = (int)
wxJSONValue&
wxJSONValue::operator = ( bool b )
{
    wxJSONRefData* data = SetType( wxJSONTYPE_BOOL );
    data->m_value.m_valBool = b;
    return *this;
}

//! \overload operator = (int)
wxJSONValue&
wxJSONValue::operator = ( unsigned int ui )
{
    wxJSONRefData* data = SetType( wxJSONTYPE_UINT );
    data->m_value.VAL_UINT = ui;
    return *this;
}

//! \overload operator = (int)
wxJSONValue&
wxJSONValue::operator = ( long l )
{
    wxJSONRefData* data = SetType( wxJSONTYPE_INT );
    data->m_value.VAL_INT = l;
    return *this;
}

//! \overload operator = (int)
wxJSONValue&
wxJSONValue::operator = ( unsigned  long ul )
{
    wxJSONRefData* data = SetType( wxJSONTYPE_UINT );
    data->m_value.VAL_UINT = ul;
    return *this;
}


//! \overload operator = (int)
wxJSONValue&
wxJSONValue::operator = ( short i )
{
    wxJSONRefData* data = SetType( wxJSONTYPE_INT );
    data->m_value.VAL_INT = i;
    return *this;
}


//! \overload operator = (int)
wxJSONValue&
wxJSONValue::operator = ( unsigned short ui )
{
    wxJSONRefData* data = SetType( wxJSONTYPE_UINT );
    data->m_value.VAL_UINT = ui;
    return *this;
}

//! \overload operator = (int)
wxJSONValue&
wxJSONValue::operator = ( double d )
{
    wxJSONRefData* data = SetType( wxJSONTYPE_DOUBLE );
    data->m_value.m_valDouble = d;
    return *this;
}


//! \overload operator = (int)
wxJSONValue&
wxJSONValue::operator = ( const wxChar* str )
{
    wxJSONRefData* data = SetType( wxJSONTYPE_CSTRING );
    data->m_value.m_valCString = str;
#if !defined( WXJSON_USE_CSTRING )
    data->m_type = wxJSONTYPE_STRING;
    data->m_valString.assign( str );
#endif
    return *this;
}

//! \overload operator = (int)
wxJSONValue&
wxJSONValue::operator = ( const wxString& str )
{
    wxJSONRefData* data = SetType( wxJSONTYPE_STRING );
    data->m_valString.assign( str );
    return *this;
}


//! Assigns to this object a memory buffer type
/*!
 As with the ctor, this function makes a deep copy of the
 memory buffer \c buff so changes made to the original buffer
 does not reflect to the memory buffer stored in this JSON value.
*/
wxJSONValue&
wxJSONValue::operator = ( const wxMemoryBuffer& buff )
{
    wxJSONRefData* data = SetType( wxJSONTYPE_MEMORYBUFF );
    data->m_memBuff = new wxMemoryBuffer();
    const void* ptr = buff.GetData();
    size_t      len = buff.GetDataLen();
    if ( data->m_memBuff && len )  {
        data->m_memBuff->AppendData( ptr, len );
    }
    return *this;
}


//! Assignment operator using reference counting.
/*!
 Unlike all other assignment operators, this one makes a
 swallow copy of the other JSON value object.
 The function calls \c Ref() to get a shared referenced
 data.
 \sa \ref json_internals_cow
*/
wxJSONValue&
wxJSONValue::operator = ( const wxJSONValue& other )
{
    Ref( other );
    return *this;
}


// finding elements


//! Return a value or a default value.
/*!
 This function returns a copy of the value object for the specified key.
 If the key is not found, a copy of \c defaultValue is returned.
 Note that the returned values are not real copy of the \c key or the
 default values because \e copy-on-write is used by this class.
 However, you have to treat them as real copies; in other words, if you
 change the values of the returned object your changes does not reflect
 in the otiginal value.
 Example:
 \code
  wxJSONValue defaultValue( 0 );
  wxJSONvalue v1;
  v1["key"] = 100;   // 'v1["key"]' contains the integer 100

  // 'v2' contains 100 but it is a swallow copy of 'v1["key"]'
  wxJSONValue v2 = v1.Get( "key", defaultValue );

  // 'v1["key"]' still contains 100
  v2 = 200;

  // if you want your change to be reflected in the 'v1' object
  // you have to assign it
  v1["key"] = v2;
 \endcode
*/
wxJSONValue
wxJSONValue::Get( const wxString& key, const wxJSONValue& defaultValue ) const
{
    // NOTE: this function does many wxJSONValue copies.
    // so implementing COW is a good thing

    // this is the first copy (the default value)
    wxJSONValue v( defaultValue );

    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );
    if ( data->m_type == wxJSONTYPE_OBJECT )  {
        wxJSONInternalMap::iterator it = data->m_valMap.find( key );
        if ( it != data->m_valMap.end() )  {
            v = it->second;
        }
    }
    return v;
}


// protected functions

//! Find an element
/*!
 The function returns a pointer to the element at index \c index
 or a NULL pointer if \c index does not exist.
 A NULL pointer is also returned if the object does not contain an
 array nor a key/value map.
*/
wxJSONValue*
wxJSONValue::Find( unsigned index ) const
{
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );

    wxJSONValue* vp = 0;

    if ( data->m_type == wxJSONTYPE_ARRAY )  {
        size_t size = data->m_valArray.GetCount();
        if ( index < size )  {
            vp = &(data->m_valArray.Item( index ));
        }
    }
    return vp;
}

//! Find an element
/*!
 The function returns a pointer to the element with key \c key
 or a NULL pointer if \c key does not exist.
 A NULL pointer is also returned if the object does not contain a
 key/value map.
*/
wxJSONValue*
wxJSONValue::Find( const wxString& key ) const
{
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );

    wxJSONValue* vp = 0;

    if ( data->m_type == wxJSONTYPE_OBJECT )  {
        wxJSONInternalMap::iterator it = data->m_valMap.find( key );
        if ( it != data->m_valMap.end() )  {
            vp = &(it->second);
        }
    }
    return vp;
}



//! Return a string description of the type
/*!
 This static function is only usefull for debugging purposes and
 should not be used by users of this class.
 It simply returns a string representation of the JSON value
 type stored in a object.
 For example, if \c type is wxJSONTYPE_INT the function returns the
 string "wxJSONTYPE_INT".
 If \c type is out of range, an empty string is returned (should
 never happen).
*/
wxString
wxJSONValue::TypeToString( wxJSONType type )
{
  static const wxChar* str[] = {
    _T( "wxJSONTYPE_INVALID" ),   // 0
    _T( "wxJSONTYPE_NULL" ),    // 1
    _T( "wxJSONTYPE_INT" ),     // 2
    _T( "wxJSONTYPE_UINT" ),    // 3
    _T( "wxJSONTYPE_DOUBLE" ),  // 4
    _T( "wxJSONTYPE_STRING" ),  // 5
    _T( "wxJSONTYPE_CSTRING" ), // 6
    _T( "wxJSONTYPE_BOOL" ),    // 7
    _T( "wxJSONTYPE_ARRAY" ),   // 8
    _T( "wxJSONTYPE_OBJECT" ),  // 9
    _T( "wxJSONTYPE_LONG" ),    // 10
    _T( "wxJSONTYPE_INT64" ),   // 11
    _T( "wxJSONTYPE_ULONG" ),   // 12
    _T( "wxJSONTYPE_UINT64" ),  // 13
    _T( "wxJSONTYPE_SHORT" ),   // 14
    _T( "wxJSONTYPE_USHORT" ),  // 15
    _T( "wxJSONTYPE_MEMORYBUFF" ),  // 16
  };

    wxString s;
    int idx = (int) type;
    if ( idx >= 0 && idx < 17 )  {
        s = str[idx];
    }
    return s;
}

//! Returns informations about the object
/*!
 The function is only usefull for debugging purposes and will probably
 be dropped in future versions.
 Returns a string that contains info about the object such as:

 \li the type of the object
 \li the size
 \li the progressive counter
 \li the pointer to referenced data
 \li the progressive counter of referenced data
 \li the number of share of referenced data

The \c deep parameter is used to specify if the function will be called
recursively in order to dump sub-items. If the parameter is TRUE than a
deep dump is executed.

The \c indent is the initial indentation: it is incremented by 3 every
time the Dump() function is called recursively.
*/
wxString
wxJSONValue::Dump( bool deep, int indent ) const
{
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );

    wxJSONType type = GetType();

    wxString s;
    if ( indent > 0 )   {
        s.append( indent, ' ' );
    }

    wxString s1;
    wxString s2;
#if defined( WXJSON_USE_VALUE_COUNTER )
    s1.Printf( _T("Object: Progr=%d Type=%s Size=%d comments=%d\n"),
        m_progr,
        TypeToString( type ).c_str(),
        Size(),
        data->m_comments.GetCount() );
  s2.Printf(_T("      : RefData=%p Progr=%d Num shares=%d\n"),
            data, data->m_progr, data->GetRefCount() );
#else
  s1.Printf( _T("Object: Type=%s Size=%d comments=%d\n"),
            TypeToString( type ).c_str(),
            Size(),
            data->m_comments.GetCount() );
  s2.Printf(_T("      : RefData=%p Num shares=%d\n"),
            data, data->GetRefCount() );
#endif
  s.append( s1 );
  if ( indent > 0 )   {
    s.append( indent, ' ' );
  }
  s.append( s2 );

  wxString sub;

  // if we have to do a deep dump, we call the Dump() function for
  // every sub-item
  if ( deep )   {
    indent += 3;
    const wxJSONInternalMap* map;
    int size;;
    wxJSONInternalMap::const_iterator it;
    switch ( type )    {
        case wxJSONTYPE_OBJECT :
            map = AsMap();
            size = Size();
            for ( it = map->begin(); it != map->end(); ++it )  {
                const wxJSONValue& v = it->second;
                sub = v.Dump( true, indent );
                s.append( sub );
            }
        break;
        case wxJSONTYPE_ARRAY :
            size = Size();
            for ( int i = 0; i < size; i++ )  {
                const wxJSONValue* v = Find( i );
                wxJSON_ASSERT( v );
                sub = v->Dump( true, indent );
                s.append( sub );
            }
            break;
        default :
            break;
        }
    }
    return s;
}

//! Returns informations about the object
/*!
 The function is only usefull for debugging purposes and will probably
 be dropped in future versions.
 You should not rely on this function to exist in future versions.
*/
wxString
wxJSONValue::GetInfo() const
{
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );

    wxString s;
#if defined( WXJSON_USE_VALUE_CONTER )
    s.Printf( _T("Object: Progr=%d Type=%s Size=%d comments=%d\n"),
            data->m_progr,
            wxJSONValue::TypeToString( data->m_type ).c_str(),
            Size(),
            data->m_comments.GetCount() );
#else
    s.Printf( _T("Object: Type=%s Size=%d comments=%d\n"),
            wxJSONValue::TypeToString( data->m_type ).c_str(),
            Size(), data->m_comments.GetCount() );
#endif
    if ( data->m_type == wxJSONTYPE_OBJECT ) {
        wxArrayString arr = GetMemberNames();
        for ( unsigned int i = 0; i < arr.size(); i++ )  {
            s.append( _T("    Member name: "));
            s.append( arr[i] );
            s.append( _T("\n") );
        }
    }
    return s;
}

//! The comparison function
/*!
 This function returns TRUE if this object looks like \c other.
 Note that this class does not define a comparison operator
 (the classical \b operator== function) because the notion
 of \b equal for JSON values objects is not applicable.
 The comment strings array are not compared: JSON value objects
 are \b the \b same if they contains the same values, regardless the
 comment's strings.

 Note that the function does not return the element that cause the
 comparison to return FALSE. There is not a good structure to
 tell this information.
 If you need it for debugging purposes, you have to turn on the
 \b sameas tracing feature by setting the WXTRACE environment
 variable (you need a debug version of the application):

 \code
   export WXTRACE=sameas     // for unix systems that use bash
 \endcode

 Note that if the two JSON value objects share the same referenced
 data, the function immediatly returns TRUE without doing a deep
 comparison which is, sure, useless.
 For further info see \ref json_internals_compare.
*/
bool
wxJSONValue::IsSameAs( const wxJSONValue& other ) const
{
    // this is a recursive function: it calls itself
    // for every 'value' object in an array or map
    bool r = false;

    // some variables used in the switch statement
    int size;
    wxJSONInternalMap::const_iterator it;

    // get the referenced data for the two objects
    wxJSONRefData* data = GetRefData();
    wxJSONRefData* otherData = other.GetRefData();

    if ( data == otherData ) {
        wxLogTrace( compareTraceMask, _T("(%s) objects share the same referenced data - r=TRUE"),
             __PRETTY_FUNCTION__ );
    return true;
    }


    // if the type does not match the function compares the values if
    // they are of compatible types such as INT, UINT and DOUBLE
    if ( data->m_type != otherData->m_type )  {
        // if the types are not compatible, returns false
        // otherwise compares the compatible types: INT, UINT and DOUBLE
        double val;
        switch ( data->m_type )  {
            case wxJSONTYPE_INT :
                if ( otherData->m_type == wxJSONTYPE_UINT )    {
                    // compare the bits and returns true if value is between 0 and LLONG_MAX
                    if ( (data->m_value.VAL_UINT <= LLONG_MAX ) &&
                            (data->m_value.VAL_UINT == otherData->m_value.VAL_UINT))
                        {
                                r = true;
                        }
                }
                else if ( otherData->m_type == wxJSONTYPE_DOUBLE )    {
                    val = data->m_value.VAL_INT;
                    if ( val == otherData->m_value.m_valDouble )    {
                        r = true;
                    }
                }
                else    {
                    r = false;
                }
                break;
            case wxJSONTYPE_UINT :
                if ( otherData->m_type == wxJSONTYPE_INT )    {
                    // compare the bits and returns true if value is between 0 and LLONG_MAX
                    if ( (data->m_value.VAL_UINT <= LLONG_MAX ) &&
                            (data->m_value.VAL_UINT == otherData->m_value.VAL_UINT))
                        {
                            r = true;
                        }
                }
                else if ( otherData->m_type == wxJSONTYPE_DOUBLE )    {
                    val = data->m_value.VAL_UINT;
                    if ( val == otherData->m_value.m_valDouble )    {
                        r = true;
                    }
                }
                else    {
                    r = false;
                }
                break;
            case wxJSONTYPE_DOUBLE :
                if ( otherData->m_type == wxJSONTYPE_INT )    {
                    val = otherData->m_value.VAL_INT;
                    if ( val == data->m_value.m_valDouble )    {
                        r = true;
                    }
                }
                else if ( otherData->m_type == wxJSONTYPE_UINT )    {
                    val = otherData->m_value.VAL_UINT;
                    if ( val == data->m_value.m_valDouble )    {
                        r = true;
                    }
                }
                else    {
                    r = false;
                }
                break;
            default:
                r = false;
            break;
        }
        return r;
    }

    // the two objects have the same 'm_type'

    // for comparing wxJSONTYPE_CSTRING we use two temporary wxString
    // objects: this is to avoid using strcmp() and wcscmp() which
    // may not be available on all platforms
    wxString s1, s2;
    r = true;
    int r1;

    switch ( data->m_type )  {
        case wxJSONTYPE_INVALID :
        case wxJSONTYPE_NULL :
            // there is no need to compare the values
            break;
        case wxJSONTYPE_INT :
            if ( data->m_value.VAL_INT != otherData->m_value.VAL_INT )  {
                r = false;
            }
            break;
        case wxJSONTYPE_UINT :
            if ( data->m_value.VAL_UINT != otherData->m_value.VAL_UINT )  {
                r = false;
            }
            break;
        case wxJSONTYPE_DOUBLE :
            if ( data->m_value.m_valDouble != otherData->m_value.m_valDouble )  {
                r = false;
            }
            break;
        case wxJSONTYPE_CSTRING :
            s1 = wxString( data->m_value.m_valCString );
            s2 = wxString( otherData->m_value.m_valCString );
            if ( s1 != s2 )  {
                r = false;
            }
            break;
        case wxJSONTYPE_BOOL :
            if ( data->m_value.m_valBool != otherData->m_value.m_valBool )  {
                r = false;
            }
            break;
        case wxJSONTYPE_STRING :
            if ( data->m_valString != otherData->m_valString )  {
                r = false;
            }
            break;
        case wxJSONTYPE_MEMORYBUFF :
            // we cannot simply use the operator ==; we need a deep comparison
            r1 = CompareMemoryBuff( *(data->m_memBuff), *(otherData->m_memBuff));
            if ( r1 != 0 )   {
                r = false;
            }
            break;
        case wxJSONTYPE_ARRAY :
            size = Size();
            wxLogTrace( compareTraceMask, _T("(%s) Comparing an array object - size=%d"),
                    __PRETTY_FUNCTION__, size );

            if ( size != other.Size() )  {
                wxLogTrace( compareTraceMask, _T("(%s) Sizes does not match"),
                        __PRETTY_FUNCTION__ );
                return false;
            }
            // compares every element in this object with the element of
            // the same index in the 'other' object
            for ( int i = 0; i < size; i++ )  {
                wxLogTrace( compareTraceMask, _T("(%s) Comparing array element=%d"),
                        __PRETTY_FUNCTION__, i );
                wxJSONValue v1 = ItemAt( i );
                wxJSONValue v2 = other.ItemAt( i );

                if ( !v1.IsSameAs( v2 ))  {
                    return false;
                }
            }
            break;
        case wxJSONTYPE_OBJECT :
            size = Size();
            wxLogTrace( compareTraceMask, _T("(%s) Comparing a map obejct - size=%d"),
                        __PRETTY_FUNCTION__, size );

            if ( size != other.Size() )  {
                wxLogTrace( compareTraceMask, _T("(%s) Comparison failed - sizes does not match"),
                                __PRETTY_FUNCTION__ );
                return false;
            }
            // for every key calls itself on the value found in
            // the other object. if 'key' does no exist, returns FALSE
            for ( it = data->m_valMap.begin(); it != data->m_valMap.end(); it++ )  {
                wxString key = it->first;
                wxLogTrace( compareTraceMask, _T("(%s) Comparing map object - key=%s"),
                                __PRETTY_FUNCTION__, key.c_str() );
                wxJSONValue otherVal = other.ItemAt( key );
                bool isSame = it->second.IsSameAs( otherVal );
                if ( !isSame )  {
                    wxLogTrace( compareTraceMask, _T("(%s) Comparison failed for the last object"),
                                    __PRETTY_FUNCTION__ );
                    return false;
                }
            }
            break;
        default :
            // should never happen
            wxFAIL_MSG( _T("wxJSONValue::IsSameAs() unexpected wxJSONType"));
            break;
    }
    return r;
}

//! Add a comment to this JSON value object.
/*!
 The function adds a comment string to this JSON value object and returns
 the total number of comment strings belonging to this value.
 Note that the comment string must be a valid C/C++ comment because the
 wxJSONWriter does not modify it.
 In other words, a C++ comment string must start with '//' and must end with
 a new-line character. If the final LF char is missing, the
 automatically adds it.
 You can also add C-style comments which must be enclosed in the usual
 C-comment characters.
 For C-style comments, the function does not try to append the final comment
 characters but allows trailing whitespaces and new-line chars.
 The \c position parameter is one of:

 \li wxJSONVALUE_COMMENT_BEFORE: the comment will be written before the value
 \li wxJSONVALUE_COMMENT_INLINE: the comment will be written on the same line
 \li wxJSONVALUE_COMMENT_AFTER: the comment will be written after the value
 \li wxJSONVALUE_COMMENT_DEFAULT: the old value of comment's position is not
    changed; if no comments were added to the value object this is the
    same as wxJSONVALUE_COMMENT_BEFORE.

 To know more about comment's storage see \ref json_comment_add

*/
int
wxJSONValue::AddComment( const wxString& str, int position )
{
    wxJSONRefData* data = COW();
    wxJSON_ASSERT( data );

    wxLogTrace( traceMask, _T("(%s) comment=%s"), __PRETTY_FUNCTION__, str.c_str() );
    int r = -1;
    int len = str.length();
    if ( len < 2 )  {
        wxLogTrace( traceMask, _T("     error: len < 2") );
        return -1;
    }
    if ( str[0] != '/' )  {
        wxLogTrace( traceMask, _T("     error: does not start with\'/\'") );
        return -1;
    }
    if ( str[1] == '/' )  {       // a C++ comment: check that it ends with '\n'
        wxLogTrace( traceMask, _T("     C++ comment" ));
        if ( str.GetChar(len - 1) != '\n' )  {
            wxString temp( str );
            temp.append( 1, '\n' );
            data->m_comments.Add( temp );
            wxLogTrace( traceMask, _T("     C++ comment: LF added") );
        }
        else  {
            data->m_comments.Add( str );
        }
        r = data->m_comments.size();
    }
    else if ( str[1] == '*' )  {  // a C-style comment: check that it ends with '*/'
        wxLogTrace( traceMask, _T("     C-style comment") );
        int lastPos = len - 1;
        wxChar ch = str.GetChar( lastPos );
        // skip leading whitespaces
        while ( ch == ' ' || ch == '\n' || ch == '\t' )  {
            --lastPos;
            ch = str.GetChar( lastPos );
        }
        if ( str.GetChar( lastPos ) == '/' &&  str.GetChar( lastPos - 1 ) == '*' ) {
            data->m_comments.Add( str );
            r = data->m_comments.size();
        }
    }
    else  {
        wxLogTrace( traceMask, _T("     error: is not a valid comment string") );
        r = -1;
    }
    // if the comment was stored, store the position
    if ( r >= 0 && position != wxJSONVALUE_COMMENT_DEFAULT )  {
        data->m_commentPos = position;
    }
    return r;
}

//! Add one or more comments to this JSON value object.
/*!
 The function adds the strings contained in \c comments to the comment's
 string array of this value object by calling the AddComment( const wxString&,int)
 function for every string in the \c comment array.
 Returns the number of strings correctly added.
*/
int
wxJSONValue::AddComment( const wxArrayString& comments, int position )
{
    int siz = comments.GetCount(); int r = 0;
    for ( int i = 0; i < siz; i++ ) {
        int r2 = AddComment( comments[i], position );
        if ( r2 >= 0 )  {
            ++r;
        }
    }
    return r;
}

//! Return a comment string.
/*!
 The function returns the comment string at index \c idx.
 If \c idx is out of range, an empty string is returned.
 If \c idx is equal to -1, then the function returns a string
 that contains all comment's strings stored in the array.
*/
wxString
wxJSONValue::GetComment( int idx ) const
{
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );

    wxString s;
    int size = data->m_comments.GetCount();
    if ( idx < 0 )  {
        for ( int i = 0; i < size; i++ )  {
            s.append( data->m_comments[i] );
        }
    }
    else if ( idx < size )  {
        s = data->m_comments[idx];
    }
    return s;
}

//! Return the number of comment strings.
int
wxJSONValue::GetCommentCount() const
{
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );

    int d = data->m_comments.GetCount();
    wxLogTrace( traceMask, _T("(%s) comment count=%d"), __PRETTY_FUNCTION__, d );
    return d;
}

//! Return the comment position.
int
wxJSONValue::GetCommentPos() const
{
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );
    return data->m_commentPos;
}

//! Get the comment string's array.
const wxArrayString&
wxJSONValue::GetCommentArray() const
{
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );

    return data->m_comments;
}

//! Clear all comment strings
void
wxJSONValue::ClearComments()
{
    wxJSONRefData* data = COW();
    wxJSON_ASSERT( data );

    data->m_comments.clear();
}


//! Set the type of the stored value.
/*!
 The function sets the type of the stored value as specified in
 the provided argument.
 If the actual type is equal to \c type, nothing happens and this
 JSON value object retains the original type and value.
 If the type differs, however, the original type and value are
 lost.

 The function just sets the type of the object and not the
 value itself.
 If the object does not have a data structure it is allocated
 using the CreateRefData() function unless the type to be set
 is wxJSONTYPE_INVALID. In this case and if a data structure is
 not yet allocated, it is not allocated.

 If the object already contains a data structure it is not deleted
 but the type is changed in the original data structure.
 Complex values in the old structure are cleared.
 The \c type argument can be one of the following:

  \li wxJSONTYPE_INVALID: an empty (not initialized) JSON value
  \li wxJSONTYPE_NULL: a NULL value
  \li wxJSONTYPE_INT: an integer value
  \li wxJSONTYPE_UINT: an unsigned integer
  \li wxJSONTYPE_DOUBLE: a double precision number
  \li wxJSONTYPE_BOOL: a boolean
  \li wxJSONTYPE_CSTRING: a C string
  \li wxJSONTYPE_STRING: a wxString object
  \li wxJSONTYPE_ARRAY: an array of wxJSONValue objects
  \li wxJSONTYPE_OBJECT: a hashmap of key/value pairs where \e value is a wxJSONValue object
  \li wxJSONTYPE_LONG: a 32-bits integer value
  \li wxJSONTYPE_ULONG: an unsigned 32-bits integer
  \li wxJSONTYPE_INT64: a 64-bits integer value
  \li wxJSONTYPE_UINT64: an unsigned 64-bits integer
  \li wxJSONTYPE_SHORT: a signed short integer
  \li wxJSONTYPE_USHORT: an unsigned short integer
  \li wxJSONTYPE_MEMORYBUFF: a binary memory buffer

 The integer storage depends on the platform: for platforms that support 64-bits
 integers, integers are always stored as 64-bits integers.
 On platforms that do not support 64-bits integers, ints are stored as \b long \b int.
 To know more about the internal representation of integers, read
 \ref json_internals_integer.

 Note that there is no need to set a type for the object in order to assign
 a value to it.
 In other words, if you want to construct a JSON value which holds an integer
 value of 10, just use the specific constructor:
 \code
   wxJSONValue value( 10 );
 \endcode
 which sets the integer type and also the numeric value.
 Moreover, there is no need to set the type for none of the handled types,
 not only for primitive types but for complex types, too.
 For example, if you want to construct an array of JSON values, just use
 the default ctor and call the Append() member function which will append the
 first element to the array and will set the array type:
 \code
   wxJSONValue value;
   value.Append( "a string" );
 \endcode
 \sa GetType
*/
wxJSONRefData*
wxJSONValue::SetType( wxJSONType type )
{
    wxJSONRefData* data = GetRefData();
    wxJSONType oldType = GetType();

    // check that type is within the allowed range
    wxJSON_ASSERT((type >= wxJSONTYPE_INVALID) && (type <= wxJSONTYPE_MEMORYBUFF));
    if ( (type < wxJSONTYPE_INVALID) || (type > wxJSONTYPE_MEMORYBUFF) )  {
        type = wxJSONTYPE_INVALID;
    }

    // the function unshares the referenced data but does not delete the
    // structure. This is because the wxJSON reader stores comments
    // that apear before the value in a temporary value of type wxJSONTYPE_INVALID
    // which is invalid and, next, it stores the JSON value in the same
    // wxJSONValue object.
    // If we would delete the structure using 'Unref()' we loose the
    // comments
    data = COW();

    // do nothing if the actual type is the same as 'type'
    if ( type == oldType )  {
        return data;
    }

    // change the type of the referened structure
    // NOTE: integer types are always stored as the generic integer types
    if ( type == wxJSONTYPE_LONG || type == wxJSONTYPE_INT64 || type == wxJSONTYPE_SHORT )  {
        type = wxJSONTYPE_INT;
    }
    if ( type == wxJSONTYPE_ULONG || type == wxJSONTYPE_UINT64 || type == wxJSONTYPE_USHORT )  {
        type = wxJSONTYPE_UINT;
    }

    wxJSON_ASSERT( data );
    data->m_type = type;

    // clears complex objects of the old type
    switch ( oldType )  {
        case wxJSONTYPE_STRING:
            data->m_valString.clear();
            break;
        case wxJSONTYPE_ARRAY:
            data->m_valArray.Clear();
            break;
        case wxJSONTYPE_OBJECT:
            data->m_valMap.clear();
            break;
        case wxJSONTYPE_MEMORYBUFF:
            // we first have to delete the actual memory buffer, if any
            if ( data->m_memBuff )  {
                delete data->m_memBuff;
                data->m_memBuff = 0;
            }
            break;
        default :
            // there is not need to clear primitive types
            break;
    }

    // if the WXJSON_USE_CSTRING macro is not defined, the class forces
    // C-string to be stored as wxString objects
#if !defined( WXJSON_USE_CSTRING )
    if ( data->m_type == wxJSONTYPE_CSTRING )  {
        data->m_type = wxJSONTYPE_STRING;
    }
#endif
    return data;
}

//! Return the line number of this JSON value object
/*!
 The line number of a JSON value object is set to -1 when the
 object is constructed.
 The line number is set by the parser class, wxJSONReader, when
 a JSON text is read from a stream or a string.
 it is used when reading a comment line: comment lines that apear
 on the same line as a value are considered \b inline comments of
 the value.
*/
int
wxJSONValue::GetLineNo() const
{
    // return ZERO if there is not a referenced data structure
    int n = 0;
    wxJSONRefData* data = GetRefData();
    if ( data != 0 ) {
        n = data->m_lineNo;
    }
    return n;
}

//! Set the line number of this JSON value object.
void
wxJSONValue::SetLineNo( int num )
{
    wxJSONRefData* data = COW();
    wxJSON_ASSERT( data );
    data->m_lineNo = num;
}

//! Set the pointer to the referenced data.
void
wxJSONValue::SetRefData(wxJSONRefData* data)
{
    m_refData = data;
}

//! Increments the referenced data counter.
void
wxJSONValue::Ref(const wxJSONValue& clone)
{
    // nothing to be done
    if (m_refData == clone.m_refData)
        return;

    // delete reference to old data
    UnRef();

    // reference new data
    if ( clone.m_refData )    {
        m_refData = clone.m_refData;
        ++(m_refData->m_refCount);
    }
}

//! Unreferences the shared data
/*!
 The function decrements the number of shares in wxJSONRefData::m_refCount
 and if it is ZERO, deletes the referenced data.
 It is called by the destructor and by the copy-on-write functions.
*/
void
wxJSONValue::UnRef()
{
    if ( m_refData )   {
        wxASSERT_MSG( m_refData->m_refCount > 0, _T("invalid ref data count") );

        if ( --m_refData->m_refCount == 0 )    {
            delete m_refData;
            m_refData = NULL;
        }
    }
}

//! Makes an exclusive copy of shared data
void
wxJSONValue::UnShare()
{
    AllocExclusive();
}


//! Do a deep copy of the other object.
/*!
 This function allocates a new ref-data structure and copies it
 from the object \c other.
*/
void
wxJSONValue::DeepCopy( const wxJSONValue& other )
{
    UnRef();
    wxJSONRefData* data = CloneRefData( other.m_refData );
    SetRefData( data );
}

//! Return the pointer to the referenced data structure.
wxJSONRefData*
wxJSONValue::GetRefData() const
{
    wxJSONRefData* data = m_refData;
    return data;
}


//! Make a copy of the referenced data.
/*!
 The function allocates a new instance of the wxJSONRefData
 structure, copies the content of \c other and returns the pointer
 to the newly created structure.
 This function is called by the wxObject::UnRef() function
 when a non-const member function is called on multiple
 referenced data.
*/
wxJSONRefData*
wxJSONValue::CloneRefData( const wxJSONRefData* otherData ) const
{
    wxJSON_ASSERT( otherData );

    // make a static cast to pointer-to-wxJSONRefData
    const wxJSONRefData* other = otherData;

    // allocate a new instance of wxJSONRefData using the default
    // ctor; we cannot use the copy ctor of a wxJSONRefData
    wxJSONRefData* data = new wxJSONRefData();

    // copy the referenced data structure's data members
    data->m_type       = other->m_type;
    data->m_value      = other->m_value;
    data->m_commentPos = other->m_commentPos;
    data->m_comments   = other->m_comments;
    data->m_lineNo     = other->m_lineNo;
    data->m_valString  = other->m_valString;
    data->m_valArray   = other->m_valArray;
    data->m_valMap     = other->m_valMap;

    // if the data contains a wxMemoryBuffer object, then we have
    // to make a deep copy of the buffer by allocating a new one because
    // wxMemoryBuffer is not a copy-on-write structure
    if ( other->m_memBuff ) {
        data->m_memBuff = new wxMemoryBuffer();
        const void* ptr = data->m_memBuff->GetData();
        size_t len      = data->m_memBuff->GetDataLen();
        if ( data->m_memBuff && len )   {
            data->m_memBuff->AppendData( ptr, len );
        }
    }

    wxLogTrace( cowTraceMask, _T("(%s) CloneRefData() PROGR: other=%d data=%d"),
            __PRETTY_FUNCTION__, other->GetRefCount(), data->GetRefCount() );

    return data;
}

//! Create a new data structure
/*!
 The function allocates a new instance of the wxJSONRefData
 structure and returns its pointer.
 The type of the JSON value is set to wxJSONTYPE_INVALID (=
 a not initialized value).
*/
wxJSONRefData*
wxJSONValue::CreateRefData() const
{
    wxJSONRefData* data = new wxJSONRefData();
    data->m_type = wxJSONTYPE_INVALID;
    return data;
}



//! Make sure the referenced data is unique
/*!
 This function is called by all non-const member functions and makes
 sure that the referenced data is unique by calling \b UnShare()
 If the referenced data is shared acrosss other wxJSONValue instances,
 the \c UnShare() function makes a private copy of the shared data.
*/
wxJSONRefData*
wxJSONValue::COW()
{
    wxJSONRefData* data = GetRefData();
    wxLogTrace( cowTraceMask, _T("(%s) COW() START data=%p data->m_count=%d"),
             __PRETTY_FUNCTION__, data, data->GetRefCount());
    UnShare();
    data = GetRefData();
    wxLogTrace( cowTraceMask, _T("(%s) COW() END data=%p data->m_count=%d"),
             __PRETTY_FUNCTION__, data, data->GetRefCount());
    return GetRefData();
}

//! Makes a private copy of the referenced data
void
wxJSONValue::AllocExclusive()
{
    if ( !m_refData )    {
        m_refData = CreateRefData();
    }
    else if ( m_refData->GetRefCount() > 1 )    {
        // note that ref is not going to be destroyed in this case
        const wxJSONRefData* ref = m_refData;
        UnRef();

        // ... so we can still access it
        m_refData = CloneRefData(ref);
    }
    //else: ref count is 1, we are exclusive owners of m_refData anyhow

    wxASSERT_MSG( m_refData && m_refData->GetRefCount() == 1,
                  _T("wxObject::AllocExclusive() failed.") );
}

//! Convert memory buffer object to a string representation.
/*/
 The fucntion returns a string representation of the data contained in the
 memory buffer object \c buff.
 The string is conposed of two hexadecimal digits for every byte contained
 in the memory buffer; bytes are separated by a space character.
 The string starts with the actual lenght of the data enclosed in parenthesis.
 The string will contain \c len bytes if \c len is less than the length
 of the actual data in \c buff.
 Note that the (len) printed in the output referes to the length of the buffer
 which may be greater than the length that has to be printed.

 \b Example:
 This is an example of printing a memory buffer object that contains 10 bytes:
 \code
   0x80974653 (10) 00 01 02 03 04 05 06 07 08 09
 \endcode
*/
wxString
wxJSONValue::MemoryBuffToString( const wxMemoryBuffer& buff, size_t len )
{
    size_t buffLen = buff.GetDataLen();
    void*  ptr = buff.GetData();
    wxString s = MemoryBuffToString( ptr, MIN( buffLen, len ), buffLen );
    return s;
}


//! Convert a binary memory buffer to a string representation.
/*/
 The function returns a string representation of the data contained in the
 binary memory buffer pointed to by \c buff for \c len bytes.
 The string is composed of two hexadecimal digits for every byte contained
 in the memory buffer; bytes are separated by a space character.
 The string starts with pointer to binary data followed by the lenght of the
 data enclosed in parenthesis.

 \b Example:
 This is an example of printing ten bytes from a memory buffer:
 \code
   0x80974653 (10) 00 01 02 03 04 05 06 07 08 09
 \endcode

 @param buff the pointer to the memory buffer data
 @len   the length of the data that has to be printed
 @actualLen the real lenght of the memory buffer that has to be printed
        just afetr the pointer; may be greater than \c len. If this parameter
        is -1 then it is equal to \c len
*/
wxString
wxJSONValue::MemoryBuffToString( const void* buff, size_t len, size_t actualLen )
{
    wxString s;
    size_t buffLen = actualLen;
    if (buffLen == (size_t) -1 )    {
        buffLen = len;
    }
    s.Printf( _T("%p (%u) "), buff, buffLen );
    unsigned char*  ptr = (unsigned char*) buff;
    for ( unsigned int i = 0; i < len; i++ ) {
        unsigned char c = *ptr;
        ++ptr;
        // now convert the character
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
        s.Append( c1, 1 );
        s.Append( c2, 1 );
        s.Append( ' ', 1 );     // a space separates the bytes
    }
    return s;
}

//! Compares two memory buffer objects
/*!
 The function is the counterpart of the comparison operator == for two wxMemoryBuffer
 objects.
 You may noticed that the wxMemoryBuffer class does not define comparison operators so
 if you write a code snippset like the following:
 \code
    wxMemoryBuffer b1;
    wxMemoryBuffer b2;
    b1.AppendData( "1234567890", 10 );
    b2.AppendData( "1234567890", 10 );
    bool r = b1 == b2;
 \endcode

 you may expect that \b r is TRUE, because both objects contain the same data.
 This is not true. The result you get is FALSE because the default comparison operator
 is used, which just compares the data members of the class.
 The data member is the pointer to the allocated memory that contains the data and
 they are not equal.
 This function uses the (fast) \b memcmp function to compare the actual data
 contained in the nenory buffer objects thus doing a deep comparison.
 The function returns the return value of \b memcmp:

 the memcmp() function returns  an  integer  less  than,  equal  to,  or
 greater than zero if the first n bytes of \c buff1 is found, respectively, to
 be less than, to match, or be greater than the first n bytes of \c buff2.
*/
int
wxJSONValue::CompareMemoryBuff( const wxMemoryBuffer& buff1, const wxMemoryBuffer& buff2 )
{
    int r;
    size_t buff1Len = buff1.GetDataLen();
    size_t buff2Len = buff2.GetDataLen();
    if ( buff1Len > buff2Len )  {
        r = 1;
    }
    else if ( buff1Len < buff2Len )  {
        r = -1;
    }
    else    {
        r = memcmp( buff1.GetData(), buff2.GetData(), buff1Len );
    }
    return r;
}

//! Compares a memory buffer object and a memory buffer
/*!
 The function compares the data contained in a memory buffer object with a
 memory buffer.
 This function uses the (fast) \b memcmp function to compare the actual data
 contained in the nenory buffer object thus doing a deep comparison.
 The function returns the return value of \b memcmp:

 The memcmp() function returns  an  integer  less  than,  equal  to,  or
 greater than zero if the first n bytes of \c buff1 is found, respectively, to
 be less than, to match, or be greater than the first n bytes of \c buff2.
*/
int
wxJSONValue::CompareMemoryBuff( const wxMemoryBuffer& buff1, const void* buff2 )
{
    int r;
    size_t buff1Len = buff1.GetDataLen();
    r = memcmp( buff1.GetData(), buff2, buff1Len );
    return r;
}


//! Converts an array of INTs to a memory buffer
/*!
 This static function converts an array of INTs stored in a wxJSONvalue object
 into a memory buffer object.
 The wxJSONvalue object passed as parameter must be of type ARRAY and must contain
 INT types whose values are between 0 and 255.

 Every element of the array si converted to a BYTE value and appended to the returned
 wxMemoryBuffer object. The following rules apply in the conversion:
 \li if \c value is not an ARRAY type, an empty memory buffer is returned
 \li if the \c value array contains elements of type other than INT, those
	elements are ignored
 \li if the \c value array contains elements of type INT which value is outside the
	range 0..255, those elements are ignored
 \li if the \c value array contains only ignored elements an empty wxMemoryBuffer
	object is returned.

 This function can be used to get a memory buffer object from valid JSON text.
 Please note that the wxJSONReader cannot know which array of INTs represent a binary
 memory buffer unless you use the \b wxJSON \e memory \e buffer extension in the writer and
 in the reader.
*/
wxMemoryBuffer
wxJSONValue::ArrayToMemoryBuff( const wxJSONValue& value )
{
    wxMemoryBuffer buff;
    if ( value.IsArray() )  {
        int len = value.Size();
        for ( int i = 0; i < len; i++ )  {
            short int byte; unsigned char c;
            // we do not use opertaor [] because it is not const
            // bool r = value[i].AsShort( byte );
            bool r = value.ItemAt(i).AsShort( byte );
            if ( r && ( byte >= 0 && byte <= 255 ) )  {
                c = (unsigned char) byte;
                buff.AppendByte( c );
            }
        }
    }
    return buff;
}


/*************************************************************************

            64-bits integer support

*************************************************************************/

#if defined( wxJSON_64BIT_INT)


//! \overload wxJSONValue()
wxJSONValue::wxJSONValue( wxInt64 i )
{
    m_refData = 0;
    wxJSONRefData* data = Init( wxJSONTYPE_INT );
    wxJSON_ASSERT( data );
    if ( data != 0 ) {
        data->m_value.VAL_INT = i;
    }
}

//! \overload wxJSONValue()
wxJSONValue::wxJSONValue( wxUint64 ui )
{
    m_refData = 0;
    wxJSONRefData* data = Init( wxJSONTYPE_UINT );
    wxJSON_ASSERT( data );
    if ( data != 0 ) {
        data->m_value.VAL_UINT = ui;
    }
}

//! Return TRUE if the stored value is a 32-bits integer
/*!
 This function is only available on 64-bits platforms and returns
 TRUE if, and only if, the stored value is of type \b wxJSONTYPE_INT
 and the numeric value fits in a 32-bits signed integer.
 The function just calls IsLong() and returns the value returned by
 that function.
 The use of this function is deprecated: use \c IsLong() instead
*/
bool
wxJSONValue::IsInt32() const
{
    bool r = IsLong();
    return r;
}

//! Return TRUE if the stored value is a unsigned 32-bits integer
/*!
 This function is only available on 64-bits platforms and returns
 TRUE if, and only if, the stored value is of type \b wxJSONTYPE_UINT
 and the numeric value fits in a 32-bits unsigned integer.
 The function just calls IsULong() and returns the value returned by
 that function.
 The use of this function is deprecated: use \c IsULong() instead
*/
bool
wxJSONValue::IsUInt32() const
{
    bool r = IsULong();
    return r;
}


//! Return TRUE if the stored value is integer.
/*!
 This function returns TRUE if the stored value is of
 type signed integer.
 In other words, the function returns TRUE if the \c wxJSONRefData::m_type
 data member is of type \c wxJSONTYPE_INT
 The function is only available if 64-bits integer support is enabled.

 \sa \ref json_internals_integer
*/
bool
wxJSONValue::IsInt64() const
{
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );
    bool r = false;
    if ( data->m_type == wxJSONTYPE_INT ) {
        r = true;
    }
    return r;
}


//! Return TRUE if the stored value is a unsigned integer
/*!
 This function returns TRUE if the stored value is of
 type unsigned integer.
 In other words, the function returns TRUE if the \c wxJSONRefData::m_type
 data member is of type \c wxJSONTYPE_UINT.
 The function is only available if 64-bits integer support is enabled.

 \sa \ref json_internals_integer
*/
bool
wxJSONValue::IsUInt64() const
{
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );
    bool r = false;
    if ( data->m_type == wxJSONTYPE_UINT ) {
        r = true;
    }
    return r;
}

//! Returns the low-order 32 bits of the value as an integer
/*!
 This function is only available on 64-bits platforms and returns
 the low-order 32-bits of the integer stored in the JSON value.
 Note that all integer types are stored as \b wx(U)Int64 data types by
 the JSON value class and that the function does not check that the
 numeric value fits in a 32-bit integer.
 The function just calls AsLong() and casts the value in a wxInt32 data
 type

 \sa \ref wxjson_tutorial_get
*/
wxInt32
wxJSONValue::AsInt32() const
{
    wxInt32 i;
    i = (wxInt32) AsLong();
    return i;
}

//! Returns the low-order 32 bits of the value as an unsigned integer
/*!
 This function is only available on 64-bits platforms and returns
 the low-order 32-bits of the integer stored in the JSON value.
 Note that all integer types are stored as \b wx(U)Int64 data types by
 the JSON value class and that the function does not check that the
 numeric value fits in a 32-bit integer.
 The function just calls AsULong() and casts the value in a wxUInt32 data
 type

 \sa \ref wxjson_tutorial_get
*/
wxUint32
wxJSONValue::AsUInt32() const
{
    wxUint32 ui;
    ui = (wxUint32) AsULong();
    return ui;
}


//! Return the numeric value as a 64-bit integer.
/*!
 This function is only available on 64-bits platforms and returns
 the numeric value as a 64-bit integer.

 Note that the function does not check that the type of the
 value is actually an integer and it just returns the content
 of the wxJSONValueHolder union.
 However, in debug builds,  the function ASSERTs that the
 type of the stored value is wxJSONTYPE_INT.

 \sa \ref json_internals_integer
 \sa \ref wxjson_tutorial_get
*/
wxInt64
wxJSONValue::AsInt64() const
{
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );
    wxInt64 i64 = data->m_value.m_valInt64;

    wxJSON_ASSERT( IsInt64());  // exapnds only in debug builds
    return i64;
}

//! Return the numeric value as a 64-bit unsigned integer.
/*!
 This function is only available on 64-bits platforms and returns
 the numeric value as a 64-bit unsigned integer.

 Note that the function does not check that the type of the
 value is actually an integer and it just returns the content
 of the wxJSONValueHolder union.
 However, in debug builds,  the function wxJSON_ASSERTs that the
 type of the stored value is wxJSONTYPE_UINT.

 \sa \ref json_internals_integer
 \sa \ref wxjson_tutorial_get
*/
wxUint64
wxJSONValue::AsUInt64() const
{
    wxJSONRefData* data = GetRefData();
    wxJSON_ASSERT( data );
    wxUint64 ui64 = data->m_value.m_valUInt64;

    wxJSON_ASSERT( IsUInt64());  // exapnds only in debug builds
    return ui64;
}

bool
wxJSONValue::AsInt32( wxInt32& i32 ) const
{
    bool r = IsInt32();
    if ( r )    {
        i32 = AsInt32();
    }
    return r;
}

bool
wxJSONValue::AsUInt32( wxUint32& ui32 ) const
{
    bool r = IsUInt32();
    if ( r )    {
        ui32 = AsUInt32();
    }
    return r;
}

bool
wxJSONValue::AsInt64( wxInt64& i64 ) const
{
    bool r = IsInt64();
    if ( r )    {
        i64 = AsInt64();
    }
    return r;
}

bool
wxJSONValue::AsUInt64( wxUint64& ui64 ) const
{
    bool r = IsUInt64();
    if ( r )    {
        ui64 = AsUInt64();
    }
    return r;
}

//! \overload Append( const wxJSONValue& )
wxJSONValue&
wxJSONValue::Append( wxInt64 i )
{
    wxJSONValue v( i );
    wxJSONValue& r = Append( v );
    return r;
}

//! \overload Append( const wxJSONValue& )
wxJSONValue&
wxJSONValue::Append( wxUint64 ui )
{
    wxJSONValue v( ui );
    wxJSONValue& r = Append( v );
    return r;
}


//! \overload operator = (int)
wxJSONValue&
wxJSONValue::operator = ( wxInt64 i )
{
    wxJSONRefData* data = SetType( wxJSONTYPE_INT );
    data->m_value.VAL_INT = i;
    return *this;
}

//! \overload operator = (int)
wxJSONValue&
wxJSONValue::operator = ( wxUint64 ui )
{
    wxJSONRefData* data = SetType( wxJSONTYPE_UINT );
    data->m_value.VAL_UINT = ui;
    return *this;
}


#endif  // defined( wxJSON_64BIT_INT )




/*
{
}
*/

