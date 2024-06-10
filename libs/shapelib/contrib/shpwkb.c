/******************************************************************************
 * Copyright (c) 1999, Carl Anderson
 *
 * this code is based in part on the earlier work of Frank Warmerdam
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
 * shpwkb.c  - test WKB binary Input / Output
 */

#include <stdio.h>
#include <stdlib.h>
#include "shapefil.h"
#include "shpgeo.h"

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("shpwkb shp_file wkb_file\n");
        return EXIT_FAILURE;
    }

    DBFHandle old_DBF = DBFOpen(argv[1], "rb");
    if (old_DBF == NULL)
    {
        printf("Unable to open old dbf: %s\n", argv[1]);
        return EXIT_FAILURE;
    }
    SHPHandle old_SHP = SHPOpen(argv[1], "rb");
    if (old_SHP == NULL)
    {
        printf("Unable to open old shp: %s\n", argv[1]);
        DBFClose(old_DBF);
        return EXIT_FAILURE;
    }

    FILE *wkb_file = fopen(argv[2], "wb");
    if (wkb_file == NULL)
    {
        printf("Unable to open wkb_file: %s\n", argv[2]);
        DBFClose(old_DBF);
        SHPClose(old_SHP);
        return EXIT_FAILURE;
    }

    WKBStreamObj *wkbObj = calloc(3, sizeof(int));

    int nEntities;
    int nShapeType;
    SHPGetInfo(old_SHP, &nEntities, &nShapeType, NULL, NULL);

    int byRing = 0;

    for (int i = 0; i < nEntities; i++)
    {
        SHPObject *psCShape = SHPReadObject(old_SHP, i);

        if (byRing == 1)
        {
            // const int prevStart = psCShape->nVertices;
            for (int ring = (psCShape->nParts - 1); ring >= 0; ring--)
            {
                const int rStart = psCShape->panPartStart[ring];
                int numVtx;
                if (ring == psCShape->nParts - 1)
                {
                    numVtx = psCShape->nVertices - rStart;
                }
                else
                {
                    numVtx = psCShape->panPartStart[ring + 1] - rStart;
                }

                printf("(shpdata) Ring(%d) (%d for %d)\n", ring, rStart,
                       numVtx);
                SHPObject *psO = SHPClone(psCShape, ring, ring + 1);

                SHPDestroyObject(psO);
                printf("(shpdata) End Ring\n");
            }  // (ring) [0,nParts
        }      // by ring

        printf("gonna build a wkb\n");
        // const int res =
        SHPWriteOGisWKB(wkbObj, psCShape);
        printf("gonna write a wkb that is %d bytes long\n", wkbObj->StreamPos);
        fwrite((void *)wkbObj->wStream, 1, wkbObj->StreamPos, wkb_file);

        SHPDestroyObject(psCShape);
    }

    free(wkbObj);
    SHPClose(old_SHP);
    DBFClose(old_DBF);
    fclose(wkb_file);

    printf("\n");

    return EXIT_SUCCESS;
}
