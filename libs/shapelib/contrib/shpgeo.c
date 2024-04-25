/******************************************************************************
 * Copyright (c) 1999, Carl Anderson
 *
 * This code is based in part on the earlier work of Frank Warmerdam
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
 * requires shapelib 1.2
 *   gcc shpproj shpopen.o dbfopen.o -lm -lproj -o shpproj
 *
 * this may require linking with the PROJ4 projection library available from
 *
 * http://www.remotesensing.org/proj
 *
 * use -DPROJ4 to compile in Projection support
 *
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shapefil.h"

#include "shpgeo.h"

#if defined(_MSC_VER) && _MSC_VER < 1800
#include <float.h>
#define INFINITY (DBL_MAX + DBL_MAX)
#define NAN (INFINITY - INFINITY)
#endif

/* I'm using some shorthand throughout this file
 *      R+ is a Clockwise Ring and is the positive portion of an object
 *      R- is a CounterClockwise Ring and is a hole in a R+
 *      A complex object is one having at least one R-
 *      A compound object is one having more than one R+
 *	A simple object has one and only one element (R+ or R-)
 *
 *	The closed ring constraint is for polygons and assumed here
 *	Arcs or LineStrings I am calling Rings (generically open or closed)
 *	Point types are vertices or lists of vertices but not Rings
 *
 *   SHPT_POLYGON, SHPT_POLYGONZ, SHPT_POLYGONM and SHPT_MULTIPATCH
 *   can have SHPObjects that are compound as well as complex
 *
 *   SHP_POINT and its Z and M derivatives are strictly simple
 *   MULTI_POINT, SHPT_ARC and their derivatives may be simple or compound
 *
 */

/* **************************************************************************
 * asFileName
 *
 * utility function, toss part of filename after last dot
 *
 * **************************************************************************/
char *asFileName(const char *fil, char *ext)
{
    /* -------------------------------------------------------------------- */
    /*	Compute the base (layer) name.  If there is any extension	*/
    /*	on the passed in filename we will strip it off.			*/
    /* -------------------------------------------------------------------- */
    char pszBasename[120];
    strcpy(pszBasename, fil);
    int i = (int)strlen(pszBasename) - 1;
    for (; i > 0 && pszBasename[i] != '.' && pszBasename[i] != '/' &&
           pszBasename[i] != '\\';
         i--)
    {
    }

    if (pszBasename[i] == '.')
        pszBasename[i] = '\0';

    /* -------------------------------------------------------------------- */
    /*	Note that files pulled from					*/
    /*	a PC to Unix with upper case filenames won't work!		*/
    /* -------------------------------------------------------------------- */
    static char pszFullname[256];
    sprintf(pszFullname, "%s.%s", pszBasename, ext);

    return pszFullname;
}

/* **************************************************************************
 * SHPOGisType
 *
 * Convert Both ways from and to OGIS Geometry Types
 *
 * **************************************************************************/
int SHPOGisType(int GeomType, int toOGis)
{

    if (toOGis == 0) /* connect OGis -> SHP types  					*/
        switch (GeomType)
        {
            case (OGIST_POINT):
                return (SHPT_POINT);
                break;
            case (OGIST_LINESTRING):
                return (SHPT_ARC);
                break;
            case (OGIST_POLYGON):
                return (SHPT_POLYGON);
                break;
            case (OGIST_MULTIPOINT):
                return (SHPT_MULTIPOINT);
                break;
            case (OGIST_MULTILINE):
                return (SHPT_ARC);
                break;
            case (OGIST_MULTIPOLYGON):
                return (SHPT_POLYGON);
                break;
        }
    else /* ok so its SHP->OGis types 									*/
        switch (GeomType)
        {
            case (SHPT_POINT):
                return (OGIST_POINT);
                break;
            case (SHPT_POINTM):
                return (OGIST_POINT);
                break;
            case (SHPT_POINTZ):
                return (OGIST_POINT);
                break;
            case (SHPT_ARC):
                return (OGIST_LINESTRING);
                break;
            case (SHPT_ARCZ):
                return (OGIST_LINESTRING);
                break;
            case (SHPT_ARCM):
                return (OGIST_LINESTRING);
                break;
            case (SHPT_POLYGON):
                return (OGIST_MULTIPOLYGON);
                break;
            case (SHPT_POLYGONZ):
                return (OGIST_MULTIPOLYGON);
                break;
            case (SHPT_POLYGONM):
                return (OGIST_MULTIPOLYGON);
                break;
            case (SHPT_MULTIPOINT):
                return (OGIST_MULTIPOINT);
                break;
            case (SHPT_MULTIPOINTZ):
                return (OGIST_MULTIPOINT);
                break;
            case (SHPT_MULTIPOINTM):
                return (OGIST_MULTIPOINT);
                break;
            case (SHPT_MULTIPATCH):
                return (OGIST_GEOMCOLL);
                break;
        }

    return 0;
}

/* **************************************************************************
 * SHPWriteSHPStream
 *
 * Encapsulate entire SHPObject for use with Postgresql
 *
 * **************************************************************************/
static int SHPWriteSHPStream(WKBStreamObj *stream_obj, SHPObject *psCShape)
{
    int need_swap = 1;
    need_swap = ((char *)(&need_swap))[0];

    /*realloc (stream_obj, obj_storage );*/

    if (need_swap)
    {
    }
    else
    {
        memcpy(stream_obj, psCShape, 4 * sizeof(int));
        memcpy(stream_obj, psCShape, 4 * sizeof(double));
        // TODO(schwehr): What?
        // const int use_Z = 0;
        // const int use_M = 0;
        // if ( use_Z )
        //   memcpy (stream_obj, psCShape, 2 * sizeof (double) );
        // if ( use_M )
        //   memcpy (stream_obj, psCShape, 2 * sizeof (double) );

        memcpy(stream_obj, psCShape, psCShape->nParts * 2 * sizeof(int));
        memcpy(stream_obj, psCShape, psCShape->nVertices * 2 * sizeof(double));
        // if ( use_Z )
        //   memcpy (stream_obj, psCShape, psCShape->nVertices * 2 * sizeof (double) );
        // if ( use_M )
        //   memcpy (stream_obj, psCShape, psCShape->nVertices * 2 * sizeof (double) );
    }

    return (0);
}

/* **************************************************************************
 * WKBStreamWrite
 *
 * Encapsulate entire SHPObject for use with Postgresql
 *
 * **************************************************************************/
static int WKBStreamWrite(WKBStreamObj *wso, void *this, int tcount, int tsize)
{
    if (wso->NeedSwap)
        SwapG(&(wso->wStream[wso->StreamPos]), this, tcount, tsize);
    else
        memcpy(&(wso->wStream[wso->StreamPos]), this, tsize * tcount);

    wso->StreamPos += tsize;

    return 0;
}

/* **************************************************************************
 * WKBStreamRead
 *
 * Encapsulate entire SHPObject for use with Postgresql
 *
 * **************************************************************************/
static int WKBStreamRead(WKBStreamObj *wso, void *this, int tcount, int tsize)
{
    if (wso->NeedSwap)
        SwapG(this, &(wso->wStream[wso->StreamPos]), tcount, tsize);
    else
        memcpy(this, &(wso->wStream[wso->StreamPos]), tsize * tcount);

    wso->StreamPos += tsize;

    return 0;
}

/* **************************************************************************
 * SHPReadOGisWKB
 *
 * Encapsulate entire SHPObject for use with Postgresql
 *
 * **************************************************************************/
SHPObject *SHPReadOGisWKB(WKBStreamObj *stream_obj)
{
    char WKB_order;
    WKBStreamRead(stream_obj, &WKB_order, 1, sizeof(char));
    int my_order = 1;
    my_order = ((char *)(&my_order))[0];
    stream_obj->NeedSwap = !(WKB_order & my_order);

    /* convert OGis Types to SHP types  */
    int GeoType = 0;
    const int nSHPType = SHPOGisType(GeoType, 0);

    WKBStreamRead(stream_obj, &GeoType, 1, sizeof(int));

    const int thisDim = SHPDimension(nSHPType);

    // SHPObject *psCShape;
    if (thisDim & SHPD_AREA)
    {
        /* psCShape = */ SHPReadOGisPolygon(stream_obj);
    }
    else
    {
        if (thisDim & SHPD_LINE)
        {
            /* psCShape = */ SHPReadOGisLine(stream_obj);
        }
        else
        {
            if (thisDim & SHPD_POINT)
            {
                /* psCShape = */ SHPReadOGisPoint(stream_obj);
            }
        }
    }

    return (0);
}

/* **************************************************************************
 * SHPWriteOGisWKB
 *
 * Encapsulate entire SHPObject for use with Postgresql
 *
 * **************************************************************************/
int SHPWriteOGisWKB(WKBStreamObj *stream_obj, SHPObject *psCShape)
{
    /* OGis WKB can handle either byte order, but if I get to choose I'd	*/
    /* rather have it predicatable system-to-system							*/

    if (stream_obj)
    {
        if (stream_obj->wStream)
            free(stream_obj->wStream);
    }
    else
    {
        stream_obj = calloc(3, sizeof(int));
    }

    /* object size needs to be 9 bytes for the wrapper, and for each polygon	*/
    /* another 9 bytes all plus twice the total number of vertices				*/
    /* times the sizeof (double) and just pad with 10 more chars for fun		*/
    stream_obj->wStream =
        calloc(1, (9 * (psCShape->nParts + 1)) +
                      (sizeof(double) * 2 * psCShape->nVertices) + 10);

#ifdef DEBUG2
    printf(" I just allocated %d bytes to wkbObj \n",
           (int)(sizeof(int) + sizeof(int) + sizeof(int) +
                 (sizeof(int) * psCShape->nParts + 1) +
                 (sizeof(double) * 2 * psCShape->nVertices) + 10));
#endif

    /* indicate that this WKB is in LSB Order	*/
    int my_order = 1;
    my_order = ((char *)(&my_order))[0];
    /* Need to swap if this system is not  LSB (Intel Order)					*/
    char LSB = 1;
    stream_obj->NeedSwap = (my_order != LSB);

    stream_obj->StreamPos = 0;

#ifdef DEBUG2
    printf("this system is (%d) LSB recorded as needSwap %d\n", my_order,
           stream_obj->NeedSwap);
#endif

    WKBStreamWrite(stream_obj, &LSB, 1, sizeof(char));

#ifdef DEBUG2
    printf("this system in LSB \n");
#endif

    /* convert SHP Types to OGis types  */
    int GeoType = SHPOGisType(psCShape->nSHPType, 1);
    WKBStreamWrite(stream_obj, &GeoType, 1, sizeof(int));

    const int thisDim = SHPDimension(psCShape->nSHPType);

    if (thisDim & SHPD_AREA)
    {
        SHPWriteOGisPolygon(stream_obj, psCShape);
    }
    else
    {
        if (thisDim & SHPD_LINE)
        {
            SHPWriteOGisLine(stream_obj, psCShape);
        }
        else
        {
            if (thisDim & SHPD_POINT)
            {
                SHPWriteOGisPoint(stream_obj, psCShape);
            }
        }
    }

#ifdef DEBUG2
    printf("(SHPWriteOGisWKB) outta here when stream pos is %d \n",
           stream_obj->StreamPos);
#endif
    return (0);
}

/* **************************************************************************
 * SHPWriteOGisPolygon
 *
 * for this pass code to more generic OGis MultiPolygon Type
 * later add support for OGis Polygon Type
 *
 * Encapsulate entire SHPObject for use with Postgresql
 *
 * **************************************************************************/
int SHPWriteOGisPolygon(WKBStreamObj *stream_obj, SHPObject *psCShape)
{
    /* cannot have more than nParts complex objects in this object */
    SHPObject **ppsC = calloc(psCShape->nParts, sizeof(int));

    int nextring = 0;
    int cParts = 0;
    while (nextring >= 0)
    {
        ppsC[cParts] = SHPUnCompound(psCShape, &nextring);
        cParts++;
    }

#ifdef DEBUG2
    printf("(SHPWriteOGisPolygon) Uncompounded into %d parts \n", cParts);
#endif

    WKBStreamWrite(stream_obj, &cParts, 1, sizeof(int));

    int GeoType = OGIST_POLYGON;

    char Flag = 1;
    for (int cpart = 0; cpart < cParts; cpart++)
    {
        WKBStreamWrite(stream_obj, &Flag, 1, sizeof(char));
        WKBStreamWrite(stream_obj, &GeoType, 1, sizeof(int));

        SHPObject *psC = (SHPObject *)ppsC[cpart];
        WKBStreamWrite(stream_obj, &(psC->nParts), 1, sizeof(int));

        for (int ring = 0; (ring < (psC->nParts)) && (psC->nParts > 0); ring++)
        {
            int rVertices;
            if (ring < (psC->nParts - 2))
            {
                rVertices =
                    psC->panPartStart[ring + 1] - psC->panPartStart[ring];
            }
            else
            {
                rVertices = psC->nVertices - psC->panPartStart[ring];
            }
#ifdef DEBUG2
            printf("(SHPWriteOGisPolygon) scanning part %d, ring %d %d vtxs \n",
                   cpart, ring, rVertices);
#endif
            const int rPart = psC->panPartStart[ring];
            WKBStreamWrite(stream_obj, &rVertices, 1, sizeof(int));
            for (int j = rPart; j < (rPart + rVertices); j++)
            {
                WKBStreamWrite(stream_obj, &(psC->padfX[j]), 1, sizeof(double));
                WKBStreamWrite(stream_obj, &(psC->padfY[j]), 1, sizeof(double));
            } /* for each vertex */
        }     /* for each ring */
    }         /* for each complex part */

#ifdef DEBUG2
    printf("(SHPWriteOGisPolygon) outta here \n");
#endif
    return (1);
}

/* **************************************************************************
 * SHPWriteOGisLine
 *
 * for this pass code to more generic OGis MultiXXXXXXX Type
 * later add support for OGis LineString Type
 *
 * Encapsulate entire SHPObject for use with Postgresql
 *
 * **************************************************************************/
int SHPWriteOGisLine(WKBStreamObj *stream_obj, SHPObject *psCShape)
{
    return (SHPWriteOGisPolygon(stream_obj, psCShape));
}

/* **************************************************************************
 * SHPWriteOGisPoint
 *
 * for this pass code to more generic OGis MultiPoint Type
 * later add support for OGis Point Type
 *
 * Encapsulate entire SHPObject for use with Postgresql
 *
 * **************************************************************************/
int SHPWriteOGisPoint(WKBStreamObj *stream_obj, SHPObject *psCShape)
{
    WKBStreamWrite(stream_obj, &(psCShape->nVertices), 1, sizeof(int));

    for (int j = 0; j < psCShape->nVertices; j++)
    {
        WKBStreamWrite(stream_obj, &(psCShape->padfX[j]), 1, sizeof(double));
        WKBStreamWrite(stream_obj, &(psCShape->padfY[j]), 1, sizeof(double));
    } /* for each vertex */

    return (1);
}

/* **************************************************************************
 * SHPReadOGisPolygon
 *
 * for this pass code to more generic OGis MultiPolygon Type
 * later add support for OGis Polygon Type
 *
 * Encapsulate entire SHPObject for use with Postgresql
 *
 * **************************************************************************/
SHPObject *SHPReadOGisPolygon(WKBStreamObj *stream_obj)
{
    SHPObject *psC = SHPCreateObject(SHPT_POLYGON, -1, 0, NULL, NULL, 0, NULL,
                                     NULL, NULL, NULL);
    /* initialize a blank SHPObject */

    int cParts;
    WKBStreamRead(stream_obj, &cParts, 1, sizeof(char));

    int totParts = cParts;
    int totVertices = 0;

    psC->panPartStart = realloc(psC->panPartStart, cParts * sizeof(int));
    psC->panPartType = realloc(psC->panPartType, cParts * sizeof(int));

    int rVertices;
    int nParts;
    for (int cpart = 0; cpart < cParts; cpart++)
    {
        WKBStreamRead(stream_obj, &nParts, 1, sizeof(int));
        const int pRings = nParts;
        /* pRings is the number of rings prior to the Ring loop below */

        if (nParts > 1)
        {
            totParts += nParts - 1;
            psC->panPartStart =
                realloc(psC->panPartStart, totParts * sizeof(int));
            psC->panPartType =
                realloc(psC->panPartType, totParts * sizeof(int));
        }

        int rPart = 0;
        for (int ring = 0; ring < (nParts - 1); ring++)
        {
            WKBStreamRead(stream_obj, &rVertices, 1, sizeof(int));
            totVertices += rVertices;

            psC->panPartStart[ring + pRings] = rPart;
            if (ring == 0)
            {
                psC->panPartType[ring + pRings] = SHPP_OUTERRING;
            }
            else
            {
                psC->panPartType[ring + pRings] = SHPP_INNERRING;
            }

            psC->padfX = realloc(psC->padfX, totVertices * sizeof(double));
            psC->padfY = realloc(psC->padfY, totVertices * sizeof(double));

            for (int j = rPart; j < (rPart + rVertices); j++)
            {
                WKBStreamRead(stream_obj, &(psC->padfX[j]), 1, sizeof(double));
                WKBStreamRead(stream_obj, &(psC->padfY[j]), 1, sizeof(double));
            } /* for each vertex */
            rPart += rVertices;
        } /* for each ring */
    }     /* for each complex part */

    return (psC);
}

/* **************************************************************************
 * SHPReadOGisLine
 *
 * for this pass code to more generic OGis MultiLineString Type
 * later add support for OGis LineString Type
 *
 * Encapsulate entire SHPObject for use with Postgresql
 *
 * **************************************************************************/
SHPObject *SHPReadOGisLine(WKBStreamObj *stream_obj)
{
    SHPObject *psC =
        SHPCreateObject(SHPT_ARC, -1, 0, NULL, NULL, 0, NULL, NULL, NULL, NULL);
    /* initialize a blank SHPObject */

    int cParts;
    WKBStreamRead(stream_obj, &cParts, 1, sizeof(int));

    int totParts = cParts;
    int totVertices = 0;

    psC->panPartStart = realloc(psC->panPartStart, cParts * sizeof(int));
    psC->panPartType = realloc(psC->panPartType, cParts * sizeof(int));

    int rVertices;
    int nParts;
    for (int cpart = 0; cpart < cParts; cpart++)
    {
        WKBStreamRead(stream_obj, &nParts, 1, sizeof(int));
        int pRings = totParts;
        /* pRings is the number of rings prior to the Ring loop below			*/

        if (nParts > 1)
        {
            totParts += nParts - 1;
            psC->panPartStart =
                realloc(psC->panPartStart, totParts * sizeof(int));
            psC->panPartType =
                realloc(psC->panPartType, totParts * sizeof(int));
        }

        int rPart = 0;
        for (int ring = 0; ring < (nParts - 1); ring++)
        {
            WKBStreamRead(stream_obj, &rVertices, 1, sizeof(int));
            totVertices += rVertices;

            psC->panPartStart[ring + pRings] = rPart;
            if (ring == 0)
            {
                psC->panPartType[ring + pRings] = SHPP_OUTERRING;
            }
            else
            {
                psC->panPartType[ring + pRings] = SHPP_INNERRING;
            }

            psC->padfX = realloc(psC->padfX, totVertices * sizeof(double));
            psC->padfY = realloc(psC->padfY, totVertices * sizeof(double));

            for (int j = rPart; j < (rPart + rVertices); j++)
            {
                WKBStreamRead(stream_obj, &(psC->padfX[j]), 1, sizeof(double));
                WKBStreamRead(stream_obj, &(psC->padfY[j]), 1, sizeof(double));
            } /* for each vertex */
            rPart += rVertices;
        } /* for each ring */
    }     /* for each complex part */

    return (psC);
}

/* **************************************************************************
 * SHPReadOGisPoint
 *
 * Encapsulate entire SHPObject for use with Postgresql
 *
 * **************************************************************************/
SHPObject *SHPReadOGisPoint(WKBStreamObj *stream_obj)
{
    SHPObject *psC = SHPCreateObject(SHPT_MULTIPOINT, -1, 0, NULL, NULL, 0,
                                     NULL, NULL, NULL, NULL);
    /* initialize a blank SHPObject */

    int nVertices;
    WKBStreamRead(stream_obj, &nVertices, 1, sizeof(int));

    psC->padfX = realloc(psC->padfX, nVertices * sizeof(double));
    psC->padfY = realloc(psC->padfY, nVertices * sizeof(double));

    for (int j = 0; j < nVertices; j++)
    {
        WKBStreamRead(stream_obj, &(psC->padfX[j]), 1, sizeof(double));
        WKBStreamRead(stream_obj, &(psC->padfY[j]), 1, sizeof(double));
    } /* for each vertex */

    return (psC);
}

/* **************************************************************************
 * SHPDimension
 *
 * Return the Dimensionality of the SHPObject
 * a handy utility function
 *
 * **************************************************************************/
int SHPDimension(int SHPType)
{
    int dimension = 0;

    switch (SHPType)
    {
        case SHPT_POINT:
            dimension = SHPD_POINT;
            break;
        case SHPT_ARC:
            dimension = SHPD_LINE;
            break;
        case SHPT_POLYGON:
            dimension = SHPD_AREA;
            break;
        case SHPT_MULTIPOINT:
            dimension = SHPD_POINT;
            break;
        case SHPT_POINTZ:
            dimension = SHPD_POINT | SHPD_Z;
            break;
        case SHPT_ARCZ:
            dimension = SHPD_LINE | SHPD_Z;
            break;
        case SHPT_POLYGONZ:
            dimension = SHPD_AREA | SHPD_Z;
            break;
        case SHPT_MULTIPOINTZ:
            dimension = SHPD_POINT | SHPD_Z;
            break;
        case SHPT_POINTM:
            dimension = SHPD_POINT | SHPD_MEASURE;
            break;
        case SHPT_ARCM:
            dimension = SHPD_LINE | SHPD_MEASURE;
            break;
        case SHPT_POLYGONM:
            dimension = SHPD_AREA | SHPD_MEASURE;
            break;
        case SHPT_MULTIPOINTM:
            dimension = SHPD_POINT | SHPD_MEASURE;
            break;
        case SHPT_MULTIPATCH:
            dimension = SHPD_AREA;
            break;
    }

    return (dimension);
}

/* **************************************************************************
 * SHPPointinPoly_2d
 *
 * Return a Point inside an R+ of a potentially
 * complex/compound SHPObject suitable for labelling
 * return only one point even if if is a compound object
 *
 * reject non area SHP Types
 *
 * **************************************************************************/
PT SHPPointinPoly_2d(SHPObject *psCShape)
{
    PT rPT;
    if (!(SHPDimension(psCShape->nSHPType) & SHPD_AREA))
    {
        rPT.x = NAN;
        rPT.y = NAN;
        return rPT;
    }

    PT *sPT = SHPPointsinPoly_2d(psCShape);
    if (sPT)
    {
        rPT.x = sPT[0].x;
        rPT.y = sPT[0].y;
    }
    else
    {
        rPT.x = NAN;
        rPT.y = NAN;
    }
    return (rPT);
}

/* **************************************************************************
 * SHPPointsinPoly_2d
 *
 * Return a Point inside each R+ of a potentially
 * complex/compound SHPObject suitable for labelling
 * return one point for each R+ even if it is a compound object
 *
 * reject non area SHP Types
 *
 * **************************************************************************/
PT *SHPPointsinPoly_2d(SHPObject *psCShape)
{
    if (!(SHPDimension(psCShape->nSHPType) & SHPD_AREA))
        return NULL;

    PT *PIP = NULL;
    int cRing = 0;
    int nPIP = 0;
    int rMpart, ring_nVertices;
    // TODO(schwehr): Is this a bug?  Should rLen be zero'ed on each loop?
    double rLen = 0;
    double rLenMax = 0;

    SHPObject *psO;
    while ((psO = SHPUnCompound(psCShape, &cRing)) != NULL)
    {
        double *CLx = calloc(4, sizeof(double));
        double *CLy = calloc(4, sizeof(double));
        int *CLst = calloc(2, sizeof(int));
        int *CLstt = calloc(2, sizeof(int));
        // TODO(schwehr): Check for allocation failures

        // a horizontal & vertical compound line though the middle of the extents
        CLx[0] = psO->dfXMin;
        CLy[0] = (psO->dfYMin + psO->dfYMax) * 0.5;
        CLx[1] = psO->dfXMax;
        CLy[1] = (psO->dfYMin + psO->dfYMax) * 0.5;

        CLx[2] = (psO->dfXMin + psO->dfXMax) * 0.5;
        CLy[2] = psO->dfYMin;
        CLx[3] = (psO->dfXMin + psO->dfXMax) * 0.5;
        CLy[3] = psO->dfYMax;

        CLst[0] = 0;
        CLst[1] = 2;
        CLstt[0] = SHPP_RING;
        CLstt[1] = SHPP_RING;

        SHPObject *CLine = SHPCreateObject(SHPT_POINT, -1, 2, CLst, CLstt, 4,
                                           CLx, CLy, NULL, NULL);

        /* with the H & V centrline compound object, intersect it with the OBJ	*/
        SHPObject *psInt = SHPIntersect_2d(CLine, psO);
        /* return SHP type is lowest common dimensionality of the input types 	*/

        // find the longest linestring returned by the intersection
        int ring_vtx = psInt->nVertices;
        for (int ring = (psInt->nParts - 1); ring >= 0; ring--)
        {
            ring_nVertices = ring_vtx - psInt->panPartStart[ring];

            rLen += RingLength_2d(
                ring_nVertices,
                (double *)&(psInt->padfX[psInt->panPartStart[ring]]),
                (double *)&(psInt->padfY[psInt->panPartStart[ring]]));

            if (rLen > rLenMax)
            {
                rLenMax = rLen;
                rMpart = psInt->panPartStart[ring];
            }
            ring_vtx = psInt->panPartStart[ring];
        }

        // add the centerpoint of the longest ARC of the intersection to the PIP list
        nPIP++;
        PIP = realloc(PIP, sizeof(double) * 2 * nPIP);
        PIP[nPIP].x = (psInt->padfX[rMpart] + psInt->padfX[rMpart]) * 0.5;
        PIP[nPIP].y = (psInt->padfY[rMpart] + psInt->padfY[rMpart]) * 0.5;

        SHPDestroyObject(psO);
        SHPDestroyObject(CLine);

        // does SHPCreateobject use preallocated memory or does it copy the
        // contents.  To be safe conditionally release CLx, CLy, CLst, CLstt
        free(CLx);
        free(CLy);
        free(CLst);
        free(CLstt);
    }

    return (PIP);
}

/* **************************************************************************
 * SHPCentrd_2d
 *
 * Return the single mathematical / geometric centroid of a potentially
 * complex/compound SHPObject
 *
 * reject non area SHP Types
 *
 * **************************************************************************/
PT SHPCentrd_2d(SHPObject *psCShape)
{
    PT C;
    if (!(SHPDimension(psCShape->nSHPType) & SHPD_AREA))
    {
        C.x = NAN;
        C.y = NAN;
        return C;
    }

#ifdef DEBUG
    printf("for Object with %d vtx, %d parts [ %d, %d] \n", psCShape->nVertices,
           psCShape->nParts, psCShape->panPartStart[0],
           psCShape->panPartStart[1]);
#endif

    double Area = 0;
    C.x = 0.0;
    C.y = 0.0;

    /* for each ring in compound / complex object calc the ring cntrd		*/

    double ringArea;
    PT ringCentrd;
    int ringPrev = psCShape->nVertices;
    for (int ring = (psCShape->nParts - 1); ring >= 0; ring--)
    {
        const int rStart = psCShape->panPartStart[ring];
        const int ring_nVertices = ringPrev - rStart;

        RingCentroid_2d(ring_nVertices, (double *)&(psCShape->padfX[rStart]),
                        (double *)&(psCShape->padfY[rStart]), &ringCentrd,
                        &ringArea);

#ifdef DEBUG
        printf(
            "(SHPCentrd_2d)  Ring %d, vtxs %d, area: %f, ring centrd %f, %f \n",
            ring, ring_nVertices, ringArea, ringCentrd.x, ringCentrd.y);
#endif

        /* use Superposition of these rings to build a composite Centroid		*/
        /* sum the ring centrds * ringAreas,  at the end divide by total area	*/
        C.x += ringCentrd.x * ringArea;
        C.y += ringCentrd.y * ringArea;
        Area += ringArea;
        ringPrev = rStart;
    }

    /* hold on the division by AREA until were at the end					*/
    C.x = C.x / Area;
    C.y = C.y / Area;
#ifdef DEBUG
    printf("SHPCentrd_2d) Overall Area: %f, Centrd %f, %f \n", Area, C.x, C.y);
#endif
    return (C);
}

/* **************************************************************************
 * RingCentroid_2d
 *
 * Return the mathematical / geometric centroid of a single closed ring
 *
 * **************************************************************************/
int RingCentroid_2d(int nVertices, double *a, double *b, PT *C, double *Area)
{
    /* the centroid of a closed Ring is defined as
 *
 *      Cx = sum (cx * dArea ) / Total Area
 *  and
 *      Cy = sum (cy * dArea ) / Total Area
 */
    double x_base = a[0];
    double y_base = b[0];

    double Cy_accum = 0.0;
    double Cx_accum = 0.0;

    double ppx = a[1] - x_base;
    double ppy = b[1] - y_base;
    *Area = 0;

    /* Skip the closing vector */
    for (int iv = 2; iv <= nVertices - 2; iv++)
    {
        const double x = a[iv] - x_base;
        const double y = b[iv] - y_base;

        /* calc the area and centroid of triangle built out of an arbitrary 	*/
        /* base_point on the ring and each successive pair on the ring			*/

        /* Area of a triangle is the cross product of its defining vectors		*/
        /* Centroid of a triangle is the average of its vertices				*/

        const double dx_Area = ((x * ppy) - (y * ppx)) * 0.5;
        *Area += dx_Area;

        Cx_accum += (ppx + x) * dx_Area;
        Cy_accum += (ppy + y) * dx_Area;
#ifdef DEBUG2
        printf("(ringcentrd_2d)  Pp( %f, %f), P(%f, %f)\n", ppx, ppy, x, y);
        printf("(ringcentrd_2d)    dA: %f, sA: %f, Cx: %f, Cy: %f \n", dx_Area,
               *Area, Cx_accum, Cy_accum);
#endif
        ppx = x;
        ppy = y;
    }

#ifdef DEBUG2
    printf("(ringcentrd_2d)  Cx: %f, Cy: %f \n", (Cx_accum / (*Area * 3)),
           (Cy_accum / (*Area * 3)));
#endif

    /* adjust back to world coords 											*/
    C->x = (Cx_accum / (*Area * 3)) + x_base;
    C->y = (Cy_accum / (*Area * 3)) + y_base;

    return (1);
}

/* **************************************************************************
 * SHPRingDir_2d
 *
 * Test Polygon for CW / CCW  ( R+ / R- )
 *
 * return 1  for R+
 * return -1 for R-
 * return 0  for error
 * **************************************************************************/
int SHPRingDir_2d(SHPObject *psCShape, int Ring)
{
    if (Ring >= psCShape->nParts)
        return (0);

    double tX = 0.0;
    double *a = psCShape->padfX;
    double *b = psCShape->padfY;

    int last_vtx;
    if (Ring >= psCShape->nParts - 1)
    {
        last_vtx = psCShape->nVertices;
    }
    else
    {
        last_vtx = psCShape->panPartStart[Ring + 1];
    }

    /* All vertices at the corners of the extrema (rightmost lowest, leftmost lowest, 	*/
    /* topmost rightest, ...) must be less than pi wide.  If they weren't, they couldn't be	*/
    /* extrema.																			*/
    /* of course the following will fail if the Extents are even a little wrong 			*/

    int ti;
    for (int i = psCShape->panPartStart[Ring]; i < last_vtx; i++)
    {
        if (b[i] == psCShape->dfYMax && a[i] > tX)
        {
            ti = i;
        }
    }

#ifdef DEBUG2
    printf("(shpgeo:SHPRingDir) highest Rightmost Pt is vtx %d (%f, %f)\n", ti,
           a[ti], b[ti]);
#endif

    /* cross product */
    /* the sign of the cross product of two vectors indicates the right or left half-plane	*/
    /* which we can use to indicate Ring Dir													*/
    double dx0;
    double dx1;
    double dy0;
    double dy1;
    if (ti > psCShape->panPartStart[Ring] && ti < last_vtx)
    {
        dx0 = a[ti - 1] - a[ti];
        dx1 = a[ti + 1] - a[ti];
        dy0 = b[ti - 1] - b[ti];
        dy1 = b[ti + 1] - b[ti];
    }
    else
    /* if the tested vertex is at the origin then continue from 0 */
    {
        dx1 = a[1] - a[0];
        dx0 = a[last_vtx] - a[0];
        dy1 = b[1] - b[0];
        dy0 = b[last_vtx] - b[0];
    }

    //   v1 = ( (dy0 * 0) - (0 * dy1) );
    //   v2 = ( (0 * dx1) - (dx0 * 0) );
    /* these above are always zero so why do the math */
    const double v3 = ((dx0 * dy1) - (dx1 * dy0));

#ifdef DEBUG2
    printf("(shpgeo:SHPRingDir)  cross product for vtx %d was %f \n", ti, v3);
#endif

    if (v3 > 0)
    {
        return (1);
    }
    else
    {
        return (-1);
    }
}

/* **************************************************************************
 * SHPArea_2d
 *
 * Calculate the XY Area of Polygon ( can be compound / complex )
 *
 * **************************************************************************/
double SHPArea_2d(SHPObject *psCShape)
{
    if (!(SHPDimension(psCShape->nSHPType) & SHPD_AREA))
        return (-1);

    double cArea = 0;

    /* Walk each ring adding its signed Area,  R- will return a negative 	*/
    /* area, so we don't have to test for them								*/

    /* I just start at the last ring and work down to the first				*/
    int ring_vtx = psCShape->nVertices;
    for (int ring = (psCShape->nParts - 1); ring >= 0; ring--)
    {
        const int ring_nVertices = ring_vtx - psCShape->panPartStart[ring];

#ifdef DEBUG2
        printf("(shpgeo:SHPArea_2d) part %d, vtx %d \n", ring, ring_nVertices);
#endif
        cArea += RingArea_2d(
            ring_nVertices,
            (double *)&(psCShape->padfX[psCShape->panPartStart[ring]]),
            (double *)&(psCShape->padfY[psCShape->panPartStart[ring]]));

        ring_vtx = psCShape->panPartStart[ring];
    }

#ifdef DEBUG2
    printf("(shpgeo:SHPArea_2d) Area = %f \n", cArea);
#endif

    /* Area is signed, negative Areas are R-									*/
    return (cArea);
}

/* **************************************************************************
 * SHPLength_2d
 *
 * Calculate the Planar ( XY ) Length of Polygon ( can be compound / complex )
 *    or Polyline ( can be compound ).  Length on Polygon is its Perimeter
 *
 * **************************************************************************/
double SHPLength_2d(SHPObject *psCShape)
{
    if (!(SHPDimension(psCShape->nSHPType) & (SHPD_AREA | SHPD_LINE)))
        return -1.0;

    double Length = 0;
    int j = 1;
    for (int i = 1; i < psCShape->nVertices; i++)
    {
        if (psCShape->panPartStart[j] == i)
        {
            j++;
        }
        /* skip the moves with "pen up" from ring to ring */
        else
        {
            const double dx = psCShape->padfX[i] - psCShape->padfX[i - 1];
            const double dy = psCShape->padfY[i] - psCShape->padfY[i - 1];
            Length += sqrt((dx * dx) + (dy * dy));
        }
        /* simplify this equation */
    }

    return Length;
}

/* **************************************************************************
 * RingLength_2d
 *
 * Calculate the Planar ( XY ) Length of Polygon ( can be compound / complex )
 *    or Polyline ( can be compound ).  Length of Polygon is its Perimeter
 *
 * **************************************************************************/
double RingLength_2d(int nVertices, double *a, double *b)
{
    double Length = 0;
    // int j = 1;
    for (int i = 1; i < nVertices; i++)
    {
        const double dx = a[i] - b[i - 1];
        const double dy = b[i] - b[i - 1];
        Length += sqrt((dx * dx) + (dy * dy));
        /* simplify this equation */
    }

    return (Length);
}

/* **************************************************************************
 * RingArea_2d
 *
 * Calculate the Planar Area of a single closed ring
 *
 * **************************************************************************/
double RingArea_2d(int nVertices, double *a, double *b)
{
    const double x_base = a[0];
    const double y_base = b[0];

    double ppx = a[1] - x_base;
    double ppy = b[1] - y_base;
    static double Area = 0.0;

#ifdef DEBUG2
    printf("(shpgeo:RingArea) %d vertices \n", nVertices);
#endif
    for (int iv = 2; iv <= (nVertices - 1); iv++)
    {
        const double x = a[iv] - x_base;
        const double y = b[iv] - y_base;

        /* Area of a triangle is the cross product of its defining vectors		*/

        const double dx_Area = ((x * ppy) - (y * ppx)) * 0.5;

        Area += dx_Area;
#ifdef DEBUG2
        printf("(shpgeo:RingArea)  dxArea %f  sArea %f for pt(%f, %f)\n",
               dx_Area, Area, x, y);
#endif

        ppx = x;
        ppy = y;
    }

#ifdef DEBUG2
    printf("(shpgeo:RingArea)  total RingArea %f \n", Area);
#endif
    return Area;
}

/* **************************************************************************
 * SHPUnCompound
 *
 * ESRI calls this function explode
 * Return a non compound ( possibly complex ) object
 *
 * ring_number is R+ number corresponding to object
 *
 * ignore complexity in Z dimension for now
 *
 * **************************************************************************/
SHPObject *SHPUnCompound(SHPObject *psCShape, int *ringNumber)
{
    if (*ringNumber >= psCShape->nParts || *ringNumber == -1)
    {
        *ringNumber = -1;
        return NULL;
    }

    if (*ringNumber == (psCShape->nParts - 1))
    {
        *ringNumber = -1;
        return (SHPClone(psCShape, (psCShape->nParts - 1), -1));
    }

    const int lRing = *ringNumber;
    int ringDir = -1;
    int ring = (lRing + 1);
    for (; (ring < psCShape->nParts) && (ringDir < 0); ring++)
        ringDir = SHPRingDir_2d(psCShape, ring);

    if (ring == psCShape->nParts)
        *ringNumber = -1;
    else
        *ringNumber = ring;
        /*    I am strictly assuming that all R- parts of a complex object
 *	   directly follow their R+, so when we hit a new R+ its a
 *	   new part of a compound object
 *         a SHPClean may be needed to enforce this as it is not part
 *	   of ESRI's definition of a SHPfile
 */

#ifdef DEBUG2
    printf("(SHPUnCompound) asked for ring %d, lastring is %d \n", lRing, ring);
#endif
    return (SHPClone(psCShape, lRing, ring));
}

/* **************************************************************************
 * SHPIntersect_2d
 *
 *
 * prototype only for now
 *
 * return object with lowest common dimensionality of objects
 *
 * **************************************************************************/
SHPObject *SHPIntersect_2d(SHPObject *a, SHPObject *b)
{
    if ((SHPDimension(a->nSHPType) && SHPD_POINT) ||
        (SHPDimension(b->nSHPType) && SHPD_POINT))
        return (NULL);
    /* there is no intersect function like this for points  */

    SHPObject *C = SHPClone(a, 0, -1);

    return C;
}

/* **************************************************************************
 * SHPClean
 *
 * Test and fix normalization problems in shapes
 * Different tests need to be implemented for different SHPTypes
 * 	SHPT_POLYGON	check ring directions CW / CCW   ( R+ / R- )
 *				put all R- after the R+ they are members of
 *				i.e. each complex object is completed before the
 *     				next object is started
 *				check for closed rings
 *				ring must not intersect itself, even on edge
 *
 *  no other types implemented yet
 *
 * not sure why but return object in place
 * use for object casting and object verification
 * **************************************************************************/
int SHPClean(SHPObject *psCShape)
{
    return (0);
}

/* **************************************************************************
 * SHPClone
 *
 * Clone a SHPObject, replicating all data
 *
 * **************************************************************************/
SHPObject *SHPClone(SHPObject *psCShape, int lowPart, int highPart)
{
    if (highPart >= psCShape->nParts || highPart == -1)
        highPart = psCShape->nParts;

#ifdef DEBUG
    printf(" cloning SHP (%d parts) from ring %d to ring %d \n",
           psCShape->nParts, lowPart, highPart);
#endif

    const int newParts = highPart - lowPart;
    if (newParts == 0)
    {
        return (NULL);
    }

    SHPObject *psObject = (SHPObject *)calloc(1, sizeof(SHPObject));
    psObject->nSHPType = psCShape->nSHPType;
    psObject->nShapeId = psCShape->nShapeId;

    psObject->nParts = newParts;
    if (psCShape->padfX)
    {
        psObject->panPartStart = (int *)calloc(newParts, sizeof(int));
        memcpy(psObject->panPartStart, psCShape->panPartStart,
               newParts * sizeof(int));

        psObject->panPartType = (int *)calloc(newParts, sizeof(int));
        memcpy(psObject->panPartType, (int *)&(psCShape->panPartType[lowPart]),
               newParts * sizeof(int));
    }

    int newVertices;
    if (highPart != psCShape->nParts)
    {
        newVertices =
            psCShape->panPartStart[highPart] - psCShape->panPartStart[lowPart];
    }
    else
    {
        newVertices = psCShape->nVertices - psCShape->panPartStart[lowPart];
    }

#ifdef DEBUG
    int i;
    if (highPart = psCShape->nParts)
        i = psCShape->nVertices;
    else
        i = psCShape->panPartStart[highPart];
    printf(" from part %d (%d) to %d (%d) is %d vertices \n", lowPart,
           psCShape->panPartStart[lowPart], highPart, i, newVertices);
#endif
    psObject->nVertices = newVertices;
    if (psCShape->padfX)
    {
        psObject->padfX = (double *)calloc(newVertices, sizeof(double));
        memcpy(psObject->padfX,
               (double *)&(psCShape->padfX[psCShape->panPartStart[lowPart]]),
               newVertices * sizeof(double));
    }
    if (psCShape->padfY)
    {
        psObject->padfY = (double *)calloc(newVertices, sizeof(double));
        memcpy(psObject->padfY,
               (double *)&(psCShape->padfY[psCShape->panPartStart[lowPart]]),
               newVertices * sizeof(double));
    }
    if (psCShape->padfZ)
    {
        psObject->padfZ = (double *)calloc(newVertices, sizeof(double));
        memcpy(psObject->padfZ,
               (double *)&(psCShape->padfZ[psCShape->panPartStart[lowPart]]),
               newVertices * sizeof(double));
    }
    if (psCShape->padfM)
    {
        psObject->padfM = (double *)calloc(newVertices, sizeof(double));
        memcpy(psObject->padfM,
               (double *)&(psCShape->padfM[psCShape->panPartStart[lowPart]]),
               newVertices * sizeof(double));
    }

    psObject->dfXMin = psCShape->dfXMin;
    psObject->dfYMin = psCShape->dfYMin;
    psObject->dfZMin = psCShape->dfZMin;
    psObject->dfMMin = psCShape->dfMMin;

    psObject->dfXMax = psCShape->dfXMax;
    psObject->dfYMax = psCShape->dfYMax;
    psObject->dfZMax = psCShape->dfZMax;
    psObject->dfMMax = psCShape->dfMMax;

    SHPComputeExtents(psObject);
    return (psObject);
}

/************************************************************************/
/*  SwapG 				                              	*/
/*                                                                      */
/*      Swap a 2, 4 or 8 byte word.                                     */
/************************************************************************/
void SwapG(void *so, void *in, int this_cnt, int this_size)
{
    // return to a new pointer otherwise it would invalidate existing data
    // as prevent further use of it
    for (int j = 0; j < this_cnt; j++)
    {
        for (int i = 0; i < this_size / 2; i++)
        {
            ((unsigned char *)so)[i] = ((unsigned char *)in)[this_size - i - 1];
            ((unsigned char *)so)[this_size - i - 1] = ((unsigned char *)in)[i];
        }
    }
}

/* **************************************************************************
 * SwapW
 *
 * change byte order on an array of 16 bit words
 * need to change this over to shapelib, Frank Warmerdam's functions
 *
 * **************************************************************************/
void swapW(void *so, unsigned char *in, long bytes)
{
    const unsigned char map[4] = {3, 2, 1, 0};
    unsigned char *out = so;
    for (int i = 0; i <= (bytes / 4); i++)
        for (int j = 0; j < 4; j++)
            out[(i * 4) + map[j]] = in[(i * 4) + j];
}

/* **************************************************************************
 * SwapD
 *
 * change byte order on an array of (double) 32 bit words
 * need to change this over to shapelib, Frank Warmerdam's functions
 *
 * **************************************************************************/
void swapD(void *so, unsigned char *in, long bytes)
{
    const unsigned char map[8] = {7, 6, 5, 4, 3, 2, 1, 0};
    unsigned char *out = so;
    for (int i = 0; i <= (bytes / 8); i++)
        for (int j = 0; j < 8; j++)
            out[(i * 8) + map[j]] = in[(i * 8) + j];
}
