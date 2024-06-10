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
 * shpdata.c  - utility program for testing elements of the libraries
 */

#include <stdio.h>
#include <stdlib.h>
#include "shapefil.h"
#include "shpgeo.h"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("shpdata shp_file \n");
        return 1;
    }

    DBFHandle old_DBF = DBFOpen(argv[1], "rb");
    if (old_DBF == NULL)
    {
        printf("Unable to open old DBF file:%s\n", argv[1]);
        return 1;
    }

    SHPHandle old_SHP = SHPOpen(argv[1], "rb");
    if (old_SHP == NULL)
    {
        printf("Unable to open old shape files:%s\n", argv[1]);
        DBFClose(old_DBF);
        return 1;
    }

    int nEntities;
    int nShapeType;
    SHPGetInfo(old_SHP, &nEntities, &nShapeType, NULL, NULL);

    char *DBFRow = NULL;
    int byRing = 1;
    PT ringCentrd;

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
                if (ring == (psCShape->nParts - 1))
                {
                    numVtx = psCShape->nVertices - rStart;
                }
                else
                {
                    numVtx = psCShape->panPartStart[ring + 1] - rStart;
                }

                printf("(shpdata) Ring(%d) (%d for %d) \n", ring, rStart,
                       numVtx);
                SHPObject *psO = SHPClone(psCShape, ring, ring + 1);

                const int ringDir = SHPRingDir_2d(psO, 0);
                double ringArea = RingArea_2d(
                    psO->nVertices, (double *)psO->padfX, (double *)psO->padfY);
                RingCentroid_2d(psO->nVertices, (double *)psO->padfX,
                                (double *)psO->padfY, &ringCentrd, &ringArea);

                printf("(shpdata)  Ring %d, %f Area %d dir \n", ring, ringArea,
                       ringDir);

                SHPDestroyObject(psO);
                printf("(shpdata) End Ring \n");
            } /* (ring) [0,nParts  */
        }     /* by ring   */

        const double oArea = SHPArea_2d(psCShape);
        const double oLen = SHPLength_2d(psCShape);
        const PT oCentrd = SHPCentrd_2d(psCShape);
        printf("(shpdata) Part (%d) %f Area  %f length, C (%f,%f)\n", i, oArea,
               oLen, oCentrd.x, oCentrd.y);
    }

    DBFClose(old_DBF);
    SHPClose(old_SHP);

    printf("\n");

    return EXIT_SUCCESS;
}
