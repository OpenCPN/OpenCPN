/******************************************************************************
 *
 * Project:  Shapelib
 * Purpose:
 *   Altered "shpdump" and "dbfdump" to allow two files to be appended.
 *   Other Functions:
 *     Selecting from the DBF before the write occurs.
 *     Change the UNITS between Feet and Meters and Shift X,Y.
 *     Clip and Erase boundary.  The program only passes thru the
 *     data once.
 *
 *   Bill Miller   North Carolina - Department of Transportation
 *   Feb. 1997 -- bmiller@dot.state.nc.us
 *         There was not a lot of time to debug hidden problems;
 *         And the code is not very well organized or documented.
 *         The clip/erase function was not well tested.
 *   Oct. 2000 -- bmiller@dot.state.nc.us
 *         Fixed the problem when select is using numbers
 *         larger than short integer.  It now reads long integer.
 *   NOTE: DBF files created using windows NT will read as a string with
 *         a length of 381 characters.  This is a bug in "dbfopen".
 *
 * Author:   Bill Miller (bmiller@dot.state.nc.us)
 *
 ******************************************************************************
 * Copyright (c) 1999, Frank Warmerdam
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.0-or-later
 ******************************************************************************
 *
 */

#include "shapefil.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char infile[80], outfile[80], temp[400];

/* Variables for shape files */
SHPHandle hSHP;
SHPHandle hSHPappend;
int nShapeType, nEntities, iPart;
int nShapeTypeAppend, nEntitiesAppend;
SHPObject *psCShape;
double adfBoundsMin[4], adfBoundsMax[4];

/* Variables for DBF files */
DBFHandle hDBF;
DBFHandle hDBFappend;

DBFFieldType iType;
DBFFieldType jType;

char iszTitle[12];
char jszTitle[12];

int *pt;  // TODO(schwehr): Danger.  Shadowed
char iszFormat[32], iszField[1024];
char jszFormat[32], jszField[1024];
int ti, iWidth, iDecimals;
int tj, jWidth, jDecimals;

/* -------------------------------------------------------------------- */
/* Variables for the DESCRIBE function */
/* -------------------------------------------------------------------- */
bool ilist = false;
bool iall = false;
/* -------------------------------------------------------------------- */
/* Variables for the SELECT function */
/* -------------------------------------------------------------------- */
bool found = false;
bool newdbf = false;
char selectitem[40], *cpt;
long int selectvalues[150], selcount = 0;
bool iselect = false;
int iselectitem = -1;
bool iunselect = false;

/* -------------------------------------------------------------------- */
/* Variables for the CLIP and ERASE functions */
/* -------------------------------------------------------------------- */
double cxmin, cymin, cxmax, cymax;
bool iclip = false;
bool ierase = false;
bool itouch = false;
bool iinside = false;
bool icut = false;
char clipfile[80];

/* -------------------------------------------------------------------- */
/* Variables for the FACTOR function */
/* -------------------------------------------------------------------- */
double infactor, outfactor, factor = 0; /* NO FACTOR */
bool iunit = false;

/* -------------------------------------------------------------------- */
/* Variables for the SHIFT function */
/* -------------------------------------------------------------------- */
double xshift = 0, yshift = 0; /* NO SHIFT */

/* -------------------------------------------------------------------- */
/*	Change the extension.  If there is any extension on the 	*/
/*	filename, strip it off and add the new extension                */
/* -------------------------------------------------------------------- */
void setext(char *pt, const char *ext)
{
    int i = strlen(pt) - 1;
    for (; i > 0 && pt[i] != '.' && pt[i] != '/' && pt[i] != '\\'; i--)
    {
    }

    if (pt[i] == '.')
        pt[i] = '\0';

    strcat(pt, ".");
    strcat(pt, ext);
}

/************************************************************************/
/*                             openfiles()                              */
/************************************************************************/

void openfiles()
{
    /* -------------------------------------------------------------------- */
    /*      Open the DBF file.                                              */
    /* -------------------------------------------------------------------- */
    setext(infile, "dbf");
    hDBF = DBFOpen(infile, "rb");
    if (hDBF == NULL)
    {
        printf("ERROR: Unable to open the input DBF:%s\n", infile);
        exit(1);
    }
    /* -------------------------------------------------------------------- */
    /*      Open the append DBF file.                                       */
    /* -------------------------------------------------------------------- */
    if (strcmp(outfile, ""))
    {
        setext(outfile, "dbf");
        hDBFappend = DBFOpen(outfile, "rb+");
        newdbf = false;
        if (hDBFappend == NULL)
        {
            newdbf = true;
            hDBFappend = DBFCreate(outfile);
            if (hDBFappend == NULL)
            {
                printf("ERROR: Unable to open the append DBF:%s\n", outfile);
                exit(1);
            }
        }
    }
    /* -------------------------------------------------------------------- */
    /*      Open the passed shapefile.                                      */
    /* -------------------------------------------------------------------- */
    setext(infile, "shp");
    hSHP = SHPOpen(infile, "rb");

    if (hSHP == NULL)
    {
        printf("ERROR: Unable to open the input shape file:%s\n", infile);
        exit(1);
    }

    SHPGetInfo(hSHP, &nEntities, &nShapeType, NULL, NULL);

    /* -------------------------------------------------------------------- */
    /*      Open the passed append shapefile.                               */
    /* -------------------------------------------------------------------- */
    if (strcmp(outfile, ""))
    {
        setext(outfile, "shp");
        hSHPappend = SHPOpen(outfile, "rb+");

        if (hSHPappend == NULL)
        {
            hSHPappend = SHPCreate(outfile, nShapeType);
            if (hSHPappend == NULL)
            {
                printf("ERROR: Unable to open the append shape file:%s\n",
                       outfile);
                exit(1);
            }
        }
        SHPGetInfo(hSHPappend, &nEntitiesAppend, &nShapeTypeAppend, NULL, NULL);

        if (nShapeType != nShapeTypeAppend)
        {
            puts("ERROR: Input and Append shape files are of different types.");
            exit(1);
        }
    }
}

/* -------------------------------------------------------------------- */
/*	Find matching fields in the append file.                        */
/*      Output file must have zero records to add any new fields.       */
/* -------------------------------------------------------------------- */
void mergefields()
{
    ti = DBFGetFieldCount(hDBF);
    tj = DBFGetFieldCount(hDBFappend);
    /* Create a pointer array for the max # of fields in the output file */
    pt = (int *)malloc((ti + tj + 1) * sizeof(int));

    for (int i = 0; i < ti; i++)
    {
        pt[i] = -1; /* Initial pt values to -1 */
    }
    /* DBF must be empty before adding items */
    const int jRecord = DBFGetRecordCount(hDBFappend);
    int j;
    for (int i = 0; i < ti; i++)
    {
        iType = DBFGetFieldInfo(hDBF, i, iszTitle, &iWidth, &iDecimals);
        found = false;

        for (j = 0; j < tj; j++) /* Search all field names for a match */
        {
            jType =
                DBFGetFieldInfo(hDBFappend, j, jszTitle, &jWidth, &jDecimals);
            if (iType == jType && (strcmp(iszTitle, jszTitle) == 0))
            {
                if (found || newdbf)
                {
                    if (i == j)
                        pt[i] = j;
                    printf("Warning: Duplicate field name found (%s)\n",
                           iszTitle);
                    /* Duplicate field name
	               (Try to guess the correct field by position) */
                }
                else
                {
                    pt[i] = j;
                    found = true;
                }
            }
        }

        if (pt[i] == -1 && (!found)) /* Try to force into an existing field */
        { /* Ignore the field name, width, and decimal places */
            jType =
                DBFGetFieldInfo(hDBFappend, j, jszTitle, &jWidth, &jDecimals);
            if (iType == jType)
            {
                pt[i] = i;
                found = true;
            }
        }
        if ((!found) &&
            jRecord == 0) /* Add missing field to the append table */
        {                 /* The output DBF must be is empty */
            pt[i] = tj;
            tj++;
            if (DBFAddField(hDBFappend, iszTitle, iType, iWidth, iDecimals) ==
                -1)
            {
                printf("Warning: DBFAddField(%s, TYPE:%d, WIDTH:%d  DEC:%d, "
                       "ITEM#:%d of %d) failed.\n",
                       iszTitle, iType, iWidth, iDecimals, (i + 1), (ti + 1));
                pt[i] = -1;
            }
        }
    }
}

/************************************************************************/
/*                            strncasecmp2()                            */
/*                                                                      */
/*      Compare two strings up to n characters                          */
/*      If n=0 then s1 and s2 must be an exact match                    */
/************************************************************************/
int strncasecmp2(char *s1, char *s2, int n)
{
    if (n < 1)
        n = strlen(s1) + 1;

    for (int i = 0; i < n; i++)
    {
        if (*s1 != *s2)
        {
            if (*s1 >= 'a' && *s1 <= 'z')
            {
                const int j = *s1 - 32;
                if (j != *s2)
                    return (*s1 - *s2);
            }
            else
            {
                int j;
                if (*s1 >= 'A' && *s1 <= 'Z')
                {
                    j = *s1 + 32;
                }
                else
                {
                    j = *s1;
                }
                if (j != *s2)
                    return (*s1 - *s2);
            }
        }
        s1++;
        s2++;
    }
    return (0);
}

void showitems()
{
    printf("Available Items: (%d)", ti);
    long int maxrec = DBFGetRecordCount(hDBF);
    if (maxrec > 5000 && !iall)
    {
        maxrec = 5000;
        printf("  ** ESTIMATED RANGES (MEAN)  For more records use \"All\"");
    }
    else
    {
        printf("          RANGES (MEAN)");
    }

    char stmp[40] = {0};
    char slow[40] = {0};
    char shigh[40] = {0};

    for (int i = 0; i < ti; i++)
    {
        switch (DBFGetFieldInfo(hDBF, i, iszTitle, &iWidth, &iDecimals))
        {
            case FTString:
            case FTLogical:
            case FTDate:
                strcpy(slow, "~");
                strcpy(shigh, "\0");
                printf("\n  String  %3d  %-16s", iWidth, iszTitle);
                for (int iRecord = 0; iRecord < maxrec; iRecord++)
                {
                    strncpy(stmp, DBFReadStringAttribute(hDBF, iRecord, i), 39);
                    if (strcmp(stmp, "!!") > 0)
                    {
                        if (strncasecmp2(stmp, slow, 0) < 0)
                            memcpy(slow, stmp, 39);
                        if (strncasecmp2(stmp, shigh, 0) > 0)
                            memcpy(shigh, stmp, 39);
                    }
                }
                char *pt = slow + strlen(slow) - 1;
                while (*pt == ' ')
                {
                    *pt = '\0';
                    pt--;
                }
                pt = shigh + strlen(shigh) - 1;
                while (*pt == ' ')
                {
                    *pt = '\0';
                    pt--;
                }
                if (strncasecmp2(slow, shigh, 0) < 0)
                    printf("%s to %s", slow, shigh);
                else if (strncasecmp2(slow, shigh, 0) == 0)
                    printf("= %s", slow);
                else
                    printf("No Values");
                break;
            case FTInteger:
            {
                printf("\n  Integer %3d  %-16s", iWidth, iszTitle);
                long int ilow = 1999999999;
                long int ihigh = -1999999999;
                long int isum = 0;
                for (int iRecord = 0; iRecord < maxrec; iRecord++)
                {
                    const long int itmp =
                        DBFReadIntegerAttribute(hDBF, iRecord, i);
                    if (ilow > itmp)
                        ilow = itmp;
                    if (ihigh < itmp)
                        ihigh = itmp;
                    isum = isum + itmp;
                }
                const double mean = isum / maxrec;
                if (ilow < ihigh)
                    printf("%ld to %ld \t(%.1f)", ilow, ihigh, mean);
                else if (ilow == ihigh)
                    printf("= %ld", ilow);
                else
                    printf("No Values");
                break;
            }
            case FTDouble:
            {
                printf("\n  Real  %3d,%d  %-16s", iWidth, iDecimals, iszTitle);
                double dlow = 999999999999999.0;
                double dhigh = -999999999999999.0;
                double dsum = 0;
                for (int iRecord = 0; iRecord < maxrec; iRecord++)
                {
                    const double dtmp =
                        DBFReadDoubleAttribute(hDBF, iRecord, i);
                    if (dlow > dtmp)
                        dlow = dtmp;
                    if (dhigh < dtmp)
                        dhigh = dtmp;
                    dsum = dsum + dtmp;
                }
                const double mean = dsum / maxrec;
                sprintf(stmp, "%%.%df to %%.%df \t(%%.%df)", iDecimals,
                        iDecimals, iDecimals);
                if (dlow < dhigh)
                    printf(stmp, dlow, dhigh, mean);
                else if (dlow == dhigh)
                {
                    sprintf(stmp, "= %%.%df", iDecimals);
                    printf(stmp, dlow);
                }
                else
                    printf("No Values");
                break;
            }
            case FTInvalid:
                break;
        }
    }
    printf("\n");
}

void findselect()
{
    /* Find the select field name */
    iselectitem = -1;
    for (int i = 0; i < ti && iselectitem < 0; i++)
    {
        iType = DBFGetFieldInfo(hDBF, i, iszTitle, &iWidth, &iDecimals);
        if (strncasecmp2(iszTitle, selectitem, 0) == 0)
            iselectitem = i;
    }
    if (iselectitem == -1)
    {
        printf("Warning: Item not found for selection (%s)\n", selectitem);
        iselect = false;
        iall = false;
        showitems();
        printf("Continued... (Selecting entire file)\n");
    }
    /* Extract all of the select values (by field type) */
}

int selectrec(int iRecord)
{
    const long int ty =
        DBFGetFieldInfo(hDBF, iselectitem, NULL, &iWidth, &iDecimals);
    switch (ty)
    {
        case FTString:
            puts("Invalid Item");
            iselect = false;
            break;
        case FTInteger:
        {
            const long int value =
                DBFReadIntegerAttribute(hDBF, iRecord, iselectitem);
            for (int j = 0; j < selcount; j++)
            {
                if (selectvalues[j] == value)
                {
                    if (iunselect)
                        return (0); /* Keep this record */
                    else
                        return (1); /* Skip this record */
                }
            }
            break;
        }
        case FTDouble:
            puts("Invalid Item");
            iselect = false;
            break;
    }
    if (iunselect)
        return (1); /* Skip this record */
    else
        return (0); /* Keep this record */
}

void check_theme_bnd()
{
    if ((adfBoundsMin[0] >= cxmin) && (adfBoundsMax[0] <= cxmax) &&
        (adfBoundsMin[1] >= cymin) && (adfBoundsMax[1] <= cymax))
    { /** Theme is totally inside clip area **/
        if (ierase)
            nEntities = 0; /** SKIP THEME  **/
        else
            iclip = false; /** WRITE THEME (Clip not needed) **/
    }

    if (((adfBoundsMin[0] < cxmin) && (adfBoundsMax[0] < cxmin)) ||
        ((adfBoundsMin[1] < cymin) && (adfBoundsMax[1] < cymin)) ||
        ((adfBoundsMin[0] > cxmax) && (adfBoundsMax[0] > cxmax)) ||
        ((adfBoundsMin[1] > cymax) && (adfBoundsMax[1] > cymax)))
    { /** Theme is totally outside clip area **/
        if (ierase)
            iclip = false; /** WRITE THEME (Clip not needed) **/
        else
            nEntities = 0; /** SKIP THEME  **/
    }

    if (nEntities == 0)
        puts("WARNING: Theme is outside the clip area."); /** SKIP THEME  **/
}

int clip_boundary()
{
    /*** FIRST check the boundary of the feature ***/
    if (((psCShape->dfXMin < cxmin) && (psCShape->dfXMax < cxmin)) ||
        ((psCShape->dfYMin < cymin) && (psCShape->dfYMax < cymin)) ||
        ((psCShape->dfXMin > cxmax) && (psCShape->dfXMax > cxmax)) ||
        ((psCShape->dfYMin > cymax) && (psCShape->dfYMax > cymax)))
    { /** Feature is totally outside clip area **/
        if (ierase)
            return (1); /** WRITE RECORD **/
        else
            return (0); /** SKIP  RECORD **/
    }

    if ((psCShape->dfXMin >= cxmin) && (psCShape->dfXMax <= cxmax) &&
        (psCShape->dfYMin >= cymin) && (psCShape->dfYMax <= cymax))
    { /** Feature is totally inside clip area **/
        if (ierase)
            return (0); /** SKIP  RECORD **/
        else
            return (1); /** WRITE RECORD **/
    }

    if (iinside)
    { /** INSIDE * Feature might touch the boundary or could be outside **/
        if (ierase)
            return (1); /** WRITE RECORD **/
        else
            return (0); /** SKIP  RECORD **/
    }

    if (itouch)
    { /** TOUCH **/
        if (((psCShape->dfXMin <= cxmin) || (psCShape->dfXMax >= cxmax)) &&
            (psCShape->dfYMin >= cymin) && (psCShape->dfYMax <= cymax))
        { /** Feature intersects the clip boundary only on the X axis **/
            if (ierase)
                return (0); /** SKIP  RECORD **/
            else
                return (1); /** WRITE RECORD **/
        }

        if ((psCShape->dfXMin >= cxmin) && (psCShape->dfXMax <= cxmax) &&
            ((psCShape->dfYMin <= cymin) || (psCShape->dfYMax >= cymax)))
        { /** Feature intersects the clip boundary only on the Y axis **/
            if (ierase)
                return (0); /** SKIP  RECORD **/
            else
                return (1); /** WRITE RECORD **/
        }

        for (int j2 = 0; j2 < psCShape->nVertices; j2++)
        { /** At least one vertex must be inside the clip boundary **/
            if ((psCShape->padfX[j2] >= cxmin &&
                 psCShape->padfX[j2] <= cxmax) ||
                (psCShape->padfY[j2] >= cymin && psCShape->padfY[j2] <= cymax))
            {
                if (ierase)
                    return (0); /** SKIP  RECORD **/
                else
                    return (1); /** WRITE RECORD **/
            }
        }

        /** All vertices are outside the clip boundary **/
        if (ierase)
            return (1); /** WRITE RECORD **/
        else
            return (0); /** SKIP  RECORD **/
    }                   /** End TOUCH **/

    if (icut)
    { /** CUT **/
        /*** Check each vertex in the feature with the Boundary and "CUT" ***/
        /*** THIS CODE WAS NOT COMPLETED!  READ NOTE AT THE BOTTOM ***/
        int i2 = 0;
        bool prev_outside = false;
        for (int j2 = 0; j2 < psCShape->nVertices; j2++)
        {
            bool inside =
                psCShape->padfX[j2] >= cxmin && psCShape->padfX[j2] <= cxmax &&
                psCShape->padfY[j2] >= cymin && psCShape->padfY[j2] <= cymax;

            if (ierase)
                inside = !inside;
            if (inside)
            {
                if (i2 != j2)
                {
                    if (prev_outside)
                    {
                        /*** AddIntersection(i2); ***/ /*** Add intersection ***/
                        prev_outside = false;
                    }
                    psCShape->padfX[i2] =
                        psCShape->padfX[j2]; /** move vertex **/
                    psCShape->padfY[i2] = psCShape->padfY[j2];
                }
                i2++;
            }
            else
            {
                if ((!prev_outside) && (j2 > 0))
                {
                    /*** AddIntersection(i2); ***/ /*** Add intersection (Watch out for j2==i2-1) ***/
                    /*** Also a polygon may overlap twice and will split into a several parts  ***/
                    prev_outside = true;
                }
            }
        }

        printf("Vertices:%d   OUT:%d   Number of Parts:%d\n",
               psCShape->nVertices, i2, psCShape->nParts);

        psCShape->nVertices = i2;

        if (i2 < 2)
            return (0); /** SKIP RECORD **/
        /*** (WE ARE NOT CREATING INTERSECTIONS and some lines could be reduced to one point) **/

        // if (i2 == 0) return(0); /** SKIP  RECORD **/
        // else
        return (1); /** WRITE RECORD **/
    }               /** End CUT **/

    return 0;
}

#define NKEYS (sizeof(unitkeytab) / sizeof(struct unitkey))
double findunit(char *unit)
{
    struct unitkey
    {
        char *name;
        double value;
    } unitkeytab[] = {{"CM", 39.37},           {"CENTIMETER", 39.37},
                      {"CENTIMETERS", 39.37}, /** # of inches * 100 in unit **/
                      {"METER", 3937},         {"METERS", 3937},
                      {"KM", 3937000},         {"KILOMETER", 3937000},
                      {"KILOMETERS", 3937000}, {"INCH", 100},
                      {"INCHES", 100},         {"FEET", 1200},
                      {"FOOT", 1200},          {"YARD", 3600},
                      {"YARDS", 3600},         {"MILE", 6336000},
                      {"MILES", 6336000}};

    double unitfactor = 0;
    for (int j = 0; j < (int)NKEYS; j++)
    {
        if (strncasecmp2(unit, unitkeytab[j].name, 0) == 0)
            unitfactor = unitkeytab[j].value;
    }
    return (unitfactor);
}

/* -------------------------------------------------------------------- */
/*      Display a usage message.                                        */
/* -------------------------------------------------------------------- */
void error()
{
    puts("The program will append to an existing shape file or it will");
    puts("create a new file if needed.");
    puts("Only the items in the first output file will be preserved.");
    puts("When an item does not match with the append theme then the item");
    puts("might be placed to an existing item at the same position and type.");
    puts("  OTHER FUNCTIONS:");
    puts("  - Describe all items in the dbase file (Use ALL for more than 5000 "
         "recs.)");
    puts("  - Select a group of shapes from a comma separated selection list.");
    puts("  - UnSelect a group of shapes from a comma separated selection "
         "list.");
    puts("  - Clip boundary extent or by theme boundary.");
    puts("      Touch writes all the shapes that touch the boundary.");
    puts("      Inside writes all the shapes that are completely within the "
         "boundary.");
    puts("      Boundary clips are only the min and max of a theme boundary.");
    puts("  - Erase boundary extent or by theme boundary.");
    puts("      Erase is the direct opposite of the Clip function.");
    puts("  - Change coordinate value units between meters and feet.");
    puts("      There is no way to determine the input unit of a shape file.");
    puts("      Skip this function if the shape file is already in the correct "
         "unit.");
    puts("      Clip and Erase will be done before the unit is changed.");
    puts("      A shift will be done after the unit is changed.");
    puts("  - Shift X and Y coordinates.\n");
    puts("Finally, There can only be one select or unselect in the command "
         "line.");
    puts("         There can only be one clip or erase in the command line.");
    puts("         There can only be one unit and only one shift in the "
         "command line.\n");
    puts("Ex: shputils in.shp out.shp   SELECT countycode 3,5,9,13,17,27");
    puts("    shputils in.shp out.shp   CLIP   10 10 90 90 Touch   FACTOR "
         "Meter Feet");
    puts("    shputils in.shp out.shp   FACTOR Meter 3.0");
    puts("    shputils in.shp out.shp   CLIP   clip.shp Boundary Touch   SHIFT "
         "40 40");
    puts("    shputils in.shp out.shp   SELECT co 112   CLIP clip.shp Boundary "
         "Touch\n");
    puts("USAGE: shputils  <DescribeShape>   {ALL}");
    puts("USAGE: shputils  <InputShape>  <OutShape|AppendShape>");
    puts("   { <FACTOR>       <FEET|MILES|METERS|KM> "
         "<FEET|MILES|METERS|KM|factor> }");
    puts("   { <SHIFT>        <xshift> <yshift> }");
    puts("   { <SELECT|UNSEL> <Item> <valuelist> }");
    puts(
        "   { <CLIP|ERASE>   <xmin> <ymin> <xmax> <ymax> <TOUCH|INSIDE|CUT> }");
    puts(
        "   { <CLIP|ERASE>   <theme>      <BOUNDARY>     <TOUCH|INSIDE|CUT> }");
    puts("     Note: CUT is not complete and does not create intersections.");
    puts("           For more information read programmer comment.");

    /****   Clip functions for Polygon and Cut is not supported
            There are several web pages that describe methods of doing this function.
            It seem easy to implement until you start writing code.  I don't have the
            time to add these functions but a did leave a simple cut routine in the
            program that can be called by using CUT instead of TOUCH in the
            CLIP or ERASE functions.  It does not add the intersection of the line and
            the clip box, so polygons could look incomplete and lines will come up short.

            Information about clipping lines with a box:
            http://www.csclub.uwaterloo.ca/u/mpslager/articles/sutherland/wr.html
            Information about finding the intersection of two lines:
            http://www.whisqu.se/per/docs/math28.htm

            THE CODE LOOKS LIKE THIS:
            ********************************************************
            void Intersect_Lines(float x0,float y0,float x1,float y1,
            float x2,float y2,float x3,float y3,
            float *xi,float *yi)
            {
//  this function computes the intersection of the sent lines
//  and returns the intersection point, note that the function assumes
//  the lines intersect. the function can handle vertical as well
//  as horizontal lines. note the function isn't very clever, it simply
//  applies the math, but we don't need speed since this is a
//  pre-processing step
//  The Intersect_lines program came from (http://www.whisqu.se/per/docs/math28.htm)

float a1,b1,c1, // constants of linear equations
a2,b2,c2,
det_inv,  // the inverse of the determinant of the coefficientmatrix
m1,m2;    // the slopes of each line

// compute slopes, note the cludge for infinity, however, this will
// be close enough
if ((x1-x0)!=0)
m1 = (y1-y0)/(x1-x0);
else
m1 = (float)1e+10;  // close enough to infinity


if ((x3-x2)!=0)
m2 = (y3-y2)/(x3-x2);
else
m2 = (float)1e+10;  // close enough to infinity

// compute constants
a1 = m1;
a2 = m2;
b1 = -1;
b2 = -1;
c1 = (y0-m1*x0);
c2 = (y2-m2*x2);
// compute the inverse of the determinate
det_inv = 1/(a1*b2 - a2*b1);
// use Kramers rule to compute xi and yi
*xi=((b1*c2 - b2*c1)*det_inv);
*yi=((a2*c1 - a1*c2)*det_inv);
} // end Intersect_Lines
    **********************************************************/

    exit(1);
}

int main(int argc, char **argv)
{
    // Check command line usage.
    if (argc < 2)
        error();
    strcpy(infile, argv[1]);
    if (argc > 2)
    {
        strcpy(outfile, argv[2]);
        if (strncasecmp2(outfile, "LIST", 0) == 0)
        {
            ilist = true;
        }
        if (strncasecmp2(outfile, "ALL", 0) == 0)
        {
            iall = true;
        }
    }
    if (ilist || iall || argc == 2)
    {
        setext(infile, "shp");
        printf("DESCRIBE: %s\n", infile);
        strcpy(outfile, "");
    }

    // Look for other functions on the command line. (SELECT, UNIT)
    for (int i = 3; i < argc; i++)
    {
        if ((strncasecmp2(argv[i], "SEL", 3) == 0) ||
            (strncasecmp2(argv[i], "UNSEL", 5) == 0))
        {
            if (strncasecmp2(argv[i], "UNSEL", 5) == 0)
                iunselect = true;
            i++;
            if (i >= argc)
                error();
            strcpy(selectitem, argv[i]);
            i++;
            if (i >= argc)
                error();
            selcount = 0;
            strcpy(temp, argv[i]);
            cpt = temp;
            tj = atoi(cpt);
            ti = 0;
            while (tj > 0)
            {
                selectvalues[selcount] = tj;
                while (*cpt >= '0' && *cpt <= '9')
                    cpt++;
                while (*cpt > '\0' && (*cpt < '0' || *cpt > '9'))
                    cpt++;
                tj = atoi(cpt);
                selcount++;
            }
            iselect = true;
        } /*** End SEL & UNSEL ***/
        else if ((strncasecmp2(argv[i], "CLIP", 4) == 0) ||
                 (strncasecmp2(argv[i], "ERASE", 5) == 0))
        {
            if (strncasecmp2(argv[i], "ERASE", 5) == 0)
                ierase = true;
            i++;
            if (i >= argc)
                error();
            strcpy(clipfile, argv[i]);
            sscanf(argv[i], "%lf", &cxmin);
            i++;
            if (i >= argc)
                error();
            if (strncasecmp2(argv[i], "BOUND", 5) == 0)
            {
                setext(clipfile, "shp");
                hSHP = SHPOpen(clipfile, "rb");
                if (hSHP == NULL)
                {
                    printf("ERROR: Unable to open the clip shape file:%s\n",
                           clipfile);
                    exit(1);
                }
                SHPGetInfo(hSHPappend, NULL, NULL, adfBoundsMin, adfBoundsMax);
                cxmin = adfBoundsMin[0];
                cymin = adfBoundsMin[1];
                cxmax = adfBoundsMax[0];
                cymax = adfBoundsMax[1];
                printf("Theme Clip Boundary: (%lf,%lf) - (%lf,%lf)\n", cxmin,
                       cymin, cxmax, cymax);
            }
            else
            { /*** xmin,ymin,xmax,ymax ***/
                sscanf(argv[i], "%lf", &cymin);
                i++;
                if (i >= argc)
                    error();
                sscanf(argv[i], "%lf", &cxmax);
                i++;
                if (i >= argc)
                    error();
                sscanf(argv[i], "%lf", &cymax);
                printf("Clip Box: (%lf,%lf) - (%lf,%lf)\n", cxmin, cymin, cxmax,
                       cymax);
            }
            i++;
            if (i >= argc)
                error();
            if (strncasecmp2(argv[i], "CUT", 3) == 0)
                icut = true;
            else if (strncasecmp2(argv[i], "TOUCH", 5) == 0)
                itouch = true;
            else if (strncasecmp2(argv[i], "INSIDE", 6) == 0)
                iinside = true;
            else
                error();
            iclip = true;
        } /*** End CLIP & ERASE ***/
        else if (strncasecmp2(argv[i], "FACTOR", 0) == 0)
        {
            i++;
            if (i >= argc)
                error();
            infactor = findunit(argv[i]);
            if (infactor == 0)
                error();
            iunit = true;
            i++;
            if (i >= argc)
                error();
            outfactor = findunit(argv[i]);
            if (outfactor == 0)
            {
                sscanf(argv[i], "%lf", &factor);
                if (factor == 0)
                    error();
            }
            if (factor == 0)
            {
                if (infactor == 0)
                {
                    puts(
                        "ERROR: Input unit must be defined before output unit");
                    exit(1);
                }
                factor = infactor / outfactor;
            }
            printf("Output file coordinate values will be factored by %lg\n",
                   factor);
        } /*** End FACTOR ***/
        else if (strncasecmp2(argv[i], "SHIFT", 5) == 0)
        {
            i++;
            if (i >= argc)
                error();
            sscanf(argv[i], "%lf", &xshift);
            i++;
            if (i >= argc)
                error();
            sscanf(argv[i], "%lf", &yshift);
            iunit = true;
            printf("X Shift: %lg   Y Shift: %lg\n", xshift, yshift);
        } /*** End SHIFT ***/
        else
        {
            printf("ERROR: Unknown function %s\n", argv[i]);
            error();
        }
    }

    // If there is no data in this file let the user know.
    openfiles(); /* Open the infile and the outfile for shape and dbf. */
    if (DBFGetFieldCount(hDBF) == 0)
    {
        puts("There are no fields in this table!");
        exit(1);
    }

    // Print out the file bounds.
    {
        const int iRecord = DBFGetRecordCount(hDBF);
        SHPGetInfo(hSHP, NULL, NULL, adfBoundsMin, adfBoundsMax);

        printf(
            "Input Bounds:  (%lg,%lg) - (%lg,%lg)   Entities: %d   DBF: %d\n",
            adfBoundsMin[0], adfBoundsMin[1], adfBoundsMax[0], adfBoundsMax[1],
            nEntities, iRecord);

        if (strcmp(outfile, "") == 0)
        { /* Describe the shapefile; No other functions */
            ti = DBFGetFieldCount(hDBF);
            showitems();
            exit(0);
        }
    }

    if (iclip)
        check_theme_bnd();

    {
        const int jRecord = DBFGetRecordCount(hDBFappend);
        SHPGetInfo(hSHPappend, NULL, NULL, adfBoundsMin, adfBoundsMax);
        if (nEntitiesAppend == 0)
            puts("New Output File\n");
        else
            printf(
                "Append Bounds: (%lg,%lg)-(%lg,%lg)   Entities: %d  DBF: %d\n",
                adfBoundsMin[0], adfBoundsMin[1], adfBoundsMax[0],
                adfBoundsMax[1], nEntitiesAppend, jRecord);
    }
    /* -------------------------------------------------------------------- */
    /*	Find matching fields in the append file or add new items.       */
    /* -------------------------------------------------------------------- */
    mergefields();
    /* -------------------------------------------------------------------- */
    /*	Find selection field if needed.                                 */
    /* -------------------------------------------------------------------- */
    if (iselect)
        findselect();

    /* -------------------------------------------------------------------- */
    /*  Read all the records 						*/
    /* -------------------------------------------------------------------- */
    int jRecord = DBFGetRecordCount(hDBFappend);
    for (int iRecord = 0; iRecord < nEntities;
         iRecord++) /** DBFGetRecordCount(hDBF) **/
    {
        /* -------------------------------------------------------------------- */
        /*      SELECT for values if needed. (Can the record be skipped.)       */
        /* -------------------------------------------------------------------- */
        if (iselect)
            if (selectrec(iRecord) == 0)
                goto SKIP_RECORD; /** SKIP RECORD **/

        /* -------------------------------------------------------------------- */
        /*      Read a Shape record                                             */
        /* -------------------------------------------------------------------- */
        psCShape = SHPReadObject(hSHP, iRecord);

        /* -------------------------------------------------------------------- */
        /*      Clip coordinates of shapes if needed.                           */
        /* -------------------------------------------------------------------- */
        if (iclip)
            if (clip_boundary() == 0)
                goto SKIP_RECORD; /** SKIP RECORD **/

        /* -------------------------------------------------------------------- */
        /*      Read a DBF record and copy each field.                          */
        /* -------------------------------------------------------------------- */
        for (int i = 0; i < DBFGetFieldCount(hDBF); i++)
        {
            /* -------------------------------------------------------------------- */
            /*      Store the record according to the type and formatting           */
            /*      information implicit in the DBF field description.              */
            /* -------------------------------------------------------------------- */
            if (pt[i] > -1) /* if the current field exists in output file */
            {
                switch (DBFGetFieldInfo(hDBF, i, NULL, &iWidth, &iDecimals))
                {
                    case FTString:
                    case FTLogical:
                    case FTDate:
                        DBFWriteStringAttribute(
                            hDBFappend, jRecord, pt[i],
                            (DBFReadStringAttribute(hDBF, iRecord, i)));
                        break;

                    case FTInteger:
                        DBFWriteIntegerAttribute(
                            hDBFappend, jRecord, pt[i],
                            (DBFReadIntegerAttribute(hDBF, iRecord, i)));
                        break;

                    case FTDouble:
                        DBFWriteDoubleAttribute(
                            hDBFappend, jRecord, pt[i],
                            (DBFReadDoubleAttribute(hDBF, iRecord, i)));
                        break;

                    case FTInvalid:
                        break;
                }
            }
        }
        jRecord++;
        /* -------------------------------------------------------------------- */
        /*      Change FACTOR and SHIFT coordinates of shapes if needed.        */
        /* -------------------------------------------------------------------- */
        if (iunit)
        {
            for (int j = 0; j < psCShape->nVertices; j++)
            {
                psCShape->padfX[j] = psCShape->padfX[j] * factor + xshift;
                psCShape->padfY[j] = psCShape->padfY[j] * factor + yshift;
            }
        }

        /* -------------------------------------------------------------------- */
        /*      Write the Shape record after recomputing current extents.       */
        /* -------------------------------------------------------------------- */
        SHPComputeExtents(psCShape);
        SHPWriteObject(hSHPappend, -1, psCShape);

    SKIP_RECORD:
        SHPDestroyObject(psCShape);
        psCShape = NULL;
        // j=0;
    }

    /* -------------------------------------------------------------------- */
    /*      Print out the # of Entities and the file bounds.                */
    /* -------------------------------------------------------------------- */
    jRecord = DBFGetRecordCount(hDBFappend);
    SHPGetInfo(hSHPappend, &nEntitiesAppend, &nShapeTypeAppend, adfBoundsMin,
               adfBoundsMax);

    printf("Output Bounds: (%lg,%lg) - (%lg,%lg)   Entities: %d  DBF: %d\n\n",
           adfBoundsMin[0], adfBoundsMin[1], adfBoundsMax[0], adfBoundsMax[1],
           nEntitiesAppend, jRecord);

    SHPClose(hSHP);
    SHPClose(hSHPappend);
    DBFClose(hDBF);
    DBFClose(hDBFappend);

    if (nEntitiesAppend == 0)
    {
        puts("Remove the output files.");
        setext(outfile, "dbf");
        remove(outfile);
        setext(outfile, "shp");
        remove(outfile);
        setext(outfile, "shx");
        remove(outfile);
    }

    return 0;
}
