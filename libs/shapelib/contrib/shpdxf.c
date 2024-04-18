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
 * derived from a ESRI Avenue Script
 * and DXF specification from AutoCad 3 (yes 1984)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shapefil.h"

const char FLOAT_PREC[] = "%16.5f\r\n";

static void dxf_hdr(double x1, double y1, double x2, double y2, FILE *df)
{
    // Create HEADER section
    fprintf(df, "  0\r\n");
    fprintf(df, "SECTION\r\n");
    fprintf(df, "  2\r\n");
    fprintf(df, "HEADER\r\n");
    fprintf(df, "  9\r\n");
    fprintf(df, "$EXTMAX\r\n");
    fprintf(df, " 10\r\n");
    fprintf(df, FLOAT_PREC, x2);
    fprintf(df, " 20\r\n");
    fprintf(df, FLOAT_PREC, y2);
    fprintf(df, "  9\r\n");
    fprintf(df, "$EXTMIN\r\n");
    fprintf(df, " 10\r\n");
    fprintf(df, FLOAT_PREC, x1);
    fprintf(df, " 20\r\n");
    fprintf(df, FLOAT_PREC, y1);
    fprintf(df, "  9\r\n");
    fprintf(df, "$LUPREC\r\n");
    fprintf(df, " 70\r\n");
    fprintf(df, "    14\r\n");
    fprintf(df, "  0\r\n");
    fprintf(df, "ENDSEC\r\n");

    // Create TABLES section

    fprintf(df, "  0\r\n");
    fprintf(df, "SECTION\r\n");
    fprintf(df, "  2\r\n");
    fprintf(df, "TABLES\r\n");
    // Table 1 - set up line type
    fprintf(df, "  0\r\n");
    fprintf(df, "TABLE\r\n");
    fprintf(df, "  2\r\n");
    fprintf(df, "LTYPE\r\n");
    fprintf(df, " 70\r\n");
    fprintf(df, "2\r\n");
    // Entry 1 of Table 1
    fprintf(df, "  0\r\n");
    fprintf(df, "LTYPE\r\n");
    fprintf(df, "  2\r\n");
    fprintf(df, "CONTINUOUS\r\n");
    fprintf(df, " 70\r\n");
    fprintf(df, "64\r\n");
    fprintf(df, "  3\r\n");
    fprintf(df, "Solid line\r\n");
    fprintf(df, " 72\r\n");
    fprintf(df, "65\r\n");
    fprintf(df, " 73\r\n");
    fprintf(df, "0\r\n");
    fprintf(df, " 40\r\n");
    fprintf(df, "0.0\r\n");
    fprintf(df, "  0\r\n");
    fprintf(df, "ENDTAB\r\n");
    // End of TABLES section
    fprintf(df, "  0\r\n");
    fprintf(df, "ENDSEC\r\n");

    // Create BLOCKS section
    fprintf(df, "  0\r\n");
    fprintf(df, "SECTION\r\n");
    fprintf(df, "  2\r\n");
    fprintf(df, "BLOCKS\r\n");
    fprintf(df, "  0\r\n");
    fprintf(df, "ENDSEC\r\n");
    fprintf(df, "  0\r\n");
    fprintf(df, "SECTION\r\n");
    fprintf(df, "  2\r\n");
    fprintf(df, "ENTITIES\r\n");
}

static void dxf_ent_preamble(int dxf_type, char *id, FILE *df)
{
    fprintf(df, "  0\r\n");

    switch (dxf_type)
    {
        case SHPT_POLYGON:
        case SHPT_ARC:
            fprintf(df, "POLYLINE\r\n");
            break;
        default:
            fprintf(df, "POINT\r\n");
    }

    fprintf(df, "  8\r\n");
    fprintf(df, "%s\r\n", id);
    switch (dxf_type)
    {
        case SHPT_ARC:
            fprintf(df, "  6\r\n");
            fprintf(df, "CONTINUOUS\r\n");
            fprintf(df, " 66\r\n");
            fprintf(df, "1\r\n");
            break;
        case SHPT_POLYGON:
            fprintf(df, "  6\r\n");
            fprintf(df, "CONTINUOUS\r\n");
            fprintf(df, " 66\r\n");
            fprintf(df, "1\r\n");
            fprintf(df, " 70\r\n");
            fprintf(df, "1\r\n");
        default:
            break;
    }
}

static void dxf_ent(char *id, double x, double y, double z, int dxf_type,
                    FILE *df)
{
    if ((dxf_type == SHPT_ARC) || (dxf_type == SHPT_POLYGON))
    {
        fprintf(df, "  0\r\n");
        fprintf(df, "VERTEX\r\n");
        fprintf(df, "  8\r\n");
        fprintf(df, "%s\r\n", id);
    }
    fprintf(df, " 10\r\n");
    fprintf(df, FLOAT_PREC, x);
    fprintf(df, " 20\r\n");
    fprintf(df, FLOAT_PREC, y);
    fprintf(df, " 30\r\n");
    if (z != 0)
        fprintf(df, FLOAT_PREC, z);
    else
        fprintf(df, "0.0\r\n");
}

static void dxf_ent_postamble(int dxf_type, FILE *df)
{
    if ((dxf_type == SHPT_ARC) || (dxf_type == SHPT_POLYGON))
        fprintf(df, "  0\r\nSEQEND\r\n  8\r\n0\r\n");
}

#define MAX_FILESIZE 80

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("usage: shpdxf shapefile {idfield}\n");
        return -1;
    }

    char *shpFileName = argv[1];
    const size_t len = strlen(shpFileName);
    if (len < 5 || len > MAX_FILESIZE - 1)
    {
        // e.g. "a.shp"
        printf("shapefile name must be between 5 and %d characters\n",
               MAX_FILESIZE - 1);
        return EXIT_FAILURE;
    }

    char dbfFileName[MAX_FILESIZE] = "";
    strncpy(dbfFileName, shpFileName, len - 3);
    strcat(dbfFileName, "dbf");

    char dxfFileName[MAX_FILESIZE] = "";
    strncpy(dxfFileName, shpFileName, len - 3);
    strcat(dxfFileName, "dxf");

    DBFHandle dbf = DBFOpen(dbfFileName, "rb");
    if (dbf == NULL)
    {
        printf("Unable to open dbf: %s\n", dbfFileName);
        return EXIT_FAILURE;
    }

    SHPHandle shp = SHPOpen(shpFileName, "rb");
    if (shp == NULL)
    {
        printf("Unable to open shp: %s\n", shpFileName);
        DBFClose(dbf);
        return EXIT_FAILURE;
    }

    FILE *dxf = fopen(dxfFileName, "w");
    if (dxf == NULL)
    {
        printf("Unable to open dxf: %s\n", dxfFileName);
        DBFClose(dbf);
        SHPClose(shp);
        return EXIT_FAILURE;
    }

    printf("Starting conversion %s(%s) -> %s\r\n", shpFileName, dbfFileName,
           dxfFileName);

    int shp_numrec;
    int shp_type;
    double adfBoundsMin[4];
    double adfBoundsMax[4];
    SHPGetInfo(shp, &shp_numrec, &shp_type, adfBoundsMin, adfBoundsMax);
    printf("file has %d objects\r\n", shp_numrec);

    dxf_hdr(adfBoundsMin[0], adfBoundsMin[1], adfBoundsMax[0], adfBoundsMax[1],
            dxf);

    // Before proceeding, allow the user to specify the ID field to use or
    // default to the record number.

    unsigned int MaxElem = -1;
    if (argc > 3)
        MaxElem = atoi(argv[3]);

    const int nflds = DBFGetFieldCount(dbf);
    int idfld = -1;
    DBFFieldType idfld_type = FTInvalid;
    char fldName[15];

    if (argc > 2)
    {
        char idfldName[15];
        strcpy(idfldName, argv[2]);
        for (idfld = 0; idfld < nflds; idfld++)
        {
            idfld_type = DBFGetFieldInfo(dbf, idfld, fldName, NULL, NULL);
            if (!strcmp(idfldName, fldName))
                break;
        }
        if (idfld >= nflds)
        {
            printf("Id field %s not found, using default\r\n", idfldName);
            idfld = -1;
        }
        else
            printf("proceeding with field %s for LayerNames\r\n", fldName);
    }

    int zfld = 0;
    for (; zfld < nflds; zfld++)
    {
        const char zfldName[6] = "ELEV";
        DBFGetFieldInfo(dbf, zfld, fldName, NULL, NULL);
        if (!strcmp(zfldName, fldName))
            break;
    }
    if (zfld >= nflds)
        zfld = -1;

#ifdef DEBUG
    printf("proceeding with id = %d, elevation = %d\r\n", idfld, zfld);
#endif

    char id[255];

    // Proceed to process data.
    for (int recNum = 0; (recNum < shp_numrec) && (recNum < (int)MaxElem);
         recNum++)
    {
        if (idfld >= 0)
            switch (idfld_type)
            {
                case FTString:
                    sprintf(id, "lvl_%s",
                            DBFReadStringAttribute(dbf, recNum, idfld));
                    break;
                default:
                    sprintf(id, "%-20.0lf",
                            DBFReadDoubleAttribute(dbf, recNum, idfld));
            }
        else
            sprintf(id, "lvl_%-20d", (recNum + 1));

        double elev = 0.0;
        if (zfld < 0)
        {
            elev = DBFReadDoubleAttribute(dbf, recNum, zfld);
        }

#ifdef DEBUG
        printf("\r\nworking on obj %d", recNum);
#endif

        SHPObject *shape = SHPReadObject(shp, recNum);

        const int nVertices = shape->nVertices;
        const int nParts = shape->nParts;
        int *panParts = shape->panPartStart;
        int part = 0;
        for (int vrtx = 0; vrtx < nVertices; vrtx++)
        {
#ifdef DEBUG
            printf("\rworking on part %d, vertex %d", part, vrtx);
#endif
            if (panParts[part] == vrtx)
            {
#ifdef DEBUG
                printf("object preamble\r\n");
#endif
                dxf_ent_preamble(shp_type, id, dxf);
            }

            dxf_ent(id, shape->padfX[vrtx], shape->padfY[vrtx], elev, shp_type,
                    dxf);

            if (panParts[part] == (vrtx + 1) || vrtx == (nVertices - 1))
            {
                dxf_ent_postamble(shp_type, dxf);
                part++;
            }
        }
        SHPDestroyObject(shape);
    }

    // close out DXF file
    fprintf(dxf, "0\r\n");
    fprintf(dxf, "ENDSEC\r\n");
    fprintf(dxf, "0\r\n");
    fprintf(dxf, "EOF\r\n");

    DBFClose(dbf);
    SHPClose(shp);
    fclose(dxf);

    return 0;
}
