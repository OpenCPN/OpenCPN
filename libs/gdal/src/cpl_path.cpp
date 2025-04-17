/**********************************************************************
 *
 * Project:  CPL - Common Portability Library
 * Purpose:  Portable filename/path parsing, and forming ala "Glob API".
 * Author:   Frank Warmerdam, warmerda@home.com
 *
 **********************************************************************
 * Copyright (c) 1999, Frank Warmerdam
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************
 *
 * $Log: cpl_path.cpp,v $
 * Revision 1.1.1.1  2006/08/21 05:52:20  dsr
 * Initial import as opencpn, GNU Automake compliant.
 *
 * Revision 1.1.1.1  2006/04/19 03:23:28  dsr
 * Rename/Import to OpenCPN
 *
 * Revision 1.14  2003/05/28 19:22:38  warmerda
 * fixed docs
 *
 * Revision 1.13  2003/04/04 09:11:35  dron
 * strcpy() and strcat() replaced by strncpy() and strncat().
 * A lot of assertion on string sizes added.
 *
 * Revision 1.12  2002/12/13 06:14:17  warmerda
 * fixed bug with IsRelative function
 *
 * Revision 1.11  2002/12/13 06:00:54  warmerda
 * added CPLProjectRelativeFilename() and CPLIsFilenameRelative()
 *
 * Revision 1.10  2002/08/15 09:23:24  dron
 * Added CPLGetDirname() function
 *
 * Revision 1.9  2001/08/30 21:20:49  warmerda
 * expand tabs
 *
 * Revision 1.8  2001/07/18 04:00:49  warmerda
 *
 * Revision 1.7  2001/05/12 19:20:55  warmerda
 * Fixed documentation of CPLGetExtension().
 *
 * Revision 1.6  2001/03/16 22:15:08  warmerda
 * added CPLResetExtension
 *
 * Revision 1.5  2001/02/24 01:53:57  warmerda
 * Added CPLFormCIFilename()
 *
 * Revision 1.4  2001/01/19 21:18:25  warmerda
 * expanded tabs
 *
 * Revision 1.3  2000/01/26 17:53:36  warmerda
 * Fixed CPLGetExtension() for filenames with no extension.
 *
 * Revision 1.2  2000/01/24 19:32:59  warmerda
 * Fixed CPLGetExtension() to not include the dot.
 *
 * Revision 1.1  1999/10/14 19:23:39  warmerda
 * New
 *
 **********************************************************************/

#include "cpl_conv.h"
#include "cpl_string.h"


/* should be size of larged possible filename */
#define CPL_PATH_BUF_SIZE 2048
static char     szStaticResult[CPL_PATH_BUF_SIZE];

#ifdef WIN32
#define SEP_CHAR '\\'
#define SEP_STRING "\\"
#else
#define SEP_CHAR '/'
#define SEP_STRING "/"
#endif

/************************************************************************/
/*                        CPLFindFilenameStart()                        */
/************************************************************************/

static int CPLFindFilenameStart( const char * pszFilename )

{
    int         iFileStart;

    for( iFileStart = strlen(pszFilename);
         iFileStart > 0
             && pszFilename[iFileStart-1] != '/'
             && pszFilename[iFileStart-1] != '\\';
         iFileStart-- ) {}

    return iFileStart;
}

/************************************************************************/
/*                             CPLGetPath()                             */
/************************************************************************/

/**
 * Extract directory path portion of filename.
 *
 * Returns a string containing the directory path portion of the passed
 * filename.  If there is no path in the passed filename an empty string
 * will be returned (not NULL).
 *
 * <pre>
 * CPLGetPath( "abc/def.xyz" ) == "abc"
 * CPLGetPath( "/abc/def/" ) == "/abc/def"
 * CPLGetPath( "/" ) == "/"
 * CPLGetPath( "/abc/def" ) == "/abc"
 * CPLGetPath( "abc" ) == ""
 * </pre>
 *
 * @param pszFilename the filename potentially including a path.
 *
 *  @return Path in an internal string which must not be freed.  The string
 * may be destroyed by the next CPL filename handling call.  The returned
 * will generally not contain a trailing path separator.
 */

const char *CPLGetPath( const char *pszFilename )

{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#pragma GCC diagnostic ignored "-Wrestrict"
// Overflow seen by compiler is guarded by assert.

    int         iFileStart = CPLFindFilenameStart(pszFilename);

    CPLAssert( iFileStart < CPL_PATH_BUF_SIZE );

    if( iFileStart == 0 )
    {
        strcpy( szStaticResult, "" );
        return szStaticResult;
    }

    strncpy( szStaticResult, pszFilename, iFileStart );
    szStaticResult[iFileStart] = '\0';

    if( iFileStart > 1
        && (szStaticResult[iFileStart-1] == '/'
            || szStaticResult[iFileStart-1] == '\\') )
        szStaticResult[iFileStart-1] = '\0';

    return szStaticResult;
#pragma GCC diagnostic pop
}

/************************************************************************/
/*                             CPLGetDirname()                          */
/************************************************************************/

/**
 * Extract directory path portion of filename.
 *
 * Returns a string containing the directory path portion of the passed
 * filename.  If there is no path in the passed filename the dot will be
 * returned.  It is the only difference from CPLGetPath().
 *
 * <pre>
 * CPLGetDirname( "abc/def.xyz" ) == "abc"
 * CPLGetDirname( "/abc/def/" ) == "/abc/def"
 * CPLGetDirname( "/" ) == "/"
 * CPLGetDirname( "/abc/def" ) == "/abc"
 * CPLGetDirname( "abc" ) == "."
 * </pre>
 *
 * @param pszFilename the filename potentially including a path.
 *
 * @return Path in an internal string which must not be freed.  The string
 * may be destroyed by the next CPL filename handling call.  The returned
 * will generally not contain a trailing path separator.
 */

const char *CPLGetDirname( const char *pszFilename )

{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#pragma GCC diagnostic ignored "-Wrestrict"
    // Overflow seen by compiler is guarded by assert.
    int         iFileStart = CPLFindFilenameStart(pszFilename);

    CPLAssert( iFileStart < CPL_PATH_BUF_SIZE );

    if( iFileStart == 0 )
    {
        strcpy( szStaticResult, "." );
        return szStaticResult;
    }

    strncpy( szStaticResult, pszFilename, iFileStart );
    szStaticResult[iFileStart] = '\0';

    if( iFileStart > 1
        && (szStaticResult[iFileStart-1] == '/'
            || szStaticResult[iFileStart-1] == '\\') )
        szStaticResult[iFileStart-1] = '\0';

    return szStaticResult;
#pragma GCC diagnostic pop
}

/************************************************************************/
/*                           CPLGetFilename()                           */
/************************************************************************/

/**
 * Extract non-directory portion of filename.
 *
 * Returns a string containing the bare filename portion of the passed
 * filename.  If there is no filename (passed value ends in trailing directory
 * separator) an empty string is returned.
 *
 * <pre>
 * CPLGetFilename( "abc/def.xyz" ) == "def.xyz"
 * CPLGetFilename( "/abc/def/" ) == ""
 * CPLGetFilename( "abc/def" ) == "def"
 * </pre>
 *
 * @param pszFullFilename the full filename potentially including a path.
 *
 *  @return just the non-directory portion of the path in an internal string
 * which must not be freed.  The string
 * may be destroyed by the next CPL filename handling call.
 */

const char *CPLGetFilename( const char *pszFullFilename )

{
    int iFileStart = CPLFindFilenameStart( pszFullFilename );

    strncpy( szStaticResult, pszFullFilename + iFileStart, CPL_PATH_BUF_SIZE );
    szStaticResult[CPL_PATH_BUF_SIZE - 1] = '\0';

    return szStaticResult;
}

/************************************************************************/
/*                           CPLGetBasename()                           */
/************************************************************************/

/**
 * Extract basename (non-directory, non-extension) portion of filename.
 *
 * Returns a string containing the file basename portion of the passed
 * name.  If there is no basename (passed value ends in trailing directory
 * separator, or filename starts with a dot) an empty string is returned.
 *
 * <pre>
 * CPLGetBasename( "abc/def.xyz" ) == "def"
 * CPLGetBasename( "abc/def" ) == "def"
 * CPLGetBasename( "abc/def/" ) == ""
 * </pre>
 *
 * @param pszFullFilename the full filename potentially including a path.
 *
 * @return just the non-directory, non-extension portion of the path in
 * an internal string which must not be freed.  The string
 * may be destroyed by the next CPL filename handling call.
 */

const char *CPLGetBasename( const char *pszFullFilename )

{
    int iFileStart = CPLFindFilenameStart( pszFullFilename );
    int iExtStart, nLength;

    for( iExtStart = strlen(pszFullFilename);
         iExtStart > iFileStart && pszFullFilename[iExtStart] != '.';
         iExtStart-- ) {}

    if( iExtStart == iFileStart )
        iExtStart = strlen(pszFullFilename);

    nLength = iExtStart - iFileStart;

    CPLAssert( nLength < CPL_PATH_BUF_SIZE );

    strncpy( szStaticResult, pszFullFilename + iFileStart, nLength );
    szStaticResult[nLength] = '\0';

    return szStaticResult;
}


/************************************************************************/
/*                           CPLGetExtension()                          */
/************************************************************************/

/**
 * Extract filename extension from full filename.
 *
 * Returns a string containing the extention portion of the passed
 * name.  If there is no extension (the filename has no dot) an empty string
 * is returned.  The returned extension will not include the period.
 *
 * <pre>
 * CPLGetExtension( "abc/def.xyz" ) == "xyz"
 * CPLGetExtension( "abc/def" ) == ""
 * </pre>
 *
 * @param pszFullFilename the full filename potentially including a path.
 *
 * @return just the extension portion of the path in
 * an internal string which must not be freed.  The string
 * may be destroyed by the next CPL filename handling call.
 */

const char *CPLGetExtension( const char *pszFullFilename )

{
    int iFileStart = CPLFindFilenameStart( pszFullFilename );
    int iExtStart;

    for( iExtStart = strlen(pszFullFilename);
         iExtStart > iFileStart && pszFullFilename[iExtStart] != '.';
         iExtStart-- ) {}

    if( iExtStart == iFileStart )
        iExtStart = strlen(pszFullFilename)-1;

    strncpy( szStaticResult, pszFullFilename+iExtStart+1, CPL_PATH_BUF_SIZE );
    szStaticResult[CPL_PATH_BUF_SIZE - 1] = '\0';

    return szStaticResult;
}

/************************************************************************/
/*                         CPLResetExtension()                          */
/************************************************************************/

/**
 * Replace the extension with the provided one.
 *
 * @param pszPath the input path, this string is not altered.
 * @param pszExt the new extension to apply to the given path.
 *
 * @return an altered filename with the new extension.    Do not
 * modify or free the returned string.  The string may be destroyed by the
 * next CPL call.
 */

const char *CPLResetExtension( const char *pszPath, const char *pszExt )

{
    int         i;

/* -------------------------------------------------------------------- */
/*      First, try and strip off any existing extension.                */
/* -------------------------------------------------------------------- */
    strncpy( szStaticResult, pszPath, CPL_PATH_BUF_SIZE );
    szStaticResult[CPL_PATH_BUF_SIZE - 1] = '\0';
    for( i = strlen(szStaticResult) - 1; i > 0; i-- )
    {
        if( szStaticResult[i] == '.' )
        {
            szStaticResult[i] = '\0';
            break;
        }

        if( szStaticResult[i] == '/' || szStaticResult[i] == '\\'
            || szStaticResult[i] == ':' )
            break;
    }

/* -------------------------------------------------------------------- */
/*      Append the new extension.                                       */
/* -------------------------------------------------------------------- */
    CPLAssert( strlen(pszExt) + 2 < CPL_PATH_BUF_SIZE );

    strcat( szStaticResult, "." );
    strcat( szStaticResult, pszExt );

    return szStaticResult;
}

/************************************************************************/
/*                          CPLFormFilename()                           */
/************************************************************************/

/**
 * Build a full file path from a passed path, file basename and extension.
 *
 * The path, and extension are optional.  The basename may in fact contain
 * an extension if desired.
 *
 * <pre>
 * CPLFormFilename("abc/xyz","def", ".dat" ) == "abc/xyz/def.dat"
 * CPLFormFilename(NULL,"def", NULL ) == "def"
 * CPLFormFilename(NULL,"abc/def.dat", NULL ) == "abc/def.dat"
 * CPLFormFilename("/abc/xyz/","def.dat", NULL ) == "/abc/xyz/def.dat"
 * </pre>
 *
 * @param pszPath directory path to the directory containing the file.  This
 * may be relative or absolute, and may have a trailing path separator or
 * not.  May be NULL.
 *
 * @param pszBasename file basename.  May optionally have path and/or
 * extension.  May not be NULL.
 *
 * @param pszExtension file extension, optionally including the period.  May
 * be NULL.
 *
 * @return a fully formed filename in an internal static string.  Do not
 * modify or free the returned string.  The string may be destroyed by the
 * next CPL call.
 */

const char *CPLFormFilename( const char * pszPath,
                             const char * pszBasename,
                             const char * pszExtension )

{
    const char  *pszAddedPathSep = "";
    const char  *pszAddedExtSep = "";

    if( pszPath == NULL )
        pszPath = "";
    else if( strlen(pszPath) > 0
             && pszPath[strlen(pszPath)-1] != '/'
             && pszPath[strlen(pszPath)-1] != '\\' )
        pszAddedPathSep = SEP_STRING;

    if( pszExtension == NULL )
        pszExtension = "";
    else if( pszExtension[0] != '.' && strlen(pszExtension) > 0 )
        pszAddedExtSep = ".";

    CPLAssert( strlen(pszPath) + strlen(pszAddedPathSep) +
               strlen(pszBasename) + strlen(pszAddedExtSep) +
               strlen(pszExtension) + 1 < CPL_PATH_BUF_SIZE );

    strncpy( szStaticResult, pszPath, CPL_PATH_BUF_SIZE - 1 );
    strncat( szStaticResult, pszAddedPathSep, sizeof(szStaticResult)-strlen(szStaticResult)-1);
    strncat( szStaticResult, pszBasename,     sizeof(szStaticResult)-strlen(szStaticResult)-1);
    strncat( szStaticResult, pszAddedExtSep,  sizeof(szStaticResult)-strlen(szStaticResult)-1);
    strncat( szStaticResult, pszExtension,    sizeof(szStaticResult)-strlen(szStaticResult)-1);
    szStaticResult[CPL_PATH_BUF_SIZE - 1] = '\0';

    return szStaticResult;
}

/************************************************************************/
/*                          CPLFormCIFilename()                         */
/************************************************************************/

/**
 * Case insensitive file searching, returing full path.
 *
 * This function tries to return the path to a file regardless of
 * whether the file exactly matches the basename, and extension case, or
 * is all upper case, or all lower case.  The path is treated as case
 * sensitive.  This function is equivelent to CPLFormFilename() on
 * case insensitive file systems (like Windows).
 *
 * @param pszPath directory path to the directory containing the file.  This
 * may be relative or absolute, and may have a trailing path separator or
 * not.  May be NULL.
 *
 * @param pszBasename file basename.  May optionally have path and/or
 * extension.  May not be NULL.
 *
 * @param pszExtension file extension, optionally including the period.  May
 * be NULL.
 *
 * @return a fully formed filename in an internal static string.  Do not
 * modify or free the returned string.  The string may be destroyed by the
 * next CPL call.
 */

const char *CPLFormCIFilename( const char * pszPath,
                               const char * pszBasename,
                               const char * pszExtension )

{
#ifdef WIN32
    return CPLFormFilename( pszPath, pszBasename, pszExtension );
#else
    const char  *pszAddedExtSep = "";
    char        *pszFilename;
    const char  *pszFullPath;
    int         nLen = strlen(pszBasename)+2, i;
    FILE        *fp;

    if( pszExtension != NULL )
        nLen += strlen(pszExtension);

    pszFilename = (char *) CPLMalloc(nLen);

    if( pszExtension == NULL )
        pszExtension = "";
    else if( pszExtension[0] != '.' && strlen(pszExtension) > 0 )
        pszAddedExtSep = ".";

    sprintf( pszFilename, "%s%s%s",
             pszBasename, pszAddedExtSep, pszExtension );

    pszFullPath = CPLFormFilename( pszPath, pszFilename, NULL );
    fp = VSIFOpen( pszFullPath, "r" );
    if( fp == NULL )
    {
        for( i = 0; pszFilename[i] != '\0'; i++ )
        {
            if( pszFilename[i] >= 'a' && pszFilename[i] <= 'z' )
                pszFilename[i] = pszFilename[i] + 'A' - 'a';
        }

        pszFullPath = CPLFormFilename( pszPath, pszFilename, NULL );
        fp = VSIFOpen( pszFullPath, "r" );
    }

    if( fp == NULL )
    {
        for( i = 0; pszFilename[i] != '\0'; i++ )
        {
            if( pszFilename[i] >= 'A' && pszFilename[i] <= 'Z' )
                pszFilename[i] = pszFilename[i] + 'a' - 'A';
        }

        pszFullPath = CPLFormFilename( pszPath, pszFilename, NULL );
        fp = VSIFOpen( pszFullPath, "r" );
    }

    if( fp != NULL )
        VSIFClose( fp );
    else
        pszFullPath = CPLFormFilename( pszPath, pszBasename, pszExtension );

    CPLFree( pszFilename );

    return pszFullPath;
#endif
}

/************************************************************************/
/*                     CPLProjectRelativeFilename()                     */
/************************************************************************/

/**
 * Find a file relative to a project file.
 *
 * Given the path to a "project" directory, and a path to a secondary file
 * referenced from that project, build a path to the secondary file
 * that the current application can use.  If the secondary path is already
 * absolute, rather than relative, then it will be returned unaltered.
 *
 * Examples:
 * <pre>
 * CPLProjectRelativeFilename("abc/def","tmp/abc.gif") == "abc/def/tmp/abc.gif"
 * CPLProjectRelativeFilename("abc/def","/tmp/abc.gif") == "/tmp/abc.gif"
 * CPLProjectRelativeFilename("/xy", "abc.gif") == "/xy/abc.gif"
 * CPLProjectRelativeFilename("/abc/def","../abc.gif") == "/abc/def/../abc.gif"
 * CPLProjectRelativeFilename("C:\WIN","abc.gif") == "C:\WIN\abc.gif"
 * </pre>
 *
 * @param pszProjectDir the directory relative to which the secondary files
 * path should be interpreted.
 * @param pszSecondaryFilename the filename (potentially with path) that
 * is to be interpreted relative to the project directory.
 *
 * @return a composed path to the secondary file.  The returned string is
 * internal and should not be altered, freed, or depending on past the next
 * CPL call.
 */

const char *CPLProjectRelativeFilename( const char *pszProjectDir,
                                        const char *pszSecondaryFilename )

{
    if( !CPLIsFilenameRelative( pszSecondaryFilename ) )
        return pszSecondaryFilename;

    if( pszProjectDir == NULL || strlen(pszProjectDir) == 0 )
        return pszSecondaryFilename;

    strncpy( szStaticResult, pszProjectDir, CPL_PATH_BUF_SIZE );
    szStaticResult[CPL_PATH_BUF_SIZE - 1] = '\0';

    if( pszProjectDir[strlen(pszProjectDir)-1] != '/'
        && pszProjectDir[strlen(pszProjectDir)-1] != '\\' )
    {
        CPLAssert( strlen(SEP_STRING) + 1 < CPL_PATH_BUF_SIZE );

        strcat( szStaticResult, SEP_STRING );
    }

    CPLAssert( strlen(pszSecondaryFilename) + 1 < CPL_PATH_BUF_SIZE );

    strcat( szStaticResult, pszSecondaryFilename );

    return szStaticResult;
}

/************************************************************************/
/*                       CPLIsFilenameRelative()                        */
/************************************************************************/

/**
 * Is filename relative or absolute?
 *
 * The test is filesystem convention agnostic.  That is it will test for
 * Unix style and windows style path conventions regardless of the actual
 * system in use.
 *
 * @param pszFilename the filename with path to test.
 *
 * @return TRUE if the filename is relative or FALSE if it is absolute.
 */

int CPLIsFilenameRelative( const char *pszFilename )

{
    if( (strlen(pszFilename) > 2 && strncmp(pszFilename+1,":\\",2) == 0)
        || pszFilename[0] == '\\'
        || pszFilename[0] == '/' )
        return FALSE;
    else
        return TRUE;
}
