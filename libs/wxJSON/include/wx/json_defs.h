/////////////////////////////////////////////////////////////////////////////
// Name:        json_defs.h
// Purpose:     shared build defines
// Author:      Luciano Cattani
// Created:     2007/10/20
// RCS-ID:      $Id: json_defs.h,v 1.6 2008/03/12 10:48:19 luccat Exp $
// Copyright:   (c) 2007 Luciano Cattani
// Licence:     wxWidgets licence
/////////////////////////////////////////////////////////////////////////////


#ifndef _WX_JSON_DEFS_H_
#define _WX_JSON_DEFS_H_

// Defines for component version.
// The following symbols should be updated for each new component release
// since some kind of tests, like those of AM_WXCODE_CHECKFOR_COMPONENT_VERSION()
// for "configure" scripts under unix, use them.
#define wxJSON_MAJOR          1
#define wxJSON_MINOR          2
#define wxJSON_RELEASE        1

// For non-Unix systems (i.e. when building without a configure script),
// users of this component can use the following macro to check if the
// current version is at least major.minor.release
#define wxCHECK_JSON_VERSION(major,minor,release) \
    (wxJSON_MAJOR > (major) || \
    (wxJSON_MAJOR == (major) && wxJSON_MINOR > (minor)) || \
    (wxJSON_MAJOR == (major) && wxJSON_MINOR == (minor) && wxJSON_RELEASE >= (release)))


// Defines for shared builds.
// Simple reference for using these macros and for writin components
// which support shared builds:
//
// 1) use the WXDLLIMPEXP_MYCOMP in each class declaration:
//          class WXDLLIMPEXP_MYCOMP myCompClass {   [...]   };
//
// 2) use the WXDLLIMPEXP_MYCOMP in the declaration of each global function:
//          WXDLLIMPEXP_MYCOMP int myGlobalFunc();
//
// 3) use the WXDLLIMPEXP_DATA_MYCOMP() in the declaration of each global
//    variable:
//          WXDLLIMPEXP_DATA_MYCOMP(int) myGlobalIntVar;
//
#ifdef WXMAKINGDLL_JSON
    #define WXDLLIMPEXP_JSON                  WXEXPORT
    #define WXDLLIMPEXP_DATA_JSON(type)       WXEXPORT type
#elif defined(WXUSINGDLL)
    #define WXDLLIMPEXP_JSON                  WXIMPORT
    #define WXDLLIMPEXP_DATA_JSON(type)       WXIMPORT type
#else // not making nor using DLL
    #define WXDLLIMPEXP_JSON
    #define WXDLLIMPEXP_DATA_JSON(type)	    type
#endif

//dsr
#undef WXDLLIMPEXP_JSON
#undef WXDLLIMPEXP_DATA_JSON

    #define WXDLLIMPEXP_JSON
    #define WXDLLIMPEXP_DATA_JSON(type)	    type

//dsr


// the __PRETTY_FUNCTION__ macro expands to the full class's
// member name in the GNU GCC.
// For other compilers we use the standard __wxFUNCTION__ macro
#if !defined( __GNUC__ )
  #define __PRETTY_FUNCTION__   __WXFUNCTION__
#endif



// define wxJSON_USE_UNICODE if wxWidgets was built with
// unicode support
#if defined( wxJSON_USE_UNICODE )
  #undef wxJSON_USE_UNICODE
#endif
// do not modify the following lines
#if wxUSE_UNICODE == 1
  #define wxJSON_USE_UNICODE
#endif

// the following macro, if defined, cause the wxJSONValue to store
// pointers to C-strings as pointers to statically allocated
// C-strings. By default this macro is not defined
// #define wxJSON_USE_CSTRING


// the following macro, if defined, cause the wxJSONvalue and its
// referenced data structure to store and increment a static
// progressive counter in the ctor.
// this is only usefull for debugging purposes
// #define WXJSON_USE_VALUE_COUNTER


// the following macro is used by wxJSON internally and you should not
// modify it. If the platform seems to support 64-bits integers,
// the following lines define the 'wxJSON_64BIT_INT' macro
#if defined( wxLongLong_t )
#define wxJSON_64BIT_INT
#endif


//
// the following macro, if defined, cause the wxJSON library to
// always use 32-bits integers also when the platform seems to
// have native 64-bits support: by default the macro if not defined
//
// #define wxJSON_NO_64BIT_INT
//
#if defined( wxJSON_NO_64BIT_INT ) && defined( wxJSON_64BIT_INT )
#undef wxJSON_64BIT_INT
#endif

//
// it seems that some compilers do not define 'long long int' limits
// constants. For example, this is the output of the Borland BCC 5.5
// compiler when I tried to compile wxJSON with 64-bits integer support:
//   Error E2451 ..\src\jsonreader.cpp 1737: Undefined symbol 'LLONG_MAX'
//   in function wxJSONReader::Strtoll(const wxString &,__int64 *)
//   *** 1 errors in Compile ***
// so, if the constants are not defined, I define them by myself
#if !defined( LLONG_MAX )
  #define LLONG_MAX      9223372036854775807
#endif

#if !defined( ULLONG_MAX )
  #define ULLONG_MAX    18446744073709551615
#endif

#if !defined( LLONG_MIN )
  #define LLONG_MIN     -9223372036854775808
#endif



// the same applies for all other integer constants
#if !defined( INT_MIN )
  #define INT_MIN       -32768
#endif
#if !defined( INT_MAX )
  #define INT_MAX        32767
#endif
#if !defined( UINT_MAX )
  #define UINT_MAX       65535
#endif
#if !defined( LONG_MIN )
  #define LONG_MIN       -2147483648
#endif
#if !defined( LONG_MAX )
  #define LONG_MAX       2147483647
#endif
#if !defined( ULONG_MAX )
  #define ULONG_MAX       4294967295
#endif
#if !defined( SHORT_MAX )
  #define SHORT_MAX	32767
#endif
#if !defined( SHORT_MIN )
  #define SHORT_MIN	-32768
#endif
#if !defined( USHORT_MAX )
  #define USHORT_MAX	65535
#endif



//
// define the wxJSON_ASSERT() macro to expand to wxASSERT()
// unless the wxJSON_NOABORT_ASSERT is defined
// #define wxJSON_NOABORT_ASSERT
#if defined( wxJSON_NOABORT_ASSERT )
  #define wxJSON_ASSERT( cond )
#else
  #define wxJSON_ASSERT( cond )		wxASSERT( cond );
#endif


//
// the following macros are used by the wxJSONWriter::WriteStringValues()
// when the wxJSONWRITER_SPLIT_STRING flag is set
#define wxJSONWRITER_LAST_COL	50
#define wxJSONWRITER_SPLIT_COL	75
#define wxJSONWRITER_MIN_LENGTH	15
#define wxJSONWRITER_TAB_LENGTH  4


//
// some compilers (i.e. MSVC++) defines their own 'snprintf' function
// so if it is not defined, define it in the following lines
// please note that we cannot use the wxWidget's counterpart 'wxSnprintf'
// because the latter uses 'wxChar' but wxJSON only use 'char'
#if !defined(snprintf) && defined(_MSC_VER)
#define snprintf _snprintf
#endif


//
// check if wxWidgets is compiled using --enable-stl in which case
// we have to use different aproaches when declaring the array and
// key/value containers (see the docs: wxJSON internals: array and hash_map
#undef wxJSON_USE_STL
#if defined( wxUSE_STL ) && wxUSE_STL == 1
#define wxJSON_USE_STL
#endif

//
// defines the MIN and MAX macro for numeric arguments
// note that the safest way to define such functions is using templates
#define MIN(a,b)    a < b ? a : b
#define MAX(a,b)    a > b ? a : b


#endif // _WX_JSON_DEFS_H_


