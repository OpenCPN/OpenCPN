/******************************************************************************
 *
 * Project:  GDAL Core
 * Purpose:  Free standing functions for GDAL.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 *
 * $Log: gdal_misc.cpp,v $
 * Revision 1.1.1.1  2006/08/21 05:52:20  dsr
 * Initial import as opencpn, GNU Automake compliant.
 *
 * Revision 1.52  2004/02/25 09:03:15  dron
 * Added GDALPackedDMSToDec() and GDALDecToPackedDMS() functions.
 *
 * Revision 1.51  2004/02/18 14:59:55  dron
 * Properly determine pixel offset in last tiles in GDALGetRandomRasterSample().
 *
 * Revision 1.50  2004/01/18 16:43:37  dron
 * Added GDALGetDataTypeByName() function.
 *
 * Revision 1.49  2003/08/18 12:43:47  warmerda
 * always include ogr_spatialref.h
 *
 * Revision 1.48  2003/08/12 22:13:32  warmerda
 * Changed GDALReadTabFile(0 so that if the CoordSys results in a PROJCS, but
 * the UNITS keyword is set to "degrees" we just use the GEOGCS portion.  It
 * seems that the projection is "just for display", and the GCPs will actually
 * be in lat/long.
 *
 * Revision 1.47  2003/06/03 19:44:00  warmerda
 * added GDALRPCInfo support
 *
 * Revision 1.46  2003/05/23 15:52:54  warmerda
 * Cosmetic changes made.
 *
 * Revision 1.45  2003/05/21 04:31:53  warmerda
 * avoid warnings
 *
 * Revision 1.44  2003/04/30 17:13:48  warmerda
 * added docs for many C functions
 *
 * Revision 1.43  2003/03/13 14:37:17  warmerda
 * better end of range checking in GDALTermProgress
 *
 * Revision 1.42  2003/02/15 20:22:14  warmerda
 * GDALReadTabFile() returns true if it gets GCPs but cant make geotransform
 *
 * Revision 1.41  2003/01/27 21:55:52  warmerda
 * various documentation improvements
 *
 * Revision 1.40  2002/12/11 21:21:46  warmerda
 * fixed debug format problem
 *
 * Revision 1.39  2002/12/09 20:05:31  warmerda
 * fixed return flag from GDALReadTabFile
 *
 * Revision 1.38  2002/12/09 18:53:25  warmerda
 * GDALDecToDMS() now calls CPLDecToDMS()
 *
 * Revision 1.37  2002/12/05 17:55:30  warmerda
 * gdalreadtabfile should not be static
 *
 * Revision 1.36  2002/12/05 15:46:38  warmerda
 * added GDALReadTabFile()
 *
 * Revision 1.35  2002/07/09 20:33:12  warmerda
 * expand tabs
 *
 * Revision 1.34  2002/05/06 21:37:29  warmerda
 * added GDALGCPsToGeoTransform
 *
 * Revision 1.33  2002/04/25 16:18:41  warmerda
 * added extra checking
 *
 * Revision 1.32  2002/04/24 19:21:26  warmerda
 * Include <ctype.h> for toupper(), tolower().
 *
 * Revision 1.31  2002/04/19 12:22:05  dron
 * added GDALWriteWorldFile()
 *
 * Revision 1.30  2002/04/16 13:59:33  warmerda
 * added GDALVersionInfo
 *
 * Revision 1.29  2001/12/07 20:04:21  warmerda
 * fixed serious bug in random sampler
 *
 * Revision 1.28  2001/11/30 03:41:26  warmerda
 * Fixed bug with the block sampling rate being too low to satisfy large
 * sample count values.  Fixed bug with tiled images including some uninitialized
 * or zero data in the sample set on partial edge tiles.
 *
 * Revision 1.27  2001/11/26 20:14:01  warmerda
 * added GDALProjDef stubs for old 'bridges'
 *
 * Revision 1.26  2001/11/19 16:03:16  warmerda
 * moved GDALDectoDMS here
 *
 * Revision 1.25  2001/08/15 15:05:44  warmerda
 * return magnitude for complex samples in random sampler
 *
 * Revision 1.24  2001/07/18 04:04:30  warmerda
 *
 * Revision 1.23  2001/05/01 18:09:25  warmerda
 * added GDALReadWorldFile()
 */

#include "gdal_priv.h"
#include "cpl_string.h"
#include <ctype.h>

/************************************************************************/
/*                           __pure_virtual()                           */
/*                                                                      */
/*      The following is a gross hack to remove the last remaining      */
/*      dependency on the GNU C++ standard library.                     */
/************************************************************************/

#ifdef __GNUC__

extern "C"
void __pure_virtual()

{
}

#endif

/* -------------------------------------------------------------------- */
/*      GDAL Version Information.                                       */
/* -------------------------------------------------------------------- */
#ifndef GDAL_VERSION_NUM
#  define GDAL_VERSION_NUM      1200
#endif
#ifndef GDAL_RELEASE_DATE
#  define GDAL_RELEASE_DATE     20040310
#endif
#ifndef GDAL_RELEASE_NAME
#  define GDAL_RELEASE_NAME     "1.2.0.0"
#endif

/************************************************************************/
/*                          GDALVersionInfo()                           */
/************************************************************************/

/**
 * Get runtime version information.
 *
 * Available pszRequest values:
 * <ul>
    * <li> "VERSION_NUM": Returns GDAL_VERSION_NUM formatted as a string.  ie. "1170"
    * <li> "RELEASE_DATE": Returns GDAL_RELEASE_DATE formatted as a string.
    * ie. "20020416".
    * <li> "RELEASE_NAME": Returns the GDAL_RELEASE_NAME. ie. "1.1.7"
    * <li> "--version": Returns one line version message suitable for use in
    * response to --version requests.  ie. "GDAL 1.1.7, released 2002/04/16"
    * </ul>
    *
    * @param pszRequest the type of version info desired, as listed above.
    *
    * @return an internal string containing the requested information.
 */

           const char *GDALVersionInfo( const char *pszRequest )

{
    static char szResult[128];


    if( pszRequest == NULL || EQUAL(pszRequest,"VERSION_NUM") )
        sprintf( szResult, "%d", GDAL_VERSION_NUM );
    else if( EQUAL(pszRequest,"RELEASE_DATE") )
        sprintf( szResult, "%d", GDAL_RELEASE_DATE );
    else if( EQUAL(pszRequest,"RELEASE_NAME") )
        sprintf( szResult, "%s", GDAL_RELEASE_NAME );
    else // --version
        sprintf( szResult, "GDAL %s, released %d/%02d/%02d",
                 GDAL_RELEASE_NAME,
                 GDAL_RELEASE_DATE / 10000,
                 (GDAL_RELEASE_DATE % 10000) / 100,
                 GDAL_RELEASE_DATE % 100 );

    return szResult;
}

